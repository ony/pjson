#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(keywords, null)
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

TEST(keywords, null_with_spaces)
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

TEST(keywords, null_null)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "nullnull");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_ERR, tokens[1].token_type );
}

TEST(keywords, null_space_null)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "null null");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_ERR, tokens[1].token_type );
}

TEST(keywords, null_comma_null)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "null , null");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_NULL, tokens[1].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(keywords, bool_true)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "true");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_TRUE, tokens[0].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(keywords, bool_false)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "false");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_FALSE, tokens[0].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}
