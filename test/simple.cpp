#include <array>

#include <gtest/gtest.h>

#include "pjson.h"

using namespace std;

namespace {
    void pj_feed(pj_parser_ref parser, const string &s)
    { pj_feed(parser, s.data(), s.size()); }
}


TEST(simple, empty)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, space)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\r\n\t  ");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, space_chunks)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\r\n\t  ");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "\r\n\t  ");
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, null)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "null");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, null_with_spaces)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\n\t  null\n");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, DISABLED_empty_map)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "{}");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_MAP, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_MAP_E, tokens[1].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

