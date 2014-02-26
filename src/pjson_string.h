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

#ifndef __pjson_string_h__
#define __pjson_string_h__

#include "pjson.h"
#include "pjson_state.h"
#include "pjson_debug.h"

static bool pj_string_esc(pj_parser_ref parser, pj_token *token, const char *p);

static bool pj_string(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            pj_part_tok(parser, token, p);
            return false;
        }

        switch (*p)
        {
        case '"':
            if (pj_use_buf(parser))
            {
                if (!pj_add_chunk(parser, token, p)) return false;
                token->token_type = PJ_TOK_STR;
                token->str = parser->buf;
                token->len = parser->buf_ptr - parser->buf;
                parser->ptr = ++p;
                parser->chunk = p;
                parser->state = S_STR_VALUE;
                return true;
            }
            else
            {
                token->str = parser->chunk;
                token->len = p - parser->chunk;
                pj_tok(parser, token, ++p, S_STR_VALUE, PJ_TOK_STR);
                return true;
            }
            /* unreachable */
            break;

        case '\\':
            if (!pj_add_chunk(parser, token, p)) return false;
            parser->state = S_STR | F_BUF;
            return pj_string_esc(parser, token, ++p);

        default: ++p;
        }
    }
    /* unreachable */
}

static bool pj_string_esc(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;
    if (p == p_end)
    {
        pj_part_tok(parser, token, p);
        return false;
    }

    parser->chunk = p;
    parser->ptr = p;
    TRACEF("char '%c'", *p);
    switch (*p)
    {
	/* guarded chars */
    case '"': case '/': case '\\':
        return pj_string(parser, token, ++p);

    /* special chars */
    case 'b':
        if (!pj_add_block(parser, token, "\b", 1, p)) return false;
        parser->chunk = ++p;
        return pj_string(parser, token, p);
    case 'f':
        if (!pj_add_block(parser, token, "\f", 1, p)) return false;
        parser->chunk = ++p;
        return pj_string(parser, token, p);
    case 't':
        if (!pj_add_block(parser, token, "\t", 1, p)) return false;
        parser->chunk = ++p;
        return pj_string(parser, token, p);
    case 'n':
        if (!pj_add_block(parser, token, "\n", 1, p)) return false;
        parser->chunk = ++p;
        return pj_string(parser, token, p);
    case 'r':
        if (!pj_add_block(parser, token, "\r", 1, p)) return false;
        parser->chunk = ++p;
        return pj_string(parser, token, ++p);

    default:
        pj_err_tok(parser, token);
        return false;
    }
}

#endif
