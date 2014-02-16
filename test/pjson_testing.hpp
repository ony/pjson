#ifndef __pjson_hpp__
#define __pjson_hpp__

#include <string>

#include "pjson.h"

namespace {
    void pj_feed(pj_parser_ref parser, const std::string &s)
    { pj_feed(parser, s.data(), s.size()); }
}

#endif
