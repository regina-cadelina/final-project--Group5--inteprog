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

// User class 
class User : public Person {
private:
   double balance;
   vector<shared_ptr<LockBox>> lockBoxes;
   bool active;

public:
// Constructor 
User(const string& uname, const string& pass, double initialBalance = 1000.0)
       : Person(uname, pass), balance(initialBalance), active(true) {}

// Constructor for loading from file
  User(const string& uname, const string& pass, double initialBalance,
       bool isActive, const string& regDate)
       : Person(uname, pass, regDate), balance(initialBalance), active(isActive) {}

// Accessor methods 
double getBalance() const { return balance; }
   bool isActive() const { return active; }

// Set user active status 
  void setActive(bool status) {
       active = status;
       string details = "Status changed to " + string(active ? "Active" : "Inactive");
       TransactionLogger::logTransaction(
           TransactionLogger::USER_STATUS_CHANGE,
           username,
           details
       );
   }
// Create a new look box
bool createLockBox(double amount, time_t unlockTimestamp) {
       if (amount <= 0 || amount > balance) {
           cout << "Invalid amount or insufficient balance.\n";
           return false;
       }


       balance -= amount;
       auto newBox = make_shared<LockBox>(amount, unlockTimestamp, username);
       lockBoxes.push_back(newBox);


       stringstream details;
       details << "Created Lock Box for " << (unlockTimestamp - time(0)) << " seconds";
       TransactionLogger::logTransaction(
           TransactionLogger::CREATE_LOCKBOX,
           username,
           details.str(),
           amount
       );


       TransactionLogger::generateReceipt(
           TransactionLogger::CREATE_LOCKBOX,
           username,
           details.str(),
           amount,
           newBox->getId()
       );



       cout << "Lock Box created successfully! Funds locked for "
            << (unlockTimestamp - time(0)) << " seconds." << endl;
       return true;
   }

// View user's lock boxes
 void viewLockBoxes(bool showActive = true, bool showReleased = true) const {
       bool found = false;
       cout << "\n==== " << (showActive ? "ACTIVE " : "")
           << (showActive && showReleased ? "& " : "")
           << (showReleased ? "RELEASED " : "") << "LOCK BOXES ====\n";


       for (const auto& box : lockBoxes) {
           if ((showActive && box->getIsActive()) || (showReleased && !box->getIsActive())) {
               cout << "ID: " << box->getId()
                   << " | Amount: $" << fixed << setprecision(2) << box->getAmount()
                   << " | Unlocks In: ";
               if (box->getIsActive()) {
                   int secs = box->secondsRemaining();
                   if (secs > 0)
                       cout << secs << " seconds";
                   else
                       cout << "Ready to unlock";
               } else {
                   time_t released = box->getReleaseTimestamp();
                   cout << "Released at " << ctime(&released);
               }
               cout << endl;
               found = true;
           }
       }


       if (!found) {
           cout << "No lock boxes to display.\n";
       }
   }

// Check and release lock boxes that have reached their unlock time 
 void checkAndReleaseLockBoxes() {
       for (auto& box : lockBoxes) {
  if (box->getIsActive() && box->shouldRelease()) {
               box->release();
               balance += box->getAmount();


               auto event = make_shared<ReleaseEvent>(
                   box->getId(),
                   box->getReleaseTimestamp(),
                   box->getAmount(),
                   username
               );
               releaseLog.push_back(event);


               string details = "Lock Box #" + to_string(box->getId()) + " released";
               TransactionLogger::logTransaction(
                   TransactionLogger::RELEASE_LOCKBOX,
                   username,
                   details,
                   box->getAmount()
               );


               TransactionLogger::generateReceipt(
                   TransactionLogger::RELEASE_LOCKBOX,
                   username,
                   details,
                   box->getAmount(),
                   box->getId()
               );


               cout << "\n*** NOTIFICATION: Lock Box #" << box->getId()
                   << " has been unlocked! $" << fixed << setprecision(2)
                   << box->getAmount() << " has been returned to your balance. ***\n";
           }
       }
   }

// Display user details 
 void displayDetails() const override {
       cout << "Username: " << username
           << " | Balance: $" << fixed << setprecision(2) << balance
           << " | Status: " << (active ? "Active" : "Inactive")
           << " | Lock Boxes: " << lockBoxes.size()
           << " | Registration Date: " << registrationDate << endl;
 }

// Add a lock box to the user 
 void addLockBox(shared_ptr<LockBox> box) {
       lockBoxes.push_back(box);
   }

// Get all lock boxes
const vector<shared_ptr<LockBox>>& getLockBoxes() const {
       return lockBoxes;
   }

// Save user data to file
void saveToFile(ofstream& file) const override {
       file << username << "|"
           << password << "|"
           << balance << "|"
           << (active ? 1 : 0) << "|"
           << registrationDate << endl;
   }

// Load user from file
 static shared_ptr<User> loadFromFile(ifstream& file) {
       string line;
       if (getline(file, line)) {
           istringstream iss(line);
           string token;
           vector<string> tokens;


           while (getline(iss, token, '|')) {
               tokens.push_back(token);
           }


           if (tokens.size() >= 5) {
               string uname = tokens[0];
               string pass = tokens[1];
               double balance = stod(tokens[2]);
               bool active = stoi(tokens[3]) == 1;
               string regDate = tokens[4];


               return make_shared<User>(uname, pass, balance, active, regDate);
           }
 }
       return nullptr;
   }
};


// Admin class
class Admin : public Person {
public:
   // Constructor =
   Admin(const string& uname, const string& pass)
       : Person(uname, pass) {}


   // Constructor for loading from file 
   Admin(const string& uname, const string& pass, const string& regDate)
       : Person(uname, pass, regDate) {}


   // Display admin details 
   void displayDetails() const override {
       cout << "Admin Username: " << username
           << " | Registration Date: " << registrationDate << endl;
   }


   // View all users 
   void viewAllUsers() const {
       cout << "\n==== ALL USERS ====\n";
       if (users.empty()) {
           cout << "No users registered.\n";
           return;
       }


       for (const auto& user : users) {
           user->displayDetails();
       }
   }


   // Toggle user active status 
   void toggleUserStatus(const string& username) {
       for (auto& user : users) {
           if (user->getUsername() == username) {
               user->setActive(!user->isActive());
               cout << "User " << username << " status changed to "
                   << (user->isActive() ? "Active" : "Inactive") << endl;
               return;
           }
       }
       cout << "User not found.\n";
       return;
   }


   // View release log 
   void viewReleaseLog() const {
       cout << "\n==== RELEASE EVENT LOG ====\n";
       if (releaseLog.empty()) {
           cout << "No release events have occurred.\n";
           return;
       }


       for (const auto& event : releaseLog) {
           time_t released = event->getReleaseTimestamp();
           cout << "Lock Box ID: " << event->getLockBoxId()
               << " | User: " << event->getUsername()
               << " | Released At: " << ctime(&released)
               << " | Amount: $" << fixed << setprecision(2)
               << event->getReleasedAmount() << endl;
       }
   }


   // Clear release logs 
   void clearReleaseLogs() {
       releaseLog.clear();
       cout << "Release logs cleared.\n";
   }
};


// Function to register a new user 
void registerUser() {
   string username, password;
   double initialBalance;


   cout << "\n==== USER REGISTRATION ====\n";
   cout << "Enter username: ";
   cin >> username;


   // Check if username already exists
   for (const auto& user : users) {
       if (user->getUsername() == username) {
           cout << "Username already exists. Please choose another.\n";
           return;
       }
   }


   cout << "Enter password: ";
   cin >> password;


   cout << "Enter initial balance: $";
   cin >> initialBalance;


   if (initialBalance < 0) {
       cout << "Initial balance cannot be negative.\n";
       return;
   }


   users.push_back(make_shared<User>(username, password, initialBalance));


   // Log the transaction
   TransactionLogger::logTransaction(
       TransactionLogger::USER_REGISTRATION,
       username,
       "User registered",
       initialBalance
   );


   cout << "User registered successfully!\n";
}


// Function to login a user 
bool loginUser() {
   string username, password;


   cout << "\n==== USER LOGIN ====\n";
   cout << "Enter username: ";
   cin >> username;
   cout << "Enter password: ";
   cin >> password;


   for (auto& user : users) {
       if (user->getUsername() == username) {
           if (!user->isActive()) {
               cout << "This account is inactive. Please contact the admin.\n";
               return false;
           }


           if (user->checkPassword(password)) {
               currentUser = user;
               isUserLoggedIn = true;


               // Log the transaction
               TransactionLogger::logTransaction(
                   TransactionLogger::USER_LOGIN,
                   username,
                   "User login"
               );


               cout << "Login successful! Welcome, " << username << "!\n";
               currentUser->checkAndReleaseLockBoxes(); // Check for unlockable boxes on login
               return true;
           } else {
               cout << "Incorrect password.\n";
               return false;
           }
       }
   }


   cout << "User not found.\n";
   return false;
}


// Function to login admin 
bool loginAdmin() {
   string username, password;


   cout << "\n==== ADMIN LOGIN ====\n";
   cout << "Enter admin username: ";
   cin >> username;
   cout << "Enter admin password: ";
   cin >> password;


   if (systemAdmin->getUsername() == username && systemAdmin->checkPassword(password)) {
       isAdminLoggedIn = true;


       // Log the transaction
       TransactionLogger::logTransaction(
           TransactionLogger::ADMIN_LOGIN,
           username,
           "Admin login"
       );


       cout << "Admin login successful!\n";
       return true;
   }


   cout << "Invalid admin credentials.\n";
   return false;
}













