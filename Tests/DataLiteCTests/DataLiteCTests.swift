import Foundation
import Testing
import DataLiteC

class DataLiteCTests {
    let path: String
    
    init() {
        path = FileManager.default.temporaryDirectory
            .appending(component: UUID().uuidString)
            .appendingPathExtension("sqlite")
            .path
    }
    
    deinit {
        try! FileManager.default.removeItem(atPath: path)
    }
    
    @Test func testEncryptedDB() {
        var db: OpaquePointer?
        #expect(sqlite3_open(path, &db) == SQLITE_OK)
        defer { sqlite3_close(db) }
        
        #expect(sqlite3_key(db, "testpassword", Int32("testpassword".utf8.count)) == SQLITE_OK)
        
        let createSQL = "CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT);"
        #expect(sqlite3_exec(db, createSQL, nil, nil, nil) == SQLITE_OK)
        
        let insertSQL = "INSERT INTO test (value) VALUES ('Hello');"
        #expect(sqlite3_exec(db, insertSQL, nil, nil, nil) == SQLITE_OK)
    }
    
    @Test func testInvalidKey() {
        do {
            var db: OpaquePointer?
            #expect(sqlite3_open(path, &db) == SQLITE_OK)
            defer { sqlite3_close(db) }
            
            #expect(sqlite3_key(db, "correctkey", Int32("correctkey".utf8.count)) == SQLITE_OK)
            let sql = "CREATE TABLE t(id INTEGER PRIMARY KEY); INSERT INTO t(id) VALUES (1);"
            #expect(sqlite3_exec(db, sql, nil, nil, nil) == SQLITE_OK)
        }
        
        do {
            var db: OpaquePointer?
            #expect(sqlite3_open(path, &db) == SQLITE_OK)
            defer { sqlite3_close(db) }
            
            #expect(sqlite3_key(db, "wrongkey", Int32("wrongkey".utf8.count)) == SQLITE_OK)
            #expect(sqlite3_exec(db, "SELECT * FROM t;", nil, nil, nil) != SQLITE_OK)
        }
    }
    
    @Test func testCorrectKey() {
        do {
            var db: OpaquePointer?
            #expect(sqlite3_open(path, &db) == SQLITE_OK)
            defer { sqlite3_close(db) }
            
            #expect(sqlite3_key(db, "securekey", Int32("securekey".utf8.count)) == SQLITE_OK)
            let sql = "CREATE TABLE t(id INTEGER PRIMARY KEY, value TEXT);"
            #expect(sqlite3_exec(db, sql, nil, nil, nil) == SQLITE_OK)
            #expect(sqlite3_exec(db, "INSERT INTO t(value) VALUES ('Data');", nil, nil, nil) == SQLITE_OK)
        }
        
        do {
            var db: OpaquePointer?
            #expect(sqlite3_open(path, &db) == SQLITE_OK)
            defer { sqlite3_close(db) }
            
            #expect(sqlite3_key(db, "securekey", Int32("securekey".utf8.count)) == SQLITE_OK)
            
            var stmt: OpaquePointer?
            let query = "SELECT value FROM t;"
            #expect(sqlite3_prepare_v2(db, query, -1, &stmt, nil) == SQLITE_OK)
            defer { sqlite3_finalize(stmt) }
            
            #expect(sqlite3_step(stmt) == SQLITE_ROW)
            let text = String(cString: sqlite3_column_text(stmt, 0))
            #expect(text == "Data")
        }
    }
}
