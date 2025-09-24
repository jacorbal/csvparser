/**
 * @file csvparser.c
 *
 * @brief Extended CSV basic parser operations implementation
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 * @copyright Copyright (c) 2025, J. A. Corbal.
 *            Licensed under the ISC License.  See `LICENSE` for details.
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

#define _POSIX_C_SOURCE 200809L


/* System includes */
#include <ctype.h>      /* isspace */
#include <stdbool.h>    /* bool, true, false */
#include <stdio.h>      /* FILE */
#include <stdlib.h>     /* malloc, realloc, free, NULL, memcpy(?) */
#include <string.h>     /* strdup, strlen(?) */

/* Local includes */
#include <csvparser.h>


/**
 * @brief Portable @a strdup fallback
 *
 * Allocates a new buffer and copies the null-terminated string into it.
 *
 * @param s Null-terminated source string to duplicate
 *
 * @return Pointer to newly allocated duplicated string, or @c NULL
 */
/*
static char *s_safe_strdup(const char *s)
{
    if (s == NULL) {
        return NULL;
    }

    size_t n = strlen(s);
    char *r = malloc(n + 1);
    if (!r) {
        return NULL;
    }

    memcpy(r, s, n + 1);
    return r;
}
*/


/**
 * @brief Trim leading whitespace in-place
 *
 * Returns a pointer inside the provided null-terminated string @p s
 * that points to the first non-whitespace character.
 *
 * @param s Null-terminated string to trim leading whitespace from
 *
 * @return Pointer to first non-whitespace character within @p s, or to
 *         the terminating @c NULL if the string is all whitespace
 *
 * @note Does not modify the buffer contents except by returning.
 * @note Does not remove characters or allocate memory.
 */
/*
static char *s_ltrim(char *s)
{
    while (*s && isspace((unsigned char) *s)) {
        s++;
    }

    return s;
}
*/


/**
 * @brief Test whether a line should be skipped (blank or comment)
 *
 * A line is skippable if, after removing leading whitespace, it is
 * empty or starts with the '#' character.  This is used to ignore
 * comment lines and blank lines before parsing.
 *
 * @param s Null-terminated line buffer (may contain leading whitespace)
 *
 * @return true if the line should be skipped, false otherwise
 */
static bool s_line_is_skippable(const char *s)
{
    while (*s && isspace((unsigned char) *s)) {
        s++;
    }

    return (*s == '\0' || *s == '#');
}


/**
 * @brief Parse a single null-terminated line into a @e csv_row_td
 *        structure
 *
 * Parses a single line (terminated by @c NULL) into individual fields
 * according to a CSV-like grammar with support for quoted fields and
 * escaped quotes represented by two double-quotes.
 *
 * @param line  Null-terminated input line to parse (no trailing newline)
 * @param delim Field delimiter character
 *
 * @return Pointer to newly allocated @e csv_row_td on success
 *
 * @note The returned @e csv_row_td and its fields are heap-allocated
 *       and must be freed with @a csv_parser_destroy_row().
 * @note Never returns @c NULL under normal allocation semantics
 *       (allocation failures terminate).
 * @note The parser is lenient with respect to line endings (caller
 *       should strip CR/LF before calling).
 */
static csv_row_td *s_parse_line_to_row(const char *line, char delim)
{
    enum { ST_FIELD, ST_QUOTED_FIELD, ST_QUOTE_IN_QUOTED } state = ST_FIELD;
    size_t fields_cnt = 0;
    size_t fields_cap = 8;  /* Initial capacity for the fields array */
    char **fields = malloc(sizeof(char *) * fields_cap);
    if (fields == NULL) {
        return NULL;
    }

    size_t buf_cap = 128;
    size_t buf_len = 0;
    char *buf = malloc(buf_cap);
    if (buf == NULL) {
        return NULL;
    }

    const unsigned char *p = (const unsigned char *) line;
    while (*p) {
        unsigned char c = *p;

        if (state == ST_FIELD) {
            if (c == (unsigned char) delim) {
                buf[buf_len] = '\0';
                fields[fields_cnt++] = strdup((char *) buf);
                buf_len = 0;
                if (fields_cnt == fields_cap) {
                    fields_cap *= 2;
                    fields = realloc(fields, sizeof(char *) * fields_cap);
                    if (fields == NULL) {
                        return NULL;
                    }
                }
            } else if (c == '\"') {
                /* Start quoted field only if at field start */
                if (buf_len == 0) {
                    state = ST_QUOTED_FIELD;
                } else {
                    /* Quote inside unquoted field: treat literally */
                    if (buf_len + 1 >= buf_cap) {
                        buf_cap *= 2;
                        buf = realloc(buf, buf_cap);
                        if (buf == NULL) {
                            return NULL;
                        }
                    }
                    buf[buf_len++] = (char) c;
                }
            } else if (c == '\r' || c == '\n') {
                /* End of line */
                break;
            } else {
                if (buf_len + 1 >= buf_cap) {
                    buf_cap *= 2;
                    buf = realloc(buf, buf_cap);
                    if (buf == NULL) {
                        return NULL;
                    }
                }
                buf[buf_len++] = (char) c;
            }
        } else if (state == ST_QUOTED_FIELD) {
            if (c == '\"') {
                state = ST_QUOTE_IN_QUOTED;
            } else {
                if (buf_len + 1 >= buf_cap) {
                    buf_cap *= 2;
                    buf = realloc(buf, buf_cap);
                    if (buf == NULL) {
                        return NULL;
                    }
                }
                buf[buf_len++] = (char) c;
            }
        } else if (state == ST_QUOTE_IN_QUOTED) {
            if (c == '\"') {
                /* Escaped quote -> append one quote and return to
                 * quoted state */
                if (buf_len + 1 >= buf_cap) {
                    buf_cap *= 2;
                    buf = realloc(buf, buf_cap);
                    if (buf == NULL) {
                        return NULL;
                    }
                }
                buf[buf_len++] = '\"';
                state = ST_QUOTED_FIELD;
            } else if (c == (unsigned char) delim) {
                /* End quoted field */
                buf[buf_len] = '\0';
                fields[fields_cnt++] = strdup((char *) buf);
                buf_len = 0;
                if (fields_cnt == fields_cap) {
                    fields_cap *= 2;
                    fields = realloc(fields, sizeof(char *) * fields_cap);
                    if (fields == NULL) {
                        return NULL;
                    }
                }
                state = ST_FIELD;
            } else if (c == '\r' || c == '\n' || c == '\0') {
                /* End of line after closing quote */
                buf[buf_len] = '\0';
                fields[fields_cnt++] = strdup((char *) buf);
                buf_len = 0;
                state = ST_FIELD;
                break;
            } else {
                /* NOTE.  Per permissive parsing: after quote, if not
                 * delim or quote, treat as end + this char part of next
                 * field.  It'll be treated it as end of quoted field
                 * then reprocess this char in 'ST_FIELD'.
                 * To reprocess, do not advance 'p' here (use 'continue'
                 * with same pointer). */
                /* End quoted field */
                buf[buf_len] = '\0';
                fields[fields_cnt++] = strdup((char *) buf);
                buf_len = 0;
                if (fields_cnt == fields_cap) {
                    fields_cap *= 2;
                    fields = realloc(fields, sizeof(char *) * fields_cap);
                    if (fields == NULL) {
                        return NULL;
                    }
                }
                state = ST_FIELD;
                continue;   /* Reprocess current char in 'ST_FIELD' */
            }
        }

        p++;
    }

    /* At line end: push last field (if any) */
    if (state != ST_QUOTED_FIELD) {
        buf[buf_len] = '\0';
        fields[fields_cnt++] = strdup((char *) buf);
    } else {
        /* Unterminated quoted field: treat remainder as field (lenient) */
        buf[buf_len] = '\0';
        fields[fields_cnt++] = strdup((char *) buf);
    }

    free(buf);

    /* Shrink to fit */
    if (fields_cnt < fields_cap) {
        fields = realloc(fields, sizeof(char *) * fields_cnt);
        if (fields == NULL) {
            return NULL;
        }
    }

    csv_row_td *csv_row = malloc(sizeof *csv_row);
    if (csv_row == NULL) {
        return NULL;
    }
    csv_row->fields = fields;
    csv_row->num_fields = fields_cnt;

    return csv_row;
}


/**
 * @brief Read the next non-skippable line from a file using @a getline()
 *
 * Repeatedly reads lines with @a getline() until a non-skippable line is
 * found (line_is_skippable returns false) or EOF/error occurs.  Removes
 * a trailing newline and optional carriage return from the buffer so
 * the returned line is null-terminated without CR/LF.
 *
 * @param fp File pointer to read from (must be open for reading)
 * @param lineptr Pointer to a heap buffer pointer used by @a getline()
 * @param nptr    Pointer to size of @e *lineptr; may be 0
 * @param line_no Pointer to line counter to increment for each physical
 *                line read
 *
 * @return Number of characters in the returned line (excluding final
 *         @c NULL), or -1 on EOF or error (in that case @e *lineptr may
 *         be @c NULL or contain partial data)
 *
 * @note On success, @e *lineptr is set (or reused) and contains the
 *       null-terminated line; caller owns and must free @e *lineptr.
 * @note The function increments @p line_no for every physical line read
 *       (including skipped ones).
 */
static ssize_t s_read_next_non_skippable_line(FILE *fp, char **lineptr,
        size_t *nptr, size_t *line_no)
{
    ssize_t len;

    while ((len = getline(lineptr, nptr, fp)) != -1) {
        (*line_no)++;
        /* Remove trailing newline for easier processing (keep null
         * termination) */
        if (len > 0 && ((*lineptr)[len - 1] == '\n')) {
            (*lineptr)[len - 1] = '\0';
            len--;
            if (len > 0 && ((*lineptr)[len - 1] == '\r')) {
                (*lineptr)[len - 1] = '\0';
                len--;
            }
        }
        if (s_line_is_skippable(*lineptr)) {
            continue;
        }
        return len;
    }
    return -1;
}


/* Initialize the CSV parser */
csv_parser_td *csv_parser_init(const char *filename, const char *delim,
        bool has_header)
{
    csv_parser_td *csv_parser;

    csv_parser = malloc(sizeof(csv_parser_td));
    if (csv_parser == NULL) {
        return NULL;
    }

    csv_parser->fp = NULL;
    csv_parser->filename = (filename) ? strdup(filename) : NULL;
    csv_parser->line_no = 0;
    csv_parser->has_header = has_header;
    csv_parser->header = NULL;
    csv_parser->delim =
        (delim && *delim == ',') ? ',' : (delim &&
                                         *delim &&
                                         *delim != '\n' &&
                                         *delim != '\r' &&
                                         *delim != '"') ? *delim : ',';

    return csv_parser;
}


/* Deallocate the memory used by the CSV parser */
void csv_parser_destroy(csv_parser_td *csv_parser)
{
    if (csv_parser == NULL) {
        return;
    }

    if (csv_parser->filename != NULL) {
        free(csv_parser->filename);
    }

    if (csv_parser->fp != NULL) {
        fclose(csv_parser->fp);
    }

    if (csv_parser->header != NULL) {
        csv_parser_destroy_row(csv_parser->header);
    }

    free(csv_parser);
}


/* Deallocate the memory used by a row of the CSV parser */
void csv_parser_destroy_row(csv_row_td *csv_row)
{
    if (csv_row == NULL) {
        return;
    }

    for (size_t i = 0; i < csv_row->num_fields; ++i) {
        free(csv_row->fields[i]);
    }

    free(csv_row->fields);
    free(csv_row);
}


/* Get the header of the CSV file, if any */
const csv_row_td *csv_parser_header(csv_parser_td *csv_parser)
{
    if (csv_parser == NULL || !csv_parser->has_header) {
        return NULL;
    }

    if (csv_parser->header) {
        return csv_parser->header;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t len = s_read_next_non_skippable_line(csv_parser->fp, &line,
            &cap, &csv_parser->line_no);
    if (len == -1) {
        free(line);
        return NULL;
    }

    csv_parser->header = s_parse_line_to_row(line, csv_parser->delim);
    free(line);

    return csv_parser->header;
}


/* Get the current row that it's being parsed */
csv_row_td *csv_parser_row(csv_parser_td *csv_parser)
{
    if (csv_parser == NULL) {
        return NULL;
    }

    if (csv_parser->fp == NULL) {
        csv_parser->fp = fopen(csv_parser->filename, "rb");
        if (csv_parser->fp == NULL) {
            return NULL;
        }
    }

    /* If header requested but not yet consumed, consume it first */
    if (csv_parser->has_header && csv_parser->header == NULL) {
        (void) csv_parser_header(csv_parser);
        /* header consumed; continue to next row */
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t len = s_read_next_non_skippable_line(csv_parser->fp, &line,
            &cap, &csv_parser->line_no);
    if (len == -1) {
        free(line);
        return NULL;
    }

    csv_row_td *csv_row = s_parse_line_to_row(line, csv_parser->delim);
	free(line);

    return csv_row;
}
