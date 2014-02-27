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

#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "pjson.h"
#include "pjson_state.h"
#include "pjson_debug.h"

static bool pj_string_esc(pj_parser_ref parser, pj_token *token, const char *p);
static bool pj_unicode(pj_parser_ref parser, pj_token *token, const char *p);
static bool pj_unicode_esc(pj_parser_ref parser, pj_token *token, const char *p);

static bool pj_string(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    assert( parser->str.c == 0 && mbsinit(&parser->str.s) );

    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            pj_part_tok(parser, token, S_STR, p);
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
                parser->buf_last = parser->buf_ptr;
                pj_tok(parser, token, ++p, S_STR_VALUE, PJ_TOK_STR);
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
            parser->state = S_ESC | F_BUF;
            return pj_string_esc(parser, token, ++p);

        default: ++p;
        }
    }
    /* unreachable */
}

static bool pj_string_esc(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    assert( parser->str.c == 0 && mbsinit(&parser->str.s) );

    const char * const p_end = parser->chunk_end;
    if (p == p_end)
    {
        pj_part_tok(parser, token, S_ESC, p);
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
        return pj_string(parser, token, p);

    case 'u': return pj_unicode_esc(parser, token, p);

    default:
        pj_err_tok(parser, token);
        return false;
    }
}

static bool pj_unicode(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;
    uint32_t c = parser->str.c & ~0xffff;
    uint16_t c16 = parser->str.c & 0xffff;
    size_t n = pj_state(parser) - S_UNICODE;
    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            c = c | c16;
            parser->str.c = c;
            pj_part_tok(parser, token, (S_UNICODE + n) | F_BUF, p);
            return false;
        }
        if (n == 4)
        {
            bool surrogate = 0xd800 <= c16 && c16 <= 0xdbff;
            if (surrogate) /* surrogate pair */
            {
                parser->str.c = (uint32_t)c16 << 16;
                TRACEF("surrogate %08x", parser->str.c);
            }
            else
            {
                wchar_t wc;
                if (c & ~0xffff) /* this is surrogate pair */
                {
                    wc = ((c >> 16) - 0xd800) << 10;
                    wc |= c16 - 0xdc00;
                    wc += 0x010000;
                }
                else
                {
                    wc = htons(c16);
                }
                /* flush wchar */
                size_t encoded = wcrtomb(parser->buf_ptr, wc, &parser->str.s);
                if (encoded == (size_t)-1)
                {
                    TRACEF("invalid unicode: (errno=#%d) %s", errno, strerror(errno));
                    pj_err_tok(parser, token);
                    return false;
                }
                TRACEF("wc = %04x (%C) encoded into %ld bytes", wc, wc, encoded);
                parser->buf_ptr += encoded;
                parser->str.c = 0;
            }

            /* next unicode? */
            switch (*p)
            {
            case '\\':
                return pj_unicode_esc(parser, token, ++p);
            default:
                if (surrogate)
                {
                    pj_err_tok(parser, token);
                    return false;
                }
                parser->chunk = p;
                parser->str.s = (mbstate_t) { 0 }; /* reset multibyte state */
                return pj_string(parser, token, p);
            }
        }
        else
        {
            int digit;
            switch (*p)
            {
            case '0' ... '9': digit = (*p - '0'); break;
            case 'a' ... 'f': digit = 10 + (*p - 'a'); break;
            case 'A' ... 'F': digit = 10 + (*p - 'A'); break;
            default:
                pj_err_tok(parser, token);
                return false;
            }
            c16 = c16 * 0x10 + digit;
            ++p; ++n;
        }
    }
    return false;
}

static bool pj_unicode_esc(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;
    if (p == p_end)
    {
        pj_part_tok(parser, token, S_UNICODE_ESC | F_BUF, p);
        return false;
    }
    switch (*p)
    {
    case 'u':
        if (!pj_reserve(parser, token, MB_CUR_MAX, p))
        {
            parser->state = S_UNICODE_ESC | F_BUF;
            return false;
        }
        parser->state = S_UNICODE | F_BUF;
        return pj_unicode(parser, token, ++p);
    default:
        parser->state = S_ESC | F_BUF;
        /* reset multibyte state */
        parser->str.c = 0;
        parser->str.s = (mbstate_t) { 0 };
        return pj_string_esc(parser, token, p);
    }
}

#endif
