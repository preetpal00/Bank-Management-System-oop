#include "Customer.h"

Customer::Customer() 
    : Person(), address(""), dateOfBirth("") {}

Customer::Customer(int id, const std::string& name, const std::string& email, 
                   const std::string& phone, const std::string& address, 
                   const std::string& dob, const std::string& status, 
                   const std::string& createdAt)
    : Person(id, name, email, phone, "CUSTOMER", status, createdAt),
      address(address), dateOfBirth(dob) {}

void Customer::displayDetails() const {
    std::cout << "[Customer ID: " << userId << "] " << name 
              << " | Email: " << email 
              << " | Phone: " << phone 
              << " | DOB: " << dateOfBirth 
              << " | Address: " << address 
              << " | Status: " << status << std::endl;
}

nlohmann::json Customer::toJson() const {
    nlohmann::json j = Person::toJson();
    j["address"] = address;
    j["date_of_birth"] = dateOfBirth;
    return j;
}

nlohmann::json Customer::viewProfile() const {
    return toJson();
}
