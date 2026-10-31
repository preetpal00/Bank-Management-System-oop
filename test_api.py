#!/usr/bin/env python3
"""
Bank Management System - Automated End-to-End Test Suite
Tests REST APIs, Authentication, RBAC, Financial Calculations, Edge Cases, and Reports.
"""

import urllib.request
import urllib.error
import json
import sys

BASE_URL = "http://localhost:18080"
passed = 0
failed = 0

def log_test(name, success, details=""):
    global passed, failed
    if success:
        passed += 1
        print(f" [PASS] {name} {details}")
    else:
        failed += 1
        print(f"❌ [FAIL] {name} - {details}")

def request(method, path, data=None, token=None):
    url = f"{BASE_URL}{path}"
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    
    body = json.dumps(data).encode("utf-8") if data else None
    req = urllib.request.Request(url, data=body, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as res:
            res_body = res.read().decode("utf-8")
            try:
                json_data = json.loads(res_body)
            except Exception:
                json_data = res_body
            return res.status, json_data
    except urllib.error.HTTPError as e:
        err_body = e.read().decode("utf-8")
        try:
            json_err = json.loads(err_body)
        except Exception:
            json_err = err_body
        return e.code, json_err

def run_all_tests():
    print("=" * 70)
    print(" RUNNING AUTOMATED BANK MANAGEMENT SYSTEM TESTS")
    print("=" * 70)

    # 1. Static Pages Tests
    code, html = request("GET", "/")
    log_test("Static Route: GET / (Homepage)", code == 200 and "Apex Reserve" in html)

    code, html = request("GET", "/login")
    log_test("Static Route: GET /login", code == 200 and "Sign in" in html)

    code, css = request("GET", "/css/style.css")
    log_test("Static Route: GET /css/style.css", code == 200 and "--primary" in css)

    # 2. Authentication Tests
    # Invalid Login
    code, res = request("POST", "/api/auth/login", {"email": "admin@bank.com", "password": "WrongPassword!"})
    log_test("Auth: Invalid password rejected (401)", code == 401)

    # Valid Admin Login
    code, admin_login = request("POST", "/api/auth/login", {"email": "admin@bank.com", "password": "AdminPassword123!"})
    admin_token = admin_login.get("token") if code == 200 else None
    log_test("Auth: Admin valid login", code == 200 and admin_login.get("user", {}).get("role") == "ADMIN", f"Token: {admin_token[:10]}...")

    # Valid Customer Login (Rahul Sharma)
    code, cust_login = request("POST", "/api/auth/login", {"email": "rahul.sharma@example.com", "password": "Customer123!"})
    cust_token = cust_login.get("token") if code == 200 else None
    log_test("Auth: Customer valid login", code == 200 and cust_login.get("user", {}).get("role") == "CUSTOMER", f"User: Rahul")

    # Validate /api/auth/me
    code, me = request("GET", "/api/auth/me", token=cust_token)
    log_test("Auth: GET /api/auth/me with Bearer token", code == 200 and me.get("email") == "rahul.sharma@example.com")

    # 3. Role-Based Access Control (RBAC) Tests
    # Customer accessing Admin endpoint -> must be 403 Forbidden
    code, res = request("GET", "/api/customers", token=cust_token)
    log_test("RBAC: Customer blocked from Admin endpoint /api/customers (403)", code == 403)

    code, res = request("GET", "/api/dashboard/admin", token=cust_token)
    log_test("RBAC: Customer blocked from /api/dashboard/admin (403)", code == 403)

    # Admin accessing Admin endpoint -> must be 200 OK
    code, customers = request("GET", "/api/customers", token=admin_token)
    log_test("RBAC: Admin granted access to /api/customers (200)", code == 200 and isinstance(customers, list))

    code, admin_dash = request("GET", "/api/dashboard/admin", token=admin_token)
    log_test("RBAC: Admin granted access to /api/dashboard/admin (200)", code == 200 and "summary" in admin_dash)

    # 4. Customer Dashboard & Accounts
    code, cust_dash = request("GET", "/api/dashboard/customer", token=cust_token)
    log_test("Dashboard: Customer dashboard loads real metrics", code == 200 and "summary" in cust_dash and "charts" in cust_dash)

    code, accounts = request("GET", "/api/accounts", token=cust_token)
    log_test("Accounts: Customer retrieves personal accounts", code == 200 and len(accounts) >= 1)
    
    target_account = accounts[0]
    acc_id = target_account["account_id"]
    initial_balance = target_account["balance_rupees"]

    # 5. Financial Operations Tests
    # A. Deposit ₹1,000
    code, dep_res = request("POST", "/api/transactions/deposit", {
        "account_id": acc_id,
        "amount": 1000.0,
        "description": "Test Automated Deposit ₹1,000"
    }, token=cust_token)
    log_test("Banking: Deposit ₹1,000", code == 200 and dep_res.get("success"), f"Ref: {dep_res.get('reference_number')}")

    # B. Negative Deposit Attempt -> must fail
    code, neg_res = request("POST", "/api/transactions/deposit", {
        "account_id": acc_id,
        "amount": -500.0,
        "description": "Negative Deposit Attempt"
    }, token=cust_token)
    log_test("Banking: Negative deposit rejected (400)", code == 400)

    # C. Withdraw ₹500
    code, with_res = request("POST", "/api/transactions/withdraw", {
        "account_id": acc_id,
        "amount": 500.0,
        "description": "Test Automated Withdrawal ₹500"
    }, token=cust_token)
    log_test("Banking: Withdraw ₹500", code == 200 and with_res.get("success"), f"Ref: {with_res.get('reference_number')}")

    # D. Withdrawal exceeding balance -> must fail (400 Insufficient funds)
    code, exc_res = request("POST", "/api/transactions/withdraw", {
        "account_id": acc_id,
        "amount": 99999999.0,
        "description": "Excessive Withdrawal"
    }, token=cust_token)
    log_test("Banking: Excessive withdrawal blocked with InsufficientFundsException (400)", code == 400)

    # E. Fund Transfer ₹300 (Rahul to Priya Patel ACC10010002)
    code, xfer_res = request("POST", "/api/transactions/transfer", {
        "sender_account_id": acc_id,
        "receiver_account_number": "ACC10010002",
        "amount": 300.0,
        "description": "Payment for Freelance Project"
    }, token=cust_token)
    log_test("Banking: Atomic Fund Transfer ₹300 to ACC10010002", code == 200 and xfer_res.get("success"), f"Ref: {xfer_res.get('reference_number')}")

    # F. Transfer to invalid account -> 404
    code, inv_xfer = request("POST", "/api/transactions/transfer", {
        "sender_account_id": acc_id,
        "receiver_account_number": "ACC99999999",
        "amount": 100.0,
        "description": "Ghost Transfer"
    }, token=cust_token)
    log_test("Banking: Transfer to non-existent account rejected (404/400)", code in [400, 404])

    # G. Transfer to same account -> 400
    code, same_xfer = request("POST", "/api/transactions/transfer", {
        "sender_account_id": acc_id,
        "receiver_account_number": target_account["account_number"],
        "amount": 100.0,
        "description": "Self Loop Transfer"
    }, token=cust_token)
    log_test("Banking: Transfer to same account prevented (400)", code == 400)

    # 6. Transaction Filtering & Search
    code, txns = request("GET", "/api/transactions?type=DEPOSIT", token=cust_token)
    all_dep = all(t["transaction_type"] == "DEPOSIT" for t in txns.get("transactions", []))
    log_test("Transactions: Filter by DEPOSIT type", code == 200 and all_dep)

    code, txns_xfer = request("GET", "/api/transactions?type=TRANSFER", token=cust_token)
    all_xfer = all("TRANSFER" in t["transaction_type"] for t in txns_xfer.get("transactions", []))
    log_test("Transactions: Filter by TRANSFER type", code == 200 and all_xfer)

    # 7. Reports & CSV Export (Admin only)
    code, daily_rep = request("GET", "/api/reports/daily", token=admin_token)
    log_test("Reports: Admin daily audit report", code == 200 and "total_turnover_rupees" in daily_rep)

    code, monthly_rep = request("GET", "/api/reports/monthly", token=admin_token)
    log_test("Reports: Admin monthly turnover report", code == 200 and "total_volume_rupees" in monthly_rep)

    code, export_res = request("GET", "/api/reports/export", token=admin_token)
    log_test("Reports: Admin export transactions to CSV", code == 200 and export_res.get("download_url") == "/download/csv")

    code, csv_content = request("GET", "/download/csv")
    log_test("Reports: Download CSV file", code == 200 and "Reference Number,Account Number" in csv_content)

    # 8. Profile & Password Changes
    code, prof_res = request("PUT", "/api/profile", {
        "full_name": "Rahul Sharma (Verified)",
        "phone": "+91 98111 22334",
        "address": "42 MG Road, Bengaluru",
        "date_of_birth": "1992-08-20"
    }, token=cust_token)
    log_test("Profile: Update profile information", code == 200 and "Rahul Sharma (Verified)" in prof_res.get("name", ""))

    code, pwd_err = request("POST", "/api/profile/change-password", {
        "old_password": "WrongOldPassword",
        "new_password": "NewSecretPass123!"
    }, token=cust_token)
    log_test("Security: Password change with bad old password rejected (401)", code == 401)

    code, pwd_ok = request("POST", "/api/profile/change-password", {
        "old_password": "Customer123!",
        "new_password": "CustomerNewPass123!"
    }, token=cust_token)
    log_test("Security: Password change with valid credentials", code == 200 and pwd_ok.get("success"))

    # Reset password back for future tests
    request("POST", "/api/profile/change-password", {
        "old_password": "CustomerNewPass123!",
        "new_password": "Customer123!"
    }, token=cust_token)

    # 9. Logout Test
    code, logout_res = request("POST", "/api/auth/logout", token=cust_token)
    log_test("Auth: Logout successfully clears server session", code == 200)

    code, unauth_res = request("GET", "/api/auth/me", token=cust_token)
    log_test("Auth: Expired/logged-out token rejected (401)", code == 401)

    print("=" * 70)
    print(f" TEST RESULTS SUMMARY: {passed} PASSED, {failed} FAILED")
    print("=" * 70)

    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    run_all_tests()
