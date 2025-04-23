// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

#if os(Linux)
let sqlitePkg = "sqlite3"
#else
let sqlitePkg: String? = nil
#endif

let package = Package(
    name: "DataLiteC",
    platforms: [
        .macOS(.v10_14),
        .iOS(.v12)
    ],
    products: [
        .library(name: "DataLiteC", targets: ["DataLiteC"])
    ],
    targets: [
        .systemLibrary(name: "DataLiteC", pkgConfig: sqlitePkg, providers: [
            .apt(["sqlite3", "libsqlite3-dev"]),
            .brew(["sqlite"])
        ])
    ]
)
