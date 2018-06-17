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

#ifndef JSON_READER_H
#define JSON_READER_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * \brief Token codes.
 */
typedef enum
{
    JSON_TOKEN_START,       /**< Still at the start, need to read first token */
    JSON_TOKEN_STRING,      /**< Quoted string */
    JSON_TOKEN_NUMBER,      /**< Numeric value */
    JSON_TOKEN_NULL,        /**< "null" */
    JSON_TOKEN_TRUE,        /**< "true" */
    JSON_TOKEN_FALSE,       /**< "false" */
    JSON_TOKEN_LBRACE,      /**< "{" */
    JSON_TOKEN_RBRACE,      /**< "}" */
    JSON_TOKEN_LSQUARE,     /**< "[" */
    JSON_TOKEN_RSQUARE,     /**< "]" */
    JSON_TOKEN_COMMA,       /**< "," */
    JSON_TOKEN_COLON,       /**< ":" */
    JSON_TOKEN_END          /**< End of stream or error */

} JSONToken;

/**
 * \brief State information for JSON readers.
 */
typedef struct
{
    FILE *stream;           /**< Input stream to read from */
    JSONToken token;        /**< Current token type */
    char *str_value;        /**< String value for the current token */
    const char *filename;   /**< Name of the file being read from */
    long line_number;       /**< Current line number in the file */
    int saw_eof;            /**< Non-zero if already seen EOF */
    int errors;             /**< Non-zero if errors seen during parsing */

} JSONReader;

void json_init(JSONReader *reader, const char *filename, FILE *stream);
void json_free(JSONReader *reader);
JSONToken json_next_token(JSONReader *reader);
int json_is_name(JSONReader *reader, const char *name);
void json_error(JSONReader *reader, const char *format, ...);
void json_error_on_line(JSONReader *reader, long line, const char *format, ...);

#endif
