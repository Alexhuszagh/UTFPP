//  :copyright: (c) Copyright 2001-2004 Unicode, Inc.
//  :copyright: (c) 2016 The Regents of the University of California.
//  :license: Unicode, see LICENSE.md for more details.
/**
 *  \addtogroup Utf++
 *  \brief Test UTF conversions.
 */

#include "utf.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>



/** Test conversion of UTF-8 to and from all UTF encodings.
 */
void testBytes(const std::string &utf8)
{
    for (int i = 0; i < 10000; i++) {
        // utf8 <==> utf32
        auto utf32 = UTF8_TO_UTF32(utf8);
        assert(UTF32_TO_UTF8(utf32) == utf8);

        // utf16 <==> utf32
        auto utf16 = UTF32_TO_UTF16(utf32);
        assert(UTF16_TO_UTF32(utf16) == utf32);

        // utf16 <==> utf8
        assert(UTF16_TO_UTF8(utf16) == utf8);
        assert(UTF8_TO_UTF16(utf8) == utf16);
    }
}


/** Read bytes from file.
 */
std::string readFile(const std::string &path)
{
    std::ifstream stream(path);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}


int main(int argc, char *argv[])
{
    testBytes(readFile("characters.utf8"));
    testBytes(readFile("emoji.utf8"));

    return 0;
}
