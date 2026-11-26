const assert = require('node:assert/strict');
const { execFileSync } = require('node:child_process');
const { createHash } = require('node:crypto');
const { mkdtemp, readFile, rm, writeFile } = require('node:fs/promises');
const { tmpdir } = require('node:os');
const { join } = require('node:path');
const test = require('node:test');
const { runInNewContext } = require('node:vm');

const scope = 'https://example.test/tools/pludux/';
const files = [
  'index.html', 'index.js', 'index.wasm', 'manifest.webmanifest',
  'icon-192.png', 'icon-512.png', 'icon-maskable-512.png'
];
const cachePrefix = 'pludux:' + encodeURIComponent(scope) + ':';
const integrity = bytes => 'sha256-' + createHash('sha256').update(bytes).digest('base64');
const assetUrl = file => file === 'index.html' ? scope : scope + file;

async function bundle(t, version) {
  const directory = await mkdtemp(join(tmpdir(), 'pludux-offline-'));
  t.after(() => rm(directory, { recursive: true, force: true }));
  const assets = new Map(files.map(file => [assetUrl(file), file + ' ' + version]));
  await Promise.all(files.map(file => writeFile(join(directory, file), assets.get(assetUrl(file)))));
  execFileSync(process.execPath, [join(__dirname, 'generate-service-worker.cjs'), directory]);
  return { script: await readFile(join(directory, 'service-worker.js'), 'utf8'), assets };
}

function browser() {
  const entries = new Map();
  let network = new Map();
  async function fetch(request) {
    const body = network.get(request.url);
    if (body === undefined) {
      throw new Error('Asset unavailable');
    }
    assert.equal(request.cache, 'reload');
    assert.equal(request.mode, 'same-origin');
    assert.equal(request.redirect, 'error');
    if (request.integrity !== integrity(body)) {
      throw new TypeError('Asset integrity check failed');
    }
    return new Response(body);
  }

  const caches = {
    async open(name) {
      if (!entries.has(name)) {
        entries.set(name, new Map());
      }
      const entriesForRelease = entries.get(name);
      return {
        async addAll(requests) {
          const responses = await Promise.all(requests.map(fetch));
          requests.forEach((request, index) => entriesForRelease.set(request.url, responses[index]));
        },
        async put(url, response) {
          entriesForRelease.set(url, response);
        }
      };
    },
    async match(url, { cacheName }) {
      return entries.get(cacheName)?.get(url)?.clone();
    },
    async keys() { return Array.from(entries.keys()); },
    async delete(name) { return entries.delete(name); }
  };

  return {
    entries,
    serve(assets) { network = new Map(assets); },
    load({ script }) {
      const listeners = new Map();
      const self = {
        registration: { scope },
        addEventListener(type, callback) { listeners.set(type, callback); }
      };
      runInNewContext(script, { self, caches, fetch, Request, Response, URL, console });

      function dispatch(type, detail = {}) {
        let result;
        const wait = promise => { result = promise; };
        listeners.get(type)({ ...detail, waitUntil: wait, respondWith: wait });
        return result;
      }

      return {
        install: () => dispatch('install'),
        activate: () => dispatch('activate'),
        fetch: (url, mode = 'same-origin', method = 'GET') =>
          dispatch('fetch', { request: { url, mode, method } }),
        async ready() {
          let ready;
          await dispatch('message', {
            data: 'pludux:offline-status',
            ports: [{ postMessage(value) { ready = value; } }]
          });
          return ready;
        }
      };
    }
  };
}

test('precaches the complete release and serves its scoped routes without a network', async t => {
  const release = await bundle(t, 'first');
  const client = browser();
  const worker = client.load(release);
  client.serve(release.assets);
  await worker.install();
  await worker.activate();
  client.serve([]);

  assert.equal(await worker.ready(), true);
  for (const route of ['', '?source=test', 'index.html']) {
    assert.equal(await (await worker.fetch(scope + route, 'navigate')).text(), 'index.html first');
  }
  for (const file of files.slice(1)) {
    assert.equal(await (await worker.fetch(scope + file)).text(), file + ' first');
  }
  assert.equal(worker.fetch(scope + 'schemas/model.json'), undefined);
  assert.equal(worker.fetch('https://example.test/other/index.js'), undefined);
  assert.equal(worker.fetch(scope + 'index.js', 'same-origin', 'POST'), undefined);
});

test('an interrupted first install leaves no ready cache', async t => {
  const release = await bundle(t, 'first');
  release.assets.delete(scope + 'index.wasm');
  const client = browser();
  const worker = client.load(release);
  client.serve(release.assets);

  await assert.rejects(worker.install(), /Asset unavailable/);
  assert.equal(await worker.ready(), false);
  assert.equal(client.entries.size, 0);
});

test('an incomplete deployment preserves the active release until a complete update activates', async t => {
  const first = await bundle(t, 'first');
  const second = await bundle(t, 'second');
  const client = browser();
  const firstWorker = client.load(first);
  const secondWorker = client.load(second);
  client.serve(first.assets);
  await firstWorker.install();
  await firstWorker.activate();

  const unrelatedCache = 'pludux:' + encodeURIComponent('https://example.test/other/') + ':old';
  client.entries.set(unrelatedCache, new Map());
  client.serve([...second.assets, [scope + 'index.wasm', 'index.wasm first']]);
  await assert.rejects(secondWorker.install(), /Asset integrity check failed/);
  assert.equal(await firstWorker.ready(), true);
  assert.equal(await secondWorker.ready(), false);
  assert.equal(client.entries.size, 2);

  client.serve(second.assets);
  await secondWorker.install();
  assert.equal(await (await firstWorker.fetch(scope + 'index.js')).text(), 'index.js first');
  await secondWorker.activate();
  client.serve([]);
  assert.equal(await secondWorker.ready(), true);
  assert.equal(await (await secondWorker.fetch(scope, 'navigate')).text(), 'index.html second');
  assert.equal(await firstWorker.ready(), false);
  assert.equal(client.entries.size, 2);
  assert.ok(client.entries.has(unrelatedCache));
  assert.ok(Array.from(client.entries.keys()).some(name => name.startsWith(cachePrefix)));
});

test('the staged release is installable and contains the assets named in the worker', {
  skip: !process.env.PLUDUX_WEBROOT
}, async () => {
  const directory = process.env.PLUDUX_WEBROOT;
  const script = await readFile(join(directory, 'service-worker.js'), 'utf8');
  const html = await readFile(join(directory, 'index.html'), 'utf8');
  assert.ok(html.includes("if ('ON' !== 'ON')"));
  assert.match(html, /<link rel="manifest" href="manifest\.webmanifest">/);
  assert.match(html, /<link rel="apple-touch-icon" href="icon-192\.png">/);

  const manifestUrl = new URL('manifest.webmanifest', scope);
  const manifest = JSON.parse(await readFile(join(directory, 'manifest.webmanifest'), 'utf8'));
  assert.equal(manifest.name, 'Pludux');
  assert.equal(manifest.display, 'standalone');
  assert.equal(new URL(manifest.start_url, manifestUrl).href, scope);
  assert.equal(new URL(manifest.scope, manifestUrl).href, scope);

  for (const [filename, size, purpose] of [
    ['icon-192.png', 192, 'any'],
    ['icon-512.png', 512, 'any'],
    ['icon-maskable-512.png', 512, 'maskable']
  ]) {
    const icon = manifest.icons.find(icon => icon.src === filename);
    assert.ok(icon, `Missing ${filename}`);
    assert.equal(icon.sizes, `${size}x${size}`);
    assert.equal(icon.type, 'image/png');
    assert.equal(icon.purpose, purpose);
    assert.equal(new URL(icon.src, manifestUrl).href, scope + filename);
    const bytes = await readFile(join(directory, icon.src));
    assert.equal(bytes.subarray(0, 8).toString('hex'), '89504e470d0a1a0a');
    assert.equal(bytes.readUInt32BE(16), size);
    assert.equal(bytes.readUInt32BE(20), size);
  }

  for (const file of files) {
    const bytes = await readFile(join(directory, file));
    assert.ok(script.includes(JSON.stringify(file) + ':' + JSON.stringify(integrity(bytes))), file);
  }
});
