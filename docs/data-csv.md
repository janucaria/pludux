# CSV Data

Pludux loads an Asset from CSV market data.

## Verified behavior

- The first column is read as the date/time field.
- Remaining columns are loaded as numeric series.
- The date column may be ISO-like text or a numeric timestamp.
- If the loaded data is in reverse chronological order, the series is reversed
  before being stored.
- Pludux does not repair arbitrary out-of-order timestamps.
- Portfolio timelines require finite, strictly increasing timestamps.
- The Asset editor lets you map the OHLCV field names used by the importer.

## Asset editor mapping

The Asset editor shows explicit fields for:

- Name
- Open price
- High price
- Low price
- Close price
- Volume

The editor text says to enter the exact column headers and notes that header
matching is case-sensitive.

## Practical guidance

If your CSV uses different column names, adjust the field mapping in the GUI to
match the file exactly. Where the file format is not verified here, follow the
current importer/editor prompts rather than guessing labels.

## Related docs

- [Getting Started](getting-started.md)
- [GUI Guide](gui-guide.md)
- [Core Concepts](concepts.md)
