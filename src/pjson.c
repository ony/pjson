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

#include "stdbool.h"
#include "assert.h"

#include "pjson.h"

typedef enum {
    S_INIT = 0,
    S_ERR,
    S_END,
    S_SPACE
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
{ parser->state |= F_END; }

/* parsing internals */
static void pj_part_tok(pj_parser_ref parser, pj_token *token)
{
    assert( parser->ptr == parser->chunk_end );

    const size_t tok_len = parser->ptr - parser->chunk;
    if (tok_len > 0)
    {
        /* ensure that we have enough space */
        if (tok_len > parser->buf_len)
        {
            token->token_type = PJ_OVERFLOW;
            return;
        }

        (void) memcpy(parser->buf, parser->chunk, tok_len);
    }
    token->token_type = PJ_STARVING;
    parser->state |= F_BUF;
}

static void pj_err_tok(pj_parser_ref parser, pj_token *token)
{
    token->token_type = PJ_ERR;
    parser->state = S_ERR;
}

static void pj_flush_tok(pj_parser_ref parser, pj_token *token)
{
    if (pj_is_end(parser) && !pj_use_buf(parser))
    {
        parser->state = S_END;
        token->token_type = PJ_END;
        return;
    }
    /* TODO: finish token */
    parser->state = S_ERR;
    token->token_type = PJ_ERR;
}

static bool pj_poll_tok(pj_parser_ref parser, pj_token *token);

static bool pj_isspace(char c)
{
    switch (c)
    {
    case '\t': case '\n': case '\r': case ' ':
        return true;
    default:
        return false;
    }
}

static bool pj_space(pj_parser_ref parser, pj_token *token)
{
    const char *p = parser->ptr;
    const char * const p_end = parser->chunk_end;
    for (; p != p_end && pj_isspace(*p); ++p);
    parser->ptr = p;
    parser->chunk = p;
    if (p == p_end)
    {
        parser->state = S_SPACE;
        token->token_type = PJ_STARVING;
        return false;
    }
    else
    {
        parser->state = S_INIT;
        return pj_poll_tok(parser, token);
    }
}

static bool pj_poll_tok(pj_parser_ref parser, pj_token *token)
{
    const char *p = parser->ptr;
    const char * const p_end = parser->chunk_end;

    switch (pj_state(parser))
    {
    case S_END:
        token->token_type = PJ_END;
        return false;
    case S_ERR:
        token->token_type = PJ_ERR;
        return false;

    case S_SPACE: return pj_space(parser, token);

    case S_INIT:
        if (p == p_end)
        {
            token->token_type = PJ_STARVING;
            return false;
        }
        switch (*p)
        {
        case '\t': case '\n': case '\r': case ' ':
            return pj_space(parser, token);
        default:
            pj_err_tok(parser, token);
            return false;
        }

    default:
        assert(!"invalid state"); /* improperly initialized parser? */
    }

    /* TODO: parse */
    pj_flush_tok(parser, token);

    return false;
}

/* API */
void pj_feed(pj_parser_ref parser, const char *chunk, size_t len)
{
    assert( parser != NULL );
    assert( len == 0 || chunk != NULL );
    assert( parser->chunk == NULL || parser->chunk == parser->chunk_end );

    if ( parser->chunk != NULL && parser->chunk != parser->chunk_end )
    {
        parser->state = S_ERR;
        return;
    }

    parser->chunk = chunk;
    parser->ptr = chunk;
    parser->chunk_end = chunk + len;
    if (!pj_use_buf(parser)) parser->ptr = chunk;
}

void pj_feed_end(pj_parser_ref parser)
{
    assert( parser != NULL );

    pj_set_end(parser);
}

void pj_poll(pj_parser_ref parser, pj_token *tokens, size_t len)
{
    assert( parser != NULL );
    assert( len > 0 );
    assert( tokens != NULL );

    if (len == 0) return; /* nothing to fill */

    if (pj_is_end(parser))
    {
        if (parser->state != S_END && parser->state != S_ERR)
        {
            pj_flush_tok(parser, tokens);

            if (parser->state == S_END && parser->state == S_ERR)
                return;

            parser->state = S_END;
        }

        /* in one of the terminal states? */
        if (parser->state == S_END)
        {
            tokens->token_type = PJ_END;
            return;
        }
        else if (parser->state == S_ERR)
        {
            tokens->token_type = PJ_ERR;
            return;
        }
    }

    pj_token *tokens_end = tokens + len;
    for (; tokens != tokens_end && pj_poll_tok(parser, tokens); ++tokens);
}

