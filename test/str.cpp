#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(str, empty)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\"\"");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( 0, tokens[0].len );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, non_empty)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "\"abcd\"";
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, chunked)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"ab"); /* supplementary buffer used */

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "cd\"");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, chunked_realloc)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\"ab"); /* supplementary buffer used */

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_OVERFLOW, tokens[0].token_type );

    char buf[256];
    pj_realloc(&parser, buf, sizeof(buf));

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "cd\"");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}
