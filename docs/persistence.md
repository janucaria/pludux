# Workspace Persistence

Pludux uses three file forms: CSV imports one Asset's history; Model JSON
exchanges one Model; `.pludux` saves the complete GUI workspace.

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

`store` contains arenas for Assets, Watchlists, Models, Profiles, Strategies,
Systems, Markets, Brokers, and Portfolios. References use slot and generation
metadata so a deleted slot cannot silently become another object. Imported CSV
history is embedded; reopening does not require the original CSV.

`documentState` stores document-related editing/selection state and `viewState`
stores presentation state. This is application persistence, not a stable public
JSON API. Prefer editing through the GUI.

Portfolio result timelines are not durable stored resources. Any document edit
invalidates results and recreates runners. A workspace version mismatch also
resets Portfolio results where setup can otherwise be read. Do not assume
general forward migration for hand-edited internals.

## Open and save

The toolbar provides **Open**, **Save**, **Undo**, **Redo**, and **About**.
Opening another workspace prompts before discarding an active draft. A new
workspace receives a timestamped `.pludux` name on first save. Native builds use
the platform file dialog; browser builds use upload/download.

Keep source CSV and exported Model JSON under version control for reproducible
inputs. Preserve an unreadable workspace unchanged and retry with the Pludux
version that created it before attempting manual recovery.

See [Model JSON](model-json.md) and [CSV Data](data-csv.md) for exchange formats.
