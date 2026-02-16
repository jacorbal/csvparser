# csvparser -- Extended CSV basic parser

Basic CSV parser implemented in C.

## Basics

This implementation parses CSV documents and is compatible with RFC
4180's core quoting and escaping rules (fields may be quoted with double
quotes and embedded double quotes are represented as two consecutive
double quotes), but it also accepts several non‑RFC extensions: it
tolerates LF as well as CRLF line endings, skips blank lines and lines
beginning with '`#`' as comments, is lenient with unterminated quoted
fields at EOF, and does not enforce a constant number of fields per
record.

Because this implementation uses the common quoting/escape semantics,
**it's broadly compatible with many CSV files that follow RFC 4180, but
it is not strictly RFC-compliant because it accepts and tolerates
deviations** (comments, blank lines, inconsistent field counts, lenient
quote handling and newline variations) that an RFC-strict parser would
reject.

The RFC 4180 CSV specification defines a minimal, strict grammar:
records separated by CRLF, fields separated by a single delimiter,
fields that contain the delimiter/quotes/newlines must be enclosed in
double quotes, embedded double quotes are represented by two consecutive
double quotes, every record should have the same number of fields, and
no extra characters are allowed after a closing quote except the field
delimiter or the record terminator.

## License

  - Copyright (c) 2025, J. A. Corbal (<jacorbal@gmail.com>)
  - Licensed under the ISC License
  - Read the [`LICENSE`](LICENSE) file for more information
