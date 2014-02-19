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

static bool pj_string(pj_parser_ref parser, pj_token *token, const char *p)
{
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
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
                token->str = parser->buf;
                token->len = parser->buf_ptr - parser->buf;
                parser->ptr = ++p;
                parser->chunk = p;
                parser->state = S_STR_VALUE;
                return pj_poll_tok(parser, token);
            }
            else
            {
                token->str = parser->chunk;
                token->len = p - parser->chunk;
                parser->ptr = ++p;
                parser->chunk = p;
                parser->state = S_STR_VALUE;
                return pj_poll_tok(parser, token);
            }
            return true;

        case '\\':
            pj_err_tok(parser, token);
            return false;

        default: ++p;
        }
    }
    /* unreachable */
}

#endif
