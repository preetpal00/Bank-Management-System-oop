#include "Bank.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

Bank::Bank(const std::string& name, std::shared_ptr<DatabaseManager> db)
    : bankName(name), dbManager(db) {}

std::string Bank::generateReferenceNumber(const std::string& prefix) {
    auto now = std::chrono::system_clock::now();
    auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);

    std::stringstream ss;
    ss << prefix << epochSeconds << dis(gen);
    return ss.str();
}

std::string Bank::generateUniqueAccountNumber() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dis(10000000, 99999999);

    while (true) {
        std::string acc = "ACC" + std::to_string(dis(gen));
        std::string sql = "SELECT account_id FROM accounts WHERE account_number = ?;";
        nlohmann::json rows = dbManager->queryJson(sql, {acc});
        if (rows.empty()) {
            return acc;
        }
    }
}

// OOP CONCEPT: Factory Method & Smart Pointers with Runtime Polymorphism
std::shared_ptr<BankAccount> Bank::createAccountInstance(const nlohmann::json& row) {
    int accId = static_cast<int>(row["account_id"].get<long long>());
    int uId = static_cast<int>(row["user_id"].get<long long>());
    std::string accNum = row["account_number"].get<std::string>();
    long long balance = row["balance"].get<long long>();
    std::string type = row["account_type"].get<std::string>();
    std::string status = row["status"].get<std::string>();
    std::string created = row["created_at"].get<std::string>();
    std::string custName = row.contains("customer_name") && !row["customer_name"].is_null() 
                         ? row["customer_name"].get<std::string>() : "";

    if (type == "CURRENT") {
        return std::make_shared<CurrentAccount>(accId, uId, accNum, balance, status, created, custName);
    } else {
        // Savings or Student accounts follow savings rules
        return std::make_shared<SavingsAccount>(accId, uId, accNum, balance, status, created, custName);
    }
}

nlohmann::json Bank::getAllCustomers() {
    std::string sql = R"(
        SELECT u.user_id, u.full_name, u.email, u.phone, u.role, u.status, 
               u.address, u.date_of_birth, u.created_at,
               COUNT(a.account_id) AS account_count,
               COALESCE(SUM(a.balance), 0) AS total_balance_paise
        FROM users u
        LEFT JOIN accounts a ON u.user_id = a.user_id
        WHERE u.role = 'CUSTOMER'
        GROUP BY u.user_id
        ORDER BY u.user_id DESC;
    )";
    nlohmann::json rows = dbManager->queryJson(sql);
    for (auto& row : rows) {
        long long bPaise = row["total_balance_paise"].get<long long>();
        row["total_balance_rupees"] = static_cast<double>(bPaise) / 100.0;
    }
    return rows;
}

nlohmann::json Bank::getCustomerById(int customerId) {
    std::string sql = R"(
        SELECT user_id, full_name, email, phone, role, status, address, date_of_birth, created_at
        FROM users WHERE user_id = ? AND role = 'CUSTOMER';
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(customerId)});
    if (rows.empty()) {
        throw BankingException("Customer not found with ID: " + std::to_string(customerId));
    }
    auto customer = rows[0];
    customer["name"] = customer["full_name"];
    customer["accounts"] = getAccountsForUser(customerId);
    return customer;
}

nlohmann::json Bank::updateCustomer(int customerId, const std::string& name, 
                                   const std::string& phone, const std::string& address, 
                                   const std::string& dob) {
    std::string sql = R"(
        UPDATE users 
        SET full_name = ?, phone = ?, address = ?, date_of_birth = ?
        WHERE user_id = ? AND role = 'CUSTOMER';
    )";
    int affected = dbManager->executeNonQuery(sql, {name, phone, address, dob, std::to_string(customerId)});
    if (affected == 0) {
        throw BankingException("Could not update customer: record not found.");
    }
    return getCustomerById(customerId);
}

nlohmann::json Bank::setCustomerStatus(int customerId, const std::string& status) {
    if (status != "ACTIVE" && status != "INACTIVE" && status != "SUSPENDED") {
        throw BankingException("Invalid customer status: " + status);
    }
    std::string sql = "UPDATE users SET status = ? WHERE user_id = ? AND role = 'CUSTOMER';";
    int affected = dbManager->executeNonQuery(sql, {status, std::to_string(customerId)});
    if (affected == 0) {
        throw BankingException("Customer not found.");
    }
    return {{"success", true}, {"user_id", customerId}, {"new_status", status}};
}

nlohmann::json Bank::getAllAccounts() {
    std::string sql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name, u.email AS customer_email
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        ORDER BY a.account_id DESC;
    )";
    nlohmann::json rows = dbManager->queryJson(sql);
    for (auto& row : rows) {
        long long bPaise = row["balance"].get<long long>();
        row["balance_rupees"] = static_cast<double>(bPaise) / 100.0;
        
        // Masked number
        std::string num = row["account_number"].get<std::string>();
        if (num.length() > 6) {
            row["masked_account_number"] = num.substr(0, 3) + std::string(num.length() - 7, '*') + num.substr(num.length() - 4);
        } else {
            row["masked_account_number"] = num;
        }
    }
    return rows;
}

nlohmann::json Bank::getAccountsForUser(int userId) {
    std::string sql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.user_id = ?
        ORDER BY a.account_id ASC;
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(userId)});
    for (auto& row : rows) {
        long long bPaise = row["balance"].get<long long>();
        row["balance_rupees"] = static_cast<double>(bPaise) / 100.0;
        std::string num = row["account_number"].get<std::string>();
        if (num.length() > 6) {
            row["masked_account_number"] = num.substr(0, 3) + std::string(num.length() - 7, '*') + num.substr(num.length() - 4);
        } else {
            row["masked_account_number"] = num;
        }
    }
    return rows;
}

nlohmann::json Bank::getAccountById(int accountId) {
    std::string sql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name, u.email AS customer_email
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.account_id = ?;
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(accountId)});
    if (rows.empty()) {
        throw InvalidAccountException("Account not found with ID: " + std::to_string(accountId));
    }
    auto row = rows[0];
    long long bPaise = row["balance"].get<long long>();
    row["balance_rupees"] = static_cast<double>(bPaise) / 100.0;
    std::string num = row["account_number"].get<std::string>();
    row["masked_account_number"] = num.substr(0, 3) + std::string(num.length() - 7, '*') + num.substr(num.length() - 4);
    return row;
}

nlohmann::json Bank::createAccount(int userId, const std::string& accountType, long long initialDepositPaise) {
    if (accountType != "SAVINGS" && accountType != "CURRENT" && accountType != "STUDENT") {
        throw BankingException("Invalid account type. Supported: SAVINGS, CURRENT, STUDENT.");
    }
    if (initialDepositPaise < 0) {
        throw BankingException("Initial deposit cannot be negative.");
    }

    // Verify user exists
    std::string userSql = "SELECT user_id, full_name FROM users WHERE user_id = ? AND role = 'CUSTOMER';";
    nlohmann::json users = dbManager->queryJson(userSql, {std::to_string(userId)});
    if (users.empty()) {
        throw BankingException("Customer not found.");
    }

    std::string accNumber = generateUniqueAccountNumber();

    dbManager->beginTransaction();
    try {
        std::string insertAcc = R"(
            INSERT INTO accounts (user_id, account_number, account_type, balance, status)
            VALUES (?, ?, ?, ?, 'ACTIVE');
        )";
        dbManager->executeNonQuery(insertAcc, {
            std::to_string(userId), accNumber, accountType, std::to_string(initialDepositPaise)
        });
        long long accId = dbManager->getLastInsertId();

        if (initialDepositPaise > 0) {
            std::string ref = generateReferenceNumber("TXN");
            std::string txnSql = R"(
                INSERT INTO transactions (reference_number, account_id, transaction_type, amount, description, transaction_status)
                VALUES (?, ?, 'DEPOSIT', ?, 'Initial Account Opening Deposit', 'SUCCESS');
            )";
            dbManager->executeNonQuery(txnSql, {ref, std::to_string(accId), std::to_string(initialDepositPaise)});
        }

        dbManager->commit();
        return getAccountById(static_cast<int>(accId));
    } catch (...) {
        dbManager->rollback();
        throw;
    }
}

nlohmann::json Bank::setAccountStatus(int accountId, const std::string& status) {
    if (status != "ACTIVE" && status != "INACTIVE" && status != "FROZEN" && status != "CLOSED") {
        throw BankingException("Invalid status: " + status);
    }
    std::string sql = "UPDATE accounts SET status = ? WHERE account_id = ?;";
    int affected = dbManager->executeNonQuery(sql, {status, std::to_string(accountId)});
    if (affected == 0) {
        throw InvalidAccountException("Account not found.");
    }
    return {{"success", true}, {"account_id", accountId}, {"new_status", status}};
}

// ============================================================================
// FINANCIAL CORE: Deposit Money
// ============================================================================
nlohmann::json Bank::deposit(int userId, int accountId, long long amountPaise, const std::string& description) {
    if (amountPaise <= 0) {
        throw BankingException("Deposit amount must be strictly greater than ₹0.00.");
    }

    std::string sql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.account_id = ?;
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(accountId)});
    if (rows.empty()) {
        throw InvalidAccountException("Target account does not exist.");
    }

    auto accountRow = rows[0];
    if (userId > 0 && accountRow["user_id"].get<long long>() != userId) {
        throw BankingException("Unauthorized: Account does not belong to the logged-in customer.");
    }

    // OOP Polymorphic Model
    std::shared_ptr<BankAccount> account = createAccountInstance(accountRow);
    account->deposit(amountPaise, description);

    std::string ref = generateReferenceNumber("TXN");
    std::string desc = description.empty() ? "Manual Deposit" : description;

    dbManager->beginTransaction();
    try {
        std::string updateSql = "UPDATE accounts SET balance = balance + ? WHERE account_id = ?;";
        dbManager->executeNonQuery(updateSql, {std::to_string(amountPaise), std::to_string(accountId)});

        std::string txnSql = R"(
            INSERT INTO transactions (reference_number, account_id, transaction_type, amount, description, transaction_status)
            VALUES (?, ?, 'DEPOSIT', ?, ?, 'SUCCESS');
        )";
        dbManager->executeNonQuery(txnSql, {ref, std::to_string(accountId), std::to_string(amountPaise), desc});

        dbManager->commit();

        return {
            {"success", true},
            {"reference_number", ref},
            {"account_number", account->getAccountNumber()},
            {"masked_account_number", account->getMaskedAccountNumber()},
            {"amount_paise", amountPaise},
            {"amount_rupees", static_cast<double>(amountPaise) / 100.0},
            {"new_balance_rupees", static_cast<double>(account->getBalance()) / 100.0},
            {"description", desc},
            {"message", "Deposit of ₹" + std::to_string(amountPaise / 100) + " processed successfully."}
        };
    } catch (...) {
        dbManager->rollback();
        throw;
    }
}

// ============================================================================
// FINANCIAL CORE: Withdraw Money
// ============================================================================
nlohmann::json Bank::withdraw(int userId, int accountId, long long amountPaise, const std::string& description) {
    if (amountPaise <= 0) {
        throw BankingException("Withdrawal amount must be strictly greater than ₹0.00.");
    }

    std::string sql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.account_id = ?;
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(accountId)});
    if (rows.empty()) {
        throw InvalidAccountException("Target account does not exist.");
    }

    auto accountRow = rows[0];
    if (userId > 0 && accountRow["user_id"].get<long long>() != userId) {
        throw BankingException("Unauthorized: Account does not belong to the logged-in customer.");
    }

    // OOP Polymorphic Model (enforces savings minimum balance / current overdraft)
    std::shared_ptr<BankAccount> account = createAccountInstance(accountRow);
    account->withdraw(amountPaise, description);

    std::string ref = generateReferenceNumber("TXN");
    std::string desc = description.empty() ? "Cash Withdrawal" : description;

    dbManager->beginTransaction();
    try {
        std::string updateSql = "UPDATE accounts SET balance = balance - ? WHERE account_id = ? AND balance >= ?;";
        int affected = dbManager->executeNonQuery(updateSql, {
            std::to_string(amountPaise), std::to_string(accountId), std::to_string(amountPaise)
        });
        if (affected == 0) {
            throw InsufficientFundsException("Insufficient funds available for withdrawal.", amountPaise, account->getBalance());
        }

        std::string txnSql = R"(
            INSERT INTO transactions (reference_number, account_id, transaction_type, amount, description, transaction_status)
            VALUES (?, ?, 'WITHDRAWAL', ?, ?, 'SUCCESS');
        )";
        dbManager->executeNonQuery(txnSql, {ref, std::to_string(accountId), std::to_string(amountPaise), desc});

        dbManager->commit();

        return {
            {"success", true},
            {"reference_number", ref},
            {"account_number", account->getAccountNumber()},
            {"masked_account_number", account->getMaskedAccountNumber()},
            {"amount_paise", amountPaise},
            {"amount_rupees", static_cast<double>(amountPaise) / 100.0},
            {"new_balance_rupees", static_cast<double>(account->getBalance()) / 100.0},
            {"description", desc},
            {"message", "Withdrawal of ₹" + std::to_string(amountPaise / 100) + " completed successfully."}
        };
    } catch (...) {
        dbManager->rollback();
        throw;
    }
}

// ============================================================================
// FINANCIAL CORE: Fund Transfer (Single Atomic SQL Transaction)
// ============================================================================
nlohmann::json Bank::transfer(int senderUserId, int senderAccountId, const std::string& receiverAccNumber, 
                             long long amountPaise, const std::string& description) {
    if (amountPaise <= 0) {
        throw BankingException("Transfer amount must be strictly greater than ₹0.00.");
    }

    // 1. Fetch sender account
    std::string senderSql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.account_id = ?;
    )";
    nlohmann::json senderRows = dbManager->queryJson(senderSql, {std::to_string(senderAccountId)});
    if (senderRows.empty()) {
        throw InvalidAccountException("Sender account not found.");
    }
    auto senderRow = senderRows[0];

    // Verify ownership
    if (senderUserId > 0 && senderRow["user_id"].get<long long>() != senderUserId) {
        throw BankingException("Unauthorized: Sender account does not belong to you.");
    }

    // 2. Fetch receiver account
    std::string receiverSql = R"(
        SELECT a.account_id, a.user_id, a.account_number, a.account_type, 
               a.balance, a.status, a.created_at, u.full_name AS customer_name
        FROM accounts a
        JOIN users u ON a.user_id = u.user_id
        WHERE a.account_number = ?;
    )";
    nlohmann::json receiverRows = dbManager->queryJson(receiverSql, {receiverAccNumber});
    if (receiverRows.empty()) {
        throw InvalidAccountException("Beneficiary account number '" + receiverAccNumber + "' does not exist.");
    }
    auto receiverRow = receiverRows[0];

    int receiverAccountId = static_cast<int>(receiverRow["account_id"].get<long long>());

    // Prevent transferring to the exact same account
    if (senderAccountId == receiverAccountId) {
        throw BankingException("Transfer failed: Sender and receiver accounts cannot be identical.");
    }

    // Verify receiver account is active
    if (receiverRow["status"].get<std::string>() != "ACTIVE") {
        throw InvalidAccountException("Beneficiary account is not active (Status: " + receiverRow["status"].get<std::string>() + ").");
    }

    // 3. Polymorphic balance checks in C++ OOP models
    std::shared_ptr<BankAccount> senderAccount = createAccountInstance(senderRow);
    std::shared_ptr<BankAccount> receiverAccount = createAccountInstance(receiverRow);

    // This will throw InsufficientFundsException if balance/min-balance is violated
    senderAccount->withdraw(amountPaise, description);
    receiverAccount->deposit(amountPaise, description);

    std::string ref = generateReferenceNumber("TXN");
    std::string desc = description.empty() ? "Fund Transfer" : description;
    std::string senderDesc = "Transfer to " + receiverAccount->getCustomerName() + " (" + receiverAccNumber + ") - " + desc;
    std::string receiverDesc = "Transfer from " + senderAccount->getCustomerName() + " (" + senderAccount->getAccountNumber() + ") - " + desc;

    // 4. ATOMIC DATABASE TRANSACTION
    dbManager->beginTransaction();
    try {
        // Deduct from sender
        std::string deductSql = "UPDATE accounts SET balance = balance - ? WHERE account_id = ? AND balance >= ?;";
        int affSender = dbManager->executeNonQuery(deductSql, {
            std::to_string(amountPaise), std::to_string(senderAccountId), std::to_string(amountPaise)
        });
        if (affSender == 0) {
            throw InsufficientFundsException("Transfer failed: Insufficient funds in sender account.", amountPaise, senderAccount->getBalance());
        }

        // Add to receiver
        std::string creditSql = "UPDATE accounts SET balance = balance + ? WHERE account_id = ?;";
        dbManager->executeNonQuery(creditSql, {std::to_string(amountPaise), std::to_string(receiverAccountId)});

        // Record outgoing transaction
        std::string txnOutSql = R"(
            INSERT INTO transactions (reference_number, account_id, transaction_type, amount, description, transaction_status)
            VALUES (?, ?, 'TRANSFER_OUT', ?, ?, 'SUCCESS');
        )";
        dbManager->executeNonQuery(txnOutSql, {ref, std::to_string(senderAccountId), std::to_string(amountPaise), senderDesc});

        // Record incoming transaction
        std::string txnInSql = R"(
            INSERT INTO transactions (reference_number, account_id, transaction_type, amount, description, transaction_status)
            VALUES (?, ?, 'TRANSFER_IN', ?, ?, 'SUCCESS');
        )";
        dbManager->executeNonQuery(txnInSql, {ref, std::to_string(receiverAccountId), std::to_string(amountPaise), receiverDesc});

        // Record transfer details linking both accounts
        std::string detailsSql = R"(
            INSERT INTO transfer_details (reference_number, sender_account_id, receiver_account_id, amount, description)
            VALUES (?, ?, ?, ?, ?);
        )";
        dbManager->executeNonQuery(detailsSql, {
            ref, std::to_string(senderAccountId), std::to_string(receiverAccountId), std::to_string(amountPaise), desc
        });

        // Commit all changes atomically
        dbManager->commit();

        return {
            {"success", true},
            {"reference_number", ref},
            {"sender_account", senderAccount->getAccountNumber()},
            {"receiver_account", receiverAccNumber},
            {"receiver_name", receiverAccount->getCustomerName()},
            {"amount_paise", amountPaise},
            {"amount_rupees", static_cast<double>(amountPaise) / 100.0},
            {"new_balance_rupees", static_cast<double>(senderAccount->getBalance()) / 100.0},
            {"description", desc},
            {"message", "Transfer of ₹" + std::to_string(amountPaise / 100) + " to " + receiverAccount->getCustomerName() + " succeeded."}
        };
    } catch (...) {
        // Rollback every single balance change and transaction record
        dbManager->rollback();
        throw;
    }
}

// ============================================================================
// TRANSACTION HISTORY & FILTERING
// ============================================================================
nlohmann::json Bank::getTransactions(int userId, const std::string& role,
                                     const std::string& typeFilter,
                                     const std::string& startDate,
                                     const std::string& endDate,
                                     long long minPaise,
                                     long long maxPaise,
                                     const std::string& searchRef,
                                     int limit, int offset) {
    std::string sql = R"(
        SELECT t.transaction_id, t.reference_number, t.account_id, a.account_number, 
               t.transaction_type, t.amount, t.description, t.transaction_status, 
               t.created_at, u.full_name AS account_holder
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        JOIN users u ON a.user_id = u.user_id
        WHERE 1=1
    )";

    std::vector<std::string> params;

    // Customer can only view transactions from their own accounts
    if (role != "ADMIN" && userId > 0) {
        sql += " AND a.user_id = ?";
        params.push_back(std::to_string(userId));
    }

    if (!typeFilter.empty() && typeFilter != "ALL") {
        if (typeFilter == "TRANSFER") {
            sql += " AND t.transaction_type IN ('TRANSFER_IN', 'TRANSFER_OUT')";
        } else {
            sql += " AND t.transaction_type = ?";
            params.push_back(typeFilter);
        }
    }

    if (!startDate.empty()) {
        sql += " AND date(t.created_at) >= date(?)";
        params.push_back(startDate);
    }
    if (!endDate.empty()) {
        sql += " AND date(t.created_at) <= date(?)";
        params.push_back(endDate);
    }
    if (minPaise > 0) {
        sql += " AND t.amount >= ?";
        params.push_back(std::to_string(minPaise));
    }
    if (maxPaise > 0) {
        sql += " AND t.amount <= ?";
        params.push_back(std::to_string(maxPaise));
    }
    if (!searchRef.empty()) {
        sql += " AND (t.reference_number LIKE ? OR t.description LIKE ?)";
        params.push_back("%" + searchRef + "%");
        params.push_back("%" + searchRef + "%");
    }

    // Total count before pagination
    std::string countSql = "SELECT COUNT(*) as total FROM (" + sql + ") AS filtered_txns;";
    nlohmann::json countRow = dbManager->queryJson(countSql, params);
    long long totalCount = countRow.empty() ? 0 : countRow[0]["total"].get<long long>();

    sql += " ORDER BY t.created_at DESC, t.transaction_id DESC LIMIT ? OFFSET ?;";
    params.push_back(std::to_string(limit <= 0 ? 50 : limit));
    params.push_back(std::to_string(offset < 0 ? 0 : offset));

    nlohmann::json rows = dbManager->queryJson(sql, params);
    for (auto& row : rows) {
        long long amt = row["amount"].get<long long>();
        row["amount_rupees"] = static_cast<double>(amt) / 100.0;
        std::string num = row["account_number"].get<std::string>();
        if (num.length() > 6) {
            row["masked_account_number"] = num.substr(0, 3) + std::string(num.length() - 7, '*') + num.substr(num.length() - 4);
        } else {
            row["masked_account_number"] = num;
        }
    }

    return {
        {"total_count", totalCount},
        {"limit", limit},
        {"offset", offset},
        {"transactions", rows}
    };
}

nlohmann::json Bank::getTransactionByRef(const std::string& reference) {
    std::string sql = R"(
        SELECT t.transaction_id, t.reference_number, t.account_id, a.account_number, 
               t.transaction_type, t.amount, t.description, t.transaction_status, 
               t.created_at, u.full_name AS account_holder
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        JOIN users u ON a.user_id = u.user_id
        WHERE t.reference_number = ?;
    )";
    nlohmann::json rows = dbManager->queryJson(sql, {reference});
    if (rows.empty()) {
        throw BankingException("Transaction not found for reference: " + reference);
    }
    for (auto& row : rows) {
        long long amt = row["amount"].get<long long>();
        row["amount_rupees"] = static_cast<double>(amt) / 100.0;
        std::string num = row["account_number"].get<std::string>();
        if (num.length() > 6) {
            row["masked_account_number"] = num.substr(0, 3) + std::string(num.length() - 7, '*') + num.substr(num.length() - 4);
        } else {
            row["masked_account_number"] = num;
        }
    }
    return rows;
}

// ============================================================================
// DASHBOARD ANALYTICS (REAL DATABASE DATA, NO FAKE HARDCODING)
// ============================================================================
nlohmann::json Bank::getCustomerDashboard(int userId) {
    // 1. Customer summary metrics
    std::string summarySql = R"(
        SELECT 
            COALESCE(SUM(a.balance), 0) AS total_balance_paise,
            COUNT(CASE WHEN a.status = 'ACTIVE' THEN 1 END) AS active_accounts,
            COALESCE((
                SELECT SUM(t.amount) 
                FROM transactions t 
                JOIN accounts a2 ON t.account_id = a2.account_id 
                WHERE a2.user_id = ? AND t.transaction_type IN ('DEPOSIT', 'TRANSFER_IN')
            ), 0) AS total_deposits_paise,
            COALESCE((
                SELECT SUM(t.amount) 
                FROM transactions t 
                JOIN accounts a3 ON t.account_id = a3.account_id 
                WHERE a3.user_id = ? AND t.transaction_type IN ('WITHDRAWAL', 'TRANSFER_OUT')
            ), 0) AS total_withdrawals_paise
        FROM accounts a
        WHERE a.user_id = ?;
    )";
    nlohmann::json summaryRows = dbManager->queryJson(summarySql, {
        std::to_string(userId), std::to_string(userId), std::to_string(userId)
    });

    auto summary = summaryRows.empty() ? nlohmann::json::object() : summaryRows[0];
    long long totalBalance = summary.value("total_balance_paise", 0LL);
    long long activeAccs = summary.value("active_accounts", 0LL);
    long long totalDeposits = summary.value("total_deposits_paise", 0LL);
    long long totalWithdrawals = summary.value("total_withdrawals_paise", 0LL);

    // 2. User accounts list
    nlohmann::json accounts = getAccountsForUser(userId);

    // 3. Recent 5-10 transactions
    nlohmann::json recentTxns = getTransactions(userId, "CUSTOMER", "", "", "", 0, -1, "", 10, 0);

    // 4. Real Chart Analytics: Monthly income vs expense
    std::string monthlySql = R"(
        SELECT 
            strftime('%Y-%m', t.created_at) AS month,
            SUM(CASE WHEN t.transaction_type IN ('DEPOSIT', 'TRANSFER_IN') THEN t.amount ELSE 0 END) AS income_paise,
            SUM(CASE WHEN t.transaction_type IN ('WITHDRAWAL', 'TRANSFER_OUT') THEN t.amount ELSE 0 END) AS expense_paise
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        WHERE a.user_id = ?
        GROUP BY strftime('%Y-%m', t.created_at)
        ORDER BY month ASC
        LIMIT 6;
    )";
    nlohmann::json monthlyRows = dbManager->queryJson(monthlySql, {std::to_string(userId)});
    nlohmann::json monthlyChart = {
        {"labels", nlohmann::json::array()},
        {"income", nlohmann::json::array()},
        {"expense", nlohmann::json::array()}
    };
    for (const auto& m : monthlyRows) {
        monthlyChart["labels"].push_back(m["month"]);
        monthlyChart["income"].push_back(static_cast<double>(m["income_paise"].get<long long>()) / 100.0);
        monthlyChart["expense"].push_back(static_cast<double>(m["expense_paise"].get<long long>()) / 100.0);
    }

    // 5. Transaction Distribution (Doughnut chart)
    std::string distSql = R"(
        SELECT 
            t.transaction_type, 
            COUNT(*) AS count,
            SUM(t.amount) AS total_paise
        FROM transactions t
        JOIN accounts a ON t.account_id = a.account_id
        WHERE a.user_id = ?
        GROUP BY t.transaction_type;
    )";
    nlohmann::json distRows = dbManager->queryJson(distSql, {std::to_string(userId)});

    return {
        {"summary", {
            {"total_balance_paise", totalBalance},
            {"total_balance_rupees", static_cast<double>(totalBalance) / 100.0},
            {"active_accounts", activeAccs},
            {"total_deposits_paise", totalDeposits},
            {"total_deposits_rupees", static_cast<double>(totalDeposits) / 100.0},
            {"total_withdrawals_paise", totalWithdrawals},
            {"total_withdrawals_rupees", static_cast<double>(totalWithdrawals) / 100.0}
        }},
        {"accounts", accounts},
        {"recent_transactions", recentTxns["transactions"]},
        {"charts", {
            {"monthly_comparison", monthlyChart},
            {"distribution", distRows}
        }}
    };
}

nlohmann::json Bank::getAdminDashboard() {
    // 1. Core Bank Metrics
    std::string metricsSql = R"(
        SELECT 
            (SELECT COUNT(*) FROM users WHERE role = 'CUSTOMER') AS total_customers,
            (SELECT COUNT(*) FROM accounts WHERE status = 'ACTIVE') AS total_active_accounts,
            (SELECT COALESCE(SUM(balance), 0) FROM accounts) AS total_bank_balance_paise,
            (SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE transaction_type = 'DEPOSIT') AS total_deposits_paise,
            (SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE transaction_type = 'WITHDRAWAL') AS total_withdrawals_paise,
            (SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE transaction_type = 'TRANSFER_OUT') AS total_transfers_paise
    )";
    nlohmann::json metricRows = dbManager->queryJson(metricsSql);
    auto m = metricRows.empty() ? nlohmann::json::object() : metricRows[0];

    long long custCount = m.value("total_customers", 0LL);
    long long activeAccs = m.value("total_active_accounts", 0LL);
    long long bankBal = m.value("total_bank_balance_paise", 0LL);
    long long totalDep = m.value("total_deposits_paise", 0LL);
    long long totalWith = m.value("total_withdrawals_paise", 0LL);
    long long totalXfer = m.value("total_transfers_paise", 0LL);

    // 2. Recent Transactions (System-wide)
    nlohmann::json recentTxns = getTransactions(0, "ADMIN", "", "", "", 0, -1, "", 10, 0);

    // 3. Monthly Transaction Activity Chart
    std::string monthlySql = R"(
        SELECT 
            strftime('%Y-%m', created_at) AS month,
            SUM(CASE WHEN transaction_type = 'DEPOSIT' THEN amount ELSE 0 END) AS deposits_paise,
            SUM(CASE WHEN transaction_type = 'WITHDRAWAL' THEN amount ELSE 0 END) AS withdrawals_paise,
            SUM(CASE WHEN transaction_type IN ('TRANSFER_OUT', 'TRANSFER_IN') THEN amount / 2 ELSE 0 END) AS transfers_paise
        FROM transactions
        GROUP BY strftime('%Y-%m', created_at)
        ORDER BY month ASC
        LIMIT 6;
    )";
    nlohmann::json monthlyRows = dbManager->queryJson(monthlySql);
    nlohmann::json monthlyChart = {
        {"labels", nlohmann::json::array()},
        {"deposits", nlohmann::json::array()},
        {"withdrawals", nlohmann::json::array()},
        {"transfers", nlohmann::json::array()}
    };
    for (const auto& r : monthlyRows) {
        monthlyChart["labels"].push_back(r["month"]);
        monthlyChart["deposits"].push_back(static_cast<double>(r["deposits_paise"].get<long long>()) / 100.0);
        monthlyChart["withdrawals"].push_back(static_cast<double>(r["withdrawals_paise"].get<long long>()) / 100.0);
        monthlyChart["transfers"].push_back(static_cast<double>(r["transfers_paise"].get<long long>()) / 100.0);
    }

    // 4. Account Type Distribution
    std::string typeDistSql = R"(
        SELECT account_type, COUNT(*) as count, COALESCE(SUM(balance), 0) AS total_paise
        FROM accounts
        GROUP BY account_type;
    )";
    nlohmann::json typeDistRows = dbManager->queryJson(typeDistSql);

    // 5. Admin Logs
    std::string logsSql = R"(
        SELECT l.log_id, l.admin_id, u.full_name AS admin_name, l.action, l.description, l.created_at
        FROM admin_logs l
        JOIN users u ON l.admin_id = u.user_id
        ORDER BY l.log_id DESC LIMIT 10;
    )";
    nlohmann::json logs = dbManager->queryJson(logsSql);

    return {
        {"summary", {
            {"total_customers", custCount},
            {"total_active_accounts", activeAccs},
            {"total_bank_balance_paise", bankBal},
            {"total_bank_balance_rupees", static_cast<double>(bankBal) / 100.0},
            {"total_deposits_paise", totalDep},
            {"total_deposits_rupees", static_cast<double>(totalDep) / 100.0},
            {"total_withdrawals_paise", totalWith},
            {"total_withdrawals_rupees", static_cast<double>(totalWith) / 100.0},
            {"total_transfers_paise", totalXfer},
            {"total_transfers_rupees", static_cast<double>(totalXfer) / 100.0}
        }},
        {"recent_transactions", recentTxns["transactions"]},
        {"charts", {
            {"monthly_activity", monthlyChart},
            {"account_distribution", typeDistRows}
        }},
        {"admin_logs", logs}
    };
}

void Bank::logAdminAction(int adminId, const std::string& action, const std::string& desc) {
    std::string sql = R"(
        INSERT INTO admin_logs (admin_id, action, description)
        VALUES (?, ?, ?);
    )";
    dbManager->executeNonQuery(sql, {std::to_string(adminId), action, desc});
}
