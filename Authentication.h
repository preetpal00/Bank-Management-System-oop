#pragma once

#include <string>
#include <memory>
#include "DatabaseManager.h"
#include "json.hpp"

// ============================================================================
// OOP CONCEPT: Encapsulation & Security Management
// Authentication class handles PBKDF2 password hashing, salt generation,
// token creation, session validation, and user role authorization.
// ============================================================================

class Authentication {
private:
    std::shared_ptr<DatabaseManager> dbManager;

    // Cryptographic helper methods
    std::string generateRandomSalt(size_t length = 16);
    std::string hashPassword(const std::string& password, const std::string& salt);
    bool verifyPassword(const std::string& password, const std::string& storedHash);
    std::string generateSessionToken();

public:
    explicit Authentication(std::shared_ptr<DatabaseManager> db);

    // Register a new customer
    nlohmann::json registerCustomer(const std::string& fullName, const std::string& email,
                                    const std::string& phone, const std::string& address,
                                    const std::string& dob, const std::string& password);

    // Login for both Admin and Customer
    nlohmann::json login(const std::string& emailOrUsername, const std::string& password);

    // Token validation: returns user profile if token is valid and unexpired
    nlohmann::json validateSession(const std::string& token);

    // Logout
    bool logout(const std::string& token);

    // Change Password
    bool changePassword(int userId, const std::string& oldPassword, const std::string& newPassword);
};
