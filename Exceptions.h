#pragma once

#include <stdexcept>
#include <string>

// ============================================================================
// OOP CONCEPT: Exception Handling & Class Hierarchy
// Custom banking exceptions derived from std::runtime_error (inheritance).
// Provides meaningful error reporting and separation of error handling logic.
// ============================================================================

/**
 * @brief Base exception for all bank-related domain errors.
 */
class BankingException : public std::runtime_error {
public:
    explicit BankingException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when a withdrawal or transfer exceeds available funds/minimum balance.
 */
class InsufficientFundsException : public BankingException {
private:
    long long requestedAmount;
    long long availableBalance;

public:
    InsufficientFundsException(const std::string& message, long long requested, long long available)
        : BankingException(message), requestedAmount(requested), availableBalance(available) {}

    long long getRequestedAmount() const { return requestedAmount; }
    long long getAvailableBalance() const { return availableBalance; }
};

/**
 * @brief Thrown when an account is not found, inactive, or frozen.
 */
class InvalidAccountException : public BankingException {
public:
    explicit InvalidAccountException(const std::string& message)
        : BankingException(message) {}
};

/**
 * @brief Thrown on authentication or authorization failures (bad credentials, expired token).
 */
class AuthenticationException : public BankingException {
public:
    explicit AuthenticationException(const std::string& message)
        : BankingException(message) {}
};

/**
 * @brief Thrown on database-level execution or constraint errors.
 */
class DatabaseException : public BankingException {
public:
    explicit DatabaseException(const std::string& message)
        : BankingException(message) {}
};
