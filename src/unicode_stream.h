#pragma once
#include <fcntl.h>

#include <codecvt>
#include <iostream>
#include <locale>
#include <memory>
#include <string>

#ifdef __MINGW32__
#include <ext/stdio_filebuf.h>
using ifstream = std::istream;
using ofstream = std::ostream;
#else  // __MINGW32__
using ifstream = std::ifstream;
using ofstream = std::ofstream;
#endif // __MINGW32__

template <typename T>
struct UnicodeStream {
#ifdef __MINGW32__
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> filebuf;
#endif
    T stream;
    bool is_open;
};

template <typename T>
static UnicodeStream<T> unicode_fstream(std::string path, std::ios_base::openmode mode) {
#ifdef WIN32
#ifdef __MINGW32__
    // MINGW32
    int flags = 0;
    if (mode & std::ios::out) {
        flags |= _O_WRONLY | _O_CREAT;
    }
    if (mode & std::ios::in) {
        flags |= _O_RDONLY;
    }
    if (mode & std::ios::binary) {
        flags |= _O_BINARY;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::u16string tmp = convert.from_bytes(path);
    std::wstring wpath(tmp.begin(), tmp.end());

    int fd = _wopen(wpath.c_str(), flags);
    __gnu_cxx::stdio_filebuf<char> *buffer = new __gnu_cxx::stdio_filebuf<char>(fd, mode, 1);
    return UnicodeStream<T>{
        std::unique_ptr<__gnu_cxx::stdio_filebuf<char>>(buffer),
        T(buffer),
        (fd >= 0),
    };
#else  // __MINGW32__
    // MSVC
    auto stream = T(path.widen());
    bool is_open = stream.is_open();
    return UnicodeStream<T>{std::move(stream), is_open};
#endif // __MINGW32__
#else  // WIN32
    // not windows
    auto stream = T(path);
    bool is_open = stream.is_open();
    return UnicodeStream<T>{std::move(stream), is_open};
#endif // WIN32
}

static auto unicode_ifstream(std::string path, std::ios_base::openmode mode = std::ios::in) {
    return unicode_fstream<ifstream>(path, mode);
}

static auto unicode_ofstream(std::string path, std::ios_base::openmode mode = std::ios::out) {
    return unicode_fstream<ofstream>(path, mode);
}
