#define ASIO_STANDALONE 1
#include "crow_all.h"
#include "json.hpp"
#include "DatabaseManager.h"
#include "Authentication.h"
#include "Bank.h"
#include "ReportManager.h"
#include "Exceptions.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>

// Helper to set standard CORS headers
void setCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Accept");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
}

// JSON response builder
crow::response makeJsonResponse(int statusCode, const nlohmann::json& body) {
    crow::response res(statusCode, body.dump());
    res.add_header("Content-Type", "application/json");
    setCorsHeaders(res);
    return res;
}

// Helper to serve local static files (HTML, CSS, JS)
crow::response serveStaticFile(const std::string& relativePath, const std::string& mimeType) {
    std::ifstream file(relativePath, std::ios::binary);
    if (!file.is_open()) {
        nlohmann::json err = {{"error", "File not found: " + relativePath}};
        return makeJsonResponse(404, err);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    crow::response res(200, ss.str());
    res.add_header("Content-Type", mimeType);
    res.add_header("Cache-Control", "no-cache");
    setCorsHeaders(res);
    return res;
}

// Extract Bearer token from Crow request
std::string extractBearerToken(const crow::request& req) {
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty()) {
        return "";
    }
    if (authHeader.rfind("Bearer ", 0) == 0) {
        return authHeader.substr(7);
    }
    return authHeader;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  BANK MANAGEMENT SYSTEM - C++ REST API BACKEND" << std::endl;
    std::cout << "  BCA University Final Year Project" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Initialize core system components with smart pointers
    auto dbManager = std::make_shared<DatabaseManager>("database/bank_management.db");
    auto authManager = std::make_shared<Authentication>(dbManager);
    auto bank = std::make_shared<Bank>("Apex Reserve Bank of India", dbManager);
    auto reportManager = std::make_shared<ReportManager>(dbManager);

    crow::SimpleApp app;

    // Global OPTIONS handler for CORS preflight
    CROW_ROUTE(app, "/api/<path>")
        .methods(crow::HTTPMethod::Options)
    ([](const crow::request&, const std::string&) {
        crow::response res(200);
        setCorsHeaders(res);
        return res;
    });

    CROW_ROUTE(app, "/api/<path>/<path>")
        .methods(crow::HTTPMethod::Options)
    ([](const crow::request&, const std::string&, const std::string&) {
        crow::response res(200);
        setCorsHeaders(res);
        return res;
    });

    CROW_ROUTE(app, "/api/<path>/<path>/<path>")
        .methods(crow::HTTPMethod::Options)
    ([](const crow::request&, const std::string&, const std::string&, const std::string&) {
        crow::response res(200);
        setCorsHeaders(res);
        return res;
    });

    // ------------------------------------------------------------------------
    // STATIC FILE ROUTES (Serving the Frontend Directly from Backend Port 18080)
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/")([]() {
        return serveStaticFile("frontend/index.html", "text/html");
    });
    CROW_ROUTE(app, "/login")([]() {
        return serveStaticFile("frontend/login.html", "text/html");
    });
    CROW_ROUTE(app, "/register")([]() {
        return serveStaticFile("frontend/register.html", "text/html");
    });
    CROW_ROUTE(app, "/dashboard")([]() {
        return serveStaticFile("frontend/dashboard.html", "text/html");
    });
    CROW_ROUTE(app, "/accounts")([]() {
        return serveStaticFile("frontend/accounts.html", "text/html");
    });
    CROW_ROUTE(app, "/customers")([]() {
        return serveStaticFile("frontend/customers.html", "text/html");
    });
    CROW_ROUTE(app, "/transactions")([]() {
        return serveStaticFile("frontend/transactions.html", "text/html");
    });
    CROW_ROUTE(app, "/deposit")([]() {
        return serveStaticFile("frontend/deposit.html", "text/html");
    });
    CROW_ROUTE(app, "/withdraw")([]() {
        return serveStaticFile("frontend/withdraw.html", "text/html");
    });
    CROW_ROUTE(app, "/transfer")([]() {
        return serveStaticFile("frontend/transfer.html", "text/html");
    });
    CROW_ROUTE(app, "/reports")([]() {
        return serveStaticFile("frontend/reports.html", "text/html");
    });
    CROW_ROUTE(app, "/profile")([]() {
        return serveStaticFile("frontend/profile.html", "text/html");
    });

    // Static CSS & JS & Image handlers
    CROW_ROUTE(app, "/css/<string>")([](const std::string& file) {
        return serveStaticFile("frontend/css/" + file, "text/css");
    });
    CROW_ROUTE(app, "/js/<string>")([](const std::string& file) {
        return serveStaticFile("frontend/js/" + file, "application/javascript");
    });
    CROW_ROUTE(app, "/assets/images/<string>")([](const std::string& file) {
        std::string mime = "image/png";
        if (file.find(".svg") != std::string::npos) mime = "image/svg+xml";
        else if (file.find(".jpg") != std::string::npos || file.find(".jpeg") != std::string::npos) mime = "image/jpeg";
        return serveStaticFile("frontend/assets/images/" + file, mime);
    });

    // ------------------------------------------------------------------------
    // AUTHENTICATION REST APIS
    // ------------------------------------------------------------------------

    // POST /api/auth/register
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::Post)
    ([authManager](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            std::string name = body.value("full_name", "");
            std::string email = body.value("email", "");
            std::string phone = body.value("phone", "");
            std::string address = body.value("address", "");
            std::string dob = body.value("date_of_birth", "");
            std::string password = body.value("password", "");

            auto result = authManager->registerCustomer(name, email, phone, address, dob, password);
            return makeJsonResponse(201, result);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        } catch (const std::exception& e) {
            return makeJsonResponse(500, {{"error", "Internal server error: " + std::string(e.what())}});
        }
    });

    // POST /api/auth/login
    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::Post)
    ([authManager](const crow::request& req) {
        try {
            auto body = nlohmann::json::parse(req.body);
            std::string email = body.value("email", body.value("username", ""));
            std::string password = body.value("password", "");

            auto result = authManager->login(email, password);
            return makeJsonResponse(200, result);
        } catch (const AuthenticationException& e) {
            return makeJsonResponse(401, {{"error", e.what()}});
        } catch (const std::exception& e) {
            return makeJsonResponse(500, {{"error", "Internal server error: " + std::string(e.what())}});
        }
    });

    // POST /api/auth/logout
    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::Post)
    ([authManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        authManager->logout(token);
        return makeJsonResponse(200, {{"success", true}, {"message", "Logged out successfully."}});
    });

    // GET /api/auth/me
    CROW_ROUTE(app, "/api/auth/me").methods(crow::HTTPMethod::Get)
    ([authManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized: Invalid or expired session."}});
        }
        return makeJsonResponse(200, session["user"]);
    });

    // ------------------------------------------------------------------------
    // DASHBOARD APIS
    // ------------------------------------------------------------------------

    // GET /api/dashboard/customer
    CROW_ROUTE(app, "/api/dashboard/customer").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        int userId = static_cast<int>(session["user"]["user_id"].get<long long>());
        auto data = bank->getCustomerDashboard(userId);
        return makeJsonResponse(200, data);
    });

    // GET /api/dashboard/admin
    CROW_ROUTE(app, "/api/dashboard/admin").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        if (session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Administrative access required."}});
        }
        auto data = bank->getAdminDashboard();
        return makeJsonResponse(200, data);
    });

    // ------------------------------------------------------------------------
    // CUSTOMER MANAGEMENT APIS
    // ------------------------------------------------------------------------

    // GET /api/customers
    CROW_ROUTE(app, "/api/customers").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }
        auto list = bank->getAllCustomers();
        return makeJsonResponse(200, list);
    });

    // GET /api/customers/{id}
    CROW_ROUTE(app, "/api/customers/<int>").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req, int customerId) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        // Allow customer to see self, or admin to see any
        int loggedUserId = static_cast<int>(session["user"]["user_id"].get<long long>());
        std::string role = session["user"]["role"].get<std::string>();
        if (role != "ADMIN" && loggedUserId != customerId) {
            return makeJsonResponse(403, {{"error", "Forbidden: Access denied."}});
        }

        try {
            auto cust = bank->getCustomerById(customerId);
            return makeJsonResponse(200, cust);
        } catch (const BankingException& e) {
            return makeJsonResponse(404, {{"error", e.what()}});
        }
    });

    // POST /api/customers (Admin adds customer)
    CROW_ROUTE(app, "/api/customers").methods(crow::HTTPMethod::Post)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string name = body.value("full_name", "");
            std::string email = body.value("email", "");
            std::string phone = body.value("phone", "");
            std::string address = body.value("address", "");
            std::string dob = body.value("date_of_birth", "");
            std::string password = body.value("password", "Customer123!");

            auto res = authManager->registerCustomer(name, email, phone, address, dob, password);
            int adminId = static_cast<int>(session["user"]["user_id"].get<long long>());
            bank->logAdminAction(adminId, "CUSTOMER_CREATE", "Admin created customer: " + name + " (" + email + ")");
            return makeJsonResponse(201, res);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // PUT /api/customers/{id}
    CROW_ROUTE(app, "/api/customers/<int>").methods(crow::HTTPMethod::Put)
    ([authManager, bank](const crow::request& req, int customerId) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string name = body.value("full_name", "");
            std::string phone = body.value("phone", "");
            std::string address = body.value("address", "");
            std::string dob = body.value("date_of_birth", "");

            auto updated = bank->updateCustomer(customerId, name, phone, address, dob);
            int adminId = static_cast<int>(session["user"]["user_id"].get<long long>());
            bank->logAdminAction(adminId, "CUSTOMER_UPDATE", "Updated details for customer ID: " + std::to_string(customerId));
            return makeJsonResponse(200, updated);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // PATCH /api/customers/{id}/status
    CROW_ROUTE(app, "/api/customers/<int>/status").methods(crow::HTTPMethod::Patch)
    ([authManager, bank](const crow::request& req, int customerId) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string status = body.value("status", "ACTIVE");
            auto res = bank->setCustomerStatus(customerId, status);
            int adminId = static_cast<int>(session["user"]["user_id"].get<long long>());
            bank->logAdminAction(adminId, "CUSTOMER_STATUS", "Set customer ID " + std::to_string(customerId) + " status to " + status);
            return makeJsonResponse(200, res);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // ------------------------------------------------------------------------
    // ACCOUNT MANAGEMENT APIS
    // ------------------------------------------------------------------------

    // GET /api/accounts
    CROW_ROUTE(app, "/api/accounts").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        std::string role = session["user"]["role"].get<std::string>();
        int userId = static_cast<int>(session["user"]["user_id"].get<long long>());

        if (role == "ADMIN") {
            auto accounts = bank->getAllAccounts();
            return makeJsonResponse(200, accounts);
        } else {
            auto accounts = bank->getAccountsForUser(userId);
            return makeJsonResponse(200, accounts);
        }
    });

    // GET /api/accounts/{id}
    CROW_ROUTE(app, "/api/accounts/<int>").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req, int accountId) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto acc = bank->getAccountById(accountId);
            int loggedUserId = static_cast<int>(session["user"]["user_id"].get<long long>());
            std::string role = session["user"]["role"].get<std::string>();

            if (role != "ADMIN" && acc["user_id"].get<long long>() != loggedUserId) {
                return makeJsonResponse(403, {{"error", "Forbidden: Access denied to this account."}});
            }
            return makeJsonResponse(200, acc);
        } catch (const BankingException& e) {
            return makeJsonResponse(404, {{"error", e.what()}});
        }
    });

    // POST /api/accounts (Create account)
    CROW_ROUTE(app, "/api/accounts").methods(crow::HTTPMethod::Post)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string role = session["user"]["role"].get<std::string>();
            int userId = static_cast<int>(session["user"]["user_id"].get<long long>());

            // Admin can specify user_id; customer can only create for self
            if (role == "ADMIN" && body.contains("user_id")) {
                userId = body["user_id"].get<int>();
            }

            std::string accType = body.value("account_type", "SAVINGS");
            double initRupees = body.value("initial_deposit", 0.0);
            long long initPaise = static_cast<long long>(initRupees * 100.0);

            auto newAcc = bank->createAccount(userId, accType, initPaise);
            if (role == "ADMIN") {
                int adminId = static_cast<int>(session["user"]["user_id"].get<long long>());
                bank->logAdminAction(adminId, "ACCOUNT_CREATE", "Created account " + newAcc["account_number"].get<std::string>() + " for user " + std::to_string(userId));
            }
            return makeJsonResponse(201, newAcc);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // PATCH /api/accounts/{id}/status
    CROW_ROUTE(app, "/api/accounts/<int>/status").methods(crow::HTTPMethod::Patch)
    ([authManager, bank](const crow::request& req, int accountId) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string status = body.value("status", "ACTIVE");
            auto res = bank->setAccountStatus(accountId, status);
            int adminId = static_cast<int>(session["user"]["user_id"].get<long long>());
            bank->logAdminAction(adminId, "ACCOUNT_STATUS", "Set account ID " + std::to_string(accountId) + " status to " + status);
            return makeJsonResponse(200, res);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // ------------------------------------------------------------------------
    // TRANSACTION APIS (DEPOSIT, WITHDRAW, TRANSFER, HISTORY)
    // ------------------------------------------------------------------------

    // POST /api/transactions/deposit
    CROW_ROUTE(app, "/api/transactions/deposit").methods(crow::HTTPMethod::Post)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            int accountId = body.value("account_id", 0);
            double amountRupees = body.value("amount", 0.0);
            std::string desc = body.value("description", "Deposit");

            if (accountId <= 0 || amountRupees <= 0) {
                return makeJsonResponse(400, {{"error", "Invalid account ID or amount."}});
            }

            long long paise = static_cast<long long>(amountRupees * 100.0);
            int userId = (session["user"]["role"].get<std::string>() == "ADMIN") 
                         ? 0 : static_cast<int>(session["user"]["user_id"].get<long long>());

            auto res = bank->deposit(userId, accountId, paise, desc);
            return makeJsonResponse(200, res);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        } catch (const std::exception& e) {
            return makeJsonResponse(500, {{"error", e.what()}});
        }
    });

    // POST /api/transactions/withdraw
    CROW_ROUTE(app, "/api/transactions/withdraw").methods(crow::HTTPMethod::Post)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            int accountId = body.value("account_id", 0);
            double amountRupees = body.value("amount", 0.0);
            std::string desc = body.value("description", "Withdrawal");

            if (accountId <= 0 || amountRupees <= 0) {
                return makeJsonResponse(400, {{"error", "Invalid account ID or amount."}});
            }

            long long paise = static_cast<long long>(amountRupees * 100.0);
            int userId = (session["user"]["role"].get<std::string>() == "ADMIN") 
                         ? 0 : static_cast<int>(session["user"]["user_id"].get<long long>());

            auto res = bank->withdraw(userId, accountId, paise, desc);
            return makeJsonResponse(200, res);
        } catch (const InsufficientFundsException& e) {
            return makeJsonResponse(400, {
                {"error", e.what()},
                {"requested_rupees", static_cast<double>(e.getRequestedAmount()) / 100.0},
                {"available_rupees", static_cast<double>(e.getAvailableBalance()) / 100.0}
            });
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        } catch (const std::exception& e) {
            return makeJsonResponse(500, {{"error", e.what()}});
        }
    });

    // POST /api/transactions/transfer
    CROW_ROUTE(app, "/api/transactions/transfer").methods(crow::HTTPMethod::Post)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            int senderAccId = body.value("sender_account_id", 0);
            std::string receiverAccNum = body.value("receiver_account_number", "");
            double amountRupees = body.value("amount", 0.0);
            std::string desc = body.value("description", "Fund Transfer");

            if (senderAccId <= 0 || receiverAccNum.empty() || amountRupees <= 0) {
                return makeJsonResponse(400, {{"error", "Sender account, receiver account number, and amount (>0) are required."}});
            }

            long long paise = static_cast<long long>(amountRupees * 100.0);
            int userId = (session["user"]["role"].get<std::string>() == "ADMIN") 
                         ? 0 : static_cast<int>(session["user"]["user_id"].get<long long>());

            auto res = bank->transfer(userId, senderAccId, receiverAccNum, paise, desc);
            return makeJsonResponse(200, res);
        } catch (const InsufficientFundsException& e) {
            return makeJsonResponse(400, {
                {"error", e.what()},
                {"requested_rupees", static_cast<double>(e.getRequestedAmount()) / 100.0},
                {"available_rupees", static_cast<double>(e.getAvailableBalance()) / 100.0}
            });
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        } catch (const std::exception& e) {
            return makeJsonResponse(500, {{"error", e.what()}});
        }
    });

    // GET /api/transactions
    CROW_ROUTE(app, "/api/transactions").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        int userId = static_cast<int>(session["user"]["user_id"].get<long long>());
        std::string role = session["user"]["role"].get<std::string>();

        // Query parameters
        std::string type = req.url_params.get("type") ? req.url_params.get("type") : "";
        std::string start = req.url_params.get("start_date") ? req.url_params.get("start_date") : "";
        std::string end = req.url_params.get("end_date") ? req.url_params.get("end_date") : "";
        std::string search = req.url_params.get("search") ? req.url_params.get("search") : "";

        double minR = req.url_params.get("min_amount") ? std::stod(req.url_params.get("min_amount")) : 0.0;
        double maxR = req.url_params.get("max_amount") ? std::stod(req.url_params.get("max_amount")) : -1.0;
        int limit = req.url_params.get("limit") ? std::stoi(req.url_params.get("limit")) : 50;
        int offset = req.url_params.get("offset") ? std::stoi(req.url_params.get("offset")) : 0;

        long long minP = static_cast<long long>(minR * 100.0);
        long long maxP = maxR >= 0 ? static_cast<long long>(maxR * 100.0) : -1LL;

        auto result = bank->getTransactions(userId, role, type, start, end, minP, maxP, search, limit, offset);
        return makeJsonResponse(200, result);
    });

    // GET /api/transactions/{reference}
    CROW_ROUTE(app, "/api/transactions/<string>").methods(crow::HTTPMethod::Get)
    ([authManager, bank](const crow::request& req, const std::string& ref) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }

        try {
            auto rows = bank->getTransactionByRef(ref);
            return makeJsonResponse(200, rows);
        } catch (const BankingException& e) {
            return makeJsonResponse(404, {{"error", e.what()}});
        }
    });

    // ------------------------------------------------------------------------
    // REPORTS APIS
    // ------------------------------------------------------------------------

    // GET /api/reports/daily
    CROW_ROUTE(app, "/api/reports/daily").methods(crow::HTTPMethod::Get)
    ([authManager, reportManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }
        std::string d = req.url_params.get("date") ? req.url_params.get("date") : "";
        auto rep = reportManager->getDailyReport(d);
        return makeJsonResponse(200, rep);
    });

    // GET /api/reports/monthly
    CROW_ROUTE(app, "/api/reports/monthly").methods(crow::HTTPMethod::Get)
    ([authManager, reportManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }
        std::string m = req.url_params.get("month") ? req.url_params.get("month") : "";
        auto rep = reportManager->getMonthlyReport(m);
        return makeJsonResponse(200, rep);
    });

    // GET /api/reports/accounts
    CROW_ROUTE(app, "/api/reports/accounts").methods(crow::HTTPMethod::Get)
    ([authManager, reportManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }
        auto rep = reportManager->getAccountSummaryReport();
        return makeJsonResponse(200, rep);
    });

    // GET /api/reports/export (Generates CSV and returns download URL / content)
    CROW_ROUTE(app, "/api/reports/export").methods(crow::HTTPMethod::Get)
    ([authManager, reportManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>() || session["user"]["role"].get<std::string>() != "ADMIN") {
            return makeJsonResponse(403, {{"error", "Forbidden: Admin privileges required."}});
        }
        try {
            std::string path = reportManager->exportTransactionsToCsv("database/transactions_export.csv");
            return makeJsonResponse(200, {
                {"success", true},
                {"download_url", "/download/csv"},
                {"file_path", path},
                {"message", "CSV Report generated successfully."}
            });
        } catch (const BankingException& e) {
            return makeJsonResponse(500, {{"error", e.what()}});
        }
    });

    // GET /download/csv
    CROW_ROUTE(app, "/download/csv")([]() {
        return serveStaticFile("database/transactions_export.csv", "text/csv");
    });

    // ------------------------------------------------------------------------
    // PROFILE APIS
    // ------------------------------------------------------------------------

    // GET /api/profile
    CROW_ROUTE(app, "/api/profile").methods(crow::HTTPMethod::Get)
    ([authManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        return makeJsonResponse(200, session["user"]);
    });

    // PUT /api/profile
    CROW_ROUTE(app, "/api/profile").methods(crow::HTTPMethod::Put)
    ([authManager, bank](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        try {
            auto body = nlohmann::json::parse(req.body);
            int userId = static_cast<int>(session["user"]["user_id"].get<long long>());
            std::string name = body.value("full_name", session["user"]["name"].get<std::string>());
            std::string phone = body.value("phone", session["user"]["phone"].get<std::string>());
            std::string address = body.value("address", session["user"]["address"].get<std::string>());
            std::string dob = body.value("date_of_birth", session["user"]["date_of_birth"].get<std::string>());

            auto updated = bank->updateCustomer(userId, name, phone, address, dob);
            return makeJsonResponse(200, updated);
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    // POST /api/profile/change-password
    CROW_ROUTE(app, "/api/profile/change-password").methods(crow::HTTPMethod::Post)
    ([authManager](const crow::request& req) {
        std::string token = extractBearerToken(req);
        auto session = authManager->validateSession(token);
        if (!session["valid"].get<bool>()) {
            return makeJsonResponse(401, {{"error", "Unauthorized"}});
        }
        try {
            auto body = nlohmann::json::parse(req.body);
            int userId = static_cast<int>(session["user"]["user_id"].get<long long>());
            std::string oldPass = body.value("old_password", "");
            std::string newPass = body.value("new_password", "");

            authManager->changePassword(userId, oldPass, newPass);
            return makeJsonResponse(200, {{"success", true}, {"message", "Password changed successfully."}});
        } catch (const AuthenticationException& e) {
            return makeJsonResponse(401, {{"error", e.what()}});
        } catch (const BankingException& e) {
            return makeJsonResponse(400, {{"error", e.what()}});
        }
    });

    std::cout << "[Server] Modern Banking REST API starting on port 18080..." << std::endl;
    std::cout << "[Server] Access Frontend at: http://localhost:18080" << std::endl;
    app.port(18080).multithreaded().run();

    return 0;
}
