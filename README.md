# DataLiteC

**DataLiteC** is a Swift package that provides low-level access to an embedded, fully-encrypted SQLite engine. It includes [SQLCipher](https://github.com/sqlcipher/sqlcipher) and [LibTomCrypt](https://github.com/libtom/libtomcrypt) as source dependencies, with no external installation required.

## Overview

**DataLiteC** provides direct access to the C API of SQLCipher, a fully encrypted drop-in replacement for SQLite. It is designed for Swift projects that require precise, low-level control over encrypted database operations without relying on system-installed libraries.

The package statically includes both SQLCipher and LibTomCrypt as source code, making it fully self-contained and portable across **macOS**, **iOS**, and **Linux** platforms.

DataLiteC does not introduce any abstractions or Swift-specific layers — it simply exposes the raw C interface, allowing complete control and maximum compatibility with the underlying engine.

## Installation

To add **DataLiteC** to your project, include it in the `dependencies` section of your `Package.swift` file:

```swift
// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "YourProject",
    dependencies: [
        .package(url: "https://github.com/angd-dev/data-lite-c.git", from: "2.0.0")
    ],
    targets: [
        .target(name: "YourTarget", dependencies: [
            .product(name: "DataLiteC", package: "data-lite-c")
        ])
    ]
)
```

No additional setup or system packages are required — all dependencies are bundled and built automatically.

## Usage

Once the package is added, you can import `DataLiteC` and use the raw SQLCipher C API directly in Swift:

```swift
import DataLiteC

var db: OpaquePointer?
if sqlite3_open(":memory:", &db) == SQLITE_OK {
    print("Successfully opened connection to database.")
    sqlite3_close(db)
} else {
    print("Unable to open database.")
}
```

To use encryption with SQLCipher, set the encryption key immediately after opening the database:

```swift
if sqlite3_open("secure.db", &db) == SQLITE_OK {
    let key = "your-secret-passphrase"
    sqlite3_key(db, key, Int32(key.utf8.count))

    // It's good practice to verify key correctness
    if sqlite3_exec(db, "SELECT count(*) FROM sqlite_master;", nil, nil, nil) == SQLITE_OK {
        print("Successfully opened encrypted database.")
    } else {
        print("Invalid encryption key or corrupted database.")
    }

    sqlite3_close(db)
}
```

## License

### DataLiteC

This project is licensed under the MIT License. See the `LICENSE` file for details.

### SQLCipher

```
Copyright (c) 2025, ZETETIC LLC
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the ZETETIC LLC nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ZETETIC LLC ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ZETETIC LLC BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### LibTomCrypt

```
                          The LibTom license

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
```
