#include "DatabaseManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& path) 
    : db(nullptr), dbPath(path) {
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        throw DatabaseException("Failed to open SQLite database at " + path + ": " + err);
    }

    // Enable SQLite foreign key enforcement
    char* zErrMsg = nullptr;
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON; PRAGMA journal_mode = WAL;", nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "unknown error";
        sqlite3_free(zErrMsg);
        std::cerr << "[DatabaseManager] Warning setting PRAGMAs: " << err << std::endl;
    }

    initializeTables();
    seedSampleDataIfEmpty();
}

DatabaseManager::~DatabaseManager() {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void DatabaseManager::initializeTables() {
    std::lock_guard<std::mutex> lock(dbMutex);
    const char* schemaSql = R"(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            full_name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            phone TEXT,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('ADMIN', 'CUSTOMER')),
            address TEXT,
            date_of_birth TEXT,
            status TEXT DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'INACTIVE', 'SUSPENDED')),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS accounts (
            account_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            account_number TEXT UNIQUE NOT NULL,
            account_type TEXT NOT NULL CHECK(account_type IN ('SAVINGS', 'CURRENT', 'STUDENT')),
            balance INTEGER NOT NULL DEFAULT 0 CHECK(balance >= 0),
            status TEXT DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'INACTIVE', 'FROZEN', 'CLOSED')),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            reference_number TEXT NOT NULL,
            account_id INTEGER NOT NULL,
            transaction_type TEXT NOT NULL CHECK(transaction_type IN ('DEPOSIT', 'WITHDRAWAL', 'TRANSFER_IN', 'TRANSFER_OUT', 'ADJUSTMENT')),
            amount INTEGER NOT NULL CHECK(amount > 0),
            description TEXT,
            transaction_status TEXT DEFAULT 'SUCCESS' CHECK(transaction_status IN ('SUCCESS', 'FAILED', 'PENDING')),
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS transfer_details (
            transfer_id INTEGER PRIMARY KEY AUTOINCREMENT,
            reference_number TEXT UNIQUE NOT NULL,
            sender_account_id INTEGER NOT NULL,
            receiver_account_id INTEGER NOT NULL,
            amount INTEGER NOT NULL CHECK(amount > 0),
            description TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (sender_account_id) REFERENCES accounts(account_id),
            FOREIGN KEY (receiver_account_id) REFERENCES accounts(account_id)
        );

        CREATE TABLE IF NOT EXISTS admin_logs (
            log_id INTEGER PRIMARY KEY AUTOINCREMENT,
            admin_id INTEGER NOT NULL,
            action TEXT NOT NULL,
            description TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (admin_id) REFERENCES users(user_id)
        );

        CREATE TABLE IF NOT EXISTS sessions (
            session_token TEXT PRIMARY KEY,
            user_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            expires_at DATETIME NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
        CREATE INDEX IF NOT EXISTS idx_accounts_account_number ON accounts(account_number);
        CREATE INDEX IF NOT EXISTS idx_accounts_user_id ON accounts(user_id);
        CREATE INDEX IF NOT EXISTS idx_transactions_account_id ON transactions(account_id);
        CREATE INDEX IF NOT EXISTS idx_transactions_ref ON transactions(reference_number);
        CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(created_at);
        CREATE INDEX IF NOT EXISTS idx_transfer_ref ON transfer_details(reference_number);
        CREATE INDEX IF NOT EXISTS idx_sessions_token ON sessions(session_token);
    )";

    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, schemaSql, nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "unknown";
        sqlite3_free(zErrMsg);
        throw DatabaseException("Failed to initialize database tables: " + err);
    }
}

void DatabaseManager::seedSampleDataIfEmpty() {
    std::lock_guard<std::mutex> lock(dbMutex);
    sqlite3_stmt* stmt = nullptr;
    const char* countSql = "SELECT COUNT(*) FROM users;";
    int rc = sqlite3_prepare_v2(db, countSql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;

    int userCount = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userCount = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (userCount == 0) {
        std::cout << "[DatabaseManager] Database is empty. Seeding initial records..." << std::endl;
        std::ifstream seedFile("database/seed.sql");
        if (seedFile.is_open()) {
            std::stringstream buffer;
            buffer << seedFile.rdbuf();
            std::string seedSql = buffer.str();
            char* zErrMsg = nullptr;
            rc = sqlite3_exec(db, seedSql.c_str(), nullptr, nullptr, &zErrMsg);
            if (rc != SQLITE_OK) {
                std::string err = zErrMsg ? zErrMsg : "unknown";
                sqlite3_free(zErrMsg);
                std::cerr << "[DatabaseManager] Warning executing seed.sql: " << err << std::endl;
            } else {
                std::cout << "[DatabaseManager] Initial seed data inserted successfully." << std::endl;
            }
        }
    }
}

void DatabaseManager::beginTransaction() {
    std::lock_guard<std::mutex> lock(dbMutex);
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "unknown error";
        sqlite3_free(zErrMsg);
        throw DatabaseException("Failed to begin transaction: " + err);
    }
}

void DatabaseManager::commit() {
    std::lock_guard<std::mutex> lock(dbMutex);
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string err = zErrMsg ? zErrMsg : "unknown error";
        sqlite3_free(zErrMsg);
        throw DatabaseException("Failed to commit transaction: " + err);
    }
}

void DatabaseManager::rollback() {
    std::lock_guard<std::mutex> lock(dbMutex);
    char* zErrMsg = nullptr;
    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &zErrMsg);
    if (zErrMsg) sqlite3_free(zErrMsg);
}

int DatabaseManager::executeNonQuery(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(dbMutex);
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseException(std::string("SQL prepare error: ") + sqlite3_errmsg(db));
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("SQL execution error: " + err);
    }

    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

long long DatabaseManager::getLastInsertId() {
    std::lock_guard<std::mutex> lock(dbMutex);
    return sqlite3_last_insert_rowid(db);
}

nlohmann::json DatabaseManager::queryJson(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::mutex> lock(dbMutex);
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw DatabaseException(std::string("SQL prepare error: ") + sqlite3_errmsg(db));
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    nlohmann::json results = nlohmann::json::array();
    int colCount = sqlite3_column_count(stmt);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        nlohmann::json row = nlohmann::json::object();
        for (int i = 0; i < colCount; ++i) {
            std::string colName = sqlite3_column_name(stmt, i);
            int colType = sqlite3_column_type(stmt, i);

            if (colType == SQLITE_INTEGER) {
                row[colName] = sqlite3_column_int64(stmt, i);
            } else if (colType == SQLITE_FLOAT) {
                row[colName] = sqlite3_column_double(stmt, i);
            } else if (colType == SQLITE_TEXT) {
                const char* txt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[colName] = txt ? std::string(txt) : "";
            } else if (colType == SQLITE_NULL) {
                row[colName] = nullptr;
            }
        }
        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return results;
}
