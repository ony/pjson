#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

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

TEST(simple, something_bad)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "something bad");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type );
}

TEST(simple, string_in_array)
{
    pj_parser parser;
    array<char, 256> buf;
    pj_init(&parser, buf.data(), buf.size());

    pj_feed(&parser, "[\"Hello World\"]");

    array<pj_token, 4> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "Hello World", std::string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[2].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[3].token_type );
    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, newline_string_in_array)
{
    pj_parser parser;
    array<char, 256> buf;
    pj_init(&parser, buf.data(), buf.size());

    pj_feed(&parser, "[\"Hello\\nWorld\"]");

    array<pj_token, 4> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "Hello\nWorld", std::string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[2].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[3].token_type );
    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, unicode_str_in_array)
{
    pj_parser parser;
    array<char, 256> buf;
    pj_init(&parser, buf.data(), buf.size());

    pj_feed(&parser, "[\"Hello\\u0000World\"]");

    array<pj_token, 4> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( std::string("Hello\000World", 11), std::string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[2].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[3].token_type );
    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, comments)
{
    pj_parser parser;
    array<char, 256> buf;
    pj_init(&parser, buf.data(), buf.size());

    pj_feed(&parser, "[// comment\n\"// Hello /* comment */\" /* comment */]");

    array<pj_token, 4> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( std::string("// Hello /* comment */"), std::string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[2].token_type );
    EXPECT_EQ( PJ_STARVING, tokens[3].token_type );
    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(simple, comments_chunked)
{
    pj_parser parser;
    array<char, 256> buf;
    pj_init(&parser, buf.data(), buf.size());

    pj_feed(&parser, "[/");

    array<pj_token, 4> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_STARVING, tokens[1].token_type );
    pj_feed(&parser, "/ 4,\n5,\n/");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "5", std::string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_STARVING, tokens[1].token_type );
    pj_feed(&parser, "* 6, *");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_STARVING, tokens[0].token_type );
    pj_feed(&parser, "/ 7 ");
    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_NUM, tokens[0].token_type );
    EXPECT_EQ( "7", std::string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}
