// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

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
        .target(
            name: "DataLiteC",
            sources: ["libtomcrypt", "sqlcipher"],
            publicHeadersPath: "sqlcipher",
            cSettings: [
                .headerSearchPath("libtomcrypt/headers"),
                .define("MIN(x,y)", to: "(((x)<(y))?(x):(y))"),
                .define("MAX(x,y)", to: "(((x)>(y))?(x):(y))"),
                .define("SQLCIPHER_CRYPTO_LIBTOMCRYPT"),
                .define("SQLITE_HOMEGROWN_RECURSIVE_MUTEX"),
                .define("SQLITE_HAS_CODEC"),
                .define("SQLITE_EXTRA_INIT", to: "sqlcipher_extra_init"),
                .define("SQLITE_EXTRA_SHUTDOWN", to: "sqlcipher_extra_shutdown"),
                .define("SQLITE_THREADSAFE", to: "1"),
                .define("SQLITE_TEMP_STORE", to: "2"),
                .define("SQLITE_SECURE_DELETE"),
                .define("SQLITE_ENABLE_COLUMN_METADATA"),
                .define("SQLITE_ENABLE_DBSTAT_VTAB", .when(configuration: .debug)),
                .define("SQLITE_ENABLE_STMTVTAB", .when(configuration: .debug)),
                .define("SQLITE_ENABLE_API_ARMOR", .when(configuration: .debug)),
                .define("SQLITE_DQS", to: "0"),
                .define("SQLITE_ENABLE_PREUPDATE_HOOK"),
                .define("SQLITE_LIKE_DOESNT_MATCH_BLOBS"),
                .define("SQLITE_OMIT_DEPRECATED"),
                .define("SQLITE_OMIT_SHARED_CACHE"),
                .define("SQLITE_USE_URI"),
                .define("HAVE_GETHOSTUUID", to: "0"),
                .define("HAVE_STDINT_H"),
                .define("LTC_NO_TEST"),
                .define("LTC_NO_FILE")
            ],
            linkerSettings: [
                .linkedLibrary("log", .when(platforms: [.android]))
            ]
        ),
        .testTarget(
            name: "DataLiteCTests",
            dependencies: ["DataLiteC"],
            cSettings: [
                .define("SQLITE_HAS_CODEC")
            ]
        )
    ]
)
