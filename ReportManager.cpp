#include "ReportManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

ReportManager::ReportManager(std::shared_ptr<DatabaseManager> db)
    : dbManager(db) {}

nlohmann::json ReportManager::getDailyReport(const std::string& dateStr) {
    std::string targetDate = dateStr;
    if (targetDate.empty()) {
        std::string dateSql = "SELECT date('now') AS curdate;";
        auto r = dbManager->queryJson(dateSql);
        targetDate = r.empty() ? "2026-01-01" : r[0]["curdate"].get<std::string>();
    }

    std::string sql = R"(
        SELECT 
            transaction_type, 
            COUNT(*) AS total_count, 
            COALESCE(SUM(amount), 0) AS total_paise
        FROM transactions
        WHERE date(created_at) = date(?)
        GROUP BY transaction_type;
    )";

    nlohmann::json rows = dbManager->queryJson(sql, {targetDate});
    long long totalTurnover = 0;
    for (auto& row : rows) {
        long long paise = row["total_paise"].get<long long>();
        totalTurnover += paise;
        row["total_rupees"] = static_cast<double>(paise) / 100.0;
    }

    std::string detailSql = R"(
        SELECT t.reference_number, a.account_number, u.full_name, t.transaction_type, 
               t.amount, t.description, t.created_at
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        JOIN users u ON a.user_id = u.user_id
        WHERE date(t.created_at) = date(?)
        ORDER BY t.created_at DESC;
    )";
    nlohmann::json details = dbManager->queryJson(detailSql, {targetDate});
    for (auto& d : details) {
        long long amt = d["amount"].get<long long>();
        d["amount_rupees"] = static_cast<double>(amt) / 100.0;
    }

    return {
        {"report_date", targetDate},
        {"total_turnover_rupees", static_cast<double>(totalTurnover) / 100.0},
        {"breakdown", rows},
        {"transactions", details}
    };
}

nlohmann::json ReportManager::getMonthlyReport(const std::string& monthStr) {
    std::string targetMonth = monthStr; // Format: YYYY-MM
    if (targetMonth.empty()) {
        std::string dateSql = "SELECT strftime('%Y-%m', 'now') AS curmonth;";
        auto r = dbManager->queryJson(dateSql);
        targetMonth = r.empty() ? "2026-01" : r[0]["curmonth"].get<std::string>();
    }

    std::string sql = R"(
        SELECT 
            transaction_type, 
            COUNT(*) AS total_count, 
            COALESCE(SUM(amount), 0) AS total_paise
        FROM transactions
        WHERE strftime('%Y-%m', created_at) = ?
        GROUP BY transaction_type;
    )";

    nlohmann::json rows = dbManager->queryJson(sql, {targetMonth});
    long long totalVolume = 0;
    for (auto& row : rows) {
        long long paise = row["total_paise"].get<long long>();
        totalVolume += paise;
        row["total_rupees"] = static_cast<double>(paise) / 100.0;
    }

    return {
        {"report_month", targetMonth},
        {"total_volume_rupees", static_cast<double>(totalVolume) / 100.0},
        {"breakdown", rows}
    };
}

nlohmann::json ReportManager::getAccountSummaryReport() {
    std::string sql = R"(
        SELECT 
            account_type, 
            status,
            COUNT(*) AS account_count, 
            COALESCE(SUM(balance), 0) AS total_balance_paise,
            COALESCE(AVG(balance), 0) AS avg_balance_paise
        FROM accounts
        GROUP BY account_type, status
        ORDER BY account_type;
    )";
    nlohmann::json rows = dbManager->queryJson(sql);
    for (auto& r : rows) {
        long long tot = r["total_balance_paise"].get<long long>();
        double avg = r["avg_balance_paise"].is_number() ? r["avg_balance_paise"].get<double>() : 0.0;
        r["total_balance_rupees"] = static_cast<double>(tot) / 100.0;
        r["avg_balance_rupees"] = avg / 100.0;
    }
    return rows;
}

// ============================================================================
// OOP CONCEPT: File Handling with std::ofstream (Persistent CSV Export)
// ============================================================================
std::string ReportManager::exportTransactionsToCsv(const std::string& outputFilePath) {
    std::string sql = R"(
        SELECT t.reference_number, a.account_number, u.full_name AS customer_name,
               t.transaction_type, t.amount, t.description, t.transaction_status, t.created_at
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        JOIN users u ON a.user_id = u.user_id
        ORDER BY t.created_at DESC;
    )";

    nlohmann::json rows = dbManager->queryJson(sql);

    // Open file stream
    std::ofstream csvFile(outputFilePath, std::ios::out | std::ios::trunc);
    if (!csvFile.is_open()) {
        throw BankingException("Failed to open file for writing CSV export: " + outputFilePath);
    }

    // Write CSV Header
    csvFile << "Reference Number,Account Number,Customer Name,Type,Amount (INR),Description,Status,Date Time\n";

    // Write Data Rows
    for (const auto& r : rows) {
        std::string ref = r["reference_number"].get<std::string>();
        std::string acc = r["account_number"].get<std::string>();
        std::string name = r["customer_name"].get<std::string>();
        std::string type = r["transaction_type"].get<std::string>();
        long long amtPaise = r["amount"].get<long long>();
        double amtRupees = static_cast<double>(amtPaise) / 100.0;
        std::string desc = r["description"].get<std::string>();
        std::string status = r["transaction_status"].get<std::string>();
        std::string date = r["created_at"].get<std::string>();

        // Escape quotes if description has commas
        std::string escapedDesc = desc;
        if (escapedDesc.find(',') != std::string::npos) {
            escapedDesc = "\"" + escapedDesc + "\"";
        }

        csvFile << ref << ","
                << acc << ","
                << "\"" << name << "\","
                << type << ","
                << std::fixed << std::setprecision(2) << amtRupees << ","
                << escapedDesc << ","
                << status << ","
                << date << "\n";
    }

    csvFile.close();
    std::cout << "[ReportManager] Exported " << rows.size() << " records to " << outputFilePath << std::endl;
    return outputFilePath;
}
