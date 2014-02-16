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

#include "pjson_state.h"
#include "pjson_general.h"

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

    pj_token *tokens_end = tokens + len;

    if (pj_is_end(parser))
    {
        if (parser->state != S_END && parser->state != S_ERR)
        {
            pj_flush_tok(parser, tokens);

            if (parser->state == S_END && parser->state == S_ERR)
                return;

            parser->state = S_END;
        }

        /* next token */
        if (++tokens == tokens_end) return;

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

    for (; tokens != tokens_end && pj_poll_tok(parser, tokens); ++tokens);
}
