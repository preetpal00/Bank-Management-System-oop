# Bank Management System (BMS)
### Full-Stack Digital Banking Web Application Using C++17, Crow, SQLite3, HTML5, CSS3, & Vanilla JavaScript

A complete, professional-grade Bank Management System designed for a **Bachelor of Computer Applications (BCA)** final-year project.

This is a real, working full-stack application featuring a compiled **C++17** REST API backend, **SQLite3** relational database with atomic transactions, and a modern responsive **HTML5/CSS3/Vanilla JS** banking dashboard powered by **Chart.js**.

---

## 1. Technology Architecture

### Frontend
- **HTML5**: Semantic document structure for banking interfaces.
- **CSS3**: Custom design system featuring a premium Navy & Teal palette (`#142D4E`, `#245B91`, `#36A6A0`, `#F4F7FB`), responsive cards, CSS grid, and mobile drawer sidebar.
- **Vanilla JavaScript (ES6+)**: Fetch API for asynchronous communication, dynamic token injection, and DOM management. *(No heavy frameworks like React/Vue/Angular).*
- **Chart.js**: Real-time rendering of monthly income vs. expense bar charts, transaction doughnut breakdown, and activity metrics.
- **Font Awesome 6**: Banking and fintech vector icons.

### Backend
- **C++17**: High-performance, memory-safe compiled core banking logic.
- **Crow C++**: Lightweight, fast C++ microservice HTTP framework running locally on **port 18080**.
- **nlohmann/json**: Modern JSON serialization and deserialization.
- **Asio (Standalone)**: Header-only asynchronous networking (Boost-free).
- **CommonCrypto / PBKDF2**: Cryptographic password hashing (10,000 iterations with random salt).
- **CMake & Make**: Universal build configurations.

### Database
- **SQLite3**: Stored in a physical `.db` file at [`database/bank_management.db`](file:///Users/preet/Downloads/Bank%20Management%20System/database/bank_management.db).
- **Currency Storage**: Stored in **integer paise** (1 INR = 100 paise) to eliminate floating-point precision loss.
- **Data Integrity**: Enforces foreign keys (`PRAGMA foreign_keys = ON`), prepared statements, and atomic SQL transactions.

---

## 2. Key Features

### For Customers
- **Personalized Dashboard**: Real-time balance cards, active account count, total deposits, and withdrawals.
- **Visual Debit Cards**: ATM-style debit cards displaying masked account numbers and current balance.
- **Deposit Simulation**: Instant deposit into Savings, Current, or Student accounts with quick-amount chips.
- **Withdraw Money**: Balance check and automatic enforcement of minimum balance rules (e.g. ₹500 for Savings).
- **Fund Transfer**: Atomic inter-account transfer with rollback if either step fails.
- **Transaction Statement**: Search by reference, filter by transaction type, date range, and amount.
- **Digital Receipts**: Instant modal preview with transaction status, reference, and breakdown.
- **Personal Profile**: Update contact details and change password with cryptographic hash verification.

### For Administrators
- **Executive Bank Dashboard**: Total simulated reserves across all accounts, customer count, active accounts, and total transfers.
- **Customer Directory**: Add new customers, search customer directory, edit details, and activate/deactivate accounts.
- **Account Management**: Open new accounts, assign account types, and freeze/unfreeze accounts.
- **Audit Reports**: Daily turnover audit, monthly volume breakdown, and account distribution metrics.
- **CSV Data Export**: Export all historical database transactions into CSV via C++ file streams (`std::ofstream`).
- **Administrative Audit Trail**: Automatic logging of administrative actions.

---

## 3. Object-Oriented Programming (OOP) Implementation

This project implements all **15 mandatory C++ OOP concepts**:

1. **Classes & Objects**: Domain entities (`BankAccount`, `Customer`, `Admin`, `Transaction`, `Bank`).
2. **Encapsulation**: Private and protected data members with getters and setters.
3. **Abstraction**: Abstract base class `BankAccount` with pure virtual functions (`deposit() = 0`, `withdraw() = 0`).
4. **Inheritance**: `Person` $\rightarrow$ `Customer`, `Admin`; `BankAccount` $\rightarrow$ `SavingsAccount`, `CurrentAccount`.
5. **Runtime Polymorphism**: Virtual methods dynamically dispatched via `std::shared_ptr<BankAccount>`.
6. **Compile-Time Polymorphism**: Overloaded `deposit()` methods with single or multi-parameter signatures.
7. **Constructors & Destructors**: Virtual destructors ensure safe polymorphic cleanup.
8. **Static Data Members**: `BankAccount::totalAccountsInstantiated` tracks in-memory account instances.
9. **Friend Functions**: `friend void auditAccount(...)` provides audited access to internal state.
10. **Operator Overloading**: `operator==` for account equality; `operator<` for transaction chronological order.
11. **Custom Exception Handling**: Custom hierarchy derived from `std::runtime_error` (`InsufficientFundsException`, etc.).
12. **Composition & Association**: `Bank` coordinates `DatabaseManager` and account collections.
13. **File Handling**: Persistent CSV generation using `std::ofstream`.
14. **Dynamic Memory Management**: RAII with modern smart pointers (`std::shared_ptr`, `std::make_shared`).
15. **Virtual Base & Method Overriding**: Concrete implementation of account rules in derived classes.

*For an in-depth viva voce preparation guide, see [`docs/OOP_CONCEPTS.md`](file:///Users/preet/Downloads/Bank%20Management%20System/docs/OOP_CONCEPTS.md).*

---

## 4. Project Folder Structure

```
bank-management-system/
│
├── frontend/                     # Modern Web UI
│   ├── index.html                # Welcome / Landing page
│   ├── login.html                # Sign In page (with 1-click demo buttons)
│   ├── register.html             # Customer Registration page
│   ├── dashboard.html            # Unified Customer & Admin Dashboard
│   ├── accounts.html             # My Accounts & Account Management
│   ├── customers.html            # Customer Directory (Admin only)
│   ├── transactions.html         # Transaction History & Digital Receipts
│   ├── deposit.html              # Deposit Funds page
│   ├── withdraw.html             # Withdraw Funds page
│   ├── transfer.html             # Inter-Account Fund Transfer page
│   ├── reports.html              # Audit Reports & CSV Export (Admin only)
│   ├── profile.html              # Customer/Admin Profile & Password Change
│   ├── css/
│   │   ├── style.css             # Base theme, typography, navbar, sidebar, modals
│   │   ├── dashboard.css         # Metrics grid, ATM cards, Chart.js wrappers
│   │   ├── forms.css             # Floating inputs, quick-amount preset chips
│   │   └── responsive.css        # Mobile and tablet breakpoints
│   └── js/
│       ├── api.js                # Fetch API client, session management, toast alerts
│       ├── auth.js               # Login and registration controllers
│       ├── dashboard.js          # Metrics loading and Chart.js rendering
│       ├── accounts.js           # Account creation and status toggles
│       ├── transactions.js       # Transaction filtering and receipt modals
│       ├── customers.js          # Admin customer management
│       └── reports.js            # Analytical reporting and CSV export triggers
│
├── backend/                      # C++17 Core Engine
│   ├── include/
│   │   ├── Exceptions.h          # Custom banking exception hierarchy
│   │   ├── Person.h              # Base class for users
│   │   ├── Customer.h            # Customer derived class
│   │   ├── Admin.h               # Administrator derived class
│   │   ├── BankAccount.h         # Abstract base bank account
│   │   ├── SavingsAccount.h      # Savings account (min balance & interest)
│   │   ├── CurrentAccount.h      # Current account (overdraft allowance)
│   │   ├── Transaction.h         # Transaction encapsulation
│   │   ├── DatabaseManager.h     # SQLite3 RAII wrapper & prepared statements
│   │   ├── Authentication.h      # PBKDF2 password hashing & session management
│   │   ├── Bank.h                # Central banking coordinator
│   │   ├── ReportManager.h       # CSV file generator & analytical queries
│   │   ├── crow_all.h            # Crow C++ microservice framework
│   │   └── json.hpp              # nlohmann/json library
│   ├── src/
│   │   ├── main.cpp              # REST API routing and static web server
│   │   ├── Person.cpp
│   │   ├── Customer.cpp
│   │   ├── Admin.cpp
│   │   ├── BankAccount.cpp
│   │   ├── SavingsAccount.cpp
│   │   ├── CurrentAccount.cpp
│   │   ├── Transaction.cpp
│   │   ├── DatabaseManager.cpp
│   │   ├── Authentication.cpp
│   │   ├── Bank.cpp
│   │   └── ReportManager.cpp
│   └── CMakeLists.txt            # CMake configuration
│
├── database/                     # Relational Database Storage
│   ├── schema.sql                # DDL tables, checks, and performance indexes
│   ├── seed.sql                  # Realistic sample Indian customers & transactions
│   └── bank_management.db       # Active SQLite3 database file
│
├── docs/                         # Project Documentation
│   ├── PROJECT_REPORT.md         # Complete university BCA project report
│   ├── API_DOCUMENTATION.md      # Detailed REST API specification
│   ├── DATABASE_DESIGN.md        # Relational ER diagrams and schema
│   └── OOP_CONCEPTS.md           # BCA viva voce questions and answers
│
├── tests/
│   ├── test_api.py               # Automated end-to-end test suite (31 tests)
│   └── test_cases.md             # Formal test cases and verification results
│
├── Makefile                      # Fast direct compilation and run targets
├── README.md                     # Project documentation
└── requirements.txt              # Optional test dependencies
```

---

## 5. Demo Credentials

The database is pre-seeded with realistic demonstration accounts:

| Role | Email / Username | Password | Notes |
| :--- | :--- | :--- | :--- |
| **Administrator** | `admin@bank.com` | `AdminPassword123!` | Full administrative access to reports, customer directory, and bank reserves. |
| **Customer 1** | `rahul.sharma@example.com` | `Customer123!` | Savings Account (`ACC10010001`) & Current Account (`ACC10010006`). |
| **Customer 2** | `priya.patel@example.com` | `Customer123!` | Current Account (`ACC10010002`). |
| **Customer 3** | `amit.verma@example.com` | `Customer123!` | Savings Account (`ACC10010003`). |
| **Customer 4** | `sneha.reddy@example.com` | `Customer123!` | Student Account (`ACC10010004`). |
| **Customer 5** | `vikram.singh@example.com` | `Customer123!` | Savings Account (`ACC10010005`). |

> **Tip**: On the Login page (`/login`), click the **"Demo Admin"** or **"Demo Customer (Rahul)"** buttons for instant 1-click login!

---

## 6. How to Build & Run Locally (macOS & Linux)

### Step 1: Open Terminal in the project root directory
```bash
cd "/Users/preet/Downloads/Bank Management System"
```

### Step 2: Build the C++ Backend
You can build using standard `make` or `cmake`:

**Option A: Using Make (Fastest)**
```bash
make
```

**Option B: Using CMake**
```bash
cmake -B build -S backend
cmake --build build
cp build/bank_backend .
```

### Step 3: Run the Server
```bash
./bank_backend
```
The server will start on port `18080`:
```
==========================================================
  BANK MANAGEMENT SYSTEM - C++ REST API BACKEND
  BCA University Final Year Project
==========================================================
[Server] Modern Banking REST API starting on port 18080...
[Server] Access Frontend at: http://localhost:18080
```

### Step 4: Open in Web Browser
Navigate to:
```
http://localhost:18080
```
The entire frontend (all 12 pages, CSS styles, JavaScript, and Chart.js charts) is served directly through the C++ backend!

---

## 7. Running the Automated Test Suite

A comprehensive test suite covering 31 test scenarios (Authentication, RBAC, Financial Operations, Negative Amounts, Minimum Balance Violation, Atomic Rollback, and CSV Export) is included in [`tests/test_api.py`](file:///Users/preet/Downloads/Bank%20Management%20System/tests/test_api.py).

While `./bank_backend` is running in one terminal, run:
```bash
python3 tests/test_api.py
```
Expected output:
```
======================================================================
 TEST RESULTS SUMMARY: 31 PASSED, 0 FAILED
======================================================================
```

---

## 8. Academic Project Limitations

This software was engineered as an **educational simulation** for a BCA University examination:
1. Currency is simulated and restricted to Indian Rupees (INR ₹).
2. Deposits are manually recorded simulations and are not linked to actual banking networks or live payment gateways.
3. No real credit card or debit card numbers are handled.
