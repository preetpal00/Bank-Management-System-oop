-- ============================================================================
-- Bank Management System - Database Seed Data (SQLite3)
-- Compliant with BCA University Project Requirements
-- Currency amounts stored in integer paise (1 INR = 100 paise)
-- ============================================================================

PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
-- 1. USERS (1 Admin + 5 Sample Customers)
-- Default Admin Password: AdminPassword123!
-- Default Customer Password: Customer123!
-- Hashes generated via PBKDF2-HMAC-SHA256 (10,000 iterations) with cryptographic salt
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO users (user_id, full_name, email, phone, password_hash, role, address, date_of_birth, status, created_at)
VALUES 
(1, 'System Administrator', 'admin@bank.com', '+91 98765 43210', 
 'bca_admin_salt_2026$994ca165c4e5cf3f4a4d2b5aa0fe350d441c8f84cd54a4615a3cf885da5a2bde', 
 'ADMIN', '100 Financial District, Mumbai, Maharashtra 400051', '1985-05-15', 'ACTIVE', '2026-01-01 09:00:00'),

(2, 'Rahul Sharma', 'rahul.sharma@example.com', '+91 98111 22334', 
 'bca_cust_salt_2026$08da5f872f4e66c982c10c2f433f4a50126c319fba21501c50f36e42e7e922a4', 
 'CUSTOMER', '42 MG Road, Indiranagar, Bengaluru, Karnataka 560038', '1992-08-20', 'ACTIVE', '2026-01-05 10:15:00'),

(3, 'Priya Patel', 'priya.patel@example.com', '+91 98222 33445', 
 'bca_cust_salt_2026$08da5f872f4e66c982c10c2f433f4a50126c319fba21501c50f36e42e7e922a4', 
 'CUSTOMER', '15 SG Highway, Bodakdev, Ahmedabad, Gujarat 380054', '1990-11-12', 'ACTIVE', '2026-01-10 11:30:00'),

(4, 'Amit Verma', 'amit.verma@example.com', '+91 98333 44556', 
 'bca_cust_salt_2026$08da5f872f4e66c982c10c2f433f4a50126c319fba21501c50f36e42e7e922a4', 
 'CUSTOMER', '88 Park Street, Kolkata, West Bengal 700016', '1995-03-25', 'ACTIVE', '2026-01-15 14:00:00'),

(5, 'Sneha Reddy', 'sneha.reddy@example.com', '+91 98444 55667', 
 'bca_cust_salt_2026$08da5f872f4e66c982c10c2f433f4a50126c319fba21501c50f36e42e7e922a4', 
 'CUSTOMER', '10 Jubilee Hills, Hyderabad, Telangana 500033', '2003-07-08', 'ACTIVE', '2026-01-20 16:45:00'),

(6, 'Vikram Singh', 'vikram.singh@example.com', '+91 98555 66778', 
 'bca_cust_salt_2026$08da5f872f4e66c982c10c2f433f4a50126c319fba21501c50f36e42e7e922a4', 
 'CUSTOMER', '25 Connaught Place, New Delhi, Delhi 110001', '1988-12-04', 'ACTIVE', '2026-01-25 12:20:00');

-- ----------------------------------------------------------------------------
-- 2. ACCOUNTS (6 Sample Accounts across Customers)
-- Amounts in paise: 
-- ₹50,000 = 5000000 paise
-- ₹1,25,000 = 12500000 paise
-- ₹30,000 = 3000000 paise
-- ₹15,000 = 1500000 paise
-- ₹75,000 = 7500000 paise
-- ₹20,000 = 2000000 paise
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO accounts (account_id, user_id, account_number, account_type, balance, status, created_at)
VALUES
(1, 2, 'ACC10010001', 'SAVINGS', 5000000, 'ACTIVE', '2026-01-05 10:30:00'),
(2, 3, 'ACC10010002', 'CURRENT', 12500000, 'ACTIVE', '2026-01-10 11:45:00'),
(3, 4, 'ACC10010003', 'SAVINGS', 3000000, 'ACTIVE', '2026-01-15 14:15:00'),
(4, 5, 'ACC10010004', 'STUDENT', 1500000, 'ACTIVE', '2026-01-20 17:00:00'),
(5, 6, 'ACC10010005', 'SAVINGS', 7500000, 'ACTIVE', '2026-01-25 12:35:00'),
(6, 2, 'ACC10010006', 'CURRENT', 2000000, 'ACTIVE', '2026-02-01 09:15:00');

-- ----------------------------------------------------------------------------
-- 3. TRANSACTIONS (Realistic History: Deposits, Withdrawals, Transfers)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO transactions (transaction_id, reference_number, account_id, transaction_type, amount, description, transaction_status, created_at)
VALUES
-- Rahul initial deposit ₹55,000
(1, 'TXN100001', 1, 'DEPOSIT', 5500000, 'Initial Account Opening Deposit', 'SUCCESS', '2026-01-05 10:35:00'),
-- Priya initial deposit ₹1,25,000
(2, 'TXN100002', 2, 'DEPOSIT', 12500000, 'Initial Account Opening Deposit', 'SUCCESS', '2026-01-10 11:50:00'),
-- Amit initial deposit ₹35,000
(3, 'TXN100003', 3, 'DEPOSIT', 3500000, 'Initial Account Opening Deposit', 'SUCCESS', '2026-01-15 14:20:00'),
-- Sneha initial deposit ₹10,000
(4, 'TXN100004', 4, 'DEPOSIT', 1000000, 'Scholarship Stipend Deposit', 'SUCCESS', '2026-01-20 17:05:00'),
-- Vikram initial deposit ₹75,000
(5, 'TXN100005', 5, 'DEPOSIT', 7500000, 'Business Capital Deposit', 'SUCCESS', '2026-01-25 12:40:00'),
-- Rahul second account initial deposit ₹20,000
(6, 'TXN100006', 6, 'DEPOSIT', 2000000, 'Initial Current Account Deposit', 'SUCCESS', '2026-02-01 09:20:00'),
-- Rahul withdraws ₹2,000
(7, 'TXN100007', 1, 'WITHDRAWAL', 200000, 'ATM Cash Withdrawal', 'SUCCESS', '2026-02-05 15:10:00'),
-- Amit withdraws ₹5,000
(8, 'TXN100008', 3, 'WITHDRAWAL', 500000, 'Branch Cash Withdrawal', 'SUCCESS', '2026-02-08 11:20:00'),
-- Rahul transfers ₹3,000 to Priya
(9, 'TXN100009', 1, 'TRANSFER_OUT', 300000, 'Fund transfer to Priya Patel (ACC10010002)', 'SUCCESS', '2026-02-12 14:30:00'),
(10, 'TXN100009', 2, 'TRANSFER_IN', 300000, 'Fund transfer from Rahul Sharma (ACC10010001)', 'SUCCESS', '2026-02-12 14:30:00'),
-- Priya withdraws ₹3,000
(11, 'TXN100010', 2, 'WITHDRAWAL', 300000, 'Office Supply Expense', 'SUCCESS', '2026-02-15 10:45:00'),
-- Vikram transfers ₹5,000 to Sneha
(12, 'TXN100011', 5, 'TRANSFER_OUT', 500000, 'College Tuition Aid to Sneha Reddy', 'SUCCESS', '2026-02-18 16:00:00'),
(13, 'TXN100011', 4, 'TRANSFER_IN', 500000, 'College Tuition Aid from Vikram Singh', 'SUCCESS', '2026-02-18 16:00:00'),
-- Vikram deposits ₹5,000 back
(14, 'TXN100012', 5, 'DEPOSIT', 500000, 'Cash Deposit at Branch', 'SUCCESS', '2026-02-22 13:15:00');

-- ----------------------------------------------------------------------------
-- 4. TRANSFER DETAILS
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO transfer_details (transfer_id, reference_number, sender_account_id, receiver_account_id, amount, description, created_at)
VALUES
(1, 'TXN100009', 1, 2, 300000, 'Invoice Settlement Payment', '2026-02-12 14:30:00'),
(2, 'TXN100011', 5, 4, 500000, 'College Tuition Aid to Sneha Reddy', '2026-02-18 16:00:00');

-- ----------------------------------------------------------------------------
-- 5. ADMIN LOGS
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO admin_logs (log_id, admin_id, action, description, created_at)
VALUES
(1, 1, 'SYSTEM_INIT', 'Bank Management System database initialized with default schema', '2026-01-01 09:00:00'),
(2, 1, 'CUSTOMER_CREATE', 'Registered sample customer Rahul Sharma (ID: 2)', '2026-01-05 10:15:00'),
(3, 1, 'ACCOUNT_CREATE', 'Opened Savings Account ACC10010001 for customer ID: 2', '2026-01-05 10:30:00'),
(4, 1, 'CUSTOMER_CREATE', 'Registered sample customer Priya Patel (ID: 3)', '2026-01-10 11:30:00'),
(5, 1, 'ACCOUNT_CREATE', 'Opened Current Account ACC10010002 for customer ID: 3', '2026-01-10 11:45:00');
