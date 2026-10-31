-- ============================================================================
-- Bank Management System - Database Schema (SQLite3)
-- Compliant with BCA University Project Requirements
-- Currency amounts stored in integer paise (1 INR = 100 paise)
-- ============================================================================

PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
-- TABLE 1: users
-- Stores both Administrators and Customers
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    phone TEXT,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL CHECK(role IN ('ADMIN', 'CUSTOMER')),
    address TEXT,
    date_of_birth TEXT,
    status TEXT DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'INACTIVE', 'SUSPENDED')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ----------------------------------------------------------------------------
-- TABLE 2: accounts
-- Stores bank accounts linked to customer users
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS accounts (
    account_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    account_number TEXT UNIQUE NOT NULL,
    account_type TEXT NOT NULL CHECK(account_type IN ('SAVINGS', 'CURRENT', 'STUDENT')),
    balance INTEGER NOT NULL DEFAULT 0 CHECK(balance >= 0),
    status TEXT DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'INACTIVE', 'FROZEN', 'CLOSED')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- ----------------------------------------------------------------------------
-- TABLE 3: transactions
-- Records all individual deposits, withdrawals, transfers, and adjustments
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
    reference_number TEXT NOT NULL,
    account_id INTEGER NOT NULL,
    transaction_type TEXT NOT NULL CHECK(transaction_type IN ('DEPOSIT', 'WITHDRAWAL', 'TRANSFER_IN', 'TRANSFER_OUT', 'ADJUSTMENT')),
    amount INTEGER NOT NULL CHECK(amount > 0),
    description TEXT,
    transaction_status TEXT DEFAULT 'SUCCESS' CHECK(transaction_status IN ('SUCCESS', 'FAILED', 'PENDING')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
);

-- ----------------------------------------------------------------------------
-- TABLE 4: transfer_details
-- Cross-references sender and receiver for fund transfers
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS transfer_details (
    transfer_id INTEGER PRIMARY KEY AUTOINCREMENT,
    reference_number TEXT UNIQUE NOT NULL,
    sender_account_id INTEGER NOT NULL,
    receiver_account_id INTEGER NOT NULL,
    amount INTEGER NOT NULL CHECK(amount > 0),
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender_account_id) REFERENCES accounts(account_id),
    FOREIGN KEY (receiver_account_id) REFERENCES accounts(account_id)
);

-- ----------------------------------------------------------------------------
-- TABLE 5: admin_logs
-- Audit log for administrative activities
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS admin_logs (
    log_id INTEGER PRIMARY KEY AUTOINCREMENT,
    admin_id INTEGER NOT NULL,
    action TEXT NOT NULL,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (admin_id) REFERENCES users(user_id)
);

-- ----------------------------------------------------------------------------
-- TABLE 6: sessions
-- Secure server-side authentication tokens
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS sessions (
    session_token TEXT PRIMARY KEY,
    user_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- ----------------------------------------------------------------------------
-- Performance Indexes
-- ----------------------------------------------------------------------------
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_accounts_account_number ON accounts(account_number);
CREATE INDEX IF NOT EXISTS idx_accounts_user_id ON accounts(user_id);
CREATE INDEX IF NOT EXISTS idx_transactions_account_id ON transactions(account_id);
CREATE INDEX IF NOT EXISTS idx_transactions_ref ON transactions(reference_number);
CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(created_at);
CREATE INDEX IF NOT EXISTS idx_transfer_ref ON transfer_details(reference_number);
CREATE INDEX IF NOT EXISTS idx_sessions_token ON sessions(session_token);
