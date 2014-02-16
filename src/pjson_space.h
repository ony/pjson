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

#ifndef __pjson_space_h__
#define __pjson_space_h__

#include "pjson.h"
#include "pjson_general.h"

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

#endif
