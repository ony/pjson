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

#ifndef __pjson_number_h__
#define __pjson_number_h__

#include "pjson.h"
#include "pjson_state.h"
#include "pjson_debug.h"

static bool pj_number(pj_parser_ref parser, pj_token *token, const char *p)
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
        case '0' ... '9':
            ++p;
            break;

        default:
            token->str = parser->chunk;
            token->len = p - parser->chunk;
            pj_tok(parser, token, p, S_VALUE, PJ_TOK_NUM);
            return true;
        }
    }
}

#endif
