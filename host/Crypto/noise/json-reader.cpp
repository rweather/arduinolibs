/*
 * Copyright (C) 2016 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "json-reader.h"
#include <stdarg.h>

/* Note: This is not a complete JSON parser.  It has enough support
   to process the Noise test vector format only */

#define JSON_STR_MAX 8192

/**
 * \brief Initializes a JSON reader object.
 *
 * \param reader The reader to initialize.
 * \param stream The stream to read from.
 */
void json_init(JSONReader *reader, const char *filename, FILE *stream)
{
    reader->stream = stream;
    reader->token = JSON_TOKEN_START;
    reader->str_value = 0;
    reader->filename = filename;
    reader->line_number = 1;
    reader->saw_eof = 0;
    reader->errors = 0;
}

/**
 * \brief Frees a JSON reader object.
 *
 * \param reader The reader to free.
 */
void json_free(JSONReader *reader)
{
    reader->stream = 0;
    reader->token = JSON_TOKEN_END;
    reader->filename = 0;
    reader->line_number = 1;
    reader->saw_eof = 1;
    reader->errors = 0;
    if (reader->str_value) {
        free(reader->str_value);
        reader->str_value = 0;
    }
}

/**
 * \brief Recognizes a named token.
 *
 * \param reader The reader.
 * \param token The token code that we expect to recognize.
 * \param name The name of the token.  The first character is assumed
 * to have already been recognized.
 */
static void json_named_token
    (JSONReader *reader, JSONToken token, const char *name)
{
    const char *n = name + 1;
    int ch;
    for (;;) {
        ch = getc(reader->stream);
        if (*n == '\0') {
            if (ch == ',' || ch == ' ' || ch == '\t' ||
                    ch == '\r' || ch == '\n') {
                ungetc(ch, reader->stream);
                reader->token = token;
                return;
            } else if (ch == EOF) {
                reader->token = token;
                reader->saw_eof = 1;
                return;
            } else {
                break;
            }
        }
        if (ch == EOF || ch != *n)
            break;
        ++n;
    }
    json_error(reader, "Could not recognize '%s' token", name);
    reader->token = JSON_TOKEN_END;
}

/**
 * \brief Reads the next token from the input stream.
 *
 * \param reader The reader.
 *
 * \return The token code.
 */
JSONToken json_next_token(JSONReader *reader)
{
    int ch;

    /* Bail out if we already reached the end of the stream */
    if (reader->token == JSON_TOKEN_END)
        return JSON_TOKEN_END;
    if (reader->saw_eof) {
        reader->token = JSON_TOKEN_END;
        return JSON_TOKEN_END;
    }

    /* Free the previous token's string value */
    if (reader->str_value) {
        free(reader->str_value);
        reader->str_value = 0;
    }

    /* Skip whitespace */
    for (;;) {
        ch = getc(reader->stream);
        if (ch == EOF) {
            reader->token = JSON_TOKEN_END;
            reader->saw_eof = 1;
            return reader->token;
        } else if (ch == '\n') {
            ++(reader->line_number);
        } else if (ch != ' ' && ch != '\t' && ch != '\r') {
            break;
        }
    }

    /* Parse the next token */
    if (ch == '{') {
        reader->token = JSON_TOKEN_LBRACE;
    } else if (ch == '}') {
        reader->token = JSON_TOKEN_RBRACE;
    } else if (ch == '[') {
        reader->token = JSON_TOKEN_LSQUARE;
    } else if (ch == ']') {
        reader->token = JSON_TOKEN_RSQUARE;
    } else if (ch == ',') {
        reader->token = JSON_TOKEN_COMMA;
    } else if (ch == ':') {
        reader->token = JSON_TOKEN_COLON;
    } else if (ch == 't') {
        json_named_token(reader, JSON_TOKEN_TRUE, "true");
    } else if (ch == 'f') {
        json_named_token(reader, JSON_TOKEN_FALSE, "false");
    } else if (ch == 'n') {
        json_named_token(reader, JSON_TOKEN_FALSE, "null");
    } else if (ch == '"') {
        /* Recognize very simple strings with no escaping */
        char buffer[JSON_STR_MAX];
        size_t posn = 0;
        for (;;) {
            ch = getc(reader->stream);
            if (ch == '"') {
                break;
            } else if (ch == '\r' || ch == '\n' || ch == EOF) {
                json_error(reader, "Unterminated string");
                reader->token = JSON_TOKEN_END;
                return reader->token;
            } else if (ch == '\\') {
                json_error(reader, "String escapes are not supported");
                reader->token = JSON_TOKEN_END;
                return reader->token;
            } else {
                if (posn >= (sizeof(buffer) - 1)) {
                    json_error(reader, "String is too long");
                    reader->token = JSON_TOKEN_END;
                    return reader->token;
                }
                buffer[posn++] = (char)ch;
            }
        }
        buffer[posn] = '\0';
        reader->str_value = (char *)malloc(posn + 1);
        if (!(reader->str_value)) {
            json_error(reader, "Out of memory");
            reader->token = JSON_TOKEN_END;
            return reader->token;
        }
        strcpy(reader->str_value, buffer);
        reader->token = JSON_TOKEN_STRING;
    } else {
        /* Unknown character.  Note: numbers are not yet supported. */
        json_error(reader, "Unknown character 0x%02x", ch);
        reader->token = JSON_TOKEN_END;
    }

    return reader->token;
}

/**
 * \brief Matches the current token against a specific field name.
 *
 * \param reader The reader.
 * \param name The name of the field.
 *
 * \return Returns 1 if the name matches, 0 if not.
 */
int json_is_name(JSONReader *reader, const char *name)
{
    if (reader->token != JSON_TOKEN_STRING || !reader->str_value)
        return 0;
    return !strcmp(reader->str_value, name);
}

/**
 * \brief Reports an error on the current line of the input.
 *
 * \param reader The reader.
 * \param format The printf-style format to use.
 */
void json_error(JSONReader *reader, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    fprintf(stderr, "%s:%ld: ", reader->filename, reader->line_number);
    vfprintf(stderr, format, va);
    putc('\n', stderr);
    va_end(va);
    ++(reader->errors);
}

/**
 * \brief Reports an error on a specific line of the input.
 *
 * \param reader The reader.
 * \param line The line number to report in the error message.
 * \param format The printf-style format to use.
 */
void json_error_on_line(JSONReader *reader, long line, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    fprintf(stderr, "%s:%ld: ", reader->filename, line);
    vfprintf(stderr, format, va);
    putc('\n', stderr);
    va_end(va);
    ++(reader->errors);
}
