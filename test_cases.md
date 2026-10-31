# Test Cases & Verification Report: Bank Management System

## Test Summary
- **Total Test Cases Executed**: 31
- **Passed**: 31
- **Failed**: 0
- **Test Automation Suite**: [`tests/test_api.py`](file:///Users/preet/Downloads/Bank%20Management%20System/tests/test_api.py)

---

## 1. Authentication & Security Test Cases

| Test ID | Test Scenario | Input Data | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **AUTH-01** | Valid Admin Login | `admin@bank.com`, `AdminPassword123!` | HTTP 200, Session Token issued, role: `ADMIN` | HTTP 200, token issued | **PASS** |
| **AUTH-02** | Valid Customer Login | `rahul.sharma@example.com`, `Customer123!` | HTTP 200, Session Token issued, role: `CUSTOMER` | HTTP 200, token issued | **PASS** |
| **AUTH-03** | Invalid Password Attempt | `admin@bank.com`, `WrongPassword!` | HTTP 401 Unauthorized | HTTP 401 Unauthorized | **PASS** |
| **AUTH-04** | Non-existent User Login | `nobody@bank.com`, `Pass123!` | HTTP 401 Unauthorized | HTTP 401 Unauthorized | **PASS** |
| **AUTH-05** | Customer Registration | Valid customer registration payload | HTTP 201 Created, auto-creates Savings Account | HTTP 201, `ACC10010007` created | **PASS** |
| **AUTH-06** | Duplicate Email Registration | Existing email address | HTTP 400 Bad Request ("User already exists") | HTTP 400 Bad Request | **PASS** |
| **AUTH-07** | Session Token Validation | `GET /api/auth/me` with valid token | HTTP 200, returns active user details | HTTP 200 OK | **PASS** |
| **AUTH-08** | Session Invalidation on Logout| `POST /api/auth/logout` | Token removed from DB; subsequent call yields 401 | HTTP 200; next call 401 | **PASS** |

---

## 2. Role-Based Access Control (RBAC) Test Cases

| Test ID | Test Scenario | User Role | Targeted Route | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **RBAC-01** | Customer accesses Customer Directory | `CUSTOMER` | `GET /api/customers` | HTTP 403 Forbidden | **PASS** |
| **RBAC-02** | Customer accesses Admin Dashboard | `CUSTOMER` | `GET /api/dashboard/admin` | HTTP 403 Forbidden | **PASS** |
| **RBAC-03** | Customer accesses Admin Reports | `CUSTOMER` | `GET /api/reports/daily` | HTTP 403 Forbidden | **PASS** |
| **RBAC-04** | Admin accesses Customer Directory | `ADMIN` | `GET /api/customers` | HTTP 200 OK | **PASS** |
| **RBAC-05** | Admin accesses Admin Dashboard | `ADMIN` | `GET /api/dashboard/admin` | HTTP 200 OK | **PASS** |

---

## 3. Financial Banking Logic Test Cases

| Test ID | Test Scenario | Account / Amount | Business Rule Tested | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **FIN-01** | Valid Cash Deposit | `ACC10010001`, ₹1,000.00 | Balance incremented by exactly ₹1,000 | HTTP 200, `TXN...` reference generated | **PASS** |
| **FIN-02** | Negative Deposit Attempt | `ACC10010001`, -₹500.00 | Deposit amount must be $> 0$ | HTTP 400 Bad Request | **PASS** |
| **FIN-03** | Valid Cash Withdrawal | `ACC10010001`, ₹500.00 | Balance decremented by ₹500 | HTTP 200, balance updated | **PASS** |
| **FIN-04** | Excessive Withdrawal | `ACC10010001`, ₹99,999,999.00 | Exceeds balance + overdraft limit | HTTP 400 `InsufficientFundsException` | **PASS** |
| **FIN-05** | Minimum Balance Rule | Savings Account, balance ₹600, withdraw ₹200 | Violates ₹500 minimum balance | HTTP 400 `InsufficientFundsException` | **PASS** |
| **FIN-06** | Atomic Fund Transfer | From `ACC10010001` to `ACC10010002`, ₹300 | Both balances updated, 2 transactions + transfer_details logged in 1 SQL transaction | HTTP 200 OK | **PASS** |
| **FIN-07** | Transfer to Ghost Account | To `ACC99999999` | Beneficiary validation | HTTP 404 Beneficiary Not Found | **PASS** |
| **FIN-08** | Transfer to Same Account | `ACC10010001` to `ACC10010001` | Sender and receiver cannot be identical | HTTP 400 Bad Request | **PASS** |
| **FIN-09** | Unauthorized Account Access | Customer A accessing Customer B account | Customer data isolation | HTTP 403 / 400 Access Denied | **PASS** |

---

## 4. Reports, Exports & UI Test Cases

| Test ID | Test Scenario | Method / Component | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- |
| **REP-01** | Daily Audit Report | `GET /api/reports/daily` | Real database daily turnover and records | **PASS** |
| **REP-02** | Monthly Volume Report | `GET /api/reports/monthly` | Real itemized breakdown by transaction type | **PASS** |
| **REP-03** | CSV Generation & Download | `GET /api/reports/export` & `/download/csv` | Valid CSV stream written by `std::ofstream` | **PASS** |
| **UI-01** | Responsive Dashboard | Chart.js Bar & Doughnut | Renders with zero console errors | **PASS** |
| **UI-02** | Digital Receipt Modal | `viewReceipt('TXN...')` | Displays clean modal with transaction breakdown | **PASS** |
| **UI-03** | Mobile Drawer Navigation | Viewport $\le 992$px | Collapsible hamburger menu and backdrop | **PASS** |
