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
    S_VALUE,
    S_COMMA,
    S_N, S_NU, S_NUL,
    S_T, S_TR, S_TRU,
    S_F, S_FA, S_FAL, S_FALS,

    /* numbers */
    S_NUM,
    S_MAGN, S_MAGN_Z, S_MAGN_G,
    S_FRAC, S_FRAC_NUM, S_EXP, S_EXP_SIGN, S_EXP_NUM,
    S_NUM_END,

    /* string */
    S_STR, S_ESC,
    S_UNICODE, S_UNICODE_FINISH = S_UNICODE + 4, /* 4 hex digits */
    S_UNICODE_ESC, /* handle surrogate pairs */
    S_STR_VALUE, /* str may end up as key */

    /* other stuff */
    S_COMMENT_START, S_COMMENT_LINE, S_COMMENT_REGION, S_COMMENT_END,
} state;

#define F_BUF 0x100
#define F_END 0x200

/* state manipulation */
static state pj_state(pj_parser_ref parser)
{ return (parser->state & 0xff ); }

static int pj_new_state(pj_parser_ref parser, state s)
{ return (parser->state & ~0xff) | s; }

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

static bool pj_reserve(pj_parser_ref parser, pj_token *token, size_t len, const char *p)
{
    char * buf_ptr1 = parser->buf_ptr + len;
    if (buf_ptr1 > parser->buf_end)
    {
        TRACEF("overflow required %ld more", buf_ptr1 - parser->buf_end);
        token->token_type = PJ_OVERFLOW;
        token->len = buf_ptr1 - parser->buf;
        parser->ptr = p;
        return false;
    }
    return true;
}

static bool pj_add_block(pj_parser_ref parser, pj_token *token, const char *block, size_t len, const char *p)
{
    TRACEF("pj_add_block() block = \"%.*s\" (%zd)", (int)len, block, len);
    if (len > 0)
    {
        /* ensure that we have enough space */
        if (!pj_reserve(parser, token, len, p)) return false;

        (void) memcpy(parser->buf_ptr, block, len);
        parser->buf_ptr += len;
    }
    return true;
}

static bool pj_add_chunk(pj_parser_ref parser, pj_token *token, const char *p)
{
    const char * const block = parser->chunk;
    return pj_add_block(parser, token, block, p - block, p);
}

static void pj_part_tok(pj_parser_ref parser, pj_token *token, state s, const char *p)
{
    TRACE_FUNC();
    assert( p == parser->chunk_end );
    assert( !pj_use_buf(parser) || (parser->buf <= parser->buf_last && parser->buf_last <= parser->buf_ptr) );

    parser->state = s;
    if (p > parser->chunk)
    {
        if (!pj_add_chunk(parser, token, p)) return;
        parser->state |= F_BUF;
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
    parser->state = pj_new_state(parser, s) & ~F_BUF;
    token->token_type = tok;
}

static void pj_buf_tok_flush(pj_parser_ref parser, pj_token *token,
                             const char *p, state s, pj_token_type tok)
{
    const char *last = parser->buf_last, *ptr = parser->buf_ptr;
    token->str = last;
    token->len = ptr - last;
    parser->buf_last = ptr;
    pj_tok(parser, token, p, s, tok);
}

static bool pj_buf_tok(pj_parser_ref parser, pj_token *token,
                       const char *p, const char *p1, state s,
                       pj_token_type tok)
{
    if (!pj_add_chunk(parser, token, p)) return false;
    pj_buf_tok_flush(parser, token, p1, s, tok);
    return true;
}

static bool pj_remember_pending(pj_parser_ref parser, pj_token *token)
{
    if (!pj_use_buf(parser) && parser->token_end != NULL)
    {
        if (!pj_add_chunk(parser, token, parser->token_end)) return false;
        parser->token_end = NULL;
        parser->state |= F_BUF;
    }
    return true;
}

#endif
