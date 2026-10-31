cmake_minimum_required(VERSION 3.15)
project(BankManagementSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Standalone Asio (No Boost required)
add_definitions(-DASIO_STANDALONE)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Backend source files
set(SOURCES
    src/main.cpp
    src/Person.cpp
    src/Customer.cpp
    src/Admin.cpp
    src/BankAccount.cpp
    src/SavingsAccount.cpp
    src/CurrentAccount.cpp
    src/Transaction.cpp
    src/DatabaseManager.cpp
    src/Authentication.cpp
    src/Bank.cpp
    src/ReportManager.cpp
)

# Executable target
add_executable(bank_backend ${SOURCES})

# Target include directories
target_include_directories(bank_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries (SQLite3, pthreads, system crypto)
find_package(SQLite3 REQUIRED)
target_link_libraries(bank_backend PRIVATE
    SQLite::SQLite3
    pthread
)

if(APPLE)
    target_link_libraries(bank_backend PRIVATE "-framework Security" "-framework CoreFoundation")
endif()
