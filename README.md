# The _brace_ C++ Library

_brace_ is a general-purpose C++ library, made up of classes and functions that I have found useful. It is very much a work in progress and I will be adding to the library from time to time. My hope is that you will find it useful as well.

The core library is header-only and I expect the core to remain header-only.

## Modern C++
_brace_ is written using *Modern C++*, which is to say C++17. It's 2023 and most compiler vendors by now have excellent support for C++17. _brace_ is a new library and I have no intention of modifying _brace_ to support older versions of the language.

## Building, Installing, and Using _brace_
Currently there is nothing to build, as _brace_ is header-only. You only need to copy the `include/brace` directory to somewhere your compiler can find it and `#include` the appropriate header files in your project's source files.

The classes and function in the _brace_ library are contained within the `brace` namespace, so you will need to preface usage with the namespace name, as in
```cpp
brace::Base64Url encoder;
```

### Naming Conventions
Classes are named using Pascal casing, as in `MyClass`. Function names, both free functions and member functions, are named using snake casing, as in `my_function`.

## What's in _brace_?
_brace_ is composed of a number of classes and functions implementing things such as:
 * [Bit twiddling](#bit-twiddling)
 * [Byte order (endianness)](#byte-order)
 * [A FILE abstraction](#file-abstraction)
 * [Various hash algorithms](#hash-algorithms)
 * [Base 32/64 encoding](#base-3264-encoding)
 * [Binary streams](#binary-streams)

## Bit Twiddling
_brace_ provides functions for setting, clearing, flipping, and testing individual bits within values, as well as rotating bits left and right. Include the file `brace/bits.h` to access these functions.

## Byte Order
_brace_ provides functions for swapping bytes for various endianness, as well as `enum class Endian`. Include the file `brace/byteorder.h` to access these functions.

## FILE Abstraction
_brace_ provides a wrapper class around the `FILE` structure used in `C` file I/O. Include the file `brace/file.h` to use the `File` class.

## Hash Algorithms
_brace_ provides classes for several hashing algorithms.
### MD5
The MD5 hash algorithm produces a 128-bit hash. To use the `MD5` class, include `brace/md5.h`.
### SHA-1
The SHA-1 hash algorithm produces a 160-bit hash. The `SHA1` class is defined in the header `brace/sha1.h`.
### SHA-224/SHA-256
The SHA-224 and SHA-256 hash algorithms produce 224-bit and 256-bit hashes respectively. The `SHA224` and `SHA256` classes are defined in `brace/sha2.h`.
### SHA-384/SHA-512
The SHA-384 and SHA-512 hash algorithms produce 384-bit and 512-bit hashes respectively. The `SHA384` and `SHA512` classes are defined in `brace/sha2.h`.

## Base 32/64 Encoding
_brace_ provides classes for Base32, Base32-Hex, Base64, and Base64-URL encoding and decoding as described in RFC-4648. The `Base32` and `Base32Hex` classes are defined in the header `brace/base32.h`. The `Base64` and `Base64Url` classes are defined in `brace/base64.h`.

## Binary Streams
_brace_ offers classes for handling binary data streams. These classes function similarly to the standard stream classes, but operate on _binary_ data rather than formatted data. Overloads of operators `>>` and `<<` are provided for extracting and inserting data of fundamental types from and into binary streams. The classes understand endianness and can byte-swap data as needed.

Classes are provided for operating on binary data in files and in fixed-length arrays.
### Binary file processing
The header `brace/binfstream.h` defines the following classes for operating on binary data in files:
* `BinIFStream` for reading binary data from a file
* `BinOFStream` for writing binary data to a file.
* `BinFStream` for both reading and writing binary data from and to a file.
### Binary memory processing
The header `brace/binastream.h` defines these classes for operating on binary data in fixed-length arrays
* `BinIArrayStream` for reading binary data from a fixed-length array
* `BinOArrayStream` for writing binary data to a fixed-length array
* `BinArrayStream` for reading a writing binary data from and to a fixed-length array.

