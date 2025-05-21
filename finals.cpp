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
#include <stream>

// Data structure includes - Team Member 2
#include <vector>    // For storing collections - Team Member 2
#include <memory>    // For shared_ptr - Team Member 2
#include <algorithm> // For algorithms like replace - Team Member 2


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



// LockBox class - Team Member 2
class LockBox {
private:
   static int nextId;
   int id;
   double amount;
   time_t unlockTimestamp;
   bool isActive;
   time_t releaseTimestamp;
   string creationTimestamp;
   string ownerUsername;


public:
   // Constructor - Team Member 2
   LockBox(double amt, time_t uTimestamp, const string& username)
       : amount(amt), unlockTimestamp(uTimestamp), isActive(true), ownerUsername(username) {
       id = nextId++;
       releaseTimestamp = 0;
       creationTimestamp = getCurrentDateTime();
   }


   // Constructor for loading from file - Team Member 2
   LockBox(int boxId, double amt, time_t uTimestamp, bool active,
           time_t rTimestamp, const string& timestamp, const string& username)
       : id(boxId), amount(amt), unlockTimestamp(uTimestamp), isActive(active),
       releaseTimestamp(rTimestamp), creationTimestamp(timestamp), ownerUsername(username) {
       if (boxId >= nextId) {
           nextId = boxId + 1;
       }
   }


   // Accessor methods - Team Member 2
   int getId() const { return id; }
   double getAmount() const { return amount; }
   time_t getUnlockTimestamp() const { return unlockTimestamp; }
   bool getIsActive() const { return isActive; }
   time_t getReleaseTimestamp() const { return releaseTimestamp; }
   string getCreationTimestamp() const { return creationTimestamp; }
   string getOwnerUsername() const { return ownerUsername; }


   // Release the lock box - Team Member 2
   void release() {
       isActive = false;
       releaseTimestamp = time(0);
   }


   // Calculate seconds remaining until unlock - Team Member 2
   int secondsRemaining() const {
       if (!isActive) return 0;
       time_t now = time(0);
       return static_cast<int>(unlockTimestamp - now);
   }


   // Check if lock box should be released - Team Member 2
   bool shouldRelease() const {
       if (!isActive) return false;
       time_t now = time(0);
       return now >= unlockTimestamp;
   }


   // Save to file stream - Team Member 2
   void saveToFile(ofstream& file) const {
       file << id << "|"
           << amount << "|"
           << unlockTimestamp << "|"
           << (isActive ? 1 : 0) << "|"
           << releaseTimestamp << "|"
           << creationTimestamp << "|"
           << ownerUsername << endl;
   }


   // Static method to load from file stream - Team Member 2
   static shared_ptr<LockBox> loadFromFile(ifstream& file) {
       string line;
       if (getline(file, line)) {
           istringstream iss(line);
           string token;
           vector<string> tokens;


           while (getline(iss, token, '|')) {
               tokens.push_back(token);
           }


           if (tokens.size() >= 7) {
               int id = stoi(tokens[0]);
               double amount = stod(tokens[1]);
               time_t unlockTimestamp = static_cast<time_t>(stoll(tokens[2]));
               bool active = stoi(tokens[3]) == 1;
               time_t releaseTimestamp = static_cast<time_t>(stoll(tokens[4]));
               string timestamp = tokens[5];
               string username = tokens[6];


               return make_shared<LockBox>(id, amount, unlockTimestamp, active, releaseTimestamp, timestamp, username);
           }
       }
       return nullptr;
   }
};


int LockBox::nextId = 1; // Static member initialization - Team Member 2


// ReleaseEvent class - Team Member 2
class ReleaseEvent {
private:
   int lockBoxId;
   time_t releaseTimestamp;
   double releasedAmount;
   string username;
   string timestamp;  // Exact timestamp when the release occurred


public:
   // Constructor - Team Member 2
   ReleaseEvent(int lbId, time_t rTimestamp, double amount, const string& uname)
       : lockBoxId(lbId), releaseTimestamp(rTimestamp), releasedAmount(amount), username(uname) {
       timestamp = getCurrentDateTime();
   }


   // Constructor for loading from file - Team Member 2
   ReleaseEvent(int lbId, time_t rTimestamp, double amount, const string& uname, const string& ts)
       : lockBoxId(lbId), releaseTimestamp(rTimestamp), releasedAmount(amount), username(uname), timestamp(ts) {}


   // Accessor methods - Team Member 2
   int getLockBoxId() const { return lockBoxId; }
   time_t getReleaseTimestamp() const { return releaseTimestamp; }
   double getReleasedAmount() const { return releasedAmount; }
   string getUsername() const { return username; }
   string getTimestamp() const { return timestamp; }


   // Save to file stream - Team Member 2
   void saveToFile(ofstream& file) const {
       file << lockBoxId << "|"
           << releaseTimestamp << "|"
           << releasedAmount << "|"
           << username << "|"
           << timestamp << endl;
   }


   // Static method to load from file stream - Team Member 2
   static shared_ptr<ReleaseEvent> loadFromFile(ifstream& file) {
       string line;
       if (getline(file, line)) {
           istringstream iss(line);
           string token;
           vector<string> tokens;


           while (getline(iss, token, '|')) {
               tokens.push_back(token);
           }


           if (tokens.size() >= 5) {
               int id = stoi(tokens[0]);
               time_t rTimestamp = static_cast<time_t>(stoll(tokens[1]));
               double amount = stod(tokens[2]);
               string uname = tokens[3];
               string ts = tokens[4];


               return make_shared<ReleaseEvent>(id, rTimestamp, amount, uname, ts);
           }
       }
       return nullptr;
   }
};
