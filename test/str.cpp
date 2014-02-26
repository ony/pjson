#include <array>

#include <gtest/gtest.h>

#include "pjson_testing.hpp"

using namespace std;

TEST(str, empty)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\"\",");

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

    string sample = "\"abcd\","; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, multiple)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "\"abcd\", \"efgh\""; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "efgh", string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );
}

TEST(str, non_empty_final) /* require proper pj_flush_tok */
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    string sample = "\"abcd\""; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed_end(&parser);
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_END, tokens[1].token_type );
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

    pj_feed(&parser, "cd\",");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, chunk_without_buf)
{
    pj_parser parser;
    pj_init(&parser, 0, 0);

    pj_feed(&parser, "\"");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    string sample = "abcd\"";
    pj_feed(&parser, sample);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, multiple_chunked)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"ab");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "cd\",\"ef");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );

    pj_feed(&parser, "gh\"");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "efgh", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, multiple_chunks)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"ab");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "cdef");

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "gh\"");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcdefgh", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, multiple_per_chunk)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"ab");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "cd\",\"efgh\"");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "efgh", string(tokens[1].str, tokens[1].len) );
    EXPECT_EQ( PJ_STARVING, tokens[2].token_type );
}

TEST(str, chunked_key)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"abcd\""; /* keep in memory */
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );

    pj_feed(&parser, ":");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_KEY, tokens[0].token_type );
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

    pj_feed(&parser, "cd\",");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, chunked_overflow)
{
    pj_parser parser;

    char buf[256];
    pj_init(&parser, buf, 2); /* lie that we have only 2 bytes buffer */

    pj_feed(&parser, "\"abc"); /* supplementary buffer used */

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_OVERFLOW, tokens[0].token_type );

    pj_realloc(&parser, buf, sizeof(buf));
    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "d\",");

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "abcd", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, guarded_chars)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"a\\\"b\\/c\\\\d\"";
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "a\"b/c\\d", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, special_chars)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"a\\tb\\nc\\rd\\fe\\bf\"";
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "a\tb\nc\rd\fe\bf", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}
