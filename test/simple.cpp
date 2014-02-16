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

    pj_feed_end(&parser);

    pj_poll(&parser, tokens.data(), tokens.size());
    EXPECT_EQ( PJ_ERR, tokens[0].token_type );
}
