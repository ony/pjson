#include <array>
#include <iostream>
#include <fstream>

#include <gtest/gtest.h>

#ifdef HAVE_YAJL
#include <yajl/yajl_parse.h>
#endif

#include "pjson_testing.hpp"

using namespace std;

const size_t chunk_size = 4096;
const size_t repeats = 10000;

TEST(performance, measure_locale_count)
{
    ifstream ifs(JSON_BIG_SAMPLE_FILE);
    for (size_t n = 0; n < repeats; ++n)
    {
        ifs.seekg(0, ios_base::beg);

        char chunk[chunk_size];
        bool eof = false;
        size_t specials = 0;
        for (;!eof;)
        {
            streamsize sz = ifs.readsome(chunk, sizeof(chunk));
            if (sz == 0)
            {
                eof = true;
            }
            for (size_t i = 0; i < sz; ++i)
            {
                switch (chunk[i])
                {
                case '{': case '}':
                case '[': case ']':
                case ',': case ':':
                    ++specials;
                    break;
                default: ;
                    // nothing
                }
            }
        }
    }
}


TEST(performance, measure_locale_pjson)
{
    ifstream ifs(JSON_BIG_SAMPLE_FILE);
    for (size_t n = 0; n < repeats; ++n)
    {
        ifs.seekg(0, ios_base::beg);

        pj_parser parser;
        char buf[256];
        pj_init(&parser, buf, sizeof(buf));

        char chunk[chunk_size];
        bool eof = false;
        for (;!eof;)
        {
            streamsize sz = ifs.readsome(chunk, sizeof(chunk));
            if (sz == 0)
            {
                eof = true;
                pj_feed_end(&parser);
            }
            else
            {
                pj_feed(&parser, chunk, sz);
            }
            for (bool starving = false, end = false; !starving && !end;)
            {
                array<pj_token, 128> tokens;
                pj_poll(&parser, tokens.data(), tokens.size());
                for (size_t i = 0; i < tokens.size(); ++i)
                {
                    if (tokens[i].token_type == PJ_STARVING)
                    {
                        starving = true;
                        break;
                    }
                    else if (tokens[i].token_type == PJ_END)
                    {
                        end = true;
                        break;
                    }
                    else if (tokens[i].token_type == PJ_ERR)
                    {
                        FAIL() << "Error?";
                    }
                    else if (tokens[i].token_type == PJ_OVERFLOW)
                    {
                        FAIL() << "Shouldn't have overflow?";
                    }
                }
            }
        }
    }
}

#ifdef HAVE_YAJL
TEST(performance, measure_locale_yajl_dummy)
{
    ifstream ifs(JSON_BIG_SAMPLE_FILE);
    for (size_t n = 0; n < repeats; ++n)
    {
        ifs.seekg(0, ios_base::beg);

        yajl_handle hand = yajl_alloc(0, 0, 0);

        char chunk[chunk_size];
        bool eof = false;
        for (;!eof;)
        {
            streamsize sz = ifs.readsome(chunk, sizeof(chunk));
            if (sz == 0)
            {
                eof = true;
                ASSERT_EQ( yajl_status_ok, yajl_complete_parse(hand) );
            }
            else
            {
                ASSERT_EQ( yajl_status_ok, yajl_parse(hand, reinterpret_cast<unsigned char *>(chunk), sz) );
            }
        }
        yajl_free(hand);
    }
}

namespace {
    int cb_bool(void *, int) { return true; }
    int cb_str(void *, const char *, size_t ) { return true; }
    int cb_ustr(void *, const unsigned char *, size_t ) { return true; }
    int cb_kwd(void *) { return true; }

}

TEST(performance, measure_locale_yajl)
{
    yajl_callbacks dummy_callbacks = {
        .yajl_null = cb_kwd,
        .yajl_boolean = cb_bool,
        /* hmm.. non-trivial?...
        .yajl_start_map = cb_kwd,
        .yajl_end_map = cb_kwd,
        .yajl_start_array = cb_kwd,
        .yajl_end_array = cb_kwd,
        .yajl_number = cb_str,
        .yajl_map_key = cb_ustr,
        .yajl_string = cb_ustr,
        */
    };
    dummy_callbacks.yajl_start_map = cb_kwd,
    dummy_callbacks.yajl_end_map = cb_kwd,
    dummy_callbacks.yajl_start_array = cb_kwd,
    dummy_callbacks.yajl_end_array = cb_kwd,
    dummy_callbacks.yajl_number = cb_str,
    dummy_callbacks.yajl_map_key = cb_ustr,
    dummy_callbacks.yajl_string = cb_ustr;

    ifstream ifs(JSON_BIG_SAMPLE_FILE);
    for (size_t n = 0; n < repeats; ++n)
    {
        ifs.seekg(0, ios_base::beg);

        yajl_handle hand = yajl_alloc(&dummy_callbacks, 0, 0);

        char chunk[chunk_size];
        bool eof = false;
        for (;!eof;)
        {
            streamsize sz = ifs.readsome(chunk, sizeof(chunk));
            if (sz == 0)
            {
                eof = true;
                ASSERT_EQ( yajl_status_ok, yajl_complete_parse(hand) );
            }
            else
            {
                ASSERT_EQ( yajl_status_ok, yajl_parse(hand, reinterpret_cast<unsigned char *>(chunk), sz) );
            }
        }
        yajl_free(hand);
    }
}
#endif
