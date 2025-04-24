# DataLiteC Package

**DataLiteC** is a Swift package that provides a system interface to the SQLite C library. It supports **macOS**, **iOS**, and **Linux** platforms, using the system SQLite library via `pkg-config`.

## Overview

DataLiteC is a Swift system library wrapper that links directly to the SQLite C API. It does not add any abstractions and enables direct usage of SQLite's C functions within Swift code.

## Installation

### Adding DataLiteC to Your Project

To use DataLiteC in your Swift project, add it to the dependencies section of your `Package.swift` file:

```swift
// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "YourProject",
    dependencies: [
        .package(url: "https://github.com/angd-dev/data-lite-c.git", from: "1.0.0")
    ],
    targets: [
        .target(
            name: "YourTarget",
            dependencies: [
                .product(name: "DataLiteC", package: "data-lite-c")
            ]
        )
    ]
)
```

### System Requirements

DataLiteC requires SQLite to be installed on your system:

- **Linux**: Install `sqlite3` and development headers:

  ```bash
  sudo apt-get install sqlite3 libsqlite3-dev
  ```

- **macOS**: Install SQLite using Homebrew:

  ```bash
  brew install sqlite
  ```

## Usage

Once the package is added, you can import it and use SQLite's C API directly:

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

## Troubleshooting

If you encounter build errors such as missing `sqlite3.h`, ensure that `pkg-config` can locate SQLite:

```bash
pkg-config --cflags sqlite3
```

If the command returns nothing, manually set the `PKG_CONFIG_PATH`:

```bash
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
