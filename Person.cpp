#include "Person.h"

// Default constructor
Person::Person() 
    : userId(0), name(""), email(""), phone(""), role("CUSTOMER"), status("ACTIVE"), createdAt("") {}

// Parameterized constructor
Person::Person(int id, const std::string& name, const std::string& email, 
               const std::string& phone, const std::string& role, 
               const std::string& status, const std::string& createdAt)
    : userId(id), name(name), email(email), phone(phone), 
      role(role), status(status), createdAt(createdAt) {}

void Person::displayDetails() const {
    std::cout << "[Person ID: " << userId << "] " << name 
              << " | Email: " << email 
              << " | Phone: " << phone 
              << " | Role: " << role 
              << " | Status: " << status << std::endl;
}

void Person::updateDetails(const std::string& newName, const std::string& newPhone) {
    if (!newName.empty()) {
        this->name = newName;
    }
    if (!newPhone.empty()) {
        this->phone = newPhone;
    }
}

nlohmann::json Person::toJson() const {
    return {
        {"user_id", userId},
        {"name", name},
        {"email", email},
        {"phone", phone},
        {"role", role},
        {"status", status},
        {"created_at", createdAt}
    };
}
