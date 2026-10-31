# REST API Specification: Bank Management System

The **Bank Management System** provides a standard JSON REST API running on **port 18080**.

- Base URL: `http://localhost:18080/api`
- Content-Type: `application/json`
- Authentication Header: `Authorization: Bearer <session_token>`

---

## 1. Authentication Endpoints

### 1.1 Customer Registration
- **Endpoint**: `POST /api/auth/register`
- **Access**: Public
- **Request Body**:
```json
{
  "full_name": "Ramesh Kumar",
  "email": "ramesh@example.com",
  "phone": "+91 98765 00000",
  "address": "42 MG Road, Bengaluru",
  "date_of_birth": "1994-06-15",
  "password": "Password123!"
}
```
- **Response (201 Created)**:
```json
{
  "success": true,
  "user_id": 7,
  "full_name": "Ramesh Kumar",
  "email": "ramesh@example.com",
  "account_number": "ACC10010007",
  "message": "Customer registered successfully. Welcome to Modern Bank!"
}
```

---

### 1.2 User Login
- **Endpoint**: `POST /api/auth/login`
- **Access**: Public
- **Request Body**:
```json
{
  "email": "rahul.sharma@example.com",
  "password": "Customer123!"
}
```
- **Response (200 OK)**:
```json
{
  "success": true,
  "token": "a1b2c3d4e5f6...",
  "user": {
    "user_id": 2,
    "name": "Rahul Sharma",
    "email": "rahul.sharma@example.com",
    "role": "CUSTOMER",
    "status": "ACTIVE"
  }
}
```

---

### 1.3 User Logout
- **Endpoint**: `POST /api/auth/logout`
- **Access**: Authenticated
- **Headers**: `Authorization: Bearer <token>`
- **Response (200 OK)**:
```json
{
  "success": true,
  "message": "Logged out successfully."
}
```

---

### 1.4 Get Current User Profile
- **Endpoint**: `GET /api/auth/me`
- **Access**: Authenticated
- **Response (200 OK)**:
```json
{
  "user_id": 2,
  "name": "Rahul Sharma",
  "email": "rahul.sharma@example.com",
  "phone": "+91 98111 22334",
  "role": "CUSTOMER",
  "status": "ACTIVE",
  "address": "42 MG Road, Indiranagar, Bengaluru",
  "date_of_birth": "1992-08-20",
  "created_at": "2026-01-05 10:15:00"
}
```

---

## 2. Dashboard Endpoints

### 2.1 Customer Dashboard
- **Endpoint**: `GET /api/dashboard/customer`
- **Access**: Authenticated Customer
- **Response (200 OK)**: Returns metrics, active account cards, recent transactions, and Chart.js datasets.

### 2.2 Administrator Dashboard
- **Endpoint**: `GET /api/dashboard/admin`
- **Access**: Authenticated Admin
- **Response (200 OK)**: Returns total bank simulated reserves, customer count, active accounts, system transaction activity, and audit logs.

---

## 3. Financial Transaction Endpoints

### 3.1 Deposit Money
- **Endpoint**: `POST /api/transactions/deposit`
- **Access**: Authenticated
- **Request Body**:
```json
{
  "account_id": 1,
  "amount": 1000.0,
  "description": "Salary Deposit"
}
```
- **Response (200 OK)**:
```json
{
  "success": true,
  "reference_number": "TXN1793461...",
  "account_number": "ACC10010001",
  "masked_account_number": "ACC*****0001",
  "amount_rupees": 1000.0,
  "new_balance_rupees": 51000.0,
  "message": "Deposit of ₹1000 processed successfully."
}
```

---

### 3.2 Withdraw Money
- **Endpoint**: `POST /api/transactions/withdraw`
- **Access**: Authenticated
- **Request Body**:
```json
{
  "account_id": 1,
  "amount": 500.0,
  "description": "ATM Cash Withdrawal"
}
```
- **Response (200 OK)**:
```json
{
  "success": true,
  "reference_number": "TXN1793462...",
  "account_number": "ACC10010001",
  "amount_rupees": 500.0,
  "new_balance_rupees": 50500.0,
  "message": "Withdrawal of ₹500 completed successfully."
}
```
- **Error Response (400 Bad Request)**:
```json
{
  "error": "Withdrawal violates minimum balance requirement of ₹500",
  "requested_rupees": 55000.0,
  "available_rupees": 50500.0
}
```

---

### 3.3 Fund Transfer
- **Endpoint**: `POST /api/transactions/transfer`
- **Access**: Authenticated
- **Request Body**:
```json
{
  "sender_account_id": 1,
  "receiver_account_number": "ACC10010002",
  "amount": 300.0,
  "description": "Invoice Payment"
}
```
- **Response (200 OK)**:
```json
{
  "success": true,
  "reference_number": "TXN1793463...",
  "sender_account": "ACC10010001",
  "receiver_account": "ACC10010002",
  "receiver_name": "Priya Patel",
  "amount_rupees": 300.0,
  "new_balance_rupees": 50200.0,
  "message": "Transfer of ₹300 to Priya Patel succeeded."
}
```

---

### 3.4 Transaction History & Filters
- **Endpoint**: `GET /api/transactions`
- **Access**: Authenticated
- **Query Parameters**:
  - `type`: `ALL`, `DEPOSIT`, `WITHDRAWAL`, `TRANSFER`
  - `start_date`: `YYYY-MM-DD`
  - `end_date`: `YYYY-MM-DD`
  - `min_amount`: Numeric
  - `max_amount`: Numeric
  - `search`: String
  - `limit`: Number (default: 50)
  - `offset`: Number (default: 0)

---

## 4. Reports & Data Export (Admin Only)

### 4.1 Daily Audit Report
- **Endpoint**: `GET /api/reports/daily?date=YYYY-MM-DD`
- **Access**: Admin

### 4.2 Monthly Turnover Report
- **Endpoint**: `GET /api/reports/monthly?month=YYYY-MM`
- **Access**: Admin

### 4.3 Export Transactions to CSV
- **Endpoint**: `GET /api/reports/export`
- **Access**: Admin
- **Response (200 OK)**:
```json
{
  "success": true,
  "download_url": "/download/csv",
  "file_path": "database/transactions_export.csv",
  "message": "CSV Report generated successfully."
}
```

---

## 5. HTTP Status Code Conventions
- `200 OK`: Request succeeded.
- `201 Created`: Resource successfully registered or created.
- `400 Bad Request`: Validation failure, insufficient funds, or illegal balance operation.
- `401 Unauthorized`: Missing or invalid session token.
- `403 Forbidden`: Administrative privileges required.
- `404 Not Found`: Account or transaction not found.
- `500 Internal Server Error`: Unexpected internal database or server fault.
