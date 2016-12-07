//  :copyright: (c) Copyright 2001-2004 Unicode, Inc.
//  :copyright: (c) 2016 The Regents of the University of California.
//  :license: Unicode, see LICENSE.md for more details.
/**
 *  \addtogroup Utf++
 *  \brief Convert Unicode code points between encodings.
 *
 *  Converts from any Unicode encoding (UTF8, UTF16, UTF32) in the native
 *  endian format to another format.
 *
 *  \warning The low level functions will exit early if they could overrun
 *  their destination buffer.
 */

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>


namespace utf
{
namespace detail
{
// MACROS
// ------

#define UTF32(x) static_cast<uint32_t>(x);
#define UTF16(x) static_cast<uint16_t>(x);
#define UTF8(x) static_cast<uint8_t>(x);

// CONSTANTS
// ---------

extern const std::array<uint8_t, 7> FIRST_BYTE_MARK;
extern const std::array<uint8_t, 256> UTF8_BYTES;
extern const std::array<uint32_t, 6> UTF8_OFFSETS;

// EXCEPTIONS
// ----------

/** \brief Illegal character detected.
 */
struct IllegalCharacterError: public std::exception
{
    virtual const char * what() const throw()
    {
        return "Illegal character found during conversion.\n";
    }
};


/** \brief Not enough space in output buffer.
 */
struct BufferRangeError: public std::exception
{
    virtual const char * what() const throw()
    {
        return "Cannot add characters too buffer, output is too small.\n";
    }
};


/** \brief Replace illegal Unicode character if checkStrict is off.
 */
uint32_t checkStrict(bool strict);


// CHARACTERS
// ----------

// FROM UTF32

/** \brief Convert UTF-32 character to UTF-16.
 */
template <typename Iter16>
void utf32To16(uint32_t c,
    Iter16 &begin,
    Iter16 &end,
    bool strict)
{
    // limits
    constexpr uint32_t maxUtf32 = 0x0010FFFF;
    constexpr uint32_t highBegin = 0xD800;
    constexpr uint32_t lowBegin = 0xDC00;
    constexpr uint32_t maxbmp = 0x0000FFFF;
    constexpr int shift = 10;
    constexpr uint32_t base = 0x0010000UL;
    constexpr uint32_t mask = 0x3FFUL;

    // variables
    if (c <= maxbmp) {
        if (c >= highBegin && c <= lowBegin) {
            *begin++ = checkStrict(strict);
        } else {
            *begin++ = UTF16(c);
        }
    } else if (c > maxUtf32) {
        *begin++ = checkStrict(strict);
    } else {
        if (begin + 1 > end) {
            throw BufferRangeError();
        }

        c -= base;
        *begin++ = UTF16((c >> shift) + highBegin);
        *begin++ = UTF16((c & mask) + lowBegin);
    }
}


/** \brief Convert UTF-32 character to UTF-8.
 */
template <typename Iter8>
void utf32To8(uint32_t c,
    Iter8 &begin,
    Iter8 end,
    bool strict)
{
    // limits
    constexpr uint32_t maxUtf32 = 0x0010FFFF;
    constexpr uint32_t bytemark = 0x80;
    constexpr uint32_t bytemask = 0xBF;

    // calculate bytes to write
    short bytes;
    if (c < 0x80) {
        bytes = 1;
    } else if (c < 0x800) {
        bytes = 2;
    } else if (c < 0x10000) {
        bytes = 3;
    } else if (c <= maxUtf32) {
        bytes = 4;
    } else {
        bytes = 3;
        c = checkStrict(strict);
    }

    // check range
    if (begin + bytes > end) {
        throw BufferRangeError();
    }

    // write to buffer
    begin += bytes;
    switch (bytes) {
        case 4:
            *--begin = UTF8((c | bytemark) & bytemask);
            c >>= 6;
        case 3:
            *--begin = UTF8((c | bytemark) & bytemask);
            c >>= 6;
        case 2:
            *--begin = UTF8((c | bytemark) & bytemask);
            c >>= 6;
        case 1:
            *--begin = UTF8(c | FIRST_BYTE_MARK[bytes]);
    }
    begin += bytes;
}


// FROM UTF16

/** \brief Convert UTF16 characters to UTF32.
 */
template <typename Iter16>
uint32_t utf16To32(Iter16 &begin,
    Iter16 end,
    bool strict)
{
    // limits
    constexpr uint32_t highBegin = 0xD800;
    constexpr uint32_t highEnd = 0xDBFF;
    constexpr uint32_t lowBegin = 0xDC00;
    constexpr uint32_t lowEnd = 0xDFFF;
    constexpr int shift = 10;
    constexpr uint32_t base = 0x0010000UL;

    const uint32_t c1 = *begin++;
    if (c1 >= highBegin && c1 <= highEnd) {
        // surrogate pairs
        const uint32_t c2 = *begin++;
        if (c2 >= lowBegin && c2 <= lowEnd) {
            return ((c1 - highBegin) << shift) + (c2 - lowBegin) + base;
        } else {
            return checkStrict(strict);
        }
    } else if (c1 >= lowBegin && c1 <= lowEnd) {
        return checkStrict(strict);
    } else {
        return c1;
    }
}


/** \brief Convert UTF-8 character to UTF-32.
 */
template <typename Iter8>
uint32_t utf8To32(Iter8 &begin,
    Iter8 end,
    bool strict)
{
    uint32_t c = 0;
    uint8_t bytes = UTF8_BYTES[*begin];

    // check range
    if (begin + bytes >= end) {
        // sourc ebuffer, check whether or not we have space to replace
        throw IllegalCharacterError();
    }

    // get our UTF-32 character
    switch (bytes) {
        case 5:
            c = checkStrict(strict);
            c <<= 6;
        case 4:
            c = checkStrict(strict);
            c <<= 6;
        case 3:
            c += *begin++;
            c <<= 6;
        case 2:
            c += *begin++;
            c <<= 6;
        case 1:
            c += *begin++;
            c <<= 6;
        case 0:
            c += *begin++;
    }
    c -= UTF8_OFFSETS[bytes];

    return c;
}


// ARRAYS
// ------


/** \brief Convert UTF32 to UTF16.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter32, typename Iter16>
size_t utf32To16(Iter32 srcBegin,
    Iter32 srcEnd,
    Iter16 dstBegin,
    Iter16 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        utf32To16(*src++, dst, dstEnd, strict);
    }

    return dst - dstBegin;
}


/** \brief Convert UTF16 to UTF32.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter16, typename Iter32>
size_t utf16To32(Iter16 srcBegin,
    Iter16 srcEnd,
    Iter32 dstBegin,
    Iter32 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        *dst++ = utf16To32(src, srcEnd, strict);
    }

    return dst - dstBegin;
}


/** \brief Convert UTF16 to UTF8.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter16, typename Iter8>
size_t utf16To8(Iter16 srcBegin,
    Iter16 srcEnd,
    Iter8 dstBegin,
    Iter8 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        utf32To8(utf16To32(src, srcEnd, strict), dst, dstEnd, strict);
    }

    return dst - dstBegin;
}


/** \brief Convert UTF8 to UTF16.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter8, typename Iter16>
size_t utf8To16(Iter8 srcBegin,
    Iter8 srcEnd,
    Iter16 dstBegin,
    Iter16 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        utf32To16(utf8To32(src, srcEnd, strict), dst, dstEnd, strict);
    }

    return dst - dstBegin;
}


/** \brief Convert UTF32 to UTF8.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter32, typename Iter8>
size_t utf32To8(Iter32 srcBegin,
    Iter32 srcEnd,
    Iter8 dstBegin,
    Iter8 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        utf32To8(*src++, dst, dstEnd, strict);
    }

    return dst - dstBegin;
}


/** \brief Convert UTF8 to UTF32.
 *
 *  \return     Number of bytes written to dst.
 */
template <typename Iter8, typename Iter32>
size_t utf8To32(Iter8 srcBegin,
    Iter8 srcEnd,
    Iter32 dstBegin,
    Iter32 dstEnd,
    bool strict = true)
{
    auto src = srcBegin;
    auto dst = dstBegin;
    while (src < srcEnd && dst < dstEnd) {
        *dst++ = utf8To32(src, srcEnd, strict);
    }

    return dst - dstBegin;
}

}   /* detail */

// FUNCTIONS
// ---------

/** \brief STL wrapper for utf8To16.
 */
std::string utf8To16(const std::string &string);

/** \brief STL wrapper for utf8To32.
 */
std::string utf8To32(const std::string &string);

/** \brief STL wrapper for utf16To8.
 */
std::string utf16To8(const std::string &string);

/** \brief STL wrapper for utf16To32.
 */
std::string utf16To32(const std::string &string);

/** \brief STL wrapper for utf32To8.
 */
std::string utf32To8(const std::string &string);

/** \brief STL wrapper for utf32To16.
 */
std::string utf32To16(const std::string &string);

}   /* utf */


// API
// ---

#define UTF8_TO_UTF16(x) utf::utf8To16(x)
#define UTF8_TO_UTF32(x) utf::utf8To32(x)
#define UTF16_TO_UTF8(x) utf::utf16To8(x)
#define UTF16_TO_UTF32(x) utf::utf16To32(x)
#define UTF32_TO_UTF8(x) utf::utf32To8(x)
#define UTF32_TO_UTF16(x) utf::utf32To16(x)
