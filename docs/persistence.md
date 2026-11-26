# Workspace Persistence

Pludux can import Asset history and trade-call tables from CSV. Strategy JSON
exchanges one Strategy. A `.pludux` file saves the complete GUI workspace.

## Workspace content

A workspace currently has this conceptual root:

```json
{
  "pludux": {
    "$version": "<application version>",
    "store": {},
    "documentState": {},
    "viewState": {}
  }
}
```

`store` contains arenas for Assets, Calls, Watchlists, Strategies, Profiles,
Systems, Markets, Brokers, and Portfolios. References use slot and generation
metadata so a deleted slot cannot silently become another object. Systems store
their Profile reference, Main and Failsafe Strategy setups, performance
settings, and optional Calls table with its column mappings and Entry Priority.
Each setup stores its Strategy reference, input overrides, Entry Filter, and
logical `OTHER.CALL` field mappings. The workspace embeds Asset history and
the Calls library, including each table's name, headers, and rows.

Desktop and Web use the same workspace format. Resource slot and value indices,
stored counts, and Strategy indices are unsigned 32-bit integers. Valid slot
indices range from 0 to 4,294,967,294. Handle and arena slot generations are
unsigned 64-bit integers. An unset optional resource reference is saved as JSON
`null`. This includes unset selections in `viewState`.

`documentState` stores document-related editing/selection state and `viewState`
stores presentation state. This is application persistence, not a stable public
JSON API. Prefer editing through the GUI.

The workspace reader is intentionally tolerant of omitted resource fields.
When a canonical field is absent, Pludux uses the value from a freshly
constructed C++ resource; this includes names, references, collections,
settings, limits, fees, and sizing configuration. `viewState` and each of its
fields are also optional and use fresh-state defaults. Unknown properties are
ignored, but former property names are not aliases and therefore do not supply
the missing canonical value.

Portfolio Strategy selections retain their last child while Portfolio scope is
active. Saving and reopening preserves that remembered child. Older workspaces
that omit a selection's `active` field treat that child as active by default.

The workspace requires `pludux`, `$version`, `store`, `documentState`, the store
descriptor and arena bookkeeping, and the resource collections and document
handle lists. Only optional resource references accept `null`. A file with the
wrong JSON type, an invalid enum or method name, a fractional value in an
integer field, a negative value in an unsigned field, or a number outside its
allowed range is rejected. Workspace files use strict JSON and reject comments
and trailing commas.

Older files that encoded an unset handle as a platform-dependent integer may
fail to open in current builds. If a readable workspace predates Calls, omitted
Calls collections and settings default to an empty Calls library and Signal
entry sources.

Portfolio result timelines are not durable stored resources. Any document edit
invalidates results and recreates runners. A workspace version mismatch also
resets Portfolio results where setup can otherwise be read. Do not assume
general forward migration for hand-edited internals.

Saving always writes the complete canonical current format, regardless of
which optional fields were omitted from the loaded file.

## Open and save

The toolbar provides **Open**, **Save**, **Undo**, **Redo**, and **About**.
Opening another workspace prompts before discarding an active draft. A new
workspace receives a timestamped `.pludux` name on first save. Native builds use
the platform file dialog; browser builds use upload/download.

Keep source CSV and exported Strategy JSON under version control for reproducible
inputs. Preserve an unreadable workspace unchanged and retry with the Pludux
version that created it before attempting manual recovery.

See [Strategy JSON](strategy-json.md) and [CSV Data](data-csv.md) for exchange formats.
