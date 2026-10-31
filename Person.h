#pragma once

#include <string>
#include <iostream>
#include "json.hpp"

// ============================================================================
// OOP CONCEPT: Abstraction & Encapsulation (Base Class)
// Person encapsulates common demographic and contact data shared by all actors.
// It serves as the base class in the user hierarchy (Inheritance).
// ============================================================================

class Person {
protected:
    // Protected data members: accessible by derived classes (Customer, Admin)
    int userId;
    std::string name;
    std::string email;
    std::string phone;
    std::string role;       // "ADMIN" or "CUSTOMER"
    std::string status;     // "ACTIVE", "INACTIVE", etc.
    std::string createdAt;

public:
    // Default and parameterized constructors
    Person();
    Person(int id, const std::string& name, const std::string& email, 
           const std::string& phone, const std::string& role, 
           const std::string& status = "ACTIVE", const std::string& createdAt = "");

    // Virtual destructor: critical for safe deletion through base-class pointers
    virtual ~Person() = default;

    // Encapsulation: Getters
    int getUserId() const { return userId; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getPhone() const { return phone; }
    std::string getRole() const { return role; }
    std::string getStatus() const { return status; }
    std::string getCreatedAt() const { return createdAt; }

    // Encapsulation: Setters
    void setName(const std::string& newName) { name = newName; }
    void setPhone(const std::string& newPhone) { phone = newPhone; }
    void setStatus(const std::string& newStatus) { status = newStatus; }

    // Virtual methods for polymorphism
    virtual void displayDetails() const;
    virtual void updateDetails(const std::string& newName, const std::string& newPhone);

    // Serialization to JSON for REST API
    virtual nlohmann::json toJson() const;
};
