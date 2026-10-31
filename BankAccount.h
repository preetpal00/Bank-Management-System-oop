#pragma once

#include <string>
#include <iostream>
#include "json.hpp"
#include "Exceptions.h"

// ============================================================================
// OOP CONCEPT: Abstraction & Pure Virtual Functions (Abstract Class)
// BankAccount defines the common banking interface and state.
// Because deposit() and withdraw() are pure virtual (= 0), BankAccount
// cannot be directly instantiated, enforcing specialized derived account rules.
//
// OOP CONCEPT: Operator Overloading & Friend Function
// - friend void auditAccount(const BankAccount& account, std::ostream& os);
// - friend std::ostream& operator<<(std::ostream& os, const BankAccount& account);
// - bool operator==(const BankAccount& other) const;
//
// OOP CONCEPT: Static Data Members
// - static int totalAccountsInstantiated tracks in-memory account instances.
// ============================================================================

class BankAccount {
protected:
    int accountId;
    int userId;
    std::string accountNumber;
    long long balance;         // In integer paise (1 INR = 100 paise)
    std::string accountType;   // "SAVINGS", "CURRENT", "STUDENT"
    std::string status;        // "ACTIVE", "INACTIVE", "FROZEN", "CLOSED"
    std::string createdAt;
    std::string customerName;  // Populated when joined with user details

    // OOP CONCEPT: Static Data Member
    static int totalAccountsInstantiated;

public:
    // Constructors
    BankAccount();
    BankAccount(int accountId, int userId, const std::string& accNumber, 
                long long balancePaise, const std::string& type, 
                const std::string& status = "ACTIVE", const std::string& createdAt = "",
                const std::string& customerName = "");

    // OOP CONCEPT: Virtual Destructor for safe polymorphic cleanup
    virtual ~BankAccount();

    // OOP CONCEPT: Static Member Functions
    static int getTotalAccountsInstantiated();

    // Encapsulation: Getters
    int getAccountId() const { return accountId; }
    int getUserId() const { return userId; }
    std::string getAccountNumber() const { return accountNumber; }
    long long getBalance() const { return balance; }
    double getBalanceInRupees() const { return static_cast<double>(balance) / 100.0; }
    std::string getAccountType() const { return accountType; }
    std::string getStatus() const { return status; }
    std::string getCreatedAt() const { return createdAt; }
    std::string getCustomerName() const { return customerName; }

    // Encapsulation: Setters
    void setBalance(long long newBalance) { balance = newBalance; }
    void setStatus(const std::string& newStatus) { status = newStatus; }
    void setCustomerName(const std::string& name) { customerName = name; }

    // Masked account number helper (e.g. ACC10010001 -> ACC*****0001)
    std::string getMaskedAccountNumber() const;

    // OOP CONCEPT: Pure Virtual Functions (Runtime Polymorphism Interface)
    virtual bool deposit(long long amountPaise, const std::string& description) = 0;
    virtual bool withdraw(long long amountPaise, const std::string& description) = 0;
    virtual void displayAccountDetails() const = 0;

    // OOP CONCEPT: Function Overloading (Compile-Time Polymorphism)
    // Deposit or withdraw with default description
    bool deposit(long long amountPaise);
    bool withdraw(long long amountPaise);

    // Serialization
    virtual nlohmann::json toJson() const;

    // OOP CONCEPT: Operator Overloading
    bool operator==(const BankAccount& other) const;
    bool operator!=(const BankAccount& other) const;

    // OOP CONCEPT: Friend Functions
    // Friend function has access to protected members for auditing
    friend void auditAccount(const BankAccount& account, std::ostream& os);

    // Friend stream insertion operator
    friend std::ostream& operator<<(std::ostream& os, const BankAccount& account);
};
