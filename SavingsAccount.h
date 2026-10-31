#pragma once

#include "BankAccount.h"

// ============================================================================
// OOP CONCEPT: Inheritance & Runtime Polymorphism (Derived Account Class)
// SavingsAccount provides concrete banking rules for savings accounts:
// 1. Enforces minimum balance requirement (e.g. ₹500.00 = 50,000 paise).
// 2. Calculates simulated annual interest (e.g. 4.0% p.a.).
// 3. Implements pure virtual methods deposit(), withdraw(), displayAccountDetails().
// ============================================================================

class SavingsAccount : public BankAccount {
private:
    long long minimumBalancePaise; // 50,000 paise = ₹500.00
    double annualInterestRate;     // 4.0%

public:
    SavingsAccount();
    SavingsAccount(int accountId, int userId, const std::string& accNumber, 
                   long long balancePaise, const std::string& status = "ACTIVE", 
                   const std::string& createdAt = "", const std::string& customerName = "",
                   long long minBalance = 50000, double interestRate = 4.0);

    // Getters for savings-specific attributes
    long long getMinimumBalance() const { return minimumBalancePaise; }
    double getMinimumBalanceInRupees() const { return static_cast<double>(minimumBalancePaise) / 100.0; }
    double getAnnualInterestRate() const { return annualInterestRate; }

    // OOP CONCEPT: Polymorphism / Virtual Function Overriding
    bool deposit(long long amountPaise, const std::string& description) override;
    bool withdraw(long long amountPaise, const std::string& description) override;
    void displayAccountDetails() const override;
    nlohmann::json toJson() const override;

    // Simulated interest calculation helper
    long long calculateSimulatedAnnualInterest() const;
};
