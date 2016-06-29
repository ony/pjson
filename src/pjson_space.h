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

static bool pj_comment_line(pj_parser_ref parser, pj_token *token, const char *p, state s)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        if (p == p_end)
        {
            parser->ptr = p;
            parser->state = S_COMMENT_LINE;
            parser->state0 = s;
            if (!pj_remember_pending(parser, token)) return false;
            parser->chunk = p;
            token->token_type = PJ_STARVING;
            return false;
        }

        switch (*p)
        {
        case '\n':
            parser->ptr = p+1;
            parser->state = s;
            return pj_poll_tok(parser, token);
        default:
            ++p;
        }
    }
}

static bool pj_comment_end(pj_parser_ref parser, pj_token *token, const char *p, state s);

static bool pj_comment_region(pj_parser_ref parser, pj_token *token, const char *p, state s)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        if (p == p_end)
        {
            parser->ptr = p;
            parser->state = S_COMMENT_REGION;
            parser->state0 = s;
            if (!pj_remember_pending(parser, token)) return false;
            parser->chunk = p;
            token->token_type = PJ_STARVING;
            return false;
        }

        switch (*p)
        {
        case '*':
            return pj_comment_end(parser, token, p+1, s);

        default:
            ++p;
        }
    }
}

static bool pj_comment_end(pj_parser_ref parser, pj_token *token, const char *p, state s)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        if (p == p_end)
        {
            parser->ptr = p;
            parser->state = S_COMMENT_END;
            parser->state0 = s;
            if (!pj_remember_pending(parser, token)) return false;
            parser->chunk = p;
            token->token_type = PJ_STARVING;
            return false;
        }

        switch (*p)
        {
        case '*':
            ++p;
            break;
        case '/':
            parser->ptr = p+1;
            parser->state = s;
            return pj_poll_tok(parser, token);

        default:
            return pj_comment_region(parser, token, p+1, s);
        }
    }
}

static bool pj_comment_start(pj_parser_ref parser, pj_token *token, const char *p, state s)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        if (p == p_end)
        {
            parser->ptr = p;
            parser->state = S_COMMENT_START;
            parser->state0 = s;
            if (!pj_remember_pending(parser, token)) return false;
            parser->chunk = p;
            token->token_type = PJ_STARVING;
            return false;
        }

        switch (*p)
        {
        case '*':
            return pj_comment_region(parser, token, p+1, s);
        case '/':
            return pj_comment_line(parser, token, p+1, s);

        default:
            pj_err_tok(parser, token);
            return false;
        }
    }
}

static bool pj_space(pj_parser_ref parser, pj_token *token, const char *p, state s)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        if (p == p_end)
        {
            parser->ptr = p;
            parser->state = s;
            return pj_poll_tok(parser, token);
        }

        switch (*p)
        {
        case '\t': case '\n': case '\r': case ' ':
            ++p;
            break;
        case '/':
            return pj_comment_start(parser, token, p+1, s);
        default:
            parser->ptr = p;
            parser->state = s;
            return pj_poll_tok(parser, token);
        }
    }
}

#endif
