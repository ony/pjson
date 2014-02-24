#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(map, null_space_map)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "null {}");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_NULL, tokens[0].token_type );
    EXPECT_EQ( PJ_ERR, tokens[1].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type );
}

TEST(map, empty_map)
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

TEST(map, maps_array)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "[{},{}]");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_MAP, tokens[1].token_type );
    EXPECT_EQ( PJ_TOK_MAP_E, tokens[2].token_type );

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_MAP, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_MAP_E, tokens[1].token_type );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[2].token_type );
}

TEST(map, sample)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "{\"alpha\": true, \"beta\": false}");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_MAP, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "alpha", string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_KEY, tokens[2].token_type );

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_TRUE, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "beta", string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_KEY, tokens[2].token_type );

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_FALSE, tokens[0].token_type );
    EXPECT_EQ( PJ_TOK_MAP_E, tokens[1].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );
}
