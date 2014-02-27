#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(number, multiple)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = " 41,43 "; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "41", string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_TOK_NUM, tokens[1].token_type );
    EXPECT_EQ( "43", string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );
}

TEST(number, integers)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "42 "; /* keep in memory (space to force token end) */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "42", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, 0, 0);
    sample = "-5 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "-5", string(tokens[0].str, tokens[0].len) );
}

TEST(number, reals)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "3.141592 "; /* keep in memory (space to force token end) */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "3.141592", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, 0, 0);
    sample = "-0.5 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "-0.5", string(tokens[0].str, tokens[0].len) );
}

TEST(number, eng_reals)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "0.3141592e1 "; /* keep in memory (space to force token end) */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "0.3141592e1", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, 0, 0);
    sample = "-0.3141592e1 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "-0.3141592e1", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, 0, 0);
    sample = "314.1592e-2 "; /* keep in memory (space to force token end) */
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "314.1592e-2", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, 0, 0);
    sample = "-314.1592e-2 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "-314.1592e-2", string(tokens[0].str, tokens[0].len) );
}

TEST(number, bad_numbers)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = " --5 "; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type ) << "--5 shouldn't be a valid number";

    pj_init(&parser, 0, 0);
    sample = " 00 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type ) << "00 shouldn't be a valid number";
}

TEST(number, bad_fractons)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = " .5 "; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type ) << ".5 shouldn't be a valid number";

    pj_init(&parser, 0, 0);
    sample = " 0.0.1 ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type ) << "0.0.1 shouldn't be a valid number";

    pj_init(&parser, 0, 0);
    sample = " 0e ";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type ) << "0e shouldn't be a valid number";
}

TEST(number, chunked)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "4");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "2 ");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "42", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, buf, sizeof(buf));
    pj_feed(&parser, "-");

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );
    pj_feed(&parser, "5 ");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "-5", string(tokens[0].str, tokens[0].len) );

    pj_init(&parser, buf, sizeof(buf));
    pj_feed(&parser, "3.");

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );
    pj_feed(&parser, "141592 ");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "3.141592", string(tokens[0].str, tokens[0].len) );
}
