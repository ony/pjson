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

#ifndef __pjson_debug_h__
#define __pjson_debug_h__

#ifdef ENABLE_TRACES
#include <stdio.h>

#define PRINT_TRACE(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define TRACE_TOKEN(token) trace_token(__FILE__, __LINE__, token)
#else
#define PRINT_TRACE(fmt, args...)
#define TRACE_TOKEN(token)
#endif

#define TRACEF(fmt, ...) PRINT_TRACE("%s+%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)
#define TRACE(s) TRACEF("%s", (s))
#define TRACE_PARSER(parser, p) TRACEF( \
    "parser { .state = %d, .input = \"%.*s\" }", \
    (parser)->state, (int)((parser)->chunk_end - (p)), (p) \
    )

/* expect presence of "parser" in scope of "call" */
#define TRACE_FUNC() TRACEF("%s() state = %d, input = \"%.*s\"", \
    __func__, (parser)->state, (int)((parser)->chunk_end - (parser)->ptr), (parser)->ptr)

static inline const char *token_type_name(pj_token_type token_type)
{
    switch (token_type)
    {
    case PJ_END: return "PJ_END";
    case PJ_ERR: return "PJ_ERR";
    case PJ_STARVING: return "PJ_STARVING";
    case PJ_OVERFLOW: return "PJ_OVERFLOW";
    case PJ_TOK_STR: return "PJ_TOK_STR";
    case PJ_TOK_ARR: return "PJ_TOK_ARR";
    case PJ_TOK_ARR_E: return "PJ_TOK_ARR_E";
    case PJ_TOK_MAP: return "PJ_TOK_MAP";
    case PJ_TOK_MAP_E: return "PJ_TOK_MAP_E";
    default: return "<todo>";
    }
}

#ifdef ENABLE_TRACES
static inline void trace_token(const char *file, int line, pj_token *token)
{
    switch (token->token_type)
    {
    case PJ_TOK_STR:
        PRINT_TRACE("%s+%d: token %d (STR) \"%.*s\"", file, line, token->token_type, (int)token->len, token->str);
        break;
    default:
        TRACEF("%s+%d: token %d (%s)", file, line, token->token_type, token_type_name(token->token_type));
    }
}
#endif

#endif
