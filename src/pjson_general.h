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

static bool pj_poll_tok(pj_parser_ref parser, pj_token *token);

#ifndef __pjson_general_h__
#define __pjson_general_h__

#include "pjson.h"
#include "pjson_state.h"
#include "pjson_space.h"

/* parsing internals */
static void pj_flush_tok(pj_parser_ref parser, pj_token *token)
{
    if (pj_is_end(parser) && !pj_use_buf(parser))
    {
        parser->state = S_END;
        token->token_type = PJ_END;
        return;
    }
    /* TODO: finish token */
    pj_err_tok(parser, token);
}

static bool pj_null(pj_parser_ref parser, pj_token *token)
{
    static const char * const s_null = "null";

    const char *p = parser->ptr;
    const char * const p_end = parser->chunk_end;
    const char * s = s_null + (parser->state - S_N) + 1;

    for (;;)
    {
        if (p == p_end)
        {
            token->token_type = PJ_STARVING;
            return false;
        }
        if (*p != *s)
        {
            parser->ptr = p;
            pj_err_tok(parser, token);
            return false;
        }
        ++p, ++s; /* next char - next state */
        if (*s == '\0')
        {
            token->token_type = PJ_TOK_NULL;
            parser->ptr = p;
            parser->chunk = p;
            parser->state = S_VALUE;
            return true;
        }
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

    case S_SPACE:
        return pj_space(parser, token, S_SPACE);

    case S_INIT:
        if (p == p_end)
        {
            token->token_type = PJ_STARVING;
            return false;
        }
        switch (*p)
        {
        case '\t': case '\n': case '\r': case ' ':
            parser->ptr = ++p;
            return pj_space(parser, token, S_SPACE);
        case 'n':
            parser->state = S_N;
            parser->ptr = ++p;
            return pj_null(parser, token);
        default:
            pj_err_tok(parser, token);
            return false;
        }

    case S_N ... S_NUL:
        return pj_null(parser, token);

    case S_VALUE:
        if (p == p_end)
        {
            token->token_type = PJ_STARVING;
            return false;
        }
        switch (*p)
        {
        case '\t': case '\n': case '\r': case ' ':
            parser->ptr = ++p;
            return pj_space(parser, token, S_VALUE);
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

#endif
