#include <iostream>
#include <sqlite3.h>
#include <string>
#include <sstream>  // Added for istringstream
#include <iomanip>  // Added for get_time
#include <ctime>
#include <random>
#include <cstdlib>
#include "utilities.h"

using namespace std;

//getTerminalWidth();//for finding the center
//void printCentered(const string& text);



string returnit(sqlite3* db, const string& froms, const string& wheres, const string& equals, const string& returns) {
    string result;
    sqlite3_stmt* stmt;
    string sql = "SELECT " + returns + " FROM " + froms + " WHERE " + wheres + " = ?";

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return result;
    }

    // Bind the parameter
    if (sqlite3_bind_text(stmt, 1, equals.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return result;
    }

    // Execute the statement and fetch the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    } else {
        cerr << "No results found" << endl;
    }

    // Clean up
    sqlite3_finalize(stmt);

    return result;
}
// Function to create the tables if they don't exist
void createTableIfNotExists(sqlite3* db) {
    // SQL command to create the Semester_Info table
    char* createSemesterInfoTableSQL = "CREATE TABLE IF NOT EXISTS Semester_Info("
                                        "ID INTEGER PRIMARY KEY, "
                                       "NO INTEGER, "
                                       "AMOUNT REAL NOT NULL, "
                                       "INITIAL_DATE TEXT NOT NULL, "
                                       "END_DATE TEXT NOT NULL, "
                                       "PUNISH_DATE TEXT NOT NULL, "
                                       "PUNISHMENT_AMOUNT REAL NOT NULL, "
                                       "GRADE INTEGER NOT NULL, "
                                       "MAX_PUNISH_AMOUNT REAL NOT NULL, "
                                       "REG_FEE REAL NOT NULL, "
                                       "PASSWORD REAL NOT NULL, "
                                       "SCHOOL_NAME TEXT NOT NULL);";

    // SQL command to create the Users table
    char* createUsersTableSQL = "CREATE TABLE IF NOT EXISTS Users("
                                "USER_ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                                "FNAME TEXT NOT NULL, "
                                "LNAME TEXT NOT NULL, "
                                "AGE INTEGER NOT NULL, "
                                "SEX TEXT NOT NULL, "
                                "GRADE INTEGER NOT NULL, "
                                "CONTACT TEXT NOT NULL, "
                                "ADDRESS TEXT NOT NULL, "
                                "DATETIME TEXT DEFAULT CURRENT_TIMESTAMP);";

    // SQL command to create the Transactions table
    char* createTransactionsTableSQL = "CREATE TABLE IF NOT EXISTS Transactions("
                                       "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                                       "AMOUNT REAL NOT NULL, "
                                       "REASON TEXT NOT NULL, "
                                       "USER_ID INTEGER NOT NULL, "
                                       "STATUS INTEGER NOT NULL, "
                                       "DATETIME TEXT DEFAULT CURRENT_TIMESTAMP, "
                                       "FOREIGN KEY(USER_ID) REFERENCES Users(USER_ID));";

    char* errorMessage = nullptr;

    // Execute SQL command to create the Semester_Info table
    int exit1 = sqlite3_exec(db, createSemesterInfoTableSQL, NULL, 0, &errorMessage);
    if (exit1 != SQLITE_OK) {
        cerr << "Error Creating Semester_Info Table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    } else {
        cout << "Semester_Info Table created successfully." << endl;
    }

    // Execute SQL command to create the Users table
    int exit2 = sqlite3_exec(db, createUsersTableSQL, NULL, 0, &errorMessage);
    if (exit2 != SQLITE_OK) {
        cerr << "Error Creating Users Table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    } else {
        cout << "Users Table created successfully." << endl;
    }

    // Execute SQL command to create the Transactions table
    int exit3 = sqlite3_exec(db, createTransactionsTableSQL, NULL, 0, &errorMessage);
    if (exit3 != SQLITE_OK) {
        cerr << "Error Creating Transactions Table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    } else {
        cout << "Transactions Table created successfully." << endl;
    }

    cout << "\033[1m \t\tAll tables created successfully. \033[0m "<< endl;
}

void displayTransactions(sqlite3* db);
// Helper function to add months to a given date
string addMonthsToDate(const string& date, int months) {
    tm tm = {};
    istringstream ss(date);
    ss >> get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        cerr << "Failed to parse date" << endl;
        return "";
    }
    tm.tm_mon += months;
    mktime(&tm);
    char buffer[11];
    strftime(buffer, 11, "%Y-%m-%d", &tm);
    return string(buffer);
}

// Function to insert student data into the Student table
void registerStudent(sqlite3* db) {
    std::string fname, lname, gender, address, age, contact,term;
    int grade,status=0;
    char* zErrMsg=0;

    mt19937 gen(time(0));

    uniform_int_distribution<> digitDistrib(1, 9);
    int numDigit = digitDistrib(gen); // random ID generator
    uniform_int_distribution<> distrib(0, pow(10, numDigit) - 1);
    int id = distrib(gen);

    cout << endl << id << endl;

    std::cout << "Enter first name: ";
    std::cin >> fname;
    std::cout << "Enter last name: ";
    std::cin >> lname;
    std::cout << "Enter Age: ";
    std::cin >> age;
    std::cout << "Enter gender: ";
    std::cin >> gender;
    std::cout << "Enter grade: ";
    std::cin >> grade;
    std::cout << "Enter address: ";
    std::cin.ignore();
    std::getline(std::cin, address);

    std::cout << "Enter contact: ";
    std::cin >> contact;


    std::string insertDataSQL = "INSERT INTO Users (USER_ID, FNAME, LNAME, AGE, SEX, ADDRESS, GRADE, CONTACT) "
                                "VALUES ('" + to_string(id) + "', '" + fname + "', '" + lname + "', '" + age + "', '" + gender + "', '" + address + "', " + std::to_string(grade) + ", " + contact + ");";

    char* errorMessage;
    int exit = sqlite3_exec(db, insertDataSQL.c_str(), NULL, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        std::cerr << "Error Inserting Data: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Data inserted Successfully" << std::endl;
    }


    sqlite3_stmt* stmt;

    std::string sql = "SELECT NO, AMOUNT FROM Semester_Info WHERE GRADE = " + std::to_string(grade);

    // Prepare the statement
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for selecting info from Semester_Info: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    else{
        cout << "Successfully fetched.\n";
    }
        int no = 1;
    // Execute the query and process the results
    while (sqlite3_step(stmt) == SQLITE_ROW) {

       // double amount = sqlite3_column_double(stmt, 2);
       // cout<<amount<<endl;
        no++;

    }
string sgrade = to_string(grade);
     string amount = returnit(db, "Semester_Info", "GRADE", sgrade, "AMOUNT");
string regfee = returnit(db, "Semester_Info", "GRADE", sgrade, "AMOUNT");
cout << "in for loop"<<endl;


        sql = "INSERT INTO Transactions(USER_ID,REASON,AMOUNT,STATUS)"
              "VALUES ("+ to_string(id) + ", '" + "registration"  + "'," + regfee +","+to_string(1)+ ");";


    if (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error while inserting to transaction table: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    else {
        cout << "Successfully inserted to transaction table.\n";
    }

    for (int i=2; i!=no; ++i){
            term = "Term" + to_string(i);
    cout << endl << term << endl;
        sql = "INSERT INTO Transactions(USER_ID,REASON,AMOUNT,STATUS)"
              "VALUES ("+ to_string(id) + ", '" + term  + "'," + amount +","+to_string(status)+ ");";


    if (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error while inserting to transaction table: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    else {
        cout << "Successfully inserted to transaction table.\n";
    } }


    char didpay;
    cout<<"DID THE USER PAY FOR THE TERM 1 TOO(Y/N)? ";
    cin>>didpay;
    if(didpay=='Y'||didpay=='y'){
      term = "Term1";
    //cout << endl << term << endl;
        sql = "INSERT INTO Transactions(USER_ID,REASON,AMOUNT,STATUS)"
              "VALUES ("+ to_string(id) + ", '" + term  + "'," + amount +","+to_string(1)+ ");";


    if (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error while inserting to transaction table: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    else {
        cout << "Successfully inserted to transaction table.\n";
    }


    }else{

          term = "Term1";
    //cout << endl << term << endl;
        sql = "INSERT INTO Transactions(USER_ID,REASON,AMOUNT,STATUS)"
              "VALUES ("+ to_string(id) + ", '" + term  + "'," +amount +","+to_string(0)+ ");";


    if (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error while inserting to transaction table: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    else {
        cout << "Successfully inserted to transaction table.\n";
    }
 }





        displayTransactions(db);
         sqlite3_finalize(stmt);

}



void getTransactionsForUser(sqlite3 *db, int user_id) {
    sqlite3_stmt *stmt;
    char sql[] = "SELECT ID, AMOUNT, REASON, STATUS, DATETIME FROM Transactions WHERE USER_ID = ?;";

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Bind the user_id parameter
    sqlite3_bind_int(stmt, 1, user_id);

    // Execute the statement and loop through the result rows
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char *reason = sqlite3_column_text(stmt, 2);
        int status = sqlite3_column_int(stmt, 3);
        const unsigned char *datetime = sqlite3_column_text(stmt, 4);

        printf("ID: %d, AMOUNT: %.2f, REASON: %s, STATUS: %d, DATETIME: %s\n",
               id, amount, reason, status, datetime);
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

// Function to search for a student by name
void searchStudent(sqlite3* db) {
    string searchCriteria, searchValue;
    cout << "Enter the search criteria (FNAME, LNAME, GRADE, AGE, SEX): ";
    cin >> searchCriteria;
    cout << "Enter the search value: ";
    cin.ignore();
    getline(cin, searchValue);

    string searchSQL = "SELECT USER_ID, FNAME, LNAME, AGE, SEX, GRADE, CONTACT, ADDRESS, DATETIME FROM Users WHERE " + searchCriteria + " LIKE '%" + searchValue + "%';";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, searchSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Searching Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "ID\tFNAME\tLNAME\tAGE\tSEX\tGRADE\tCONTACT\tADDRESS\tDATETIME" << endl;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* fname = sqlite3_column_text(stmt, 1);
        const unsigned char* lname = sqlite3_column_text(stmt, 2);
        const unsigned char* age = sqlite3_column_text(stmt, 3);
        const unsigned char* sex = sqlite3_column_text(stmt, 4);
        int grade = sqlite3_column_int(stmt, 5);
        const unsigned char* contact = sqlite3_column_text(stmt, 6);
        const unsigned char* address = sqlite3_column_text(stmt, 7);
        const unsigned char* datetime = sqlite3_column_text(stmt, 8);

        cout << id << "\t" << fname << "\t" << lname << "\t" << age << "\t" << sex << "\t" << grade << "\t" << contact << "\t" << address << "\t" << datetime << endl;
    }

    sqlite3_finalize(stmt);
}

// Function to search for transactions
void searchTransactions(sqlite3* db) {
    string searchCriteria, searchValue;
    cout << "Enter the search criteria (USER_ID, AMOUNT, REASON, STATUS, DATE): ";
    cin >> searchCriteria;
    cout << "Enter the search value: ";
    cin.ignore();
    getline(cin, searchValue);

    string searchSQL = "SELECT ID, AMOUNT, REASON, USER_ID, STATUS, DATETIME FROM Transactions WHERE " + searchCriteria + " LIKE '%" + searchValue + "%';";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, searchSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Searching Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "ID\tAMOUNT\tREASON\tUSER_ID\tSTATUS\tDATETIME" << endl;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char* reason = sqlite3_column_text(stmt, 2);
        int user_id = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const unsigned char* datetime = sqlite3_column_text(stmt, 5);

        cout << id << "\t" << amount << "\t" << reason << "\t" << user_id << "\t" << (status == 1 ? "Paid" : "Unpaid") << "\t" << datetime << endl;
    }

    sqlite3_finalize(stmt);
}
// Function to display student data from the Student table
void displayStudentData(sqlite3* db) {
    string selectDataSQL = "SELECT User_ID, FNAME, LNAME, AGE, SEX, ADDRESS, GRADE FROM Users;";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, selectDataSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "ID\tFNAME\tLNAME\tAGE\tGENDER\tADDRESS\tGRADE" << endl;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* fname = sqlite3_column_text(stmt, 1);
        const unsigned char* lname = sqlite3_column_text(stmt, 2);
        const unsigned char* dob = sqlite3_column_text(stmt, 3);
        const unsigned char* gender = sqlite3_column_text(stmt, 4);
        const unsigned char* address = sqlite3_column_text(stmt, 5);
        int grade = sqlite3_column_int(stmt, 6);
       // double feesPaid = sqlite3_column_double(stmt, 7);

        cout << id << "\t" << fname << "\t" << lname << "\t" << dob << "\t" << gender << "\t" << address << "\t" << grade << endl;
    }

    sqlite3_finalize(stmt);

}

// Function to insert a transaction
void insertTransaction(sqlite3* db, int user_id, double amount, const string& reason, int status) {
    string insertTransactionSQL = "INSERT INTO Transactions (AMOUNT, REASON, USER_ID, STATUS) "
                                       "VALUES (" + to_string(amount) + ", '" + reason + "', " + to_string(user_id) + ", " + to_string(status) + ");";

    char* errorMessage;
    int exit = sqlite3_exec(db, insertTransactionSQL.c_str(), NULL, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        cerr << "Error Inserting Transaction: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    } else {
        cout << "Transaction inserted successfully" << endl;
    }
}

// Function to mark a transaction as paid
void markTransactionAsPaid(sqlite3* db, int transaction_id) {
    string updateSQL = "UPDATE Transactions SET STATUS = 1 WHERE ID = " + to_string(transaction_id) + ";";

    char* errorMessage;
    int exit = sqlite3_exec(db, updateSQL.c_str(), NULL, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        cerr << "Error Updating Transaction: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    } else {
        cout << "Transaction marked as paid successfully" << endl;
    }
}
void updateStudent(sqlite3* db, int user_id) {
    string fname, lname, gender, address, age, contact;
    string gradeInput;
    int grade = -1; // -1 to indicate no change

    cout << "Enter new first name (leave blank to keep unchanged): ";
    cin.ignore();
    getline(cin, fname);
    cout << "Enter new last name (leave blank to keep unchanged): ";
    getline(cin, lname);
    cout << "Enter new age (leave blank to keep unchanged): ";
    getline(cin, age);
    cout << "Enter new gender (leave blank to keep unchanged): ";
    getline(cin, gender);
    cout << "Enter new grade (leave blank to keep unchanged): ";
    getline(cin, gradeInput);
    if (!gradeInput.empty()) {
        grade = stoi(gradeInput);
    }
    cout << "Enter new address (leave blank to keep unchanged): ";
    getline(cin, address);
    cout << "Enter new contact (leave blank to keep unchanged): ";
    getline(cin, contact);

    stringstream updateSQL;
    updateSQL << "UPDATE Users SET ";

    bool needsComma = false;
    if (!fname.empty()) {
        updateSQL << "FNAME = '" << fname << "'";
        needsComma = true;
    }
    if (!lname.empty()) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "LNAME = '" << lname << "'";
        needsComma = true;
    }
    if (!age.empty()) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "AGE = '" << age << "'";
        needsComma = true;
    }
    if (!gender.empty()) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "SEX = '" << gender << "'";
        needsComma = true;
    }
    if (grade != -1) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "GRADE = " << grade;
        needsComma = true;
    }
    if (!address.empty()) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "ADDRESS = '" << address << "'";
        needsComma = true;
    }
    if (!contact.empty()) {
        if (needsComma) updateSQL << ", ";
        updateSQL << "CONTACT = '" << contact << "'";
        needsComma = true;
    }
    updateSQL << " WHERE USER_ID = " << user_id << ";";

    char* errorMessage;
    int exit = sqlite3_exec(db, updateSQL.str().c_str(), NULL, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        cerr << "Error Updating Data: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    } else {
        cout << "Data updated successfully" << endl;
    }
}

// Function to delete a student and their transactions
void deleteStudent(sqlite3* db, int user_id) {
    string deleteUserSQL = "DELETE FROM Users WHERE USER_ID = " + to_string(user_id) + ";";
    string deleteTransactionsSQL = "DELETE FROM Transactions WHERE USER_ID = " + to_string(user_id) + ";";

    char* errorMessage;

    int exit1 = sqlite3_exec(db, deleteUserSQL.c_str(), NULL, 0, &errorMessage);
    if (exit1 != SQLITE_OK) {
        cerr << "Error Deleting User: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    int exit2 = sqlite3_exec(db, deleteTransactionsSQL.c_str(), NULL, 0, &errorMessage);
    if (exit2 != SQLITE_OK) {
        cerr << "Error Deleting Transactions: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    cout << "User and their transactions deleted successfully" << endl;
}
// Function to display user information based on ID
void userInfo(sqlite3* db, int user_id) {
    string selectUserSQL = "SELECT USER_ID, FNAME, LNAME, AGE, SEX, ADDRESS, GRADE, CONTACT, DATETIME FROM Users WHERE USER_ID = " + to_string(user_id) + ";";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, selectUserSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* fname = sqlite3_column_text(stmt, 1);
        const unsigned char* lname = sqlite3_column_text(stmt, 2);
        const unsigned char* age = sqlite3_column_text(stmt, 3);
        const unsigned char* sex = sqlite3_column_text(stmt, 4);
        const unsigned char* address = sqlite3_column_text(stmt, 5);
        int grade = sqlite3_column_int(stmt, 6);
        const unsigned char* contact = sqlite3_column_text(stmt, 7);
        const unsigned char* datetime = sqlite3_column_text(stmt, 8);

        cout << "ID: " << id << endl;
        cout << "First Name: " << fname << endl;
        cout << "Last Name: " << lname << endl;
        cout << "Age: " << age << endl;
        cout << "Sex: " << sex << endl;
        cout << "Address: " << address << endl;
        cout << "Grade: " << grade << endl;
        cout << "Contact: " << contact << endl;
        cout << "Registered Date: " << datetime << endl;
    } else {
        cout << "No user found with ID " << user_id << endl;
    }

    sqlite3_finalize(stmt);
}
// Function to add a billing item
void addBillingItem(sqlite3* db) {
    string itemName, description;
    double price;

    cout << "Enter item name: ";
    cin >> itemName;
    cout << "Enter description: ";
    cin.ignore();
    getline(cin, description);
    cout << "Enter price: ";
    cin >> price;

    string insertItemSQL = "INSERT INTO Billing_Items (ITEM_NAME, DESCRIPTION, PRICE) "
                                "VALUES ('" + itemName + "', '" + description + "', " + to_string(price) + ");";

    char* errorMessage;
    int exit = sqlite3_exec(db, insertItemSQL.c_str(), NULL, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        cerr << "Error Adding Billing Item: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    } else {
        cout << "Billing Item added successfully" << endl;
    }
}

// Function to display transactions
void displayTransactions(sqlite3* db) {
    string selectTransactionsSQL = "SELECT ID, AMOUNT, REASON, USER_ID, STATUS, DATETIME FROM Transactions;";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, selectTransactionsSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "ID\tAMOUNT\tREASON\tUSER_ID\tSTATUS\tDATETIME" << endl;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char* reason = sqlite3_column_text(stmt, 2);
        int user_id = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const unsigned char* datetime = sqlite3_column_text(stmt, 5);

        cout << id << "\t" << amount << "\t" << reason << "\t" << user_id << "\t" << (status == 1 ? "Paid" : "Unpaid") << "\t" << datetime << endl;
    }

    sqlite3_finalize(stmt);
}

void insertIntoSemesterInfo(sqlite3* db) {
    int no;
    double amount, punishmentAmount, maxPunishAmount, regFee, schoolFee;
    string initialDate, endDate, punishDate, schoolName, password;
    char* errorMessage = nullptr;
    cout << "LOOKS LIKE ITS YOUR FIRST TIME HERE! LETS SET YOU UP!"<<endl;
    cout << "Enter the number of semesters: ";
    cin >> no;

    cout << "Enter the maximum punishment amount: ";
    cin >> maxPunishAmount;
        cout << "Enter the school name: ";
    cin.ignore();
    getline(cin, schoolName);
    cout << "Enter the password: ";
    cin >> password;
       string idate[no];
       string edate[no];
       string pdate[no];
       for(int i = 0; i!=no; i++){

            //cin>>semsfee[i];
    cout << "Enter the initial date of the "<<i+1<<" Term (YYYY-MM-DD): ";
    cin >> idate[i];
    cout << "Enter the end date of the "<<i+1<<" Term (YYYY-MM-DD): ";
    cin >> edate[i];
    cout << "Enter the punishment date of the "<<i+1<<" Term  (YYYY-MM-DD): ";
    cin >> pdate[i];
       }


        for(int i = 9; i!=13; i++){

     cout << "Enter the registration fee of Grade "<<i<<": ";
    cin >> regFee;
    cout << "Enter the amount for each semester of Grade "<<i<<": ";
    cin >> amount;

    cout << "Enter the punishment amount (per month): ";
    cin >> punishmentAmount;

   for(int j = 0; j!=no; j++){

    string sql = "INSERT INTO Semester_Info (NO, AMOUNT, INITIAL_DATE, END_DATE, PUNISH_DATE, PUNISHMENT_AMOUNT,GRADE, MAX_PUNISH_AMOUNT, REG_FEE, SCHOOL_NAME, PASSWORD) "
                 "VALUES (" + to_string(j+1) + ", " + to_string(amount) + ", '" + idate[j] + "', '" + edate[j] + "', '" + pdate[j] + "', " + to_string(punishmentAmount) + ", "
                 + to_string(i)+ ", " + to_string(maxPunishAmount) + ", " + to_string(regFee) + ", '" + schoolName + "', '" + password + "');";

    int exit = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errorMessage);
    if (exit != SQLITE_OK) {
        cerr << "Error inserting data into Semester_Info table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    } else {
        cout << "Semester information and school fee added successfully." << endl;
    }}}
}



int getSemesterInfoCount(sqlite3* db) {
    const char* countSQL = "SELECT COUNT(*) FROM Semester_Info;";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, countSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return count;
}
// Main function to check the record count and insert if necessary
void checkAndInsertSemesterInfo(sqlite3* db) {
    int recordCount = getSemesterInfoCount(db);
    if (recordCount < 3) {
        cout << "Semester_Info table has less than 3 records." << endl;
        insertIntoSemesterInfo(db);

    } else {
        //cout << "Semester_Info table has 3 or more records. No insertion needed." << endl;
    }
}

// Function to display the status page
void statusPage(sqlite3* db) {
    string selectUsersSQL = "SELECT COUNT(*) FROM Users;";
    string selectTransactionsSQL = "SELECT COUNT(*) FROM Transactions;";
    string selectPaidTransactionsSQL = "SELECT COUNT(*) FROM Transactions WHERE STATUS = 1;";
    string selectUnpaidTransactionsSQL = "SELECT COUNT(*) FROM Transactions WHERE STATUS = 0;";

    sqlite3_stmt* stmt;
    int exit;

    exit = sqlite3_prepare_v2(db, selectUsersSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }
    int totalUsers = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        totalUsers = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    exit = sqlite3_prepare_v2(db, selectTransactionsSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }
    int totalTransactions = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        totalTransactions = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    exit = sqlite3_prepare_v2(db, selectPaidTransactionsSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }
    int totalPaidTransactions = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        totalPaidTransactions = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    exit = sqlite3_prepare_v2(db, selectUnpaidTransactionsSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }
    int totalUnpaidTransactions = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        totalUnpaidTransactions = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    cout << "STATUS PAGE" << endl;
    cout << "Total Users: " << totalUsers << endl;
    cout << "Total Transactions: " << totalTransactions << endl;
    cout << "Total Paid Transactions: " << totalPaidTransactions << endl;
    cout << "Total Unpaid Transactions: " << totalUnpaidTransactions << endl;
}

// Main menu function
void mainmenu(sqlite3* db) {
    int choice;
    while (true) {
     cout<<"Menu"<<endl
        <<"1. Register Student"<<endl
        <<"2. Display All Students"<<endl
        <<"3. Search Student"<<endl
        <<"4. Display All Transactions"<<endl
        <<"5. Search Transactions"<<endl
        <<"6. Insert Transaction"<<endl
        <<"7. Mark Transaction As Paid"<<endl
        <<"8. Update Student"<<endl
        <<"9. Delete Student"<<endl
        <<"10. Display User Information"<<endl
        <<"11. Status Page"<<endl
        <<"12. Exit"<<endl
        <<"Enter your choice: ";

    // Print the block with red color and centered
//    printColoredCenteredBlock("\033[31m", textBlock);










        cin >> choice;

        switch (choice) {
            case 1:
                registerStudent(db);
                break;
            case 2:
                displayStudentData(db);
                break;
            case 3:
                searchStudent(db);
                break;
            case 4:
                displayTransactions(db);
                break;
            case 5:
                searchTransactions(db);
                break;
            case 6: {
                int user_id;
                double amount;
                string reason;
                int status;
                cout << "Enter User ID: ";
                cin >> user_id;
                cout << "Enter Amount: ";
                cin >> amount;
                cout << "Enter Reason: ";
                cin.ignore();
                getline(cin, reason);
                cout << "Enter Status (1 for Paid, 0 for Unpaid): ";
                cin >> status;
                insertTransaction(db, user_id, amount, reason, status);
                break;
            }
            case 7: {
                int transaction_id;
                cout << "Enter Transaction ID: ";
                cin >> transaction_id;
                markTransactionAsPaid(db, transaction_id);
                break;
            }
            case 8: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                updateStudent(db, user_id);
                break;
            }
            case 9: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                deleteStudent(db, user_id);
                break;
            }
            case 10: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                userInfo(db, user_id);
                break;
            }
            case 11:
                statusPage(db);
                break;
            case 12:
                sqlite3_close(db);
               // return 0;check here
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    sqlite3_close(db);
    system("cls");
    mainmenu(db);
}

int main() {
    enableVirtualTerminalProcessing();
    sqlite3* DB;






    int exit = sqlite3_open("school_billing.db", &DB);

    if (exit) {
        cerr << "Error opening DB: " << sqlite3_errmsg(DB) << endl;
        return exit;
    } else {
        cout << "Opened Database Successfully!" << endl;
    }

    // Create the tables if they don't exist
    createTableIfNotExists(DB);
    checkAndInsertSemesterInfo(DB);
    //Userinfo(DB,63);
    mainmenu(DB);

    sqlite3_close(DB);

    return 0;
}
