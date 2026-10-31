#pragma once

#include <string>
#include <memory>
#include "DatabaseManager.h"
#include "json.hpp"

// ============================================================================
// OOP CONCEPT: File Handling & Stream I/O (Report Generation)
// ReportManager handles aggregation of financial records and writes reports
// directly to persistent CSV files using standard C++ file streams (std::ofstream).
// ============================================================================

class ReportManager {
private:
    std::shared_ptr<DatabaseManager> dbManager;

public:
    explicit ReportManager(std::shared_ptr<DatabaseManager> db);

    // Analytical report queries
    nlohmann::json getDailyReport(const std::string& dateStr = "");
    nlohmann::json getMonthlyReport(const std::string& monthStr = "");
    nlohmann::json getAccountSummaryReport();

    // OOP CONCEPT: File Handling (CSV Export)
    std::string exportTransactionsToCsv(const std::string& outputFilePath = "database/transactions_export.csv");
};
