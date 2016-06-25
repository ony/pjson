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
#include "pjson_debug.h"

/* API */
void pj_feed(pj_parser_ref parser, const char *chunk, size_t len)
{
    TRACE_FUNC();
    assert( parser != NULL );
    assert( len == 0 || chunk != NULL );
    assert( parser->chunk == NULL || parser->chunk == parser->chunk_end );
    assert( !pj_use_buf(parser) || (parser->buf <= parser->buf_last && parser->buf_last <= parser->buf_ptr) );

    if ( parser->chunk != NULL && parser->chunk != parser->chunk_end )
    {
        parser->state = S_ERR;
        return;
    }

    parser->chunk = chunk;
    parser->ptr = chunk;
    parser->chunk_end = chunk + len;
}

void pj_feed_end(pj_parser_ref parser)
{
    TRACE_FUNC();
    assert( parser != NULL );

    pj_set_end(parser);
}

void pj_realloc(pj_parser_ref parser, char *buf, size_t buf_len)
{
    TRACE_FUNC();
    assert( parser != NULL );
    assert( !pj_use_buf(parser) || (parser->buf <= parser->buf_last && parser->buf_last <= parser->buf_ptr) );
    assert( !pj_use_buf(parser) || (parser->buf_last + buf_len >= parser->buf_ptr) );

    if (pj_use_buf(parser))
    {
        /* relocate last partial token to buffer start */
        size_t prev_chunk_len = parser->buf_ptr - parser->buf_last;
        (void) memmove(buf, parser->buf_last, prev_chunk_len);
        parser->buf_ptr = buf + prev_chunk_len;
    }
    else
    {
        parser->buf_ptr = buf;
    }
    parser->buf = buf;
    parser->buf_end = buf + buf_len;
    parser->buf_last = buf;
}

void pj_poll(pj_parser_ref parser, pj_token *tokens, size_t len)
{
    TRACE_FUNC();
    assert( parser != NULL );
    assert( len > 0 );
    assert( tokens != NULL );
    assert( parser->state != S_ERR ); /* already reported an error */

    if (len == 0) return; /* nothing to fill */

    pj_token *tokens_end = tokens + len;

    if (pj_is_end(parser))
    {
        if (pj_state(parser) != S_END)
        {
            pj_flush_tok(parser, tokens);

            TRACE_TOKEN(tokens);
            TRACE_PARSER(parser, parser->ptr);

            if (parser->state == S_END || parser->state == S_ERR)
                return;

            /* next token to fill if possible */
            if (++tokens == tokens_end)
            {
                /* we should re-enter this code to give back PJ_END */
                parser->state = pj_new_state(parser, S_END);
                return;
            }
        }
        tokens->token_type = PJ_END;
        parser->state = S_END; /* this is final PJ_END */
        return;
    }

    /* prepare suppl. buffer */
    if (pj_use_buf(parser)) /* have incomplete token? */
    {
        /* relocate last partial token to buffer start */
        size_t prev_chunk_len = parser->buf_ptr - parser->buf_last;
        (void) memmove(parser->buf, parser->buf_last, prev_chunk_len);
        parser->buf_ptr = parser->buf + prev_chunk_len;
        parser->buf_last = parser->buf;
    }
    else
    {
        /* free prev tokens from suppl. buffer */
        parser->buf_ptr = parser->buf;
        parser->buf_last = parser->buf;
    }

    for (; tokens != tokens_end && pj_poll_tok(parser, tokens); ++tokens)
    {
        TRACE_TOKEN(tokens);
        TRACE_PARSER(parser, parser->ptr);
    }
#ifdef ENABLE_TRACES
    if (tokens != tokens_end)
    {
        /* dump terminate token */
        TRACE_TOKEN(tokens);
        TRACE_PARSER(parser, parser->ptr);
    }
#endif
}
