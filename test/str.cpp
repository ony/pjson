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
    EXPECT_EQ( PJ_END, tokens[0].token_type );
}

TEST(str, control)
{
    pj_parser parser;

    for (char control : {'\b', '\f', '\t', '\n', '\r'})
    {
        std::string input = std::string(1, control);
        input = "\"" + input + "\"";
        pj_init(&parser, NULL, 0);
        pj_feed(&parser, input);

        array<pj_token, 1> tokens;

        pj_poll(&parser, tokens.data(), tokens.size());
        EXPECT_EQ( PJ_ERR, tokens[0].token_type )
            << "Control character 0x" << std::hex << int(control) << " should be escaped";
    }
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
    EXPECT_EQ( 3, tokens[0].len );

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

TEST(str, strings_with_escapes)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "[\"2\\\"\",\"3\\\"\"]");

    array<pj_token, 5> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_TOK_STR, tokens[1].token_type );
    EXPECT_EQ( "2\"", string(tokens[1].str, tokens[1].len) );
    ASSERT_EQ( PJ_TOK_STR, tokens[2].token_type );
    EXPECT_EQ( "3\"", string(tokens[2].str, tokens[2].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[3].token_type );
    ASSERT_EQ( PJ_STARVING, tokens[4].token_type );
}

TEST(str, utf8_direct)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = u8"\"∆\""; // delta in utf-8
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( u8"∆", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, platform_utf8)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    char buf[MB_CUR_MAX];
    mbstate_t mbs { 0 };

    wchar_t c = u'∆';
    string c_mb = u8"∆";
    size_t n = wcrtomb(buf, c, &mbs);
    ASSERT_NE( (size_t)-1, n );
    EXPECT_EQ( c_mb.size(), n );
    EXPECT_EQ( c_mb, string(buf, n) );
}

TEST(str, platform_utf8_surrogate)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    char buf[MB_CUR_MAX];
    mbstate_t mbs { 0 };

    wchar_t c = L'𝄞';
    string c_mb = u8"𝄞";
    size_t n = wcrtomb(buf, c, &mbs);
    ASSERT_NE( (size_t)-1, n );
    EXPECT_EQ( c_mb.size(), n );
    EXPECT_EQ( c_mb, string(buf, n) );
    EXPECT_EQ( 0x1d11e, c );
}

TEST(str, utf8_escape_ascii)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"\\u0024\""; // dollar sign ($)
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( u8"$", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, utf8_escape_bmp)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"\\u2206\""; // basic multilingual plane
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( u8"∆", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, DISABLED_utf8_escape_bmp_chunks)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"\\u");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "000a\"");

    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "\n", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, escape_overflow)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, 3); /* pretend we bufsize is 3 */

    pj_feed(&parser, "[\"\\nabc\",\"\\ndef\\n\",\"\\t\"]");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_TOK_ARR, tokens[0].token_type );
    ASSERT_EQ( PJ_OVERFLOW, tokens[1].token_type );
    pj_realloc(&parser, buf, 5); /* fits "\nabc" and "\n" but not enough for additional "def" */

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "\nabc", string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_OVERFLOW, tokens[1].token_type );
    pj_poll(&parser, tokens.data(), tokens.size()); /* re-request */
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "\ndef\n", string(tokens[0].str, tokens[0].len) );
    ASSERT_EQ( PJ_OVERFLOW, tokens[1].token_type );
    pj_poll(&parser, tokens.data(), tokens.size()); /* re-request */
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "\t", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_TOK_ARR_E, tokens[1].token_type );
}

TEST(str, DISABLED_escape_chunks)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"\\");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed(&parser, "n\"");

    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( "\n", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, utf8_surrogate_pair)
{
    setlocale(LC_CTYPE, "en_US.utf8");
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    std::string sample = "\"\\ud834\\udd1e\""; // surrogate pair for G clef
    pj_feed(&parser, sample);

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_TOK_STR, tokens[0].token_type );
    EXPECT_EQ( u8"𝄞", string(tokens[0].str, tokens[0].len) );
    EXPECT_EQ( PJ_STARVING, tokens[1].token_type );
}

TEST(str, incomplete_final) /* require proper pj_flush_tok */
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));

    pj_feed(&parser, "\"abcd");

    array<pj_token, 3> tokens;

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_STARVING, tokens[0].token_type );

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    ASSERT_EQ( PJ_ERR, tokens[0].token_type );
}
