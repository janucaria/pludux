# Building and Contributing

Pludux is a CMake C++23 project built around C++ modules. The repository layers
the generic `pludux` library in `libs/`, the backtest domain in
`apps/backtest/lib/`, and the desktop/WebAssembly application and persistence
in `apps/backtest/gui/`.

## Configure and build

On Windows with Visual Studio 2022 and the MSVC toolchain configured, use the
checked-in preset:

```powershell
cmake --preset msvc-debug
cmake --build --preset msvc-build-debug -j 1
```

Other configure presets in `CMakePresets.json` are `msvc-release`,
`clang-debug`, and the Emscripten debug/release presets. Matching build presets
carry the `-build-` segment. Dependencies are obtained through CMake's declared
packages/fetches; review the preset and CMake output rather than installing
unlisted libraries ad hoc.

The desktop executable target is `pludux-backtest-gui`. Native builds use
platform file dialogs. Emscripten builds replace native open/save with browser
upload/download and embed the Model schema and samples needed by the app.

## Tests

During iteration, build the narrowest relevant test target. Before delivery,
run the affected suite, or the full configured suite:

```powershell
ctest --test-dir .out/build/Windows/msvc-debug -C Debug --output-on-failure
```

Tests live under `libs/tests`, `apps/backtest/tests`, and GUI test directories.
Do not weaken assertions to accommodate a behavior change. Add a regression
test for every bug fix or changed simulation rule.

## Architecture boundaries

- Generic time-series containers and reusable primitives belong in `libs`.
- Assets, Models, Strategies, Profiles, Systems, Markets, Brokers, Portfolios,
  runner state, accounting, and execution semantics belong in the backtest
  domain library.
- GUI windows, application state, workspace serialization, file interaction,
  and rendering belong in the GUI application.
- Model JSON parsing/serialization is a versioned interchange boundary. The
  schema is in `data/backtest/schemas` and examples are in
  `data/backtest/samples`.
- `.pludux` serialization is full application persistence, not the Model JSON
  API. See [Workspace Persistence](persistence.md).

## Domain-change checklist

Read `README.md`, `AGENTS.md`, and the relevant documentation before editing.
`docs/portfolio-backtesting.md` is authoritative for union timelines and
valuation. Preserve no-look-ahead evaluation, deterministic ordering, shared
Portfolio capital, real (not synthesized) candles, explicit fees/sizing, and
trade lifecycle rules. Update docs, schema, samples, and tests together when a
public contract changes. Format C++ with the repository `.clang-format`.
