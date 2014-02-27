/*
 * pjson is a library for parsing json into queue of tokens
 *
 * Copyright (C) 2014  Nikolay Orliuk <virkony@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __poll_json_h__
#define __poll_json_h__

#include <string.h>
#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* buffer used for forming some tokens (e.g. chunks boundaries) */
    char *buf;
    char *buf_end;
    char *buf_ptr; /* next free buf */
    const char *buf_last; /* last incomplete token */

    /* currently processing chunk */
    const char *chunk;
    const char *chunk_end;

    int state;
    const char *ptr; /* current position withing chunk */

    union {
        struct {
            uint32_t c; /* may contain surrogate pair */
            mbstate_t s;
        } str;
    };
} pj_parser, *pj_parser_ref;

typedef enum {
    /* terminal tokens */
    PJ_END, /* end of json document */
    PJ_ERR, /* error met */
    PJ_STARVING, /* can free old chunk and need feed another one */
    PJ_OVERFLOW, /* require re-allocation of supplementary buffer for to a bigger one */

    /* normal tokens */
    PJ_TOK_NULL,
    PJ_TOK_TRUE, PJ_TOK_FALSE,
    PJ_TOK_STR,
    PJ_TOK_NUM,
    PJ_TOK_MAP, PJ_TOK_KEY, PJ_TOK_MAP_E,
    PJ_TOK_ARR, PJ_TOK_ARR_E
} pj_token_type;

typedef struct {
    pj_token_type token_type;
    const char *str;
    size_t len;
} pj_token;

static void pj_init(pj_parser_ref parser, char *buf, size_t buf_len)
{
    memset(parser, 0, sizeof(*parser));
    parser->buf = buf;
    parser->buf_end = buf + buf_len;
    parser->buf_ptr = buf;
}

/* notify about re-allocated supplementary buffer */
void pj_realloc(pj_parser_ref parser, char *buf, size_t buf_len);

void pj_feed(pj_parser_ref parser, const char *chunk, size_t len);
void pj_feed_end(pj_parser_ref parser);

void pj_poll(pj_parser_ref parser, pj_token *tokens, size_t len);

#ifdef __cplusplus
}
#endif

#endif
