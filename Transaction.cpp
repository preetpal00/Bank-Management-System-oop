#include "Transaction.h"
#include <iomanip>

Transaction::Transaction()
    : transactionId(0), referenceNumber(""), accountId(0), accountNumber(""),
      transactionType("DEPOSIT"), amountPaise(0), description(""), 
      status("SUCCESS"), createdAt("") {}

Transaction::Transaction(int id, const std::string& ref, int accId, 
                         const std::string& type, long long amount, 
                         const std::string& desc, const std::string& status, 
                         const std::string& createdAt, const std::string& accNum)
    : transactionId(id), referenceNumber(ref), accountId(accId), accountNumber(accNum),
      transactionType(type), amountPaise(amount), description(desc), 
      status(status), createdAt(createdAt) {}

void Transaction::displayTransaction() const {
    std::cout << "[TXN: " << referenceNumber << "] " 
              << transactionType << " | Account: " << accountNumber 
              << " | ₹" << std::fixed << std::setprecision(2) << getAmountInRupees() 
              << " | " << description 
              << " | " << status 
              << " | " << createdAt << std::endl;
}

nlohmann::json Transaction::toJson() const {
    return {
        {"transaction_id", transactionId},
        {"reference_number", referenceNumber},
        {"account_id", accountId},
        {"account_number", accountNumber},
        {"transaction_type", transactionType},
        {"amount_paise", amountPaise},
        {"amount_rupees", getAmountInRupees()},
        {"description", description},
        {"status", status},
        {"created_at", createdAt}
    };
}

// Chronological comparison
bool Transaction::operator<(const Transaction& other) const {
    return this->createdAt < other.createdAt;
}
