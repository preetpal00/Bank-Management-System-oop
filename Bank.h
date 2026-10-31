#pragma once

#include <string>
#include <vector>
#include <memory>
#include "DatabaseManager.h"
#include "BankAccount.h"
#include "SavingsAccount.h"
#include "CurrentAccount.h"
#include "Customer.h"
#include "Admin.h"
#include "Transaction.h"
#include "json.hpp"

// ============================================================================
// OOP CONCEPT: Composition, Association & System Coordination
// Bank is the central coordinator for all banking workflows.
// It manages customer profiles, polymorphic bank accounts, financial transactions,
// and ensures database ACID guarantees for deposits, withdrawals, and transfers.
// ============================================================================

class Bank {
private:
    std::string bankName;
    std::shared_ptr<DatabaseManager> dbManager;

    // Helper to generate unique reference numbers
    std::string generateReferenceNumber(const std::string& prefix = "TXN");
    std::string generateUniqueAccountNumber();

    // Factory method demonstrating runtime polymorphism
    std::shared_ptr<BankAccount> createAccountInstance(const nlohmann::json& row);

public:
    Bank(const std::string& name, std::shared_ptr<DatabaseManager> db);

    // Customer Operations
    nlohmann::json getAllCustomers();
    nlohmann::json getCustomerById(int customerId);
    nlohmann::json updateCustomer(int customerId, const std::string& name, 
                                  const std::string& phone, const std::string& address, 
                                  const std::string& dob);
    nlohmann::json setCustomerStatus(int customerId, const std::string& status);

    // Account Operations
    nlohmann::json getAllAccounts();
    nlohmann::json getAccountsForUser(int userId);
    nlohmann::json getAccountById(int accountId);
    nlohmann::json createAccount(int userId, const std::string& accountType, long long initialDepositPaise);
    nlohmann::json setAccountStatus(int accountId, const std::string& status);

    // Financial Operations (Pure C++ business logic & SQL transactions)
    nlohmann::json deposit(int userId, int accountId, long long amountPaise, const std::string& description);
    nlohmann::json withdraw(int userId, int accountId, long long amountPaise, const std::string& description);
    nlohmann::json transfer(int senderUserId, int senderAccountId, const std::string& receiverAccNumber, 
                            long long amountPaise, const std::string& description);

    // Transaction & Dashboard Queries
    nlohmann::json getTransactions(int userId, const std::string& role,
                                   const std::string& typeFilter = "",
                                   const std::string& startDate = "",
                                   const std::string& endDate = "",
                                   long long minPaise = 0,
                                   long long maxPaise = -1,
                                   const std::string& searchRef = "",
                                   int limit = 50, int offset = 0);
    nlohmann::json getTransactionByRef(const std::string& reference);

    nlohmann::json getCustomerDashboard(int userId);
    nlohmann::json getAdminDashboard();

    // Admin Activity Log Helper
    void logAdminAction(int adminId, const std::string& action, const std::string& desc);
};
