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
#include "pjson_keyword.h"
#include "pjson_string.h"

/* parsing internals */
static void pj_flush_tok(pj_parser_ref parser, pj_token *token)
{
    assert( pj_state(parser) != S_ERR );
    assert( pj_state(parser) != S_END );
    if (pj_is_end(parser) && !pj_use_buf(parser))
    {
        parser->state = S_END;
        token->token_type = PJ_END;
        return;
    }
    /* TODO: finish token */
    pj_err_tok(parser, token);
}

static const char
    * const s_null = "null",
    * const s_true = "true",
    * const s_false = "false";

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
            return pj_keyword(parser, token, s_null, S_N, PJ_TOK_NULL);
        case 't':
            parser->state = S_T;
            parser->ptr = ++p;
            return pj_keyword(parser, token, s_true, S_T, PJ_TOK_TRUE);
        case 'f':
            parser->state = S_F;
            parser->ptr = ++p;
            return pj_keyword(parser, token, s_false, S_F, PJ_TOK_FALSE);
        case '[':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_ARR);
            return true;
        case ']':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_ARR_E);
            return true;
        case '{':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_MAP);
            return true;
        case '}':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_MAP_E);
            return true;
        case '"':
            parser->state = S_STR;
            parser->chunk = ++p;
            return pj_string(parser, token, p);

        default:
            pj_err_tok(parser, token);
            return false;
        }

    case S_N ... S_NUL:
        return pj_keyword(parser, token, s_null, S_N, PJ_TOK_NULL);

    case S_T ... S_TRU:
        return pj_keyword(parser, token, s_true, S_T, PJ_TOK_TRUE);

    case S_F ... S_FALS:
        return pj_keyword(parser, token, s_false, S_F, PJ_TOK_FALSE);

    case S_STR:
        return pj_string(parser, token, p);

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

        case ',':
            parser->ptr = ++p;
            parser->state = S_INIT;
            return pj_poll_tok(parser, token);

        case ']':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_ARR_E);
            return true;
        case '}':
            pj_tok(parser, token, ++p, S_INIT, PJ_TOK_MAP_E);
            return true;

        default:
            pj_err_tok(parser, token);
            return false;
        }

    default:
        assert(!"invalid state"); /* improperly initialized parser? */
    }

    // unreachable code
}

#endif
