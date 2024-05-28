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
string  ACSword ;
int ACS;
void reseter(sqlite3* db);
void check_pw(sqlite3* db);
void mainmenu(sqlite3* db);
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
                                   "PASSWORD TEXT NOT NULL, "
                                   "SCHOOL_NAME TEXT NOT NULL, "
                                   "ACS INTEGER, "
                                   "ACSWORD TEXT);";
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

// Function to display transactions
void displayTransactions(sqlite3* db) {
    string selectTransactionsSQL = "SELECT ID, AMOUNT, REASON, USER_ID, STATUS, DATETIME FROM Transactions;";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, selectTransactionsSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout <<left<<setw(10)<< "ID" <<left<<setw(10)<<"AMOUNT"<<left<<setw(15)<<"REASON"<<left<<setw(15)<<"USER_ID"<<left<<setw(15)<<"STATUS"<<left<<setw(10)<<"DATETIME" << endl;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char* reason = sqlite3_column_text(stmt, 2);
        int user_id = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const unsigned char* datetime = sqlite3_column_text(stmt, 5);

        cout <<left<<setw(10) <<id << left<<setw(10) << amount << left<<setw(15) << reason << left<<setw(15) << user_id << left<<setw(15) << (status == 1 ? "Paid" : "Unpaid") << left<<setw(10) << datetime << endl;

    }

    sqlite3_finalize(stmt);
}

// Function to insert student data into the Student table
void registerStudent(sqlite3* db) {
    std::string fname, lname, gender, address, age, contact,term;
    int grade,status=0;
    double amount,feesPaid;
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

        double amount = sqlite3_column_double(stmt, 2);
        cout<<amount<<endl;
        no++;

    }

cout << "in for loop"<<endl;
    for (int i=1; i<=no; ++i){
            term = "Term" + to_string(i);
    cout << endl << term << endl;
        sql = "INSERT INTO Transactions(USER_ID,REASON,AMOUNT,STATUS)"
              "VALUES ("+ to_string(id) + ", '" + term  + "'," + to_string(amount) +","+to_string(status)+ ");";


    if (sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg) != SQLITE_OK) {
            std::cerr << "SQL error while inserting to transaction table: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    else {
        cout << "Successfully inserted to transaction table.\n";
    } }

        displayTransactions(db);
         sqlite3_finalize(stmt);

}

//Gets the transaction info for the user
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
    int choice;
    cout << "Enter the search criteria "<<endl;
    cout << "1. FNAME"<<endl;
    cout << "2. LNAME"<<endl;
    cout << "3. GRADE"<<endl;
    cout << "4. AGE"<<endl;
    cout << "5. SEX"<<endl;
    cout << "6. EXIT"<<endl;
    cout << "Enter choice: ";
    cin>> choice;
    switch(choice)
    {
        case 1:
        searchCriteria= "FNAME";
        break;
        case 2:
        searchCriteria= "LNAME";
        break;
        case 3:
        searchCriteria= "GRADE";
        break;
        case 4:
        searchCriteria= "AGE";
        break;
        case 5:
        searchCriteria= "SEX";
        break;
        case 6:
            system("cls");
            mainmenu(db);
    }


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

    cout <<  left<<setw(10) <<"ID"<< left<<setw(15) <<"FNAME"<< left<<setw(15) <<"LNAME"<< left<<setw(15)
    <<"AGE"<< left<<setw(15) <<"SEX"<< left<<setw(15) <<"GRADE"<< left<<setw(15) <<"CONTACT"<< left<<setw(15) <<"ADDRESS"<< left<<setw(15) <<"DATETIME" << endl;
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

       cout <<left<<setw(10)<< id << left<<setw(15) << fname << left<<setw(15) << lname << left<<setw(15) << age << left<<setw(15)
         << sex << left<<setw(15) << grade << left<<setw(15) << contact << left<<setw(15) << address << left<<setw(15) << datetime << endl;
    }

    sqlite3_finalize(stmt);
}

// Searchs for transactions
void searchTransactions(sqlite3* db) {
    string searchCriteria, searchValue;
    int choice;
    cout << "Enter the search criteria "<<endl;
    cout << "1. USER_ID"<<endl;
    cout << "2. STATUS(0 - unpaid or 1 - paid)"<<endl;
    cout << "3. EXIT"<<endl;
    cout << "Enter choice: ";
    cin>> choice;
    switch(choice)
    {
        case 1:
        searchCriteria= "USER_ID";
        break;
        case 2:
        searchCriteria= "STATUS";
        break;
        case 3:
            system("cls");
            mainmenu(db);
    }
    cout << "Enter "<<searchCriteria<<": ";
    cin.ignore();
    getline(cin, searchValue);

    string searchSQL = "SELECT ID, AMOUNT, REASON, USER_ID, STATUS, DATETIME FROM Transactions WHERE " + searchCriteria + " LIKE '%" + searchValue + "%';";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, searchSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Searching Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << left << setw(10) << "ID" << left << setw(15) << "AMOUNT" << left << setw(15) << "REASON"
     << left << setw(15) << "USER_ID" << left << setw(15) << "STATUS" << left << setw(15) << "DATETIME" << endl;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char* reason = sqlite3_column_text(stmt, 2);
        int user_id = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const unsigned char* datetime = sqlite3_column_text(stmt, 5);

        cout <<left<<setw(10)<<id << left<<setw(15) << amount << left<<setw(15) << reason << left<<setw(15) << user_id << left<<setw(15) << (status == 1 ? "Paid" : "Unpaid") << left<<setw(15) << datetime << endl;
    }

    sqlite3_finalize(stmt);
}

// Displays student data from the Student table
void displayStudentData(sqlite3* db) {
    string selectDataSQL = "SELECT User_ID, FNAME, LNAME, AGE, SEX, ADDRESS, GRADE FROM Users;";
    sqlite3_stmt* stmt;
    int exit = sqlite3_prepare_v2(db, selectDataSQL.c_str(), -1, &stmt, NULL);
    if (exit != SQLITE_OK) {
        cerr << "Error Selecting Data: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout <<left<<setw(15)<< "ID"<<left<<setw(15)<<"FNAME"<<left<<setw(15)<<"LNAME"<<left<<setw(10)<<"AGE"<<left<<setw(10)<<"GENDER"<<left<<setw(10)<<"ADDRESS"<<left<<setw(10)<<"GRADE" << endl;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* fname = sqlite3_column_text(stmt, 1);
        const unsigned char* lname = sqlite3_column_text(stmt, 2);
        const unsigned char* dob = sqlite3_column_text(stmt, 3);
        const unsigned char* gender = sqlite3_column_text(stmt, 4);
        const unsigned char* address = sqlite3_column_text(stmt, 5);
        int grade = sqlite3_column_int(stmt, 6);
        double feesPaid = sqlite3_column_double(stmt, 7);



        cout <<left<<setw(15)<< id << left<<setw(15) << fname << left<<setw(15) << lname << left<<setw(10) << dob << left<<setw(10) << gender << left<<setw(10) << address << left<<setw(10) << grade << endl;
    }

    sqlite3_finalize(stmt);

}

// inserts transactions
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

// Updates students info
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

// Deletes a student and their transactions
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

// Displayz user information based on ID
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

// Add a billing item
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

//
void insertIntoSemesterInfo(sqlite3* db) {
    int answer;
    double amount, punishmentAmount, maxPunishAmount, regFee, schoolFee;
    string initialDate, endDate, punishDate, schoolName, password;
    char* errorMessage = nullptr;
    cout << "LOOKS LIKE ITS YOUR FIRST TIME HERE! LETS SET YOU UP!"<<endl;
    cout << "\033[31m" << "Choose academic calendar system: " << "\033[0m" << endl;
    cout<< "\n1. Semester\n2. Quarter"<<endl;
    cin>> answer;
    if (answer==1)
    {
        ACS=2;
        ACSword=" Semester";
    }
    else if (answer==2)
    {
        ACS=4;
        ACSword=" Quarter";
    }
    else
    {
        cout<< "Wrong choice";
        return; // Exit if choice is invalid
    }

    cout << "Enter the maximum punishment amount: ";
    cin >> maxPunishAmount;
        cout << "Enter the school name: ";
    cin.ignore();
    getline(cin, schoolName);
    cout << "Enter the password: ";
    cin >> password;
       string idate[ACS];
       string edate[ACS];
       string pdate[ACS];
       for(int i = 0; i!=ACS; i++)
        {
            cout << "Enter the initial date of the "<<i+1 << ACSword <<" (YYYY-MM-DD): ";
            cin >> idate[i];
            cout << "Enter the end date of the "<<i+1<< ACSword <<" (YYYY-MM-DD): ";
            cin >> edate[i];
            cout << "Enter the punishment date of the "<<i+1<< ACSword <<" (YYYY-MM-DD): ";
            cin >> pdate[i];
       }

        for(int i = 9; i!=13; i++)
            {

                cout << "Enter the registration fee of Grade "<<i<<": ";
                cin >> regFee;
                cout << "Enter the amount for each"<<ACSword<<" of Grade "<<i<<": ";
                cin >> amount;

                cout << "Enter the punishment amount (per month): ";
                cin >> punishmentAmount;

                    for(int j = 0; j!=ACS; j++)
                    {

                       // string sql = "INSERT INTO Semester_Info (NO, AMOUNT, INITIAL_DATE, END_DATE, PUNISH_DATE, PUNISHMENT_AMOUNT,GRADE, MAX_PUNISH_AMOUNT, REG_FEE, SCHOOL_NAME, PASSWORD) "
                         //           "VALUES (" + to_string(j+1) + ", " + to_string(amount) + ", '" + idate[j] + "', '" + edate[j] + "', '" + pdate[j] + "', " + to_string(punishmentAmount) + ", "
                           //         +to_string(i) + ", " + to_string(maxPunishAmount) + ", " + to_string(regFee) + ", '" + schoolName + "', '" + password + "');";
                        string sql = "INSERT INTO Semester_Info (NO, AMOUNT, INITIAL_DATE, END_DATE, PUNISH_DATE, PUNISHMENT_AMOUNT, GRADE, MAX_PUNISH_AMOUNT, REG_FEE, SCHOOL_NAME, PASSWORD, ACS, ACSWORD)"
                                     "VALUES (" + to_string(j+1) + ", " + to_string(amount) + ", '" + idate[j] + "', '" + edate[j] + "', '" + pdate[j] + "', "
                                     + to_string(punishmentAmount) + ", " + to_string(i) + ", " + to_string(maxPunishAmount) + ", " + to_string(regFee) + ", '"
                                     + schoolName + "', '" + password + "', " + to_string(ACS) + ", '" + ACSword + "');";

                        int exit = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errorMessage);
                        if (exit != SQLITE_OK) {
                            cerr << "Error inserting data into"<<ACSword<< "_Info table: " << errorMessage << endl;
                            sqlite3_free(errorMessage);
                        } else
                        {
                            cout << ACSword <<" information and school fee added successfully." << endl;

                        }
                    }
            }
}

//
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

//
void payment(sqlite3* db) {
    int id, chk, tran_id, rc, new_status;
    string sql, sts;

    cout << "Enter user_ID: ";
    cin >> id;

    sql = "SELECT * FROM Transactions WHERE USER_ID = " + to_string(id) + ";";

    sqlite3_stmt* stmt;
    chk = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    if (chk != SQLITE_OK) {
        cerr << "SQL error in SELECT statement preparation: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    cout << "Above while loop\n";
    bool found = false; // To check if any rows are found
    while ((chk = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = true;
        cout << "In while loop\n";
        int transaction_id = sqlite3_column_int(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const unsigned char* reason = sqlite3_column_text(stmt, 2);
        int user_id = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const unsigned char* datetime = (const unsigned char*)sqlite3_column_text(stmt, 5);

        if (status == 0)
            sts = "unpaid";
        else
            sts = "paid";

        cout << "Transaction ID: " << transaction_id << endl;
        cout << "Amount: " << amount << endl;
        cout << "Reason: " << reason << endl;
        cout << "User ID: " << user_id << endl;
        cout << "Status: " << sts << endl;
        cout << "Datetime: " << datetime << endl;
        cout << "-----------------------------" << endl;
    }

    if (!found) {
        cout << "No transactions found for user ID: " << id << endl;
    }

    sqlite3_finalize(stmt);

    if (!found) {
        // If no transactions were found, no need to proceed further
        return;
    }

    cout << "Enter transaction_id: ";
    cin >> tran_id;

    cout << "Enter (1) to confirm: ";
    cin >> new_status;

    sql = "UPDATE Transactions SET STATUS = ? WHERE ID = ?;";
    sqlite3_stmt* stamt;

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stamt, NULL);

    if (rc != SQLITE_OK) {
        cerr << "SQL error in UPDATE statement preparation: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_int(stamt, 1, new_status);
    sqlite3_bind_int(stamt, 2, tran_id);

    rc = sqlite3_step(stamt);

    if (rc != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Transaction status updated successfully" << endl;
    }

    sqlite3_finalize(stamt);
}


// Main menu function
void mainmenu(sqlite3* db)
 {
    int choice;
    while (true) {
      vector<string> textBlock = {
        "Menu",
        "1. Register Student",
        "2. Display All Students",
        "3. Search Student",
        "4. Display All Transactions",
        "5. Search Transactions",
        "6. Insert Transaction",
        "7. Update Student",
        "8. Delete Student",
        "9. Display User Information",
        "10. Status Page",
        "11. Update school info",
        "12. RESET",
        "13. Exit",
        "Enter your choice: "
    };

    // Print the block with red color and centered
    printColoredCenteredBlock("\033[1m\033[31m", textBlock);

        cin >> choice;
        system("cls");

        cout<<endl;
        switch (choice) {
            case 1:
                registerStudent(db);
                cout<<endl;
                break;
            case 2:
                displayStudentData(db);
                cout<<endl;
                break;
            case 3:
                searchStudent(db);
                break;
            case 4:
                displayTransactions(db);
                break;
            case 5:
                searchTransactions(db);
                cout<<endl;
                break;
            case 6: {
                payment(db);
                cout<<endl;
                break;
            }
            case 7: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                updateStudent(db, user_id);
                cout<<endl;
                break;
            }
            case 8: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                deleteStudent(db, user_id);
                cout<<endl;
                break;
            }
            case 9: {
                int user_id;
                cout << "Enter User ID: ";
                cin >> user_id;
                userInfo(db, user_id);
                cout<<endl;
                break;
            }
            case 10:
                {
                    statusPage(db);
                    cout<<endl;
                    break;
                }

            case 11:
                {
                    check_pw(db);
                    cout<<endl;
                    break;
                }
            case 12:
                {
                    reseter(db);
                    cout<<endl;
                    break;
                }

            case 13:
                {
                    sqlite3_close(db);
                    exit(0);
                }

            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    sqlite3_close(db);
    system("cls");
    cout<<endl;
    mainmenu(db);
}

//
void dropTbl(sqlite3* db){
    std::vector<std::string> tables_to_drop = {"Semester_Info","Users","Transactions"};
sqlite3_stmt *stmt;
std::string sql;
int rc;

for (const std::string &table : tables_to_drop) {
    sql = "DROP TABLE IF EXISTS " + table + ";";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare DROP TABLE statement for " << table << ": " << sqlite3_errmsg(db) << std::endl;
        continue; // Skip to the next table
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to drop table " << table << ": " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Table " << table << " dropped successfully" << std::endl;
    }

    sqlite3_finalize(stmt);
}

sqlite3_close(db);

}

//
void reseter(sqlite3* db){

cout<<"ENTER THE PASSOWRD :";
string pass;
cin>>pass;

string password = returnit(db,"Semester_Info","NO","1","PASSWORD");

if(pass!=password){
    cout<<"Wrong Password "<<endl;
    mainmenu(db);
}else{
    a:
    cout<<"ARE YOU SURE YOU WANT TO RESET EVERYTHING HERE?!!!(Y/N)"<<endl;
    char sure;
    cin>>sure;
    if(sure=='Y' || sure=='y'){
dropTbl(db);
    createTableIfNotExists(db);
    checkAndInsertSemesterInfo(db);

    }else if(sure=='n' || sure=='N'){
mainmenu(db);
    }else{
    goto a;
    }


mainmenu(db);

}

}

//
void updateSInfo(sqlite3* db)
{
    string amount,punishmentAmount, maxPunishAmount, regFee, schoolFee,initialDate, endDate, punishDate, schoolName, password;
    int answer;
    int grade = -1; // -1 to indicate no change
    stringstream updateSQL;
    updateSQL << "UPDATE Semester_Info SET ";
    bool needsComma = false;
        ACS= stoi(returnit(db,"Semester_Info","NO","1","ACS"));
        ACSword=returnit(db,"Semester_Info","NO","1","ACSWORD");
        cout << "Enter the maximum punishment amount(leave blank to keep unchanged): ";
        cin.ignore();
        getline(cin,maxPunishAmount );
        cout << "Enter the school name(leave blank to keep unchanged): ";
        getline(cin, schoolName);
        if (!schoolName.empty())
                    {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " SCHOOL_NAME= '" << schoolName << "'";
                    needsComma = true;
                    }
        cout<< "Do you want to change your password? yes(1) or no(0): ";
        cin>> answer;
        if (answer==1)
            {
                cout << "Enter the new password: ";
                cin>>password;
                if (!password.empty())
                    {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " PASSWORD= '" << password << "'";
                    needsComma = true;
                    }
            }


        cout<< "Do you want to change the dates of each"<<ACSword<<"? yes(1) or no(0)";
        cin>> answer;
        if (answer==1)
        {
                string idate[ACS];
                string edate[ACS];
                string pdate[ACS];
            for(int i = 0; i!=ACS; i++)
            {
                cout << "Enter the initial date of the "<<i+1 << ACSword <<" (YYYY-MM-DD): (leave blank to keep unchanged): ";
                cin >> idate[i];
                cout << "Enter the end date of the "<<i+1<< ACSword <<" (YYYY-MM-DD): (leave blank to keep unchanged): ";
                cin >> edate[i];
                cout << "Enter the punishment date of the "<<i+1<< ACSword <<" (YYYY-MM-DD): (leave blank to keep unchanged): ";
                cin >> pdate[i];
            }
            for(int i = 0; i!=ACS; i++)
            {
                 if ( idate[i].empty()) {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " INITIAL_DATE=" << idate[i];
                    needsComma = true;
                        }
                if ( idate[i].empty()) {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " END_DATE=" << edate[i];
                    needsComma = true;
                        }
                if ( idate[i].empty()) {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " PUNISH_DATE=" << pdate[i];
                    needsComma = true;
                        }
            }

        }
        cout<< "Do you want to change the registration fee and payment of each grade yes(1) or no(0)";
        cin>> answer;
        if (answer==1)
        {
             for(int i = 9; i!=13; i++)
             {

                cout << "Enter the registration fee of Grade "<<i<<": ";
                cin>>regFee ;
                    if (!regFee.empty())
                    {
                    if (needsComma) updateSQL << ", ";
                    updateSQL << " REG_FEE= '" << regFee << "'";
                    needsComma = true;
                    }
                cout << "Enter the amount for each"<<ACSword<<" of Grade "<<i<<": ";
                cin >> amount;
                    if (!amount.empty())
                        {
                        if (needsComma) updateSQL << ", ";
                        updateSQL << "AMOUNT= '" << amount << "'";
                        needsComma = true;
                        }
                cout << "Enter the punishment amount of Grade "<<i<<" (per month): ";
                cin >> punishmentAmount;
                    if (!punishmentAmount.empty())
                        {
                        if (needsComma) updateSQL << ", ";
                        updateSQL << " PUNISHMENT_AMOUNT= '" << punishmentAmount << "'";
                        needsComma = true;
                        }
             }
        }





            char* errorMessage;
            int exit = sqlite3_exec(db, updateSQL.str().c_str(), NULL, 0, &errorMessage);
            if (exit != SQLITE_OK)
            {
                cerr << "Error Updating Data: " << errorMessage << endl;
                sqlite3_free(errorMessage);
            } else
            {
                cout << "Data updated successfully" << endl;
            }
}
void check_pw(sqlite3* db)
{
    cout<<"ENTER THE PASSOWRD :";
    string pass;
    cin>>pass;

string password = returnit(db,"Semester_Info","NO","1","PASSWORD");
if(pass==password){
    updateSInfo(db);

}else{
    cout<<"Wrong Password "<<endl;
    mainmenu(db);
}
}

int main()
{
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
    welcome_message();
    mainmenu(DB);

    sqlite3_close(DB);

    return 0;
}
