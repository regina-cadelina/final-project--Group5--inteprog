/************************************************************
 * - Cadeliña: Core Classes and Utility Functions
 * - Estrella: LockBox and Release Event System
 * - Alcantara 3: User and Admin Classes
 * - Lique 4: Main Program and User Interface
 ************************************************************/

#include <iostream>
#include <string> 
#include <ctime>
#include <iomanip>

// File handling includes
#include <fstream> 
#include <sstream>

using namespace std;

// Forward declarations 
class Person;
class User;
class Admin;
class LockBox;
class ReleaseEvent;
class TransactionLogger;

const string TRANSACTION_LOG_FILE = "transaction_log.txt";
const string RECEIPTS_DIR = "receipts/";


// Utility function to get current date and time
string getCurrentDateTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buffer);
}

// Abstract Base Class
class Person {
protected:
    string username;
    string password;
    string registrationDate;

public:
    // Constructor
    Person(const string& uname, const string& pass)
        : username(uname), password(pass) {
        registrationDate = getCurrentDateTime();
    }

    // Constructor for loading from file
    Person(const string& uname, const string& pass, const string& regDate)
        : username(uname), password(pass), registrationDate(regDate) {}

    virtual ~Person() = default;  // Virtual destructor

    // Accessor methods
    string getUsername() const { return username; }
    string getPassword() const { return password; } // Only used for saving data
    string getRegistrationDate() const { return registrationDate; }

    // Password verification
    bool checkPassword(const string& pass) const {
        return password == pass;
    }

    // Pure virtual method
    virtual void displayDetails() const = 0;

    // Virtual method for file saving
    virtual void saveToFile(ofstream& file) const {
        file << username << "|"
            << password << "|"
            << registrationDate << endl;
    }
};

// Transaction Logger class
class TransactionLogger {
public:
    enum TransactionType {
        USER_REGISTRATION,
        USER_LOGIN,
        USER_LOGOUT,
        ADMIN_LOGIN,
        ADMIN_LOGOUT,
        CREATE_LOCKBOX,
        RELEASE_LOCKBOX,
        BALANCE_UPDATE,
        USER_STATUS_CHANGE
    };

    // Log transaction to file
    static void logTransaction(
        TransactionType type,
        const string& username,
        const string& details = "",
        double amount = 0.0
    ) {
        // Create receipts directory if it doesn't exist
        if (!filesystem::exists(RECEIPTS_DIR)) {
            filesystem::create_directory(RECEIPTS_DIR);
        }
        // Create user directory if it doesn't exist
        string userDir = RECEIPTS_DIR + username + "/";
        if (!filesystem::exists(userDir)) {
            filesystem::create_directory(userDir);
        }
        // User-specific transaction log file
        string userLogFile = userDir + "transaction_log.txt";
        ofstream logFile(userLogFile, ios::app);
        if (logFile.is_open()) {
            logFile << getCurrentDateTime() << "|"
                    << getTransactionTypeName(type) << "|"
                    << username << "|"
                    << amount << "|"
                    << details << endl;
            logFile.close();
        }
    }

    // Generate receipt for transaction
    static void generateReceipt(
        TransactionType type,
        const string& username,
        const string& details,
        double amount,
        int lockBoxId = -1
    ) {
        // Create receipts directory if it doesn't exist
        if (!filesystem::exists(RECEIPTS_DIR)) {
            filesystem::create_directory(RECEIPTS_DIR);
        }

        // Create user directory if it doesn't exist
        string userDir = RECEIPTS_DIR + username + "/";
        if (!filesystem::exists(userDir)) {
            filesystem::create_directory(userDir);
        }

        // Generate unique receipt filename
        string timestamp = getCurrentDateTime();
        replace(timestamp.begin(), timestamp.end(), ' ', '_');
        replace(timestamp.begin(), timestamp.end(), ':', '-');

        string receiptFile = userDir + getTransactionTypeName(type) + "_" +
                                timestamp + ".txt";

        ofstream receipt(receiptFile);
        if (receipt.is_open()) {
            receipt << "=== TIME-LOCKED SAVINGS SYSTEM RECEIPT ===\n";
            receipt << "Date & Time: " << getCurrentDateTime() << "\n";
            receipt << "Transaction Type: " << getTransactionTypeName(type) << "\n";
            receipt << "Username: " << username << "\n";

            if (lockBoxId != -1) {
                receipt << "Lock Box ID: " << lockBoxId << "\n";
            }

            if (amount != 0.0) {
                receipt << "Amount: $" << fixed << setprecision(2) << amount << "\n";
            }

            if (!details.empty()) {
                receipt << "Details: " << details << "\n";
            }

            receipt << "=======================================\n";
            receipt << "Thank you for using our Time-Locked Savings System!\n";
            receipt.close();

            cout << "Receipt generated: " << receiptFile << endl;
        }
    }

private:
    // Helper method to get transaction type name
    static string getTransactionTypeName(TransactionType type) {
        switch (type) {
            case USER_REGISTRATION: return "USER_REGISTRATION";
            case USER_LOGIN: return "USER_LOGIN";
            case USER_LOGOUT: return "USER_LOGOUT";
            case ADMIN_LOGIN: return "ADMIN_LOGIN";
            case ADMIN_LOGOUT: return "ADMIN_LOGOUT";
            case CREATE_LOCKBOX: return "CREATE_LOCKBOX";
            case RELEASE_LOCKBOX: return "RELEASE_LOCKBOX";
            case BALANCE_UPDATE: return "BALANCE_UPDATE";
            case USER_STATUS_CHANGE: return "USER_STATUS_CHANGE";
            default: return "UNKNOWN";
        }
    }
};