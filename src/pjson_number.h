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

static bool pj_number_end(pj_parser_ref parser, pj_token *token, state s, const char *p)
{
    TRACE_FUNC();
    if (pj_use_buf(parser))
    {
        if (!pj_buf_tok(parser, token, p, p, S_VALUE, PJ_TOK_NUM))
        {
            parser->state = pj_new_state(parser, s) | F_BUF;
            return false;
        }
    }
    else
    {
        token->str = parser->chunk;
        token->len = p - parser->chunk;
        pj_tok(parser, token, p, S_VALUE, PJ_TOK_NUM);
    }
    return true;
}

static void pj_number_flush(pj_parser_ref parser, pj_token *token)
{
    TRACE_FUNC();
    assert( parser->ptr == parser->chunk_end );

    const state s = pj_state(parser);
    switch (s)
    {
    case S_MAGN_Z:
    case S_MAGN_G:
    case S_FRAC_NUM:
    case S_EXP_NUM:
        (void) pj_number_end(parser, token, s, parser->ptr);
        break;
    default:
        pj_err_tok(parser, token);
    }
}

static bool pj_exponent_number(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    TRACE_PARSER(parser, p);

    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            pj_part_tok(parser, token, S_EXP_NUM, p);
            return false;
        }

        switch (*p)
        {
        case '0' ... '9':
            ++p;
            break;
        case 'e': case 'E': case '.': case '-': case '+':
            pj_err_tok(parser, token);
            return false;

        default:
            return pj_number_end(parser, token, S_EXP_NUM, p);
        }
    }
}

static bool pj_exponent_sign(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    TRACE_PARSER(parser, p);

    if (p == p_end)
    {
        pj_part_tok(parser, token, S_EXP_SIGN, p);
        return false;
    }

    switch (*p)
    {
    case '0' ... '9':
        return pj_exponent_number(parser, token, ++p);

    default:
        pj_err_tok(parser, token);
        return false;
    }
}


static bool pj_exponent_start(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    if (p == p_end)
    {
        pj_part_tok(parser, token, S_EXP, p);
        return false;
    }

    switch (*p)
    {
    case '-': case '+':
        return pj_exponent_sign(parser, token, ++p);
    case '0' ... '9':
        return pj_exponent_number(parser, token, ++p);
    case 'e': case 'E': case '.':
        pj_err_tok(parser, token);
        return false;

    default:
        pj_err_tok(parser, token);
        return false;
    }
}

static bool pj_fraction_number(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            pj_part_tok(parser, token, S_FRAC_NUM, p);
            return false;
        }

        switch (*p)
        {
        case '0' ... '9':
            ++p;
            break;
        case 'e': case 'E':
            return pj_exponent_start(parser, token, ++p);
        case '-': case '+': case '.':
            pj_err_tok(parser, token);
            return false;

        default:
            return pj_number_end(parser, token, S_FRAC_NUM, p);
        }
    }
}

static bool pj_fraction_start(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    if (p == p_end)
    {
        pj_part_tok(parser, token, S_FRAC, p);
        return false;
    }

    switch (*p)
    {
    case '0' ... '9': return pj_fraction_number(parser, token, ++p);

    default:
        pj_err_tok(parser, token);
        return false;
    }
}

static bool pj_magnitude_general(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    for (;;)
    {
        TRACE_PARSER(parser, p);
        if (p == p_end)
        {
            pj_part_tok(parser, token, S_MAGN_G, p);
            return false;
        }

        switch (*p)
        {
        case '0' ... '9':
            ++p;
            break;
        case '.':
            return pj_fraction_start(parser, token, ++p);
        case 'e': case 'E':
            return pj_exponent_start(parser, token, ++p);
        case '-': case '+':
            pj_err_tok(parser, token);
            return false;
        default:
            return pj_number_end(parser, token, S_MAGN_G, p);
        }
    }
}

static bool pj_magnitude_zero(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    if (p == p_end)
    {
        pj_part_tok(parser, token, S_MAGN_Z, p);
        return false;
    }
    switch (*p)
    {
    case '-': case '+':
    case '0':
        pj_err_tok(parser, token);
        return false;
    case '1' ... '9':
        return pj_magnitude_general(parser, token, ++p);
    case '.':
        return pj_fraction_start(parser, token, ++p);
    case 'e': case 'E':
        return pj_exponent_start(parser, token, ++p);
    default:
        return pj_number_end(parser, token, S_MAGN_Z, p);
    }
}


static bool pj_magnitude_start(pj_parser_ref parser, pj_token *token, const char *p)
{
    TRACE_FUNC();
    const char * const p_end = parser->chunk_end;

    if (p == p_end)
    {
        pj_part_tok(parser, token, S_MAGN, p);
        return false;
    }
    switch (*p)
    {
    case '0':
        return pj_magnitude_zero(parser, token, ++p);
    case '1' ... '9':
        return pj_magnitude_general(parser, token, ++p);
    case '-': case '+': case '.': case 'e': case 'E':
        pj_err_tok(parser, token);
        return false;
    default:
        pj_err_tok(parser, token);
        return false;
    }
}

static bool pj_number(pj_parser_ref parser, pj_token *token, state s, const char *p)
{
    TRACE_FUNC();
    assert( p != parser->chunk_end );

    switch (s)
    {
    case S_MAGN: return pj_magnitude_start(parser, token, p);
    case S_MAGN_Z: return pj_magnitude_zero(parser, token, p);
    case S_MAGN_G: return pj_magnitude_general(parser, token, p);
    case S_FRAC: return pj_fraction_start(parser, token, p);
    case S_FRAC_NUM: return pj_fraction_number(parser, token, p);
    case S_EXP: return pj_exponent_start(parser, token, p);
    case S_EXP_SIGN: return pj_exponent_sign(parser, token, p);
    case S_EXP_NUM: return pj_exponent_number(parser, token, p);

    case S_NUM:
        switch (*p)
        {
        case '-':
            return pj_magnitude_start(parser, token, ++p);
        case '0':
            return pj_magnitude_zero(parser, token, ++p);
        case '1' ... '9':
            return pj_magnitude_general(parser, token, ++p);
        default:
            pj_err_tok(parser, token);
            return false;
        }

    default:
        assert("!unreachable");
        pj_err_tok(parser, token);
        return false;
    }
    /* unreachable */
}

#endif
