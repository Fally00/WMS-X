//Included files
#include "models/Customer.h"
#include "output/output.h"

//Needed libraries
#include <stdexcept>
#include <string>

// ─────────────────────────────────────────────
// Helpers / validation
// ─────────────────────────────────────────────

void Customer::touch() {
    modifiedAt = std::time(nullptr);
}

void Customer::validate() const {
    if (id < 0) throw std::invalid_argument("Customer id must be non-negative");
    if (name.empty()) throw std::invalid_argument("Customer name must not be empty");
    // Phone is optional, but if provided do a light check
    if (!phone.empty()) {
        bool hasDigit = false;
        for (char c : phone) {
            if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        }
        if (!hasDigit) throw std::invalid_argument("Phone number must contain at least one digit");
    }
}

// ─────────────────────────────────────────────
// Constructors
// ─────────────────────────────────────────────

Customer::Customer(
    int id,
    const std::string& name,
    const std::string& phone,
    const std::string& address,
    const std::string& email)
    : id(id),
      name(name),
      phone(phone),
      address(address),
      email(email) {
    createdAt = modifiedAt = std::time(nullptr);
    validate();
}

// ─────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────

int Customer::getId() const { return id; }
const std::string& Customer::getName() const { return name; }
const std::string& Customer::getPhone() const { return phone; }
const std::string& Customer::getAddress() const { return address; }
const std::string& Customer::getEmail() const { return email; }
std::time_t Customer::getCreatedAt() const { return createdAt; }
std::time_t Customer::getModifiedAt() const { return modifiedAt; }

// ─────────────────────────────────────────────
// Setters
// ─────────────────────────────────────────────

void Customer::setName(const std::string& n) {
    if (n.empty()) throw std::invalid_argument("Name must not be empty");
    name = n;
    touch();
}

void Customer::setPhone(const std::string& p) {
    phone = p;
    touch();
}

void Customer::setAddress(const std::string& a) {
    address = a;
    touch();
}

void Customer::setEmail(const std::string& e) {
    email = e;
    touch();
}

// ─────────────────────────────────────────────
// Operators
// ─────────────────────────────────────────────

bool Customer::operator==(const Customer& o) const { return id == o.id; }
bool Customer::operator<(const Customer& o) const { return id < o.id; }

// Print Customer Details
void printCustomer(const Customer& customer) {
    std::vector<std::string> headers = {"ID", "Name", "Phone", "Address", "Email"};
    std::vector<std::vector<std::string>> rows = {{
        std::to_string(customer.getId()),
        customer.getName(),
        customer.getPhone(),
        customer.getAddress(),
        customer.getEmail()
    }};
    OutputFormatter::printTable(headers, rows);
}
