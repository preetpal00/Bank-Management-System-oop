#pragma once

#include "Person.h"
#include <vector>
#include <memory>

// Forward declaration
class BankAccount;

// ============================================================================
// OOP CONCEPT: Inheritance (Derived Class)
// Customer inherits state and behavior from Person, adding customer-specific
// attributes (address, dateOfBirth) and customer-level methods.
// ============================================================================

class Customer : public Person {
private:
    std::string address;
    std::string dateOfBirth;

public:
    // Constructors
    Customer();
    Customer(int id, const std::string& name, const std::string& email, 
             const std::string& phone, const std::string& address, 
             const std::string& dob, const std::string& status = "ACTIVE", 
             const std::string& createdAt = "");

    // Getters and Setters
    std::string getAddress() const { return address; }
    std::string getDateOfBirth() const { return dateOfBirth; }
    void setAddress(const std::string& newAddress) { address = newAddress; }
    void setDateOfBirth(const std::string& dob) { dateOfBirth = dob; }

    // OOP CONCEPT: Polymorphism / Method Overriding
    void displayDetails() const override;
    nlohmann::json toJson() const override;

    // Domain methods
    nlohmann::json viewProfile() const;
};
