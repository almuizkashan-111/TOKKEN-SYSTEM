
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>
#include <cstdio>
using namespace std;

// ============================================================
// COMMON DATA STRUCTURES AND CONSTANTS
// ============================================================

const int HOSPITAL_COUNT = 8;
const int DEPARTMENT_COUNT = 4;
const int DOCTORS_PER_DEPT = 2;
const int SERVICE_MINUTES = 15;

const string DATA_FILE = "patients.dat";
const string PIN_FILE = "receptionist_pin.dat";

enum Status {
    WAITING,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

struct Patient {
    int token;
    int order;
    string name;
    string phone;
    string hospital;
    string department;
    string doctor;
    string symptoms;
    string date;
    Status status;
    bool emergency;
    Patient* next;
};

Patient* head = NULL;
int nextToken = 1;
int nextOrder = 1;
unsigned long long receptionistPIN = 0;

string hospitals[HOSPITAL_COUNT] = {
    "City General Hospital",
    "Community Clinic",
    "Bone and Joint Hospital",
    "Sunrise Multispeciality Hospital",
    "Green Valley Medical Centre",
    "Metro ENT Hospital",
    "Aarogya Skin and Wellness Centre",
    "Northside Family Hospital"
};

string departments[DEPARTMENT_COUNT] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

string doctors[DEPARTMENT_COUNT][DOCTORS_PER_DEPT] = {
    {"Dr. A. Sharma", "Dr. R. Mehta"},
    {"Dr. S. Verma", "Dr. P. Kapoor"},
    {"Dr. K. Singh", "Dr. N. Joshi"},
    {"Dr. M. Gupta", "Dr. T. Rao"}
};

// ============================================================
// COMMON HELPER FUNCTIONS
// ============================================================

string todayDate() {
    time_t now = time(NULL);
    tm* t = localtime(&now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", t);
    return buffer;
}

string currentTime() {
    time_t now = time(NULL);
    tm* t = localtime(&now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
    return buffer;
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readChoice() {
    int value;

    while (!(cin >> value)) {
        cout << "Enter a valid number: ";
        clearInput();
    }

    clearInput();
    return value;
}

string readText(string message) {
    string value;

    cout << message;
    getline(cin, value);

    return value;
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

bool validPhone(string phone) {
    if (phone.length() != 10)
        return false;

    for (int i = 0; i < 10; i++) {
        if (phone[i] < '0' || phone[i] > '9')
            return false;
    }

    return true;
}

string cleanField(string value) {
    for (int i = 0; i < (int)value.length(); i++) {
        if (value[i] == '|')
            value[i] = '/';
    }
    return value;
}

string statusName(Status status) {
    switch (status) {
        case WAITING: return "Waiting";
        case IN_CONSULTATION: return "In Consultation";
        case COMPLETED: return "Completed";
        case CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

void heading(string title) {
    cout << "\n============================================================\n";
    cout << "              " << title << "\n";
    cout << "============================================================\n";
}

// ============================================================
// MEMBER 3 - BACKEND 1
// PATIENT RECORDS AND TOKEN MANAGEMENT
// ============================================================

// Patient linked-list insertion
void addPatientNode(Patient* p) {
    p->next = NULL;

    if (head == NULL) {
        head = p;
        return;
    }

    Patient* temp = head;

    while (temp->next != NULL)
        temp = temp->next;

    temp->next = p;
}

// Find patient using token
Patient* findToken(int token) {
    Patient* temp = head;

    while (temp != NULL) {
        if (temp->token == token)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

// Find patient using phone number
Patient* findPhone(string phone) {
    Patient* temp = head;

    while (temp != NULL) {
        if (temp->phone == phone)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

// Check active booking for same phone on same day
bool hasActiveBooking(string phone) {
    Patient* temp = head;

    while (temp != NULL) {
        if (temp->phone == phone &&
            temp->date == todayDate() &&
            (temp->status == WAITING ||
             temp->status == IN_CONSULTATION)) {
            return true;
        }

        temp = temp->next;
    }

    return false;
}

// Generate daily token number
int generateToken() {
    return nextToken++;
}

// Register a patient in backend
Patient* createPatient(
    string name,
    string phone,
    string hospital,
    string department,
    string doctor,
    string symptoms,
    bool emergency
) {
    if (name.empty() || !validPhone(phone))
        return NULL;

    if (hasActiveBooking(phone))
        return NULL;

    Patient* p = new Patient;

    p->token = generateToken();
    p->order = nextOrder++;
    p->name = cleanField(name);
    p->phone = cleanField(phone);
    p->hospital = cleanField(hospital);
    p->department = cleanField(department);
    p->doctor = cleanField(doctor);
    p->symptoms = cleanField(symptoms);
    p->date = todayDate();
    p->status = WAITING;
    p->emergency = emergency;
    p->next = NULL;

    addPatientNode(p);
    return p;
}

// Cancel patient token in backend
bool cancelToken(int token, string phone) {
    Patient* p = findToken(token);

    if (p == NULL || p->phone != phone)
        return false;

    if (p->status != WAITING)
        return false;

    p->status = CANCELLED;
    return true;
}

// Basic PIN hash for this educational project
unsigned long long hashPIN(string pin) {
    unsigned long long hash = 1469598103934665603ULL;

    for (int i = 0; i < (int)pin.length(); i++) {
        hash ^= (unsigned char)pin[i];
        hash *= 1099511628211ULL;
    }

    return hash;
}

void savePIN(string pin) {
    ofstream file(PIN_FILE.c_str());
    if (file)
        file << hashPIN(pin);
}

void loadPIN() {
    ifstream file(PIN_FILE.c_str());

    if (!(file >> receptionistPIN)) {
        receptionistPIN = hashPIN("1234");
        savePIN("1234");
    }
}

bool authenticatePIN(string pin) {
    return hashPIN(pin) == receptionistPIN;
}

bool changeReceptionistPIN(string oldPIN, string newPIN) {
    if (!authenticatePIN(oldPIN))
        return false;

    if (newPIN.length() < 4)
        return false;

    savePIN(newPIN);
    receptionistPIN = hashPIN(newPIN);
    return true;
}

// Save all patient records
void saveData() {
    ofstream file(DATA_FILE.c_str());

    if (!file) {
        cout << "Unable to save patient records.\n";
        return;
    }

    Patient* p = head;

    while (p != NULL) {
        file << p->token << "|"
             << p->order << "|"
             << p->name << "|"
             << p->phone << "|"
             << p->hospital << "|"
             << p->department << "|"
             << p->doctor << "|"
             << p->symptoms << "|"
             << p->date << "|"
             << (int)p->status << "|"
             << p->emergency << "\n";

        p = p->next;
    }
}

// Load patient records
void loadData() {
    ifstream file(DATA_FILE.c_str());

    if (!file)
        return;

    string line;

    while (getline(file, line)) {
        string fields[11];
        int field = 0;
        size_t start = 0;

        while (field < 10) {
            size_t pos = line.find('|', start);

            if (pos == string::npos)
                break;

            fields[field++] = line.substr(start, pos - start);
            start = pos + 1;
        }

        if (field != 10)
            continue;

        fields[10] = line.substr(start);

        try {
            Patient* p = new Patient;

            p->token = stoi(fields[0]);
            p->order = stoi(fields[1]);
            p->name = fields[2];
            p->phone = fields[3];
            p->hospital = fields[4];
            p->department = fields[5];
            p->doctor = fields[6];
            p->symptoms = fields[7];
            p->date = fields[8];
            p->status = (Status)stoi(fields[9]);
            p->emergency = (fields[10] == "1");
            p->next = NULL;

            addPatientNode(p);

            if (p->order >= nextOrder)
                nextOrder = p->order + 1;

            if (p->date == todayDate() && p->token >= nextToken)
                nextToken = p->token + 1;
        }
        catch (...) {
            continue;
        }
    }
}

// Delete all allocated linked-list nodes
void destroyRecords() {
    while (head != NULL) {
        Patient* temp = head;
        head = head->next;
        delete temp;
    }
}

// ============================================================
// MEMBER 4 - BACKEND 2
// QUEUE AND SYSTEM MANAGEMENT
// ============================================================

// Check whether patient belongs to selected doctor
bool sameDoctor(Patient* p, string doctor) {
    return p->doctor == doctor;
}

// Count waiting patients for a doctor
int countWaitingForDoctor(string doctor) {
    int count = 0;
    Patient* p = head;

    while (p != NULL) {
        if (sameDoctor(p, doctor) && p->status == WAITING)
            count++;

        p = p->next;
    }

    return count;
}

// Find next patient: emergency first, FIFO within priority
Patient* getNextPatient(string doctor) {
    Patient* p = head;
    Patient* emergencyPatient = NULL;
    Patient* normalPatient = NULL;

    while (p != NULL) {
        if (p->doctor == doctor && p->status == WAITING) {
            if (p->emergency) {
                if (emergencyPatient == NULL ||
                    p->order < emergencyPatient->order)
                    emergencyPatient = p;
            }
            else {
                if (normalPatient == NULL ||
                    p->order < normalPatient->order)
                    normalPatient = p;
            }
        }

        p = p->next;
    }

    return emergencyPatient != NULL ?
           emergencyPatient : normalPatient;
}

// Count patients ahead of selected token
int countAhead(Patient* target) {
    int count = 0;
    Patient* p = head;

    while (p != NULL) {
        if (p != target &&
            p->doctor == target->doctor &&
            p->status == WAITING) {

            if (target->emergency) {
                if (p->emergency && p->order < target->order)
                    count++;
            }
            else {
                if (p->emergency ||
                    (!p->emergency && p->order < target->order))
                    count++;
            }
        }

        p = p->next;
    }

    return count;
}

// Calculate estimated waiting time
int estimatedWait(Patient* target) {
    return countAhead(target) * SERVICE_MINUTES;
}

// Mark a waiting patient as emergency
bool setEmergency(int token) {
    Patient* p = findToken(token);

    if (p == NULL || p->status != WAITING)
        return false;

    p->emergency = true;
    return true;
}

// Call the next patient
Patient* callNext(string doctor) {
    Patient* p = getNextPatient(doctor);

    if (p == NULL)
        return NULL;

    p->status = IN_CONSULTATION;
    return p;
}

// Complete consultation
bool finishConsultation(int token) {
    Patient* p = findToken(token);

    if (p == NULL || p->status != IN_CONSULTATION)
        return false;

    p->status = COMPLETED;
    return true;
}

// Statistics calculations
int countByStatus(Status status) {
    int count = 0;
    Patient* p = head;

    while (p != NULL) {
        if (p->status == status)
            count++;

        p = p->next;
    }

    return count;
}

int countEmergencyWaiting() {
    int count = 0;
    Patient* p = head;

    while (p != NULL) {
        if (p->emergency && p->status == WAITING)
            count++;

        p = p->next;
    }

    return count;
}

// ============================================================
// MEMBER 1 - FRONTEND 1
// COMPLETE PATIENT PORTAL
// All patient menu cases remain in this section.
// ============================================================

void patientHospitalList() {
    heading("AVAILABLE HOSPITALS");

    for (int i = 0; i < HOSPITAL_COUNT; i++)
        cout << i + 1 << ". " << hospitals[i] << "\n";
}

void patientDepartmentList() {
    heading("AVAILABLE DEPARTMENTS");

    for (int i = 0; i < DEPARTMENT_COUNT; i++)
        cout << i + 1 << ". " << departments[i] << "\n";
}

void patientDoctorList(int dept) {
    heading("AVAILABLE DOCTORS");

    for (int i = 0; i < DOCTORS_PER_DEPT; i++)
        cout << i + 1 << ". " << doctors[dept][i] << "\n";
}

void patientBookingReceipt(Patient* p) {
    if (p == NULL)
        return;

    heading("BOOKING CONFIRMATION");

    cout << "Patient Name : " << p->name << "\n";
    cout << "Phone        : " << p->phone << "\n";
    cout << "Hospital     : " << p->hospital << "\n";
    cout << "Department   : " << p->department << "\n";
    cout << "Doctor       : " << p->doctor << "\n";
    cout << "Token Number : " << p->token << "\n";
    cout << "Date         : " << p->date << "\n";
    cout << "Time         : " << currentTime() << "\n";
    cout << "Priority     : "
         << (p->emergency ? "Emergency" : "Normal") << "\n";
    cout << "Status       : " << statusName(p->status) << "\n";

    cout << "\nEstimated waiting time: "
         << estimatedWait(p) << " minutes\n";
}

// Patient-facing booking interface
void patientBookToken() {
    heading("BOOK YOUR TOKEN");

    string name = readText("Enter patient name: ");
    string phone = readText("Enter 10-digit phone number: ");

    if (!validPhone(phone)) {
        cout << "Invalid phone number.\n";
        return;
    }

    patientHospitalList();
    cout << "Select hospital: ";
    int h = readChoice();

    if (h < 1 || h > HOSPITAL_COUNT) {
        cout << "Invalid hospital selection.\n";
        return;
    }

    patientDepartmentList();
    cout << "Select department: ";
    int d = readChoice();

    if (d < 1 || d > DEPARTMENT_COUNT) {
        cout << "Invalid department selection.\n";
        return;
    }

    patientDoctorList(d - 1);
    cout << "Select doctor: ";
    int doc = readChoice();

    if (doc < 1 || doc > DOCTORS_PER_DEPT) {
        cout << "Invalid doctor selection.\n";
        return;
    }

    string symptoms = readText("Enter symptoms or reason for visit: ");

    Patient* p = createPatient(
        name,
        phone,
        hospitals[h - 1],
        departments[d - 1],
        doctors[d - 1][doc - 1],
        symptoms,
        false
    );

    if (p == NULL) {
        cout << "Booking failed. Check details or existing active booking.\n";
        return;
    }

    saveData();
    patientBookingReceipt(p);
}

// Patient token details display
void patientTokenDetails() {
    heading("TOKEN DETAILS");

    int token;
    cout << "Enter token number: ";
    token = readChoice();

    Patient* p = findToken(token);

    if (p == NULL) {
        cout << "Token not found.\n";
        return;
    }

    patientBookingReceipt(p);
}

// Patient status interface
void patientCheckStatus() {
    heading("CHECK TOKEN STATUS");

    int token;
    cout << "Enter token number: ";
    token = readChoice();

    Patient* p = findToken(token);

    if (p == NULL) {
        cout << "Token not found.\n";
        return;
    }

    cout << "Patient: " << p->name << "\n";
    cout << "Status: " << statusName(p->status) << "\n";

    if (p->status == WAITING) {
        cout << "People ahead: " << countAhead(p) << "\n";
        cout << "Estimated wait: " << estimatedWait(p)
             << " minutes\n";
    }
}

// Patient history interface
void patientHistory() {
    heading("PATIENT BOOKING HISTORY");

    string phone = readText("Enter registered phone number: ");
    bool found = false;

    Patient* p = head;

    while (p != NULL) {
        if (p->phone == phone) {
            cout << "\nToken: " << p->token
                 << " | Date: " << p->date
                 << " | Doctor: " << p->doctor
                 << " | Status: " << statusName(p->status)
                 << "\n";
            found = true;
        }

        p = p->next;
    }

    if (!found)
        cout << "No records found.\n";
}

// Patient cancellation interface
void patientCancelBooking() {
    heading("CANCEL TOKEN");

    int token = readChoice();
    string phone = readText("Enter registered phone number: ");

    if (cancelToken(token, phone)) {
        saveData();
        cout << "Token cancelled successfully.\n";
    }
    else {
        cout << "Cancellation failed. Only waiting tokens can be cancelled.\n";
    }
}

// Symptom-based department navigation only
void patientDepartmentSuggestion() {
    heading("DEPARTMENT SUGGESTION");

    cout << "This is only a basic navigation guide, not a diagnosis.\n";
    cout << "For medical advice, consult a qualified healthcare professional.\n\n";

    cout << "1. Fever, cold, general weakness -> General Medicine\n";
    cout << "2. Ear, nose or throat concerns -> ENT\n";
    cout << "3. Bone, joint or movement concerns -> Orthopaedics\n";
    cout << "4. Skin or hair concerns -> Dermatology\n";
    cout << "\nFor severe or urgent symptoms, seek immediate medical care.\n";
}

void patientGuidelines() {
    heading("PATIENT GUIDELINES");

    cout << "1. Keep your token number safe.\n";
    cout << "2. Reach the hospital before your estimated turn.\n";
    cout << "3. Carry relevant medical documents.\n";
    cout << "4. Contact reception if your details are incorrect.\n";
    cout << "5. Emergency priority is assigned by hospital staff.\n";
}

void patientHelp() {
    heading("PATIENT HELP");

    cout << "1. Book a token using Book Token.\n";
    cout << "2. Use Check Status to see your queue position.\n";
    cout << "3. Use Token Details to view your booking.\n";
    cout << "4. Use Booking History to see previous records.\n";
    cout << "5. Contact reception for assistance.\n";
}

void patientMenu() {
    int choice;

    do {
        heading("PATIENT PORTAL");

        cout << "1. Book Token\n";
        cout << "2. View Token Details\n";
        cout << "3. Check Token Status\n";
        cout << "4. Booking History\n";
        cout << "5. Cancel Token\n";
        cout << "6. Department Suggestion\n";
        cout << "7. Hospital List\n";
        cout << "8. Department List\n";
        cout << "9. Patient Guidelines\n";
        cout << "10. Help\n";
        cout << "0. Back to Main Menu\n";

        cout << "\nEnter choice: ";
        choice = readChoice();

        // FRONTEND 1 CASES ARE KEPT TOGETHER
        switch (choice) {
            case 1: patientBookToken(); break;
            case 2: patientTokenDetails(); break;
            case 3: patientCheckStatus(); break;
            case 4: patientHistory(); break;
            case 5: patientCancelBooking(); break;
            case 6: patientDepartmentSuggestion(); break;
            case 7: patientHospitalList(); break;
            case 8: patientDepartmentList(); break;
            case 9: patientGuidelines(); break;
            case 10: patientHelp(); break;
            case 0: cout << "Leaving Patient Portal.\n"; break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

// ============================================================
// MEMBER 2 - FRONTEND 2
// COMPLETE RECEPTIONIST PORTAL
// All receptionist menu cases remain in this section.
// ============================================================

bool receptionistLogin() {
    heading("RECEPTIONIST LOGIN");

    string pin = readText("Enter receptionist PIN: ");

    if (authenticatePIN(pin)) {
        cout << "Login successful.\n";
        return true;
    }

    cout << "Incorrect PIN.\n";
    return false;
}

void receptionistShowHospitals() {
    heading("HOSPITAL LIST");

    for (int i = 0; i < HOSPITAL_COUNT; i++)
        cout << i + 1 << ". " << hospitals[i] << "\n";
}

// Walk-in registration interface
void receptionistWalkIn() {
    heading("WALK-IN PATIENT REGISTRATION");

    string name = readText("Patient name: ");
    string phone = readText("Phone number: ");

    if (!validPhone(phone)) {
        cout << "Invalid phone number.\n";
        return;
    }

    receptionistShowHospitals();
    cout << "Select hospital: ";
    int h = readChoice();

    if (h < 1 || h > HOSPITAL_COUNT)
        return;

    patientDepartmentList();
    cout << "Select department: ";
    int d = readChoice();

    if (d < 1 || d > DEPARTMENT_COUNT)
        return;

    patientDoctorList(d - 1);
    cout << "Select doctor: ";
    int doc = readChoice();

    if (doc < 1 || doc > DOCTORS_PER_DEPT)
        return;

    string symptoms = readText("Symptoms/reason: ");

    cout << "Is this an emergency marked by staff? (1=Yes, 0=No): ";
    int emergency = readChoice();

    if (emergency != 0 && emergency != 1)
        emergency = 0;

    Patient* p = createPatient(
        name,
        phone,
        hospitals[h - 1],
        departments[d - 1],
        doctors[d - 1][doc - 1],
        symptoms,
        emergency == 1
    );

    if (p == NULL) {
        cout << "Registration failed. Check details or active booking.\n";
        return;
    }

    saveData();
    patientBookingReceipt(p);
}

// Multiple walk-in registration interface
void receptionistMultipleWalkIn() {
    heading("MULTIPLE WALK-IN REGISTRATION");

    cout << "Enter number of patients to register: ";
    int total = readChoice();

    if (total < 1 || total > 20) {
        cout << "Enter a number between 1 and 20.\n";
        return;
    }

    for (int i = 0; i < total; i++) {
        cout << "\nPatient " << i + 1 << ":\n";
        receptionistWalkIn();
    }

    saveData();
}

// Search patient interface
void receptionistSearchPatient() {
    heading("SEARCH PATIENT");

    int token;
    cout << "Enter token number: ";
    token = readChoice();

    Patient* p = findToken(token);

    if (p == NULL) {
        cout << "Patient not found.\n";
        return;
    }

    patientBookingReceipt(p);
}

// Receptionist cancellation interface
void receptionistCancelToken() {
    heading("RECEPTIONIST TOKEN CANCELLATION");

    int token;
    cout << "Enter token number: ";
    token = readChoice();

    Patient* p = findToken(token);

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

// Queue display interface
void receptionistQueueDisplay() {
    heading("DOCTOR-WISE WAITING QUEUE");

    receptionistShowHospitals();
    cout << "Select hospital: ";
    int h = readChoice();

    if (h < 1 || h > HOSPITAL_COUNT)
        return;

    patientDepartmentList();
    cout << "Select department: ";
    int d = readChoice();

    if (d < 1 || d > DEPARTMENT_COUNT)
        return;

    patientDoctorList(d - 1);
    cout << "Select doctor: ";
    int doc = readChoice();

    if (doc < 1 || doc > DOCTORS_PER_DEPT)
        return;

    string doctor = doctors[d - 1][doc - 1];

    cout << "\nQueue for " << doctor << ":\n";

    int position = 1;
    Patient* p = head;

    while (p != NULL) {
        if (p->doctor == doctor && p->status == WAITING) {
            cout << position++ << ". Token " << p->token
                 << " | " << p->name
                 << " | " << (p->emergency ? "EMERGENCY" : "NORMAL")
                 << " | Waiting " << estimatedWait(p) << " min\n";
        }

        p = p->next;
    }

    if (position == 1)
        cout << "No waiting patients.\n";
}

// Emergency priority interface
void receptionistEmergencyPriority() {
    heading("EMERGENCY PRIORITY");

    cout << "Enter token number: ";
    int token = readChoice();

    if (setEmergency(token)) {
        saveData();
        cout << "Emergency priority updated.\n";
    }
    else {
        cout << "Unable to update priority. Check token status.\n";
    }
}

// Call next patient interface
void receptionistCallNext() {
    heading("CALL NEXT PATIENT");

    patientDepartmentList();
    cout << "Select department: ";
    int d = readChoice();

    if (d < 1 || d > DEPARTMENT_COUNT)
        return;

    patientDoctorList(d - 1);
    cout << "Select doctor: ";
    int doc = readChoice();

    if (doc < 1 || doc > DOCTORS_PER_DEPT)
        return;

    Patient* p = callNext(doctors[d - 1][doc - 1]);

    if (p == NULL) {
        cout << "No waiting patient for this doctor.\n";
        return;
    }

    saveData();

    cout << "Now calling Token " << p->token
         << " - " << p->name << "\n";
}

// Complete consultation interface
void receptionistCompleteConsultation() {
    heading("COMPLETE CONSULTATION");

    cout << "Enter token number: ";
    int token = readChoice();

    if (finishConsultation(token)) {
        saveData();
        cout << "Consultation marked as completed.\n";
    }
    else {
        cout << "Invalid token or patient is not in consultation.\n";
    }
}

// Statistics display interface
void receptionistStatistics() {
    heading("SYSTEM STATISTICS");

    cout << "Waiting patients       : " << countByStatus(WAITING) << "\n";
    cout << "In consultation        : " << countByStatus(IN_CONSULTATION) << "\n";
    cout << "Completed consultations: " << countByStatus(COMPLETED) << "\n";
    cout << "Cancelled tokens       : " << countByStatus(CANCELLED) << "\n";
    cout << "Emergency waiting      : " << countEmergencyWaiting() << "\n";
}

// Change PIN interface
void receptionistChangePIN() {
    heading("CHANGE RECEPTIONIST PIN");

    string oldPIN = readText("Enter current PIN: ");
    string newPIN = readText("Enter new PIN (minimum 4 characters): ");

    if (changeReceptionistPIN(oldPIN, newPIN))
        cout << "PIN changed successfully.\n";
    else
        cout << "PIN change failed. Check current PIN or new PIN length.\n";
}

// Complete receptionist menu
void receptionistMenu() {
    if (!receptionistLogin())
        return;

    int choice;

    do {
        heading("RECEPTIONIST DASHBOARD");

        cout << "1. Register Walk-in Patient\n";
        cout << "2. Register Multiple Walk-in Patients\n";
        cout << "3. Search Patient\n";
        cout << "4. Cancel Token\n";
        cout << "5. View Doctor-wise Queue\n";
        cout << "6. Mark Emergency Priority\n";
        cout << "7. Call Next Patient\n";
        cout << "8. Complete Consultation\n";
        cout << "9. View Statistics\n";
        cout << "10. Change PIN\n";
        cout << "0. Logout\n";

        cout << "\nEnter choice: ";
        choice = readChoice();

        // FRONTEND 2 CASES ARE KEPT TOGETHER
        switch (choice) {
            case 1: receptionistWalkIn(); break;
            case 2: receptionistMultipleWalkIn(); break;
            case 3: receptionistSearchPatient(); break;
            case 4: receptionistCancelToken(); break;
            case 5: receptionistQueueDisplay(); break;
            case 6: receptionistEmergencyPriority(); break;
            case 7: receptionistCallNext(); break;
            case 8: receptionistCompleteConsultation(); break;
            case 9: receptionistStatistics(); break;
            case 10: receptionistChangePIN(); break;
            case 0: cout << "Receptionist logged out.\n"; break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

// ============================================================
// MAIN SYSTEM MENU
// ============================================================

void mainMenu() {
    int choice;

    do {
        heading("HOSPITAL TOKEN BOOKING AND QUEUE MANAGEMENT");

        cout << "Date: " << todayDate() << "\n";
        cout << "Time: " << currentTime() << "\n\n";

        cout << "1. Patient Portal\n";
        cout << "2. Receptionist Portal\n";
        cout << "0. Exit System\n";

        cout << "\nEnter choice: ";
        choice = readChoice();

        switch (choice) {
            case 1:
                patientMenu();
                break;

            case 2:
                receptionistMenu();
                break;

            case 0:
                saveData();
                cout << "System data saved. Goodbye.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 0);
}

// ============================================================
// PROGRAM ENTRY POINT
// ============================================================

int main() {
    loadPIN();
    loadData();

    mainMenu();

    saveData();
    destroyRecords();

    return 0;
}
