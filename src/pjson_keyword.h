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

#ifndef __pjson_keyword_h__
#define __pjson_keyword_h__

#include "pjson.h"
#include "pjson_state.h"

static bool pj_keyword(pj_parser_ref parser, pj_token *token,
                       const char * const keyword,
                       state base_s, pj_token_type tok)
{
    const char *p = parser->ptr;
    const char * const p_end = parser->chunk_end;
    const char * s = keyword + (parser->state - base_s) + 1;

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
            pj_tok(parser, token, p, S_VALUE, tok);
            return true;
        }
    }
}

#endif
