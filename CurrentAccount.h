#pragma once

#include "BankAccount.h"

// ============================================================================
// OOP CONCEPT: Inheritance & Runtime Polymorphism (Derived Account Class)
// CurrentAccount provides commercial banking rules:
// 1. Minimum balance is zero (flexible working capital).
// 2. Supports optional authorized overdraft limit.
// 3. Implements pure virtual methods deposit(), withdraw(), displayAccountDetails().
// ============================================================================

class CurrentAccount : public BankAccount {
private:
    long long overdraftLimitPaise; // Overdraft allowance in paise (default: 0)

public:
    CurrentAccount();
    CurrentAccount(int accountId, int userId, const std::string& accNumber, 
                   long long balancePaise, const std::string& status = "ACTIVE", 
                   const std::string& createdAt = "", const std::string& customerName = "",
                   long long overdraftPaise = 0);

    long long getOverdraftLimit() const { return overdraftLimitPaise; }
    double getOverdraftLimitInRupees() const { return static_cast<double>(overdraftLimitPaise) / 100.0; }
    void setOverdraftLimit(long long limit) { overdraftLimitPaise = limit; }

    // OOP CONCEPT: Polymorphism / Virtual Function Overriding
    bool deposit(long long amountPaise, const std::string& description) override;
    bool withdraw(long long amountPaise, const std::string& description) override;
    void displayAccountDetails() const override;
    nlohmann::json toJson() const override;
};
