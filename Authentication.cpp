#include "Authentication.h"
#include <CommonCrypto/CommonKeyDerivation.h>
#include <CommonCrypto/CommonDigest.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

Authentication::Authentication(std::shared_ptr<DatabaseManager> db)
    : dbManager(db) {}

std::string Authentication::generateRandomSalt(size_t length) {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string salt;
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dist(generator)];
    }
    return salt;
}

std::string Authentication::hashPassword(const std::string& password, const std::string& salt) {
    unsigned char derivedKey[32];
    int rc = CCKeyDerivationPBKDF(
        kCCPBKDF2,
        password.c_str(), password.length(),
        reinterpret_cast<const uint8_t*>(salt.c_str()), salt.length(),
        kCCPRFHmacAlgSHA256,
        10000,
        derivedKey, sizeof(derivedKey)
    );

    if (rc != 0) {
        throw AuthenticationException("Error computing cryptographic password hash.");
    }

    std::stringstream ss;
    for (size_t i = 0; i < sizeof(derivedKey); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(derivedKey[i]);
    }
    return salt + "$" + ss.str();
}

bool Authentication::verifyPassword(const std::string& password, const std::string& storedHash) {
    size_t dollarPos = storedHash.find('$');
    if (dollarPos == std::string::npos) {
        return false;
    }
    std::string salt = storedHash.substr(0, dollarPos);
    std::string calculated = hashPassword(password, salt);
    return calculated == storedHash;
}

std::string Authentication::generateSessionToken() {
    static const char hexChars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::string token;
    for (int i = 0; i < 48; ++i) {
        token += hexChars[dis(gen)];
    }
    return token;
}

nlohmann::json Authentication::registerCustomer(const std::string& fullName, const std::string& email,
                                                const std::string& phone, const std::string& address,
                                                const std::string& dob, const std::string& password) {
    // Validation
    if (fullName.empty() || email.empty() || password.empty()) {
        throw BankingException("Full name, email, and password are required.");
    }
    if (password.length() < 6) {
        throw BankingException("Password must be at least 6 characters long.");
    }

    // Check if email already exists
    std::string checkSql = "SELECT user_id FROM users WHERE LOWER(email) = LOWER(?);";
    nlohmann::json existing = dbManager->queryJson(checkSql, {email});
    if (!existing.empty()) {
        throw BankingException("A user with this email address already exists.");
    }

    // Salt and hash
    std::string salt = generateRandomSalt(16);
    std::string hash = hashPassword(password, salt);

    std::string insertSql = R"(
        INSERT INTO users (full_name, email, phone, password_hash, role, address, date_of_birth, status)
        VALUES (?, ?, ?, ?, 'CUSTOMER', ?, ?, 'ACTIVE');
    )";

    dbManager->executeNonQuery(insertSql, {fullName, email, phone, hash, address, dob});
    long long newUserId = dbManager->getLastInsertId();

    // Automatically create a default Savings Account for newly registered customer
    // Format: ACC + (10010000 + user_id)
    std::string newAccNum = "ACC" + std::to_string(10010000 + newUserId);
    std::string accSql = R"(
        INSERT INTO accounts (user_id, account_number, account_type, balance, status)
        VALUES (?, ?, 'SAVINGS', 0, 'ACTIVE');
    )";
    dbManager->executeNonQuery(accSql, {std::to_string(newUserId), newAccNum});

    return {
        {"success", true},
        {"user_id", newUserId},
        {"full_name", fullName},
        {"email", email},
        {"account_number", newAccNum},
        {"message", "Customer registered successfully. Welcome to Modern Bank!"}
    };
}

nlohmann::json Authentication::login(const std::string& emailOrUsername, const std::string& password) {
    std::string sql = "SELECT user_id, full_name, email, role, status, password_hash FROM users WHERE LOWER(email) = LOWER(?);";
    nlohmann::json rows = dbManager->queryJson(sql, {emailOrUsername});

    if (rows.empty()) {
        throw AuthenticationException("Invalid email or password.");
    }

    auto user = rows[0];
    std::string storedHash = user["password_hash"].get<std::string>();
    std::string status = user["status"].get<std::string>();

    if (status != "ACTIVE") {
        throw AuthenticationException("Account is currently " + status + ". Please contact administration.");
    }

    if (!verifyPassword(password, storedHash)) {
        throw AuthenticationException("Invalid email or password.");
    }

    // Generate session token (valid for 24 hours)
    std::string token = generateSessionToken();
    std::string sessionSql = R"(
        INSERT INTO sessions (session_token, user_id, expires_at)
        VALUES (?, ?, datetime('now', '+1 day'));
    )";
    dbManager->executeNonQuery(sessionSql, {token, std::to_string(user["user_id"].get<long long>())});

    return {
        {"success", true},
        {"token", token},
        {"user", {
            {"user_id", user["user_id"]},
            {"name", user["full_name"]},
            {"email", user["email"]},
            {"role", user["role"]},
            {"status", user["status"]}
        }}
    };
}

nlohmann::json Authentication::validateSession(const std::string& token) {
    if (token.empty()) {
        return {{"valid", false}};
    }

    std::string sql = R"(
        SELECT s.session_token, u.user_id, u.full_name, u.email, u.phone, 
               u.role, u.status, u.address, u.date_of_birth, u.created_at
        FROM sessions s
        JOIN users u ON s.user_id = u.user_id
        WHERE s.session_token = ? AND datetime(s.expires_at) > datetime('now');
    )";

    nlohmann::json rows = dbManager->queryJson(sql, {token});
    if (rows.empty()) {
        return {{"valid", false}};
    }

    auto user = rows[0];
    if (user["status"].get<std::string>() != "ACTIVE") {
        return {{"valid", false}, {"reason", "Account is not active."}};
    }

    return {
        {"valid", true},
        {"user", {
            {"user_id", user["user_id"]},
            {"name", user["full_name"]},
            {"email", user["email"]},
            {"phone", user["phone"]},
            {"role", user["role"]},
            {"status", user["status"]},
            {"address", user["address"]},
            {"date_of_birth", user["date_of_birth"]},
            {"created_at", user["created_at"]}
        }}
    };
}

bool Authentication::logout(const std::string& token) {
    if (token.empty()) return true;
    std::string sql = "DELETE FROM sessions WHERE session_token = ?;";
    dbManager->executeNonQuery(sql, {token});
    return true;
}

bool Authentication::changePassword(int userId, const std::string& oldPassword, const std::string& newPassword) {
    if (newPassword.length() < 6) {
        throw BankingException("New password must be at least 6 characters long.");
    }

    std::string sql = "SELECT password_hash FROM users WHERE user_id = ?;";
    nlohmann::json rows = dbManager->queryJson(sql, {std::to_string(userId)});
    if (rows.empty()) {
        throw AuthenticationException("User not found.");
    }

    std::string storedHash = rows[0]["password_hash"].get<std::string>();
    if (!verifyPassword(oldPassword, storedHash)) {
        throw AuthenticationException("Current password is incorrect.");
    }

    std::string newSalt = generateRandomSalt(16);
    std::string newHash = hashPassword(newPassword, newSalt);

    std::string updateSql = "UPDATE users SET password_hash = ? WHERE user_id = ?;";
    dbManager->executeNonQuery(updateSql, {newHash, std::to_string(userId)});
    return true;
}
