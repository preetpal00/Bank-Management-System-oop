# ==============================================================================
# Bank Management System - Build Automation
# Supports both direct compilation via clang++ (fast) and CMake build
# ==============================================================================

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -DASIO_STANDALONE -Ibackend/include
LDFLAGS = -lsqlite3 -lpthread -framework Security -framework CoreFoundation

SRC = backend/src/main.cpp \
      backend/src/Person.cpp \
      backend/src/Customer.cpp \
      backend/src/Admin.cpp \
      backend/src/BankAccount.cpp \
      backend/src/SavingsAccount.cpp \
      backend/src/CurrentAccount.cpp \
      backend/src/Transaction.cpp \
      backend/src/DatabaseManager.cpp \
      backend/src/Authentication.cpp \
      backend/src/Bank.cpp \
      backend/src/ReportManager.cpp

TARGET = bank_backend

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "==> Compiling Bank Management System C++ Backend..."
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)
	@echo "==> Build successful: ./$(TARGET)"

run: $(TARGET)
	@echo "==> Starting Bank Management System on http://localhost:18080..."
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf build

cmake-build:
	mkdir -p build && cd build && cmake ../backend && cmake --build .
	cp build/bank_backend .

.PHONY: all run clean cmake-build
