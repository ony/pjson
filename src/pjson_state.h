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

#ifndef __pjson_state_h__
#define __pjson_state_h__

#include "pjson.h"
#include "pjson_debug.h"

typedef enum {
    S_INIT = 0,
    S_ERR,
    S_END,
    S_SPACE,
    S_VALUE,
    S_NUM,
    S_N = 10, S_NU, S_NUL,
    S_T = 20, S_TR, S_TRU,
    S_F = 30, S_FA, S_FAL, S_FALS,
    S_STR = 40, S_ESC, S_STR_VALUE, /* str may end up as key */
} state;

#define F_BUF 0x100
#define F_END 0x200

/* state manipulation */
static state pj_state(pj_parser_ref parser)
{ return (parser->state & 0xff ); }

static bool pj_is_end(pj_parser_ref parser)
{ return (parser->state & F_END ); }

static bool pj_use_buf(pj_parser_ref parser)
{ return (parser->state & F_BUF ); }

static void pj_set_end(pj_parser_ref parser)
{
    switch (pj_state(parser))
    {
    case S_END:
    case S_ERR:
        break;
    default:
        parser->state |= F_END;
    }
}

static bool pj_add_chunk(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const chunk = parser->chunk;
    const size_t len = p - chunk;
    char * buf_ptr1 = parser->buf_ptr + len;
    if (len > 0)
    {
        /* ensure that we have enough space */
        if (buf_ptr1 > parser->buf_end)
        {
            TRACEF("overflow required %ld more", len - parser->buf_len);
            token->token_type = PJ_OVERFLOW;
            token->len = len;
            parser->ptr = p;
            return false;
        }

        (void) memcpy(parser->buf_ptr, chunk, len);
        parser->buf_ptr = buf_ptr1;
    }
    return true;
}

static void pj_part_tok(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    assert( p == parser->chunk_end );

    const char *old_ptr = parser->buf_ptr;
    if (p > parser->chunk)
    {
        if (!pj_add_chunk(parser, token, p)) return;
        if (!pj_use_buf(parser))
        {
            parser->state |= F_BUF;
            parser->buf_last = old_ptr;
        }
    }
    parser->chunk = p;
    parser->ptr = p;
    token->token_type = PJ_STARVING;
}

static void pj_err_tok(pj_parser_ref parser, pj_token *token)
{
    token->token_type = PJ_ERR;
    parser->state = S_ERR;
}

static void pj_tok(pj_parser_ref parser, pj_token *token,
                   const char *p, state s, pj_token_type tok)
{
    parser->ptr = p;
    parser->chunk = p;
    parser->state = s;
    token->token_type = tok;
}

#endif
