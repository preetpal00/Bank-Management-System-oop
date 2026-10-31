#include "BankAccount.h"
#include <iomanip>

// OOP CONCEPT: Initialization of Static Data Member
int BankAccount::totalAccountsInstantiated = 0;

BankAccount::BankAccount()
    : accountId(0), userId(0), accountNumber(""), balance(0), 
      accountType("SAVINGS"), status("ACTIVE"), createdAt(""), customerName("") {
    totalAccountsInstantiated++;
}

BankAccount::BankAccount(int accountId, int userId, const std::string& accNumber, 
                         long long balancePaise, const std::string& type, 
                         const std::string& status, const std::string& createdAt,
                         const std::string& customerName)
    : accountId(accountId), userId(userId), accountNumber(accNumber), 
      balance(balancePaise), accountType(type), status(status), 
      createdAt(createdAt), customerName(customerName) {
    totalAccountsInstantiated++;
}

BankAccount::~BankAccount() {
    if (totalAccountsInstantiated > 0) {
        totalAccountsInstantiated--;
    }
}

int BankAccount::getTotalAccountsInstantiated() {
    return totalAccountsInstantiated;
}

std::string BankAccount::getMaskedAccountNumber() const {
    if (accountNumber.length() <= 6) return accountNumber;
    std::string prefix = accountNumber.substr(0, 3);
    std::string suffix = accountNumber.substr(accountNumber.length() - 4);
    std::string masked = prefix + std::string(accountNumber.length() - 7, '*') + suffix;
    return masked;
}

// Compile-time polymorphism: Overloaded deposit
bool BankAccount::deposit(long long amountPaise) {
    return this->deposit(amountPaise, "Regular Account Deposit");
}

// Compile-time polymorphism: Overloaded withdraw
bool BankAccount::withdraw(long long amountPaise) {
    return this->withdraw(amountPaise, "Regular Account Withdrawal");
}

nlohmann::json BankAccount::toJson() const {
    return {
        {"account_id", accountId},
        {"user_id", userId},
        {"account_number", accountNumber},
        {"masked_account_number", getMaskedAccountNumber()},
        {"account_type", accountType},
        {"balance_paise", balance},
        {"balance_rupees", getBalanceInRupees()},
        {"status", status},
        {"created_at", createdAt},
        {"customer_name", customerName}
    };
}

// OOP CONCEPT: Operator Overloading (Equality comparison)
bool BankAccount::operator==(const BankAccount& other) const {
    return this->accountNumber == other.accountNumber;
}

bool BankAccount::operator!=(const BankAccount& other) const {
    return !(*this == other);
}

// OOP CONCEPT: Friend Function Implementation
// Friend function directly inspects account internal state for audit logs
void auditAccount(const BankAccount& account, std::ostream& os) {
    os << "[AUDIT] Account ID: " << account.accountId 
       << " | No: " << account.accountNumber 
       << " | Type: " << account.accountType
       << " | Balance (Paise): " << account.balance 
       << " (₹" << std::fixed << std::setprecision(2) << account.getBalanceInRupees() << ")"
       << " | Status: " << account.status;
}

// OOP CONCEPT: Friend Stream Insertion Operator Overloading
std::ostream& operator<<(std::ostream& os, const BankAccount& account) {
    os << "BankAccount(" << account.accountNumber << ", " 
       << account.accountType << ", ₹" 
       << std::fixed << std::setprecision(2) << account.getBalanceInRupees() << ")";
    return os;
}
