#include <gtest/gtest.h>

#include "pjson.h"

TEST(simple, case0)
{
    pj_parser parser;
    char buf[256];
    pj_init(&parser, buf, sizeof(buf));
    pj_feed(&parser, "{}", 2);
}

