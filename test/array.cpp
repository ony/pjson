#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(array, empty_arr)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "[]");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_ARR_E, tokens[1].token_type );
    ASSERT_EQ( PJ_STARVING, tokens[2].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(array, arr_bad)
{
    pj_parser parser;

    array<pj_token, 3> tokens;

    pj_init(&parser, 0, 0);
    pj_feed(&parser, "[,]");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    EXPECT_EQ( PJ_ERR, tokens[1].token_type );

    pj_init(&parser, 0, 0);
    pj_feed(&parser, "[2,]");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_NUM, tokens[1].token_type );
    EXPECT_EQ( PJ_ERR, tokens[2].token_type );

    pj_init(&parser, 0, 0);
    pj_feed(&parser, "[2,");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_NUM, tokens[1].token_type );
    ASSERT_EQ( PJ_STARVING, tokens[2].token_type );
    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type );
}

TEST(array, null_space_arr)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "null []");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_ERR, tokens[1].token_type );
}

TEST(array, arr_keywords)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "[null, true, false]");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_NULL, tokens[1].token_type );
    ASSERT_EQ( PJ_TOK_TRUE, tokens[2].token_type );

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_FALSE, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_ARR_E, tokens[1].token_type );
    ASSERT_EQ( PJ_STARVING, tokens[2].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}
