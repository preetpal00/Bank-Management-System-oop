#include "SavingsAccount.h"
#include <iomanip>

SavingsAccount::SavingsAccount()
    : BankAccount(), minimumBalancePaise(50000), annualInterestRate(4.0) {
    this->accountType = "SAVINGS";
}

SavingsAccount::SavingsAccount(int accountId, int userId, const std::string& accNumber, 
                               long long balancePaise, const std::string& status, 
                               const std::string& createdAt, const std::string& customerName,
                               long long minBalance, double interestRate)
    : BankAccount(accountId, userId, accNumber, balancePaise, "SAVINGS", status, createdAt, customerName),
      minimumBalancePaise(minBalance), annualInterestRate(interestRate) {}

bool SavingsAccount::deposit(long long amountPaise, const std::string& description) {
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

bool SavingsAccount::withdraw(long long amountPaise, const std::string& description) {
    (void)description;
    if (amountPaise <= 0) {
        throw BankingException("Withdrawal amount must be strictly greater than zero.");
    }
    if (status != "ACTIVE") {
        throw InvalidAccountException("Cannot withdraw from account: status is " + status);
    }
    // Savings account rule: Must maintain minimum balance
    if (this->balance - amountPaise < this->minimumBalancePaise) {
        throw InsufficientFundsException(
            "Withdrawal violates minimum balance requirement of ₹" + 
            std::to_string(minimumBalancePaise / 100), 
            amountPaise, 
            this->balance
        );
    }
    this->balance -= amountPaise;
    return true;
}

void SavingsAccount::displayAccountDetails() const {
    std::cout << "[Savings Account] " << accountNumber 
              << " | Holder: " << customerName
              << " | Balance: ₹" << std::fixed << std::setprecision(2) << getBalanceInRupees()
              << " | Min Balance: ₹" << getMinimumBalanceInRupees()
              << " | Interest: " << annualInterestRate << "%"
              << " | Status: " << status << std::endl;
}

long long SavingsAccount::calculateSimulatedAnnualInterest() const {
    return static_cast<long long>(balance * (annualInterestRate / 100.0));
}

nlohmann::json SavingsAccount::toJson() const {
    nlohmann::json j = BankAccount::toJson();
    j["minimum_balance_rupees"] = getMinimumBalanceInRupees();
    j["annual_interest_rate"] = annualInterestRate;
    j["estimated_annual_interest_rupees"] = static_cast<double>(calculateSimulatedAnnualInterest()) / 100.0;
    return j;
}
