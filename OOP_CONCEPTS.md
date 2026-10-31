# Object-Oriented Programming (OOP) in C++: Viva & Implementation Guide

This document explains the implementation of all **15 mandatory C++ Object-Oriented Programming (OOP)** concepts within the **Bank Management System** codebase. Use this guide to prepare for your BCA practical examination and viva voce.

---

## Quick Reference Table of OOP Concepts

| # | OOP Concept | C++ Class / File | Practical Purpose in Banking |
| :--- | :--- | :--- | :--- |
| **1** | **Classes & Objects** | All files in `backend/include/` | Encapsulates bank entities (Accounts, Customers, Transactions). |
| **2** | **Encapsulation** | `BankAccount.h`, `Person.h` | `private`/`protected` members with getters/setters protect sensitive data. |
| **3** | **Abstraction** | `BankAccount.h` | Pure virtual functions (`= 0`) define banking contracts without exposing internals. |
| **4** | **Inheritance** | `Customer.h`, `Admin.h` | Code reuse: `Customer` & `Admin` inherit common identity from `Person`. |
| **5** | **Runtime Polymorphism**| `SavingsAccount.h`, `CurrentAccount.h` | `deposit()` and `withdraw()` dynamically dispatched via base pointers. |
| **6** | **Compile-Time Polymorphism**| `BankAccount.h` | Function overloading: `deposit(amount)` and `deposit(amount, note)`. |
| **7** | **Constructors & Destructors**| `Person.cpp`, `BankAccount.cpp` | Resource initialization and `virtual ~BankAccount()` prevents memory leaks. |
| **8** | **Static Data Members**| `BankAccount.h` | `static int totalAccountsInstantiated` tracks in-memory account instances. |
| **9** | **Friend Functions** | `BankAccount.h` | `friend void auditAccount(...)` provides audited access to internal state. |
| **10**| **Operator Overloading**| `BankAccount.h`, `Transaction.h`| `operator==` compares accounts; `operator<` sorts transactions chronologically. |
| **11**| **Exception Handling**| `Exceptions.h` | Custom exception hierarchy: `InsufficientFundsException`, `InvalidAccountException`. |
| **12**| **Composition & Association**| `Bank.h` | `Bank` has-a `DatabaseManager` and manages collections of `BankAccount` instances. |
| **13**| **File Handling** | `ReportManager.cpp` | `std::ofstream` exports transaction audit logs to CSV files. |
| **14**| **Dynamic Memory Management**| `Bank.cpp`, `main.cpp` | Smart pointers (`std::shared_ptr`, `std::make_shared`) prevent memory leaks. |
| **15**| **Virtual Base & Destructor**| `BankAccount.h`, `Person.h` | Virtual destructors ensure proper teardown in polymorphic inheritance trees. |

---

## Detailed Code Walkthroughs

### 1. Classes and Objects
- **Definition**: A class is a blueprint defining attributes (state) and methods (behavior). An object is an instance of a class.
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h)
```cpp
class BankAccount {
protected:
    int accountId;
    std::string accountNumber;
    long long balance; // in paise
...
};
```

---

### 2. Encapsulation & Data Hiding
- **Definition**: Bundling data and functions within a class and restricting direct access to prevent unauthorized state alteration.
- **Why it matters**: In a bank, users must not arbitrarily overwrite `balance = 9999999`. Balance changes must only occur through verified `deposit()` or `withdraw()` methods.
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L30-L55)
```cpp
class BankAccount {
protected:
    long long balance; // Private/Protected state

public:
    long long getBalance() const { return balance; }
    double getBalanceInRupees() const { return static_cast<double>(balance) / 100.0; }
};
```

---

### 3. Abstraction & Abstract Base Class
- **Definition**: Hiding implementation details and showing only essential features via an abstract interface.
- **Implementation**: `BankAccount` is an **Abstract Class** containing pure virtual functions (`= 0`). You cannot instantiate `BankAccount` directly; you must instantiate derived classes (`SavingsAccount` or `CurrentAccount`).
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L58-L62)
```cpp
virtual bool deposit(long long amountPaise, const std::string& description) = 0;
virtual bool withdraw(long long amountPaise, const std::string& description) = 0;
virtual void displayAccountDetails() const = 0;
```

---

### 4. Inheritance (Base & Derived Classes)
- **Definition**: Mechanism where a derived class acquires the properties and methods of a base class.
- **Hierarchy 1**: `Person` (Base) $\rightarrow$ `Customer` (Derived), `Admin` (Derived).
- **Hierarchy 2**: `BankAccount` (Base) $\rightarrow$ `SavingsAccount` (Derived), `CurrentAccount` (Derived).
- **Code Reference**: [`Customer.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/Customer.h#L17-L25)
```cpp
class Customer : public Person {
private:
    std::string address;
    std::string dateOfBirth;
public:
    Customer(int id, const std::string& name, ...);
};
```

---

### 5. Runtime Polymorphism (Virtual Functions & Overriding)
- **Definition**: Dynamic method binding at runtime using virtual method tables (VTABLE).
- **Implementation**: When `account->withdraw(amount)` is called on a `std::shared_ptr<BankAccount>`, C++ dynamically invokes `SavingsAccount::withdraw()` (which checks ₹500 minimum balance) or `CurrentAccount::withdraw()` (which checks overdraft allowance).
- **Code Reference**: [`SavingsAccount.cpp`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/src/SavingsAccount.cpp#L25-L42)
```cpp
bool SavingsAccount::withdraw(long long amountPaise, const std::string& description) override {
    if (this->balance - amountPaise < this->minimumBalancePaise) {
        throw InsufficientFundsException("Minimum balance of ₹500 required", amountPaise, balance);
    }
    this->balance -= amountPaise;
    return true;
}
```

---

### 6. Compile-Time Polymorphism (Function Overloading)
- **Definition**: Defining multiple functions with the exact same name but different parameter lists.
- **Implementation**: `BankAccount` overloads `deposit()` and `withdraw()`:
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L64-L68)
```cpp
bool deposit(long long amountPaise, const std::string& description); // Form 1: With note
bool deposit(long long amountPaise);                                 // Form 2: Default note
```

---

### 7. Constructors and Virtual Destructors
- **Definition**: Constructors initialize object state. Virtual destructors ensure that deleting an object through a base class pointer invokes the derived class destructor first, preventing resource leaks.
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L38-L44)
```cpp
virtual ~BankAccount(); // Virtual destructor ensures safe polymorphic deletion
```

---

### 8. Static Data Members and Member Functions
- **Definition**: Variables and functions associated with the class itself rather than individual instances.
- **Implementation**: `BankAccount::totalAccountsInstantiated` tracks the active in-memory accounts.
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L34-L45)
```cpp
protected:
    static int totalAccountsInstantiated;
public:
    static int getTotalAccountsInstantiated();
```

---

### 9. Friend Functions
- **Definition**: A non-member function granted private and protected access to a class.
- **Implementation**: `auditAccount()` inspects protected account attributes for auditing logs without needing public setters.
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L80-L84)
```cpp
friend void auditAccount(const BankAccount& account, std::ostream& os);
```

---

### 10. Operator Overloading
- **Definition**: Redefining C++ standard operators (`==`, `!=`, `<`, `<<`) for custom classes.
- **Implementation**:
  - `BankAccount::operator==` checks equality based on unique account number.
  - `Transaction::operator<` compares timestamps for sorting.
  - `operator<<` outputs account summary directly to streams (`std::cout << account;`).
- **Code Reference**: [`BankAccount.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/BankAccount.h#L75-L87)

---

### 11. Exception Handling Hierarchy
- **Definition**: Structured error handling separating recovery code from normal business logic.
- **Hierarchy**:
  `std::runtime_error` $\rightarrow$ `BankingException` $\rightarrow$ `InsufficientFundsException`, `InvalidAccountException`, `AuthenticationException`.
- **Code Reference**: [`Exceptions.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/Exceptions.h#L15-L35)

---

### 12. Composition & Association
- **Definition**:
  - **Composition ("Has-A" relationship)**: `Bank` owns a `DatabaseManager` and coordinates accounts.
  - **Association**: `Customer` associates with multiple `BankAccount` entities.
- **Code Reference**: [`Bank.h`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/include/Bank.h#L24-L35)

---

### 13. File Handling (Persistent CSV Generation)
- **Definition**: Using standard C++ I/O streams (`std::ofstream`) to write structured records to disk.
- **Implementation**: `ReportManager::exportTransactionsToCsv()` exports real database records into CSV files.
- **Code Reference**: [`ReportManager.cpp`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/src/ReportManager.cpp#L115-L165)

---

### 14. Dynamic Memory Management & Smart Pointers
- **Definition**: Managing heap memory using modern RAII wrappers (`std::shared_ptr`, `std::unique_ptr`, `std::make_shared`) to eliminate memory leaks and dangling pointers.
- **Code Reference**: [`Bank.cpp`](file:///Users/preet/Downloads/Bank%20Management%20System/backend/src/Bank.cpp#L38-L55)
```cpp
std::shared_ptr<BankAccount> Bank::createAccountInstance(const nlohmann::json& row) {
    if (type == "CURRENT") {
        return std::make_shared<CurrentAccount>(...);
    } else {
        return std::make_shared<SavingsAccount>(...);
    }
}
```

---

## Common Viva Voce Questions & Answers

**Q1: Why did you declare `deposit()` and `withdraw()` as pure virtual functions in `BankAccount`?**
*Answer*: Because a generic bank account does not have universal balance rules. A savings account requires maintaining a minimum balance of ₹500, whereas a current account supports overdrafts. By making them pure virtual (`= 0`), we make `BankAccount` an abstract class and force derived classes to provide concrete banking rules.

**Q2: What is the purpose of a virtual destructor in a base class?**
*Answer*: If we delete a derived class object (like `SavingsAccount`) through a base class pointer (`BankAccount*`), a non-virtual destructor only calls the base destructor, leaking memory allocated in the derived class. A virtual destructor ensures the derived destructor is invoked first, followed by the base destructor.

**Q3: Why store money amounts in integer paise instead of float/double?**
*Answer*: Floating-point formats (IEEE 754) cannot represent certain decimal values accurately (e.g. $0.1 + 0.2 \neq 0.3$). In financial applications, accumulated rounding errors cause balance discrepancies. Storing amounts as integer paise guarantees $100\%$ precision without rounding loss.

**Q4: How do you achieve compile-time vs runtime polymorphism in this system?**
*Answer*:
- **Compile-time**: Function overloading (`deposit(amount)` vs `deposit(amount, note)`) and operator overloading (`==`, `<`), resolved by the compiler based on argument types.
- **Runtime**: Virtual function overrides in `SavingsAccount` and `CurrentAccount` dispatched dynamically via vtables through `std::shared_ptr<BankAccount>`.
