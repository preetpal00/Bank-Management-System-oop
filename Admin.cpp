#include "Admin.h"

Admin::Admin() 
    : Person(0, "", "", "", "ADMIN", "ACTIVE", "") {}

Admin::Admin(int id, const std::string& name, const std::string& email, 
             const std::string& phone, const std::string& status, 
             const std::string& createdAt)
    : Person(id, name, email, phone, "ADMIN", status, createdAt) {}

void Admin::displayDetails() const {
    std::cout << "[Administrator ID: " << userId << "] " << name 
              << " | Email: " << email 
              << " | Phone: " << phone 
              << " | Status: " << status << std::endl;
}

nlohmann::json Admin::toJson() const {
    nlohmann::json j = Person::toJson();
    j["role"] = "ADMIN";
    return j;
}
