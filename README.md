# Libsass Image Size Plugin

Native libsass plugin adding functions to query image dimensions.

## Introduction

This plugin only supports PNG, JPG and GIF image formats. These are the common
image formats used on the web (beside SVG maybe). Those three image formats are
well defined and querying the dimensions is rather simple. Therefore we don't
need any 3rd party image libraries to get the job done.

## Building

You need to have [libsass][1] already [compiled][2] or [installed][3] as a
shared library (inclusive header files). It is then compiled via `cmake`. See
this example to compile it on windows via [MinGW][4] Compiler Suite:

```cmd
git clone https://github.com/sass/libsass.git
mingw32-make -C libsass BUILD=shared CC=gcc -j5
git clone https://github.com/mgreter/libsass-img-size.git
cd libsass-img-size && mkdir build && cd build
cmake -G "MinGW Makefiles" .. -DLIBSASS_DIR="..\..\libsass"
mingw32-make CC=gcc -j5 && dir img-size.dll
```

You may define `LIBSASS_INCLUDE_DIR` and `LIBSASS_LIBRARY_DIR` separately!

## API
The following functions are available when you import the img-size plugin.

- `image-size($url)` - Returns image dimensions as a list with spaces
- `image-width($url)` - Returns image width as number with `px` as unit
- `image-height($url)` - Returns image height as number with `px` as unit

The url is resolved via `sass_compiler_find_file`, which will in turn look for
the given file in all include directories. The first file encountered will be
used. If no file can be found, a error is raised.

## Copyright

© 2017 [Marcel Greter][5]

[1]: https://github.com/sass/libsass
[2]: https://github.com/sass/libsass/wiki/Building-Libsass
[3]: http://libsass.ocbnet.ch/installer/
[4]: http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/
[5]: https://github.com/mgreter

