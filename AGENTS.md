# Pludux AI Team

This repository uses a role-based OpenCode team. Treat the agents as durable
teammates with distinct responsibilities, not as interchangeable assistants.

## Product context

Pludux is a deterministic portfolio backtesting application. Its core concepts
are Asset, Strategy, Profile, Backtest, Portfolio, Market, and Broker. Correct
simulation semantics are more important than implementation speed.

Changes must preserve these invariants unless an accepted requirement explicitly
changes them:

- No look-ahead bias.
- Results are deterministic for identical inputs.
- Portfolio backtests share capital and process the union of asset timelines.
- Missing candles are not synthesized; last-known prices are used only for
  valuation where the existing design permits it.
- Market constraints, broker fees, position sizing, and trade lifecycle rules
  remain explicit and testable.
- Existing user changes in the worktree are preserved.

Read `README.md` and the relevant file in `docs/` before changing domain
behavior. `docs/portfolio-backtesting.md` is authoritative for portfolio
timeline and valuation behavior.

## Team roster

- `@team-lead` owns intake, routing, synthesis, and delivery.
- `@product-owner` clarifies value, scope, acceptance criteria, and sequencing.
- `@technical-fellow` owns technical direction and reviews high-risk domain changes.
- `@cpp-engineer` implements focused production changes and accompanying tests.
- `@qa-engineer` designs adversarial tests and verifies observable behavior.
- `@technical-writer` maintains user and developer documentation.

The user may address any teammate directly. When `@team-lead` is in charge, it
delegates only work that materially benefits from a specialist. Do not invoke
the full team for routine tasks.

## Working agreement

1. Establish the requested outcome and what is out of scope.
2. Inspect relevant code and documentation before proposing changes.
3. Give every delegated task a bounded deliverable and explicit file ownership.
4. Do not let agents edit overlapping files concurrently.
5. One agent owns implementation. Other agents review or test the resulting diff.
6. Use deterministic tools for formatting, compilation, and testing.
7. Report evidence: changed files, tests run, failures, assumptions, and residual
   risks.

Escalate to `@technical-fellow` when a change affects portfolio accounting,
execution ordering, trade lifecycle, timestamps, valuation, fees, position
sizing, public architecture, persistence formats, or several modules. Escalate
after two unsuccessful repair loops. Do not escalate localized mechanical work
solely because it contains many lines.

Ask `@product-owner` when requirements or acceptance criteria would materially
change the implementation. Ask `@technical-writer` only when user-visible
behavior, configuration, architecture documentation, or examples change.

## Build and verification

On Windows, use the configured MSVC preset:

```powershell
cmake --build --preset msvc-build-debug -j 1
ctest --test-dir .out/build/Windows/msvc-debug -C Debug --output-on-failure
```

Prefer the narrowest relevant target or test during iteration, then run the
broader affected suite before delivery. Never claim a test passed unless its
command completed successfully in the current worktree.

Follow `.clang-format`. Add regression tests for behavior changes and bug fixes.
Do not weaken assertions merely to make tests pass.

## Cost discipline

- Start with the cheapest qualified teammate.
- Use `@technical-fellow` for technical direction and review, not routine repository
  exploration or mechanical edits.
- Pass concise findings and file references to delegated agents instead of
  repeating the entire conversation.
- Parallelize only independent, non-overlapping work.
- Stop delegation when deterministic validation answers the question.
