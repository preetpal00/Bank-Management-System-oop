#include "CurrentAccount.h"
#include <iomanip>

CurrentAccount::CurrentAccount()
    : BankAccount(), overdraftLimitPaise(0) {
    this->accountType = "CURRENT";
}

CurrentAccount::CurrentAccount(int accountId, int userId, const std::string& accNumber, 
                               long long balancePaise, const std::string& status, 
                               const std::string& createdAt, const std::string& customerName,
                               long long overdraftPaise)
    : BankAccount(accountId, userId, accNumber, balancePaise, "CURRENT", status, createdAt, customerName),
      overdraftLimitPaise(overdraftPaise) {}

bool CurrentAccount::deposit(long long amountPaise, const std::string& description) {
    (void)description;
    if (amountPaise <= 0) {
        throw BankingException("Deposit amount must be strictly greater than zero.");
    }
    if (status != "ACTIVE") {
        throw InvalidAccountException("Cannot deposit to account: status is " + status);
    }
    this->balance += amountPaise;
    return true;
}

bool CurrentAccount::withdraw(long long amountPaise, const std::string& description) {
    (void)description;
    if (amountPaise <= 0) {
        throw BankingException("Withdrawal amount must be strictly greater than zero.");
    }
    if (status != "ACTIVE") {
        throw InvalidAccountException("Cannot withdraw from account: status is " + status);
    }
    // Current account rule: balance + overdraftLimit must cover withdrawal
    if (this->balance + this->overdraftLimitPaise < amountPaise) {
        throw InsufficientFundsException(
            "Withdrawal exceeds available balance and overdraft limit.", 
            amountPaise, 
            this->balance + this->overdraftLimitPaise
        );
    }
    this->balance -= amountPaise;
    return true;
}

void CurrentAccount::displayAccountDetails() const {
    std::cout << "[Current Account] " << accountNumber 
              << " | Holder: " << customerName
              << " | Balance: ₹" << std::fixed << std::setprecision(2) << getBalanceInRupees()
              << " | Overdraft: ₹" << getOverdraftLimitInRupees()
              << " | Status: " << status << std::endl;
}

nlohmann::json CurrentAccount::toJson() const {
    nlohmann::json j = BankAccount::toJson();
    j["overdraft_limit_rupees"] = getOverdraftLimitInRupees();
    return j;
}
