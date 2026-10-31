#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "json.hpp"
#include "Exceptions.h"

// ============================================================================
// OOP CONCEPT: RAII (Resource Acquisition Is Initialization) & Encapsulation
// DatabaseManager encapsulates SQLite3 operations, prepared statements, 
// connection pooling / mutex protection, and database transactions.
// ============================================================================

class DatabaseManager {
private:
    sqlite3* db;
    std::string dbPath;
    mutable std::mutex dbMutex;

public:
    explicit DatabaseManager(const std::string& path = "database/bank_management.db");
    ~DatabaseManager();

    // Prevent copy semantics (SQLite handle is a unique resource)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Database initialization
    void initializeTables();
    void seedSampleDataIfEmpty();

    // Transaction control (Single SQL database transaction for money operations)
    void beginTransaction();
    void commit();
    void rollback();

    // Raw database handle accessor (protected by mutex)
    sqlite3* getHandle() { return db; }
    std::mutex& getMutex() { return dbMutex; }

    // Prepared statement execution helpers
    int executeNonQuery(const std::string& sql, 
                        const std::vector<std::string>& params = {});

    // Last inserted rowid
    long long getLastInsertId();

    // Generic JSON array query result
    nlohmann::json queryJson(const std::string& sql, 
                             const std::vector<std::string>& params = {});
};
