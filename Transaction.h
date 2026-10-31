#pragma once

#include <string>
#include <iostream>
#include "json.hpp"

// ============================================================================
// OOP CONCEPT: Encapsulation & Operator Overloading
// Transaction encapsulates all financial movement events.
// Implements operator< for chronological and priority sorting.
// ============================================================================

class Transaction {
private:
    int transactionId;
    std::string referenceNumber;
    int accountId;
    std::string accountNumber;
    std::string transactionType; // DEPOSIT, WITHDRAWAL, TRANSFER_IN, TRANSFER_OUT, ADJUSTMENT
    long long amountPaise;       // In integer paise
    std::string description;
    std::string status;          // SUCCESS, FAILED, PENDING
    std::string createdAt;

public:
    Transaction();
    Transaction(int id, const std::string& ref, int accId, 
                const std::string& type, long long amount, 
                const std::string& desc, const std::string& status = "SUCCESS", 
                const std::string& createdAt = "", const std::string& accNum = "");

    // Encapsulation: Getters
    int getTransactionId() const { return transactionId; }
    std::string getReferenceNumber() const { return referenceNumber; }
    int getAccountId() const { return accountId; }
    std::string getAccountNumber() const { return accountNumber; }
    std::string getTransactionType() const { return transactionType; }
    long long getAmount() const { return amountPaise; }
    double getAmountInRupees() const { return static_cast<double>(amountPaise) / 100.0; }
    std::string getDescription() const { return description; }
    std::string getStatus() const { return status; }
    std::string getCreatedAt() const { return createdAt; }

    void setAccountNumber(const std::string& accNum) { accountNumber = accNum; }

    void displayTransaction() const;
    nlohmann::json toJson() const;

    // OOP CONCEPT: Operator Overloading (< for chronological sorting)
    bool operator<(const Transaction& other) const;
};
