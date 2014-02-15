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

#include "assert.h"

#include "pjson.h"

void pj_feed(pj_parser_ref parser, const char *chunk, size_t len)
{
    assert( parser->chunk == NULL || parser->chunk == parser->chunk_end );
    (void) parser;
    (void) chunk;
    (void) len;
}

void pj_feed_end(pj_parser_ref parser)
{
    assert( parser != NULL );
    (void) parser;
}

void pj_poll(pj_parser_ref parser, pj_token *tokens, size_t len)
{
    assert( parser != NULL );
    assert( len > 0 );
    (void) parser;

    if (len == 0) return; /* nothing to fill */

    tokens->token_type = PJ_END;
}

