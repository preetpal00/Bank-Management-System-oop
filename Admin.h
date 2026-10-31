#pragma once

#include "Person.h"
#include <string>

// ============================================================================
// OOP CONCEPT: Inheritance (Derived Class)
// Admin inherits from Person and encapsulates administrative capabilities.
// ============================================================================

class Admin : public Person {
public:
    Admin();
    Admin(int id, const std::string& name, const std::string& email, 
          const std::string& phone, const std::string& status = "ACTIVE", 
          const std::string& createdAt = "");

    // Method Overriding
    void displayDetails() const override;
    nlohmann::json toJson() const override;

    // Admin capability markers
    bool canManageCustomers() const { return true; }
    bool canManageAccounts() const { return true; }
    bool canGenerateReports() const { return true; }
};
