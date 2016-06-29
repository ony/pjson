/*
 * pjson is a library for parsing json into queue of tokens
 *
 * Copyright (C) 2016  Mykola Orliuk <virkony@gmail.com>
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

#pragma once

#include "pjson.h"
#include "pjson_state.h"
#include "pjson_space.h"
#include "pjson_debug.h"

static bool pj_string_value_flush(pj_parser_ref parser, pj_token *token, const char *p, state s, pj_token_type tok)
{
    if (pj_use_buf(parser))
    {
        pj_buf_tok_flush(parser, token, p, s, tok);
    }
    else
    {
        token->str = parser->chunk;
        token->len = parser->token_end - parser->chunk;
        pj_tok(parser, token, p, s, tok);
    }
    return true;
}

static bool pj_string_value(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;
    if (p == p_end)
    {
        parser->ptr = p;
        if (!pj_use_buf(parser))
        {
            if (!pj_add_chunk(parser, token, parser->token_end)) return false;
            parser->token_end = NULL;
            parser->state |= F_BUF;
        }
        parser->chunk = p;
        parser->state = pj_new_state(parser, S_STR_VALUE);
        token->token_type = PJ_STARVING;
        return false;
    }

    switch (*p)
    {
    case ':':
        return pj_string_value_flush(parser, token, p+1, S_INIT, PJ_TOK_KEY);

    case ',': case ']': case '}':
        return pj_string_value_flush(parser, token, p, S_VALUE, PJ_TOK_STR);

    case '/':
        return pj_comment_start(parser, token, p+1, S_STR_VALUE);

    case '\t': case '\n': case '\r': case ' ':
        return pj_space(parser, token, p+1, S_STR_VALUE);

    default:
        pj_err_tok(parser, token);
        return false;
    }
}
