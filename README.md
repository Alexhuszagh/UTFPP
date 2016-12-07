# UTFPP

A light-weight, fast, dependency-free, C++ library for Unicode encoding conversions. It is free for commercial and open-source projects.

**Table of Contents**

- [Motivation](#motivation)
- [Performance](#performance)
- [Testing](#testing)
- [Contributors](#contributors)
- [License](#license)

## Motivation

Unicode character set conversions in C++ are a pain, with bugs in GCC's STL [implementations](https://stackoverflow.com/questions/39441805/truncated-read-with-utf-16-encoded-text-in-c), forced wide-string/narrow-string conversions, or the use of large (ICU) or copyleft libraries (libiconv). 

Utf++ also uses template-driven libraries to add compatibility with the C++ STL, as well as minimize the total source code. The entire library is under 600 lines of code.

## Performance

The performance of ICU, UTFPP, and libiconv were compared for the conversion of 9000 code points covering most languages with 10,000 iterations for all pairwise conversions between UTF8, UTF16, and UTF32.

| Library   | Time (seconds) |
|:---------:|:--------------:|
| ICU (C++) | 2.120          |
| libiconv  | 2.795          |
| UTFPP     | 1.201          |

## Testing

Since UTF8 and UTF16 are variable width encodings, bugs in iOS, Qt3, requiring test with all subsets of Unicode code points. UTFPP assumes variable width encoding for UTF8 and UTF16, and allocates memory for the least efficient conversion by default. 

To ensure high code points (wide characters) do not cause buffer overflows, UTFPP is tested with an 2500 emojis, where more than 85% of the UTF8 and UTF16 representations require 4 bytes per code point.

## Building

Simply clone and build.

```
git clone https://github.com/Alexhuszagh/utf++.git
make
./test
```

## Contributors

- Alex Huszagh

## License

Unicode Disclaimer, see [license](LICENSE.md).
