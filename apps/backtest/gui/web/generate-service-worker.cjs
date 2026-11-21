const { createHash } = require('node:crypto');
const { createReadStream } = require('node:fs');
const { readFile, writeFile } = require('node:fs/promises');
const { join } = require('node:path');

async function generate(outputDirectory) {
  const template = await readFile(join(__dirname, 'service-worker.js.in'), 'utf8');
  const assets = {};
  for (const file of ['index.html', 'index.js', 'index.wasm']) {
    const hash = createHash('sha256');
    for await (const chunk of createReadStream(join(outputDirectory, file))) {
      hash.update(chunk);
    }
    assets[file] = 'sha256-' + hash.digest('base64');
  }

  const manifest = JSON.stringify(assets);
  const version = createHash('sha256').update(template).update(manifest).digest('hex');
  const worker = template.replace('@VERSION@', version).replace('@ASSETS@', manifest);
  await writeFile(join(outputDirectory, 'service-worker.js'), worker);
}

generate(process.argv[2]).catch(error => {
  console.error(error);
  process.exitCode = 1;
});
