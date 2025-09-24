/**
 * @file csvparser.h
 *
 * @brief Extended CSV basic parser operations declaration
 *
 * This implementation parses CSV documents and is compatible with RFC
 * 4180's core quoting and escaping rules (fields may be quoted with
 * double quotes and embedded double quotes are represented as two
 * consecutive double quotes), but it also accepts several non‑RFC
 * extensions: it tolerates LF as well as CRLF line endings, skips blank
 * lines and lines beginning with '#' as comments, is lenient with
 * unterminated quoted fields at EOF, and does not enforce a constant
 * number of fields per record.
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 * @copyright Copyright (c) 2025, J. A. Corbal.
 *            Licensed under the ISC License.  See `LICENSE` for details.
 *
 * @note The RFC 4180 CSV specification defines a minimal, strict
 *       grammar: records separated by CRLF, fields separated by
 *       a single delimiter, fields that contain the
 *       delimiter/quotes/newlines must be enclosed in double quotes,
 *       embedded double quotes are represented by two consecutive
 *       double quotes, every record should have the same number of
 *       fields, and no extra characters are allowed after a closing
 *       quote except the field delimiter or the record terminator.
 * @note Because it implements the common quoting/escape semantics, this
 *       implementation is broadly compatible with many CSV files that
 *       follow RFC 4180, but it is not strictly RFC-compliant because
 *       it accepts and tolerates deviations (comments, blank lines,
 *       inconsistent field counts, lenient quote handling and newline
 *       variations) that an RFC-strict parser would reject.
 * @note If strict RFC compliance is required, the implementation would
 *       need modifications to enforce CRLF-only record terminators,
 *       reject malformed or unterminated quoted fields, disallow extra
 *       characters after closing quotes, and validate consistent field
 *       counts across records.
 */
/*
 * ISC License
 *
 * Copyright (c) 2025, J. A. Corbal <jacorbal@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

/* System includes */
#include <stdbool.h>    /* bool, true */
#include <stdio.h>      /* FILE */


#define CSV_HAS_HEADER (true)
#define CSV_NO_HEADER (false)

#define CSV_DELIM_COMMA ","
#define CSV_DELIM_SEMICOLON ";"
#define CSV_DELIM_TAB "\t"
#define CSV_DELIM_PIPE "|"


/**
 * @typedef csv_row_td
 *
 * @brief Structure for any CSV row, including the header if any
 */
typedef struct {
    char **fields;      /**< Contents as strings in this CSV row */
    size_t num_fields;  /**< Number of fileds in this CSV row */
} csv_row_td;


/**
 * @typedef csv_parser_td
 *
 * @brief Structure for the CSV parser
 */
typedef struct {
    FILE *fp;               /**< File handler */
    char *filename;         /**< Path to the CSV file */
    char delim;             /**< Delimiter between fields */
    bool has_header;        /**< If true, first line is header */
    size_t line_no;         /**< Line number being processed */
    csv_row_td *header;     /**< Header string */
} csv_parser_td;


/* Public interface */
/**
 * @brief Initialize the CSV parser
 *
 * @param filename   Path to the CSV data file
 * @param delim      Delimiter between fields
 * @param has_header If @c true, the first line is the header
 *
 * @return Pointer to the CSV parsed information, or @c NULL otherwise
 *
 * @note If @p delim is @c '\n', @c '\r', @c '"' or @c NULL, it defaults
 *       to @c ','.
 */
csv_parser_td *csv_parser_init(const char *filename, const char *delim,
        bool has_header);

/**
 * @brief Deallocate the memory used by the CSV parser
 *
 * @param csv_parser CSV parser to free
 */
void csv_parser_destroy(csv_parser_td *csv_parser);

/**
 * @brief Deallocate the memory used by a row of the CSV parser
 *
 * @param csv_row Row to deallocate
 */
void csv_parser_destroy_row(csv_row_td *csv_row);

/**
 * @brief Get the header of the CSV file, if any
 *
 * @param csv_parser CSV parser to get its header from
 *
 * @return Header of the CSV file, or @c NULL otherwise
 */
const csv_row_td *csv_parser_header(csv_parser_td *csv_parser);

/**
 * @brief Get the current row that it's being parsed
 *
 * @param csv_parser CSV parser where to get the row from
 *
 * @return Pointer to the current row
 */
csv_row_td *csv_parser_row(csv_parser_td *csv_parser);

/**
 * @brief Macro that evaluates to the CSV fields
 */
#define csv_parser_fields(row) ((const char **) (row)->fields)

/**
 * @brief Macro that evaluates to the number of fields
 */
#define csv_parser_num_fields(row) ((row)->num_fields)


#endif /* ! CSV_PARSER_H */
