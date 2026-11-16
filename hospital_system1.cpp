
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <limits>

using namespace std;

/* =========================================================
   HOSPITAL TOKEN BOOKING AND QUEUE MANAGEMENT SYSTEM
   Complete Single-File C++ Project
   C++11 Compatible
   ========================================================= */

/* ==================== CONSTANTS ==================== */

const int HOSPITAL_COUNT = 8;
const int DEPARTMENT_COUNT = 4;
const int DOCTORS_PER_DEPT = 2;

const int SERVICE_MINUTES = 15;

const string DATA_FILE = "patients.dat";
const string PIN_FILE = "receptionist_pin.dat";

/* ==================== STATUS ==================== */

enum PatientStatus {
    WAITING = 1,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

/* ==================== HOSPITAL DATA ==================== */

const string hospitals[HOSPITAL_COUNT] = {
    "City General Hospital",
    "Community Clinic",
    "Bone and Joint Hospital",
    "Sunrise Multispeciality Hospital",
    "Green Valley Medical Centre",
    "Metro ENT Hospital",
    "Aarogya Skin and Wellness Centre",
    "Northside Family Hospital"
};

const string departments[DEPARTMENT_COUNT] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

const string doctors[DEPARTMENT_COUNT][DOCTORS_PER_DEPT] = {
    {"Dr. Amit Sharma", "Dr. Neha Verma"},
    {"Dr. Rahul Mehta", "Dr. Priya Singh"},
    {"Dr. Arjun Kapoor", "Dr. Simran Kaur"},
    {"Dr. Rohan Gupta", "Dr. Ananya Joshi"}
};

/* =========================================================
   PATIENT NODE
   BACKEND 1 - PATIENT DATA STRUCTURE
   ========================================================= */

struct Patient {
    int token;
    long long order;

    string name;
    string phone;
    string hospital;
    string department;
    string doctor;
    string symptoms;
    string bookingDate;

    PatientStatus status;
    bool emergency;

    Patient *next;
};

/* =========================================================
   MAIN SYSTEM CLASS
   ========================================================= */

class HospitalSystem {

private:

    Patient *head;

    int nextToken;
    long long nextOrder;

    unsigned long long receptionistPIN;

    /* ==================== INPUT HANDLING ==================== */

    string readText(string message) {
        string value;

        cout << message;
        getline(cin, value);

        return value;
    }

    int readNumber(string message) {
        string input;
        int number;
        char extra;

        while (true) {
            cout << message;
            getline(cin, input);

            stringstream ss(input);

            if (ss >> number && !(ss >> extra))
                return number;

            cout << "Invalid input! Enter a valid number.\n";
        }
    }

    void pauseScreen() {
        cout << "\nPress Enter to continue...";
        string temp;
        getline(cin, temp);
    }

    string cleanText(string value) {
        for (size_t i = 0; i < value.length(); i++) {
            if (value[i] == '|')
                value[i] = '/';
        }

        return value;
    }

    bool isValidPhone(string phone) {
        int digits = 0;

        for (size_t i = 0; i < phone.length(); i++) {
            if (isdigit(static_cast<unsigned char>(phone[i]))) {
                digits++;
            }
            else if (phone[i] != '+' || i != 0) {
                return false;
            }
        }

        return digits >= 10 && digits <= 15;
    }

    /* ==================== DATE AND TIME ==================== */

    string getTodayDate() {
        time_t now = time(0);
        tm *current = localtime(&now);

        char buffer[20];

        strftime(buffer, sizeof(buffer), "%Y-%m-%d", current);

        return string(buffer);
    }

    string getCurrentTime() {
        time_t now = time(0);
        tm *current = localtime(&now);

        char buffer[20];

        strftime(buffer, sizeof(buffer), "%H:%M:%S", current);

        return string(buffer);
    }

    void showDate() {
        cout << "Date: " << getTodayDate() << "\n";
    }

    /* ==================== PIN MANAGEMENT ==================== */

    unsigned long long hashPIN(string pin) {
        unsigned long long hash = 1469598103934665603ULL;

        for (size_t i = 0; i < pin.length(); i++) {
            hash ^= static_cast<unsigned char>(pin[i]);
            hash *= 1099511628211ULL;
        }

        return hash;
    }

    void loadPIN() {
        ifstream file(PIN_FILE.c_str());

        if (file.is_open()) {
            file >> receptionistPIN;
            file.close();
        }
        else {
            receptionistPIN = hashPIN("1234");
            savePIN();
        }
    }

    void savePIN() {
        ofstream file(PIN_FILE.c_str());

        if (file.is_open()) {
            file << receptionistPIN;
            file.close();
        }
    }

    /* ==================== LINKED LIST OPERATIONS ==================== */

    void addPatient(Patient *newPatient) {
        newPatient->next = NULL;

        if (head == NULL) {
            head = newPatient;
            return;
        }

        Patient *temp = head;

        while (temp->next != NULL)
            temp = temp->next;

        temp->next = newPatient;
    }

    Patient *findToken(int token) {
        string today = getTodayDate();

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->token == token &&
                temp->bookingDate == today) {
                return temp;
            }

            temp = temp->next;
        }

        return NULL;
    }

    Patient *findPatientByPhone(string phone) {
        Patient *temp = head;

        while (temp != NULL) {
            if (temp->phone == phone)
                return temp;

            temp = temp->next;
        }

        return NULL;
    }

    /* ==================== FILE HANDLING ==================== */

    void saveData() {
        ofstream file(DATA_FILE.c_str());

        if (!file.is_open()) {
            cout << "Error: Unable to save patient records.\n";
            return;
        }

        Patient *temp = head;

        while (temp != NULL) {
            file << temp->token << "|"
                 << temp->order << "|"
                 << temp->name << "|"
                 << temp->phone << "|"
                 << temp->hospital << "|"
                 << temp->department << "|"
                 << temp->doctor << "|"
                 << temp->symptoms << "|"
                 << temp->bookingDate << "|"
                 << static_cast<int>(temp->status) << "|"
                 << temp->emergency
                 << "\n";

            temp = temp->next;
        }

        file.close();
    }

    void loadData() {
        ifstream file(DATA_FILE.c_str());

        if (!file.is_open())
            return;

        string line;

        while (getline(file, line)) {
            string fields[11];
            stringstream ss(line);

            int i = 0;

            while (i < 11 && getline(ss, fields[i], '|'))
                i++;

            if (i != 11)
                continue;

            try {
                Patient *p = new Patient();

                p->token = stoi(fields[0]);
                p->order = stoll(fields[1]);

                p->name = fields[2];
                p->phone = fields[3];
                p->hospital = fields[4];
                p->department = fields[5];
                p->doctor = fields[6];
                p->symptoms = fields[7];
                p->bookingDate = fields[8];

                int statusValue = stoi(fields[9]);

                if (statusValue < WAITING || statusValue > CANCELLED) {
                    delete p;
                    continue;
                }

                p->status = static_cast<PatientStatus>(statusValue);
                p->emergency = (fields[10] == "1");
                p->next = NULL;

                addPatient(p);

                if (p->order >= nextOrder)
                    nextOrder = p->order + 1;
            }
            catch (...) {
                continue;
            }
        }

        file.close();
    }

    /* ==================== DAILY TOKEN SYSTEM ==================== */

    void updateDailyToken() {
        int maximum = 0;
        string today = getTodayDate();

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today &&
                temp->token > maximum) {
                maximum = temp->token;
            }

            temp = temp->next;
        }

        nextToken = maximum + 1;
    }

    /* ==================== DISPLAY FUNCTIONS ==================== */

    void showHospitals() {
        cout << "\n========== AVAILABLE HOSPITALS ==========\n";

        for (int i = 0; i < HOSPITAL_COUNT; i++) {
            cout << i + 1 << ". " << hospitals[i] << "\n";
        }
    }

    void showDepartments() {
        cout << "\n========== DEPARTMENTS ==========\n";

        for (int i = 0; i < DEPARTMENT_COUNT; i++) {
            cout << i + 1 << ". " << departments[i] << "\n";
        }
    }

    void showDoctors(int departmentIndex) {
        cout << "\n========== AVAILABLE DOCTORS ==========\n";

        for (int i = 0; i < DOCTORS_PER_DEPT; i++) {
            cout << i + 1 << ". "
                 << doctors[departmentIndex][i] << "\n";
        }
    }

    int selectHospital() {
        showHospitals();

        int choice = readNumber("Select hospital: ");

        if (choice < 1 || choice > HOSPITAL_COUNT) {
            cout << "Invalid hospital selection.\n";
            return -1;
        }

        return choice - 1;
    }

    int selectDepartment() {
        showDepartments();

        int choice = readNumber("Select department: ");

        if (choice < 1 || choice > DEPARTMENT_COUNT) {
            cout << "Invalid department selection.\n";
            return -1;
        }

        return choice - 1;
    }

    int selectDoctor(int departmentIndex) {
        showDoctors(departmentIndex);

        int choice = readNumber("Select doctor: ");

        if (choice < 1 || choice > DOCTORS_PER_DEPT) {
            cout << "Invalid doctor selection.\n";
            return -1;
        }

        return choice - 1;
    }

    string getStatusName(PatientStatus status) {
        switch (status) {
            case WAITING:
                return "WAITING";

            case IN_CONSULTATION:
                return "IN CONSULTATION";

            case COMPLETED:
                return "COMPLETED";

            case CANCELLED:
                return "CANCELLED";

            default:
                return "UNKNOWN";
        }
    }

    void showPatientDetails(Patient *p) {
        if (p == NULL)
            return;

        cout << "\n========== PATIENT DETAILS ==========\n";

        cout << "Token Number : " << p->token << "\n";
        cout << "Name         : " << p->name << "\n";
        cout << "Phone        : " << p->phone << "\n";
        cout << "Hospital     : " << p->hospital << "\n";
        cout << "Department   : " << p->department << "\n";
        cout << "Doctor       : " << p->doctor << "\n";
        cout << "Symptoms     : " << p->symptoms << "\n";
        cout << "Booking Date : " << p->bookingDate << "\n";
        cout << "Status       : " << getStatusName(p->status) << "\n";
        cout << "Priority     : "
             << (p->emergency ? "EMERGENCY" : "NORMAL") << "\n";
    }

    /* ==================== QUEUE HELPERS ==================== */

    bool sameDoctor(Patient *p, string hospital,
                    string department, string doctor) {
        return p->hospital == hospital &&
               p->department == department &&
               p->doctor == doctor;
    }

    int countWaitingForDoctor(string hospital,
                              string department,
                              string doctor) {
        int count = 0;
        string today = getTodayDate();

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today &&
                temp->status == WAITING &&
                sameDoctor(temp, hospital, department, doctor)) {
                count++;
            }

            temp = temp->next;
        }

        return count;
    }

    Patient *getNextPatient(string hospital,
                            string department,
                            string doctor) {
        Patient *emergencyPatient = NULL;
        Patient *normalPatient = NULL;

        string today = getTodayDate();

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today &&
                temp->status == WAITING &&
                sameDoctor(temp, hospital, department, doctor)) {

                if (temp->emergency) {
                    if (emergencyPatient == NULL ||
                        temp->order < emergencyPatient->order) {
                        emergencyPatient = temp;
                    }
                }
                else {
                    if (normalPatient == NULL ||
                        temp->order < normalPatient->order) {
                        normalPatient = temp;
                    }
                }
            }

            temp = temp->next;
        }

        if (emergencyPatient != NULL)
            return emergencyPatient;

        return normalPatient;
    }

    int countAhead(Patient *target) {
        int ahead = 0;

        if (target == NULL || target->status != WAITING)
            return 0;

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == target->bookingDate &&
                temp->status == WAITING &&
                sameDoctor(temp, target->hospital,
                           target->department, target->doctor) &&
                temp != target) {

                if (target->emergency) {
                    if (temp->emergency &&
                        temp->order < target->order) {
                        ahead++;
                    }
                }
                else {
                    if (temp->emergency ||
                        temp->order < target->order) {
                        ahead++;
                    }
                }
            }

            temp = temp->next;
        }

        return ahead;
    }

    /* ==================== BOOKING RECEIPT ==================== */

    void showBookingReceipt(Patient *p) {
        cout << "\n";
        cout << "========================================\n";
        cout << "          BOOKING CONFIRMATION\n";
        cout << "========================================\n";

        cout << "Patient Name : " << p->name << "\n";
        cout << "Token Number : " << p->token << "\n";
        cout << "Hospital     : " << p->hospital << "\n";
        cout << "Department   : " << p->department << "\n";
        cout << "Doctor       : " << p->doctor << "\n";
        cout << "Booking Date : " << p->bookingDate << "\n";
        cout << "Booking Time : " << getCurrentTime() << "\n";
        cout << "Status       : WAITING\n";

        cout << "========================================\n";
        cout << "Please keep your token number safe.\n";
    }

    /* =====================================================
       BACKEND 1 - PATIENT REGISTRATION AND TOKEN BOOKING
       ===================================================== */

    void registerPatient(bool walkIn = false) {
        Patient *p = new Patient();

        cout << "\n========== PATIENT REGISTRATION ==========\n";

        p->name = cleanText(readText("Enter patient name: "));

        if (p->name.empty()) {
            cout << "Name cannot be empty.\n";
            delete p;
            return;
        }

        p->phone = readText("Enter phone number: ");

        if (!isValidPhone(p->phone)) {
            cout << "Invalid phone number. Registration cancelled.\n";
            delete p;
            return;
        }

        p->phone = cleanText(p->phone);

        /* Prevent duplicate active booking on the same day. */

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->phone == p->phone &&
                temp->bookingDate == getTodayDate() &&
                (temp->status == WAITING ||
                 temp->status == IN_CONSULTATION)) {

                cout << "An active booking already exists for this phone number.\n";
                cout << "Existing token: " << temp->token << "\n";

                delete p;
                return;
            }

            temp = temp->next;
        }

        int hospitalIndex = selectHospital();

        if (hospitalIndex == -1) {
            delete p;
            return;
        }

        int departmentIndex = selectDepartment();

        if (departmentIndex == -1) {
            delete p;
            return;
        }

        int doctorIndex = selectDoctor(departmentIndex);

        if (doctorIndex == -1) {
            delete p;
            return;
        }

        p->hospital = hospitals[hospitalIndex];
        p->department = departments[departmentIndex];
        p->doctor = doctors[departmentIndex][doctorIndex];

        p->symptoms = cleanText(
            readText("Enter symptoms or reason for visit: ")
        );

        if (p->symptoms.empty())
            p->symptoms = "Not specified";

        p->bookingDate = getTodayDate();

        p->token = nextToken++;
        p->order = nextOrder++;

        p->status = WAITING;
        p->emergency = false;
        p->next = NULL;

        addPatient(p);

        saveData();

        cout << "\nRegistration completed successfully.\n";

        if (walkIn)
            cout << "Walk-in patient registered successfully.\n";

        showBookingReceipt(p);
    }

    /* ==================== TOKEN DETAILS ==================== */

    void showTokenDetails() {
        int token = readNumber("\nEnter token number: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Token not found for today.\n";
            return;
        }

        showPatientDetails(p);
    }

    /* ==================== TOKEN STATUS ==================== */

    void showPatientStatus() {
        int token = readNumber("\nEnter token number: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Token not found for today.\n";
            return;
        }

        showPatientDetails(p);

        if (p->status == WAITING) {
            int ahead = countAhead(p);

            cout << "\nPatients ahead: " << ahead << "\n";

            cout << "Estimated waiting time: "
                 << ahead * SERVICE_MINUTES
                 << " minutes\n";
        }
    }

    /* ==================== PATIENT HISTORY ==================== */

    void showPatientHistory() {
        string phone = readText("\nEnter registered phone number: ");

        if (!isValidPhone(phone)) {
            cout << "Invalid phone number.\n";
            return;
        }

        bool found = false;

        Patient *temp = head;

        cout << "\n========== PATIENT BOOKING HISTORY ==========\n";

        while (temp != NULL) {
            if (temp->phone == phone) {
                showPatientDetails(temp);
                found = true;
            }

            temp = temp->next;
        }

        if (!found)
            cout << "No previous records found.\n";
    }

    /* ==================== PATIENT CANCELLATION ==================== */

    void cancelPatientToken() {
        int token = readNumber("\nEnter token number: ");

        string phone = readText("Enter registered phone number: ");

        Patient *p = findToken(token);

        if (p == NULL || p->phone != phone) {
            cout << "Token or patient details do not match.\n";
            return;
        }

        if (p->status != WAITING) {
            cout << "Only waiting tokens can be cancelled.\n";
            return;
        }

        p->status = CANCELLED;

        saveData();

        cout << "Token " << token
             << " cancelled successfully.\n";
    }

    /* ==================== SYMPTOM SUGGESTION ==================== */

    void symptomSuggestion() {
        string symptoms = readText("\nDescribe your symptoms: ");

        for (size_t i = 0; i < symptoms.length(); i++) {
            symptoms[i] = static_cast<char>(
                tolower(static_cast<unsigned char>(symptoms[i]))
            );
        }

        cout << "\nSuggested department: ";

        if (symptoms.find("ear") != string::npos ||
            symptoms.find("nose") != string::npos ||
            symptoms.find("throat") != string::npos) {

            cout << "ENT\n";
        }
        else if (symptoms.find("bone") != string::npos ||
                 symptoms.find("joint") != string::npos ||
                 symptoms.find("fracture") != string::npos) {

            cout << "Orthopaedics\n";
        }
        else if (symptoms.find("skin") != string::npos ||
                 symptoms.find("rash") != string::npos ||
                 symptoms.find("itch") != string::npos) {

            cout << "Dermatology\n";
        }
        else {
            cout << "General Medicine\n";
        }

        cout << "This is a basic navigation suggestion, not a diagnosis.\n";
        cout << "For urgent or severe symptoms, seek immediate medical care.\n";
    }

    /* ==================== PATIENT INFORMATION ==================== */

    void patientHospitalInfo() {
        showHospitals();
    }

    void patientDoctorInfo() {
        for (int i = 0; i < DEPARTMENT_COUNT; i++) {
            cout << "\nDepartment: " << departments[i] << "\n";

            for (int j = 0; j < DOCTORS_PER_DEPT; j++) {
                cout << doctors[i][j] << "\n";
            }
        }
    }

    /* ==================== PATIENT HELP ==================== */

    void patientHelp() {
        cout << "\n========== PATIENT HELP AND FAQ ==========\n";

        cout << "\n1. How to book a token?\n";
        cout << "   Select Register / Book Token.\n";

        cout << "\n2. How to check token status?\n";
        cout << "   Select Check Token Status and enter your token.\n";

        cout << "\n3. How to cancel a token?\n";
        cout << "   Select Cancel Token and verify your phone number.\n";

        cout << "\n4. How to check previous bookings?\n";
        cout << "   Select Patient History.\n";

        cout << "\n5. What if I lose my token number?\n";
        cout << "   Contact the hospital receptionist.\n";
    }

    void patientGuidelines() {
        cout << "\n========== PATIENT GUIDELINES ==========\n";

        cout << "1. Enter correct personal information.\n";
        cout << "2. Keep your token number safe.\n";
        cout << "3. Reach the hospital on time.\n";
        cout << "4. Follow the queue instructions.\n";
        cout << "5. Contact reception for assistance.\n";
        cout << "6. Do not share personal information unnecessarily.\n";
        cout << "7. Follow instructions given by hospital staff.\n";
    }

    /* =====================================================
       FRONTEND 2 - RECEPTIONIST PORTAL
       ===================================================== */

    bool receptionistLogin() {
        cout << "\n========== RECEPTIONIST LOGIN ==========\n";

        for (int attempt = 1; attempt <= 3; attempt++) {
            string pin = readText("Enter receptionist PIN: ");

            if (hashPIN(pin) == receptionistPIN) {
                cout << "Login successful.\n";
                return true;
            }

            cout << "Incorrect PIN. Attempts remaining: "
                 << 3 - attempt << "\n";
        }

        cout << "Access denied.\n";
        return false;
    }

    void changePIN() {
        string oldPIN = readText("\nEnter current PIN: ");

        if (hashPIN(oldPIN) != receptionistPIN) {
            cout << "Incorrect current PIN.\n";
            return;
        }

        string newPIN = readText("Enter new PIN (minimum 4 characters): ");

        if (newPIN.length() < 4) {
            cout << "PIN must contain at least 4 characters.\n";
            return;
        }

        string confirmPIN = readText("Confirm new PIN: ");

        if (newPIN != confirmPIN) {
            cout << "PIN confirmation does not match.\n";
            return;
        }

        receptionistPIN = hashPIN(newPIN);

        savePIN();

        cout << "PIN changed successfully.\n";
    }

    void multipleWalkInRegistration() {
        int count = readNumber("\nHow many walk-in patients? ");

        if (count < 1 || count > 50) {
            cout << "Enter a number between 1 and 50.\n";
            return;
        }

        for (int i = 1; i <= count; i++) {
            cout << "\n========== WALK-IN PATIENT "
                 << i << " ==========\n";

            registerPatient(true);
        }
    }

    void searchPatient() {
        cout << "\n========== SEARCH PATIENT ==========\n";

        cout << "1. Search by today's token\n";
        cout << "2. Search by phone number\n";

        int choice = readNumber("Enter choice: ");

        if (choice == 1) {
            int token = readNumber("Enter token number: ");

            Patient *p = findToken(token);

            if (p == NULL)
                cout << "Patient not found.\n";
            else
                showPatientDetails(p);
        }
        else if (choice == 2) {
            string phone = readText("Enter phone number: ");

            bool found = false;
            Patient *temp = head;

            while (temp != NULL) {
                if (temp->phone == phone) {
                    showPatientDetails(temp);
                    found = true;
                }

                temp = temp->next;
            }

            if (!found)
                cout << "No matching patient found.\n";
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    /* =====================================================
       BACKEND 2 - QUEUE MANAGEMENT
       ===================================================== */

    void showQueue() {
        int h = selectHospital();
        if (h == -1) return;

        int d = selectDepartment();
        if (d == -1) return;

        int doc = selectDoctor(d);
        if (doc == -1) return;

        string today = getTodayDate();

        string selectedHospital = hospitals[h];
        string selectedDepartment = departments[d];
        string selectedDoctor = doctors[d][doc];

        cout << "\n========== DOCTOR-WISE WAITING QUEUE ==========\n";
        cout << "Hospital: " << selectedHospital << "\n";
        cout << "Department: " << selectedDepartment << "\n";
        cout << "Doctor: " << selectedDoctor << "\n";

        int count = 0;

        cout << "\n--- EMERGENCY PATIENTS ---\n";

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today &&
                temp->status == WAITING &&
                temp->emergency &&
                sameDoctor(temp, selectedHospital,
                           selectedDepartment, selectedDoctor)) {

                cout << "Token " << temp->token
                     << " | " << temp->name << "\n";

                count++;
            }

            temp = temp->next;
        }

        cout << "\n--- NORMAL PATIENTS ---\n";

        temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today &&
                temp->status == WAITING &&
                !temp->emergency &&
                sameDoctor(temp, selectedHospital,
                           selectedDepartment, selectedDoctor)) {

                cout << "Token " << temp->token
                     << " | " << temp->name << "\n";

                count++;
            }

            temp = temp->next;
        }

        if (count == 0)
            cout << "No waiting patients.\n";

        cout << "\nTotal waiting patients: " << count << "\n";
    }

    /* ==================== EMERGENCY PRIORITY ==================== */

    void markEmergencyPriority() {
        int token = readNumber("\nEnter token number: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Patient not found.\n";
            return;
        }

        if (p->status != WAITING) {
            cout << "Only waiting patients can be marked as emergency.\n";
            return;
        }

        if (p->emergency) {
            cout << "Patient already has emergency priority.\n";
            return;
        }

        p->emergency = true;

        saveData();

        cout << "Emergency priority assigned by receptionist.\n";
        cout << "The patient will be considered before normal waiting patients.\n";
    }

    /* ==================== CALL NEXT PATIENT ==================== */

    void callNextPatient() {
        int h = selectHospital();
        if (h == -1) return;

        int d = selectDepartment();
        if (d == -1) return;

        int doc = selectDoctor(d);
        if (doc == -1) return;

        Patient *p = getNextPatient(
            hospitals[h],
            departments[d],
            doctors[d][doc]
        );

        if (p == NULL) {
            cout << "No waiting patients for this doctor.\n";
            return;
        }

        p->status = IN_CONSULTATION;

        saveData();

        cout << "\n========== PATIENT CALLED ==========\n";
        showPatientDetails(p);
    }

    /* ==================== COMPLETE CONSULTATION ==================== */

    void completeConsultation() {
        int token = readNumber("\nEnter token number: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Patient not found.\n";
            return;
        }

        if (p->status != IN_CONSULTATION) {
            cout << "Patient is not currently in consultation.\n";
            return;
        }

        p->status = COMPLETED;

        saveData();

        cout << "Consultation completed successfully.\n";
    }

    /* ==================== WAITING POSITION ==================== */

    void showWaitingPosition() {
        int token = readNumber("\nEnter token number: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Token not found.\n";
            return;
        }

        if (p->status != WAITING) {
            cout << "Patient is not in the waiting queue.\n";
            cout << "Current status: " << getStatusName(p->status) << "\n";
            return;
        }

        int ahead = countAhead(p);

        cout << "\n========== WAITING POSITION ==========\n";
        cout << "Token Number: " << p->token << "\n";
        cout << "Patients Ahead: " << ahead << "\n";
        cout << "Estimated Waiting Time: "
             << ahead * SERVICE_MINUTES << " minutes\n";
    }

    /* ==================== STATISTICS ==================== */

    void showStatistics() {
        int waiting = 0;
        int consultation = 0;
        int completed = 0;
        int cancelled = 0;
        int emergency = 0;

        string today = getTodayDate();

        Patient *temp = head;

        while (temp != NULL) {
            if (temp->bookingDate == today) {
                if (temp->status == WAITING)
                    waiting++;

                else if (temp->status == IN_CONSULTATION)
                    consultation++;

                else if (temp->status == COMPLETED)
                    completed++;

                else if (temp->status == CANCELLED)
                    cancelled++;

                if (temp->emergency &&
                    temp->status == WAITING)
                    emergency++;
            }

            temp = temp->next;
        }

        cout << "\n========== DAILY STATISTICS ==========\n";
        cout << "Date: " << today << "\n";
        cout << "Waiting Patients       : " << waiting << "\n";
        cout << "Emergency Waiting      : " << emergency << "\n";
        cout << "In Consultation        : " << consultation << "\n";
        cout << "Completed Consultations: " << completed << "\n";
        cout << "Cancelled Tokens       : " << cancelled << "\n";
        cout << "Total Bookings Today   : "
             << waiting + consultation + completed + cancelled << "\n";
    }

    /* ==================== HOSPITAL STATISTICS ==================== */

    void showHospitalStatistics() {
        string today = getTodayDate();

        cout << "\n========== HOSPITAL-WISE STATISTICS ==========\n";

        for (int i = 0; i < HOSPITAL_COUNT; i++) {
            int total = 0;
            int waiting = 0;
            int completed = 0;

            Patient *temp = head;

            while (temp != NULL) {
                if (temp->bookingDate == today &&
                    temp->hospital == hospitals[i]) {

                    total++;

                    if (temp->status == WAITING)
                        waiting++;

                    if (temp->status == COMPLETED)
                        completed++;
                }

                temp = temp->next;
            }

            cout << "\n" << hospitals[i] << "\n";
            cout << "Total: " << total
                 << " | Waiting: " << waiting
                 << " | Completed: " << completed << "\n";
        }
    }

    /* ==================== DOCTOR-WISE STATISTICS ==================== */

    void showDoctorStatistics() {
        string today = getTodayDate();

        cout << "\n========== DOCTOR-WISE STATISTICS ==========\n";

        for (int h = 0; h < HOSPITAL_COUNT; h++) {
            cout << "\nHospital: " << hospitals[h] << "\n";

            for (int d = 0; d < DEPARTMENT_COUNT; d++) {
                for (int doc = 0; doc < DOCTORS_PER_DEPT; doc++) {
                    int waiting = 0;
                    int completed = 0;

                    Patient *temp = head;

                    while (temp != NULL) {
                        if (temp->bookingDate == today &&
                            sameDoctor(temp, hospitals[h],
                                       departments[d], doctors[d][doc])) {

                            if (temp->status == WAITING)
                                waiting++;

                            if (temp->status == COMPLETED)
                                completed++;
                        }

                        temp = temp->next;
                    }

                    cout << doctors[d][doc]
                         << " | Waiting: " << waiting
                         << " | Completed: " << completed << "\n";
                }
            }
        }
    }

    /* ==================== RECEPTIONIST CANCELLATION ==================== */

    void receptionistCancelToken() {
        int token = readNumber("\nEnter token number to cancel: ");

        Patient *p = findToken(token);

        if (p == NULL) {
            cout << "Token not found.\n";
            return;
        }

        if (p->status != WAITING) {
            cout << "Only waiting tokens can be cancelled.\n";
            return;
        }

        p->status = CANCELLED;

        saveData();

        cout << "Token cancelled by receptionist.\n";
    }

    /* ==================== RECEPTIONIST DASHBOARD ==================== */

    void receptionistMenu() {
        if (!receptionistLogin())
            return;

        int choice;

        do {
            cout << "\n";
            cout << "========================================\n";
            cout << "       RECEPTIONIST DASHBOARD\n";
            cout << "========================================\n";

            cout << "\n1. Register Walk-in Patient";
            cout << "\n2. Multiple Walk-in Registration";
            cout << "\n3. Search Patient";
            cout << "\n4. View Doctor-wise Queue";
            cout << "\n5. Call Next Patient";
            cout << "\n6. Complete Consultation";
            cout << "\n7. Mark Emergency Priority";
            cout << "\n8. Show Waiting Position";
            cout << "\n9. Cancel Patient Token";
            cout << "\n10. Daily Statistics";
            cout << "\n11. Hospital-wise Statistics";
            cout << "\n12. Doctor-wise Statistics";
            cout << "\n13. Change Receptionist PIN";
            cout << "\n0. Logout";

            choice = readNumber("\n\nEnter choice: ");

            switch (choice) {
                case 1:
                    registerPatient(true);
                    break;

                case 2:
                    multipleWalkInRegistration();
                    break;

                case 3:
                    searchPatient();
                    break;

                case 4:
                    showQueue();
                    break;

                case 5:
                    callNextPatient();
                    break;

                case 6:
                    completeConsultation();
                    break;

                case 7:
                    markEmergencyPriority();
                    break;

                case 8:
                    showWaitingPosition();
                    break;

                case 9:
                    receptionistCancelToken();
                    break;

                case 10:
                    showStatistics();
                    break;

                case 11:
                    showHospitalStatistics();
                    break;

                case 12:
                    showDoctorStatistics();
                    break;

                case 13:
                    changePIN();
                    break;

                case 0:
                    cout << "Receptionist logged out.\n";
                    break;

                default:
                    cout << "Invalid choice.\n";
            }

            if (choice != 0)
                pauseScreen();

        } while (choice != 0);
    }

    /* =====================================================
       FRONTEND 1 - PATIENT PORTAL
       ===================================================== */

    void patientWelcome() {
        cout << "\n";
        cout << "========================================\n";
        cout << "       HOSPITAL TOKEN BOOKING SYSTEM\n";
        cout << "              PATIENT PORTAL\n";
        cout << "========================================\n";

        showDate();
        cout << "Welcome! Manage your hospital bookings here.\n";
    }

    void patientMenu() {
        int choice;

        do {
            patientWelcome();

            cout << "\n1. Register / Book New Token";
            cout << "\n2. Check Token Status";
            cout << "\n3. View Token Details";
            cout << "\n4. View Patient History";
            cout << "\n5. Cancel Waiting Token";
            cout << "\n6. Symptom-Based Department Suggestion";
            cout << "\n7. Hospital Information";
            cout << "\n8. Doctor Information";
            cout << "\n9. Help and FAQ";
            cout << "\n10. Patient Guidelines";
            cout << "\n0. Return to Main Menu";

            choice = readNumber("\n\nEnter your choice: ");

            switch (choice) {
                case 1:
                    registerPatient();
                    break;

                case 2:
                    showPatientStatus();
                    break;

                case 3:
                    showTokenDetails();
                    break;

                case 4:
                    showPatientHistory();
                    break;

                case 5:
                    cancelPatientToken();
                    break;

                case 6:
                    symptomSuggestion();
                    break;

                case 7:
                    patientHospitalInfo();
                    break;

                case 8:
                    patientDoctorInfo();
                    break;

                case 9:
                    patientHelp();
                    break;

                case 10:
                    patientGuidelines();
                    break;

                case 0:
                    cout << "Returning to main menu...\n";
                    break;

                default:
                    cout << "Invalid choice. Try again.\n";
            }

            if (choice != 0)
                pauseScreen();

        } while (choice != 0);
    }

    /* ==================== MEMORY CLEANUP ==================== */

    void destroySystem() {
        Patient *temp = head;

        while (temp != NULL) {
            Patient *nextNode = temp->next;

            delete temp;

            temp = nextNode;
        }

        head = NULL;
    }

/* =========================================================
   PUBLIC SECTION
   ========================================================= */

public:

    HospitalSystem() {
        head = NULL;
        nextToken = 1;
        nextOrder = 1;
        receptionistPIN = 0;

        loadPIN();
        loadData();
        updateDailyToken();
    }

    ~HospitalSystem() {
        saveData();
        destroySystem();
    }

    /* ==================== MAIN MENU ==================== */

    void run() {
        int choice;

        do {
            cout << "\n";
            cout << "============================================\n";
            cout << "    HOSPITAL TOKEN BOOKING AND QUEUE SYSTEM\n";
            cout << "============================================\n";

            cout << "\n1. Patient Portal";
            cout << "\n2. Receptionist Portal";
            cout << "\n0. Exit Program";

            choice = readNumber("\n\nEnter your choice: ");

            switch (choice) {
                case 1:
                    patientMenu();
                    break;

                case 2:
                    receptionistMenu();
                    break;

                case 0:
                    cout << "\nThank you for using the system.\n";
                    break;

                default:
                    cout << "Invalid choice.\n";
            }

        } while (choice != 0);

        saveData();
    }
};

/* =========================================================
   MAIN FUNCTION
   ========================================================= */

int main() {
    HospitalSystem system;

    system.run();

    return 0;
}
