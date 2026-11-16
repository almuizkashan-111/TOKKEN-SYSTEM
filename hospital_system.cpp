
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>
#include <cctype>
#include <cstdlib>

using namespace std;

const int HOSPITAL_COUNT = 8;
const int DEPARTMENT_COUNT = 4;
const int DOCTORS_PER_DEPT = 2;
const int SERVICE_MINUTES = 15;

const string DATA_FILE = "patients_cpp.dat";
const string PIN_FILE = "receptionist_pin_cpp.dat";

enum Status {
    WAITING = 1,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

/* ======================== PATIENT STRUCTURE ======================== */

struct Patient {
    int token;
    string name;
    string source;
    int hospital;
    int department;
    int doctor;
    string symptoms;
    string bookingDate;
    bool emergency;
    int status;
    time_t bookingTime;
    time_t statusTime;
};

/* ======================== LINKED LIST NODE ======================== */

struct Node {
    Patient data;
    Node* next;

    Node(const Patient& p) {
        data = p;
        next = nullptr;
    }
};

/* ======================== GLOBAL DATA ======================== */

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
    {"Dr. P. Verma", "Dr. S. Kapoor"},
    {"Dr. K. Singh", "Dr. M. Joshi"},
    {"Dr. N. Gupta", "Dr. T. Rao"}
};

Node* head = nullptr;
int nextToken = 1;

/* ======================== INPUT FUNCTIONS ======================== */

string readLine(string message) {
    string value;

    cout << message;
    getline(cin, value);

    return value;
}

int readInt(string message, int minimum, int maximum) {
    string input;
    int value;

    while (true) {
        cout << message;
        getline(cin, input);

        try {
            size_t position;
            value = stoi(input, &position);

            if (position == input.length() &&
                value >= minimum && value <= maximum) {
                return value;
            }
        }
        catch (...) {
        }

        cout << "Invalid input. Enter a number between "
             << minimum << " and " << maximum << ".\n";
    }
}

void pauseScreen() {
    cout << "\nPress ENTER to continue...";
    string temp;
    getline(cin, temp);
}

/* ======================== DATE FUNCTIONS ======================== */

string getTodayDate() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    if (t == nullptr) {
        return "Unknown";
    }

    char buffer[20];

    strftime(buffer, sizeof(buffer), "%d-%m-%Y", t);

    return string(buffer);
}

string getCurrentDateTime() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    if (t == nullptr) {
        return "Unknown";
    }

    char buffer[30];

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M", t);

    return string(buffer);
}

bool isToday(const Patient& p) {
    return p.bookingDate == getTodayDate();
}

/* ======================== PIN HASH ======================== */

unsigned long hashPIN(const string& pin) {
    unsigned long hash = 0;

    for (char ch : pin) {
        hash = hash * 31 + static_cast<unsigned char>(ch);
    }

    return hash;
}

void saveDefaultPIN() {
    ofstream file(PIN_FILE, ios::binary);

    if (!file) {
        return;
    }

    unsigned long hash = hashPIN("1234");

    file.write(reinterpret_cast<const char*>(&hash),
               sizeof(hash));
}

bool verifyPIN(const string& pin) {
    ifstream file(PIN_FILE, ios::binary);

    if (!file) {
        saveDefaultPIN();
        file.open(PIN_FILE, ios::binary);

        if (!file) {
            return false;
        }
    }

    unsigned long savedHash = 0;

    file.read(reinterpret_cast<char*>(&savedHash),
              sizeof(savedHash));

    if (!file) {
        return false;
    }

    return savedHash == hashPIN(pin);
}

void changePIN() {
    cout << "\n========== CHANGE RECEPTIONIST PIN ==========\n";

    string oldPIN = readLine("Enter current PIN: ");

    if (!verifyPIN(oldPIN)) {
        cout << "Incorrect PIN.\n";
        return;
    }

    string newPIN = readLine("Enter new PIN: ");

    if (newPIN.length() < 4) {
        cout << "PIN must contain at least 4 characters.\n";
        return;
    }

    string confirmPIN = readLine("Confirm new PIN: ");

    if (newPIN != confirmPIN) {
        cout << "PIN confirmation does not match.\n";
        return;
    }

    ofstream file(PIN_FILE, ios::binary);

    if (!file) {
        cout << "Unable to update PIN.\n";
        return;
    }

    unsigned long hash = hashPIN(newPIN);

    file.write(reinterpret_cast<const char*>(&hash),
               sizeof(hash));

    cout << "PIN changed successfully.\n";
}

/* ======================== LINKED LIST OPERATIONS ======================== */

void insertPatient(const Patient& p) {
    Node* newNode = new Node(p);

    if (head == nullptr) {
        head = newNode;
        return;
    }

    Node* temp = head;

    while (temp->next != nullptr) {
        temp = temp->next;
    }

    temp->next = newNode;
}

Patient* findToken(int token) {
    Node* temp = head;
    Patient* oldRecord = nullptr;

    while (temp != nullptr) {
        if (temp->data.token == token) {
            if (isToday(temp->data)) {
                return &temp->data;
            }

            oldRecord = &temp->data;
        }

        temp = temp->next;
    }

    return oldRecord;
}

/* ======================== FILE STORAGE ======================== */

/*
   Text-based records are stored one patient per line.
   The separator character | is not accepted in user input.
*/

void saveData() {
    ofstream file(DATA_FILE);

    if (!file) {
        cout << "Warning: Unable to save patient records.\n";
        return;
    }

    file << nextToken << '\n';

    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        file << p.token << '\n';
        file << p.name << '\n';
        file << p.source << '\n';
        file << p.hospital << '\n';
        file << p.department << '\n';
        file << p.doctor << '\n';
        file << p.symptoms << '\n';
        file << p.bookingDate << '\n';
        file << p.emergency << '\n';
        file << p.status << '\n';
        file << static_cast<long long>(p.bookingTime) << '\n';
        file << static_cast<long long>(p.statusTime) << '\n';

        temp = temp->next;
    }
}

void loadData() {
    ifstream file(DATA_FILE);

    if (!file) {
        nextToken = 1;
        return;
    }

    int savedCounter = 1;

    if (!(file >> savedCounter)) {
        nextToken = 1;
        return;
    }

    file.ignore(numeric_limits<streamsize>::max(), '\n');

    while (true) {
        Patient p;
        long long booking = 0;
        long long status = 0;

        if (!(file >> p.token)) {
            break;
        }

        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!getline(file, p.name)) break;
        if (!getline(file, p.source)) break;

        if (!(file >> p.hospital)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!(file >> p.department)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!(file >> p.doctor)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!getline(file, p.symptoms)) break;
        if (!getline(file, p.bookingDate)) break;

        if (!(file >> p.emergency)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!(file >> p.status)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!(file >> booking)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!(file >> status)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        p.bookingTime = static_cast<time_t>(booking);
        p.statusTime = static_cast<time_t>(status);

        if (p.hospital < 0 || p.hospital >= HOSPITAL_COUNT ||
            p.department < 0 || p.department >= DEPARTMENT_COUNT ||
            p.doctor < 0 || p.doctor >= DOCTORS_PER_DEPT) {
            continue;
        }

        if (p.status < WAITING || p.status > CANCELLED) {
            continue;
        }

        insertPatient(p);
    }

    /*
       DAILY TOKEN RENEWAL:
       Previous records stay saved.
       Only today's highest token determines the next token.
    */

    int maximumToday = 0;
    Node* temp = head;

    while (temp != nullptr) {
        if (isToday(temp->data) &&
            temp->data.token > maximumToday) {
            maximumToday = temp->data.token;
        }

        temp = temp->next;
    }

    nextToken = maximumToday + 1;

    if (nextToken < 1) {
        nextToken = 1;
    }

    (void)savedCounter;
}

/* ======================== DISPLAY FUNCTIONS ======================== */

void showHospitals() {
    cout << "\n=============== AVAILABLE HOSPITALS ===============\n";

    for (int i = 0; i < HOSPITAL_COUNT; i++) {
        cout << i + 1 << ". " << hospitals[i] << '\n';
    }
}

void showDepartments() {
    cout << "\n=============== AVAILABLE DEPARTMENTS ===============\n";

    for (int i = 0; i < DEPARTMENT_COUNT; i++) {
        cout << i + 1 << ". " << departments[i] << '\n';
    }
}

void showDoctors(int department) {
    cout << "\n=============== AVAILABLE DOCTORS ===============\n";

    for (int i = 0; i < DOCTORS_PER_DEPT; i++) {
        cout << i + 1 << ". " << doctors[department][i] << '\n';
    }
}

void showPatient(const Patient& p) {
    cout << "\n---------------------------------------------\n";

    cout << "Token Number      : " << p.token << '\n';
    cout << "Patient Name      : " << p.name << '\n';
    cout << "Registration Type : " << p.source << '\n';
    cout << "Hospital          : " << hospitals[p.hospital] << '\n';
    cout << "Department        : " << departments[p.department] << '\n';
    cout << "Assigned Doctor   : "
         << doctors[p.department][p.doctor] << '\n';
    cout << "Symptoms          : " << p.symptoms << '\n';
    cout << "Booking Date      : " << p.bookingDate << '\n';

    cout << "Priority          : "
         << (p.emergency ? "Staff-marked priority" : "Normal")
         << '\n';

    cout << "Status            : ";

    switch (p.status) {
        case WAITING:
            cout << "Waiting\n";
            break;
        case IN_CONSULTATION:
            cout << "In Consultation\n";
            break;
        case COMPLETED:
            cout << "Completed\n";
            break;
        case CANCELLED:
            cout << "Cancelled\n";
            break;
        default:
            cout << "Unknown\n";
    }

    cout << "---------------------------------------------\n";
}

/* ======================== QUEUE CALCULATIONS ======================== */

int countWaiting(int hospital, int department, int doctor) {
    int count = 0;

    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (isToday(p) &&
            p.hospital == hospital &&
            p.department == department &&
            p.doctor == doctor &&
            p.status == WAITING) {
            count++;
        }

        temp = temp->next;
    }

    return count;
}

int countAhead(const Patient& target) {
    int count = 0;

    if (!isToday(target)) {
        return 0;
    }

    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (p.token != target.token &&
            isToday(p) &&
            p.hospital == target.hospital &&
            p.department == target.department &&
            p.doctor == target.doctor &&
            p.status == WAITING) {

            if (!target.emergency || p.emergency) {
                count++;
            }
        }

        temp = temp->next;
    }

    return count;
}

int getWaitingPosition(const Patient& p) {
    if (p.status != WAITING || !isToday(p)) {
        return 0;
    }

    return countAhead(p) + 1;
}

/* ======================== PRIORITY QUEUE ======================== */

Patient* getNextPatient(int hospital, int department, int doctor) {
    Node* temp = head;
    Patient* selected = nullptr;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (isToday(p) &&
            p.hospital == hospital &&
            p.department == department &&
            p.doctor == doctor &&
            p.status == WAITING) {

            if (selected == nullptr ||
                p.emergency > selected->emergency ||
                (p.emergency == selected->emergency &&
                 p.token < selected->token)) {
                selected = &p;
            }
        }

        temp = temp->next;
    }

    return selected;
}

void showQueue(int hospital, int department, int doctor) {
    cout << "\n=============== WAITING QUEUE ===============\n";

    cout << "Hospital   : " << hospitals[hospital] << '\n';
    cout << "Department : " << departments[department] << '\n';
    cout << "Doctor     : " << doctors[department][doctor] << '\n';

    if (countWaiting(hospital, department, doctor) == 0) {
        cout << "\nNo patients are waiting today.\n";
        return;
    }

    cout << left
         << setw(10) << "Token"
         << setw(23) << "Patient"
         << setw(15) << "Priority"
         << setw(10) << "Position" << '\n';

    int position = 1;

    for (int priority = 1; priority >= 0; priority--) {
        Node* temp = head;

        while (temp != nullptr) {
            Patient& p = temp->data;

            if (isToday(p) &&
                p.hospital == hospital &&
                p.department == department &&
                p.doctor == doctor &&
                p.status == WAITING &&
                p.emergency == static_cast<bool>(priority)) {

                cout << left
                     << setw(10) << p.token
                     << setw(23) << p.name.substr(0, 22)
                     << setw(15)
                     << (p.emergency ? "Priority" : "Normal")
                     << setw(10) << position << '\n';

                position++;
            }

            temp = temp->next;
        }
    }
}

/* ======================== CONSULTATION MANAGEMENT ======================== */

void callNextPatient(int hospital, int department, int doctor) {
    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (isToday(p) &&
            p.hospital == hospital &&
            p.department == department &&
            p.doctor == doctor &&
            p.status == IN_CONSULTATION) {

            cout << "\nThis doctor already has an active consultation.\n";
            return;
        }

        temp = temp->next;
    }

    Patient* p = getNextPatient(hospital, department, doctor);

    if (p == nullptr) {
        cout << "\nNo waiting patients for this doctor.\n";
        return;
    }

    p->status = IN_CONSULTATION;
    p->statusTime = time(nullptr);

    saveData();

    cout << "\n========== PATIENT CALLED ==========\n";
    cout << "Token    : " << p->token << '\n';
    cout << "Patient  : " << p->name << '\n';
    cout << "Doctor   : " << doctors[department][doctor] << '\n';
    cout << "Priority : "
         << (p->emergency ? "Priority" : "Normal") << '\n';
}

void completeConsultation(int hospital, int department, int doctor) {
    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (isToday(p) &&
            p.hospital == hospital &&
            p.department == department &&
            p.doctor == doctor &&
            p.status == IN_CONSULTATION) {

            p.status = COMPLETED;
            p.statusTime = time(nullptr);

            saveData();

            cout << "\nConsultation completed successfully.\n";
            cout << "Patient: " << p.name << '\n';
            cout << "Token: " << p.token << '\n';
            return;
        }

        temp = temp->next;
    }

    cout << "\nNo active consultation found for this doctor.\n";
}

/* ======================== PATIENT REGISTRATION ======================== */

void registerPatient(bool walkIn) {
    if (nextToken > 999999) {
        cout << "\nDaily token limit reached.\n";
        return;
    }

    Patient p{};

    cout << "\n========== PATIENT REGISTRATION ==========\n";

    p.name = readLine("Enter patient name: ");

    if (p.name.empty()) {
        cout << "Patient name cannot be empty.\n";
        return;
    }

    showHospitals();

    p.hospital = readInt("Select hospital: ",
                         1, HOSPITAL_COUNT) - 1;

    showDepartments();

    p.department = readInt("Select department: ",
                            1, DEPARTMENT_COUNT) - 1;

    showDoctors(p.department);

    p.doctor = readInt("Select doctor: ",
                       1, DOCTORS_PER_DEPT) - 1;

    p.symptoms = readLine("Enter symptoms or reason for visit: ");

    if (p.symptoms.empty()) {
        p.symptoms = "Not specified";
    }

    p.source = walkIn ? "Walk-in" : "Online";

    p.emergency = false;

    if (walkIn) {
        p.emergency = readInt(
            "Mark as staff-priority case? (1=Yes, 0=No): ",
            0, 1
        ) == 1;
    }

    p.token = nextToken++;
    p.status = WAITING;
    p.bookingTime = time(nullptr);
    p.statusTime = p.bookingTime;
    p.bookingDate = getTodayDate();

    insertPatient(p);
    saveData();

    cout << "\n========== REGISTRATION SUCCESSFUL ==========\n";
    cout << "Token      : " << p.token << '\n';
    cout << "Patient    : " << p.name << '\n';
    cout << "Hospital   : " << hospitals[p.hospital] << '\n';
    cout << "Department : " << departments[p.department] << '\n';
    cout << "Doctor     : " << doctors[p.department][p.doctor] << '\n';
    cout << "Priority   : "
         << (p.emergency ? "Staff-marked priority" : "Normal")
         << '\n';

    cout << "\nPlease keep your token number safe.\n";
}

/* ======================== MULTIPLE WALK-IN REGISTRATION ======================== */

void multipleWalkInRegistration() {
    int number = readInt(
        "\nHow many patients do you want to register? ", 1, 50);

    for (int i = 0; i < number; i++) {
        cout << "\n========== PATIENT " << i + 1
             << " OF " << number << " ==========\n";

        registerPatient(true);
    }
}

/* ======================== TOKEN DETAILS ======================== */

void showTokenDetails() {
    int token = readInt("\nEnter token number: ", 1, 999999);

    Patient* p = findToken(token);

    if (p == nullptr) {
        cout << "\nToken not found.\n";
        return;
    }

    showPatient(*p);

    if (p->status == WAITING && isToday(*p)) {
        int ahead = countAhead(*p);

        cout << "Waiting position: " << ahead + 1 << '\n';
        cout << "Patients ahead: " << ahead << '\n';
        cout << "Estimated waiting time: "
             << ahead * SERVICE_MINUTES << " minutes\n";

        cout << "Waiting time is only an estimate.\n";
    }
}

void showPatientStatus() {
    int token = readInt("\nEnter your token number: ", 1, 999999);

    Patient* p = findToken(token);

    if (p == nullptr) {
        cout << "\nPatient token not found.\n";
        return;
    }

    cout << "\n========== TOKEN STATUS ==========\n";
    cout << "Token: " << p->token << '\n';

    if (!isToday(*p)) {
        cout << "This token belongs to an earlier date.\n";
    }

    if (p->status == WAITING) {
        cout << "Status: Waiting\n";

        if (isToday(*p)) {
            int ahead = countAhead(*p);

            cout << "Queue position: " << ahead + 1 << '\n';
            cout << "Estimated waiting time: "
                 << ahead * SERVICE_MINUTES << " minutes\n";
        }
    } else if (p->status == IN_CONSULTATION) {
        cout << "Status: In consultation\n";
    } else if (p->status == COMPLETED) {
        cout << "Status: Consultation completed\n";
    } else if (p->status == CANCELLED) {
        cout << "Status: Token cancelled\n";
    }
}

/* ======================== EMERGENCY PRIORITY MANAGEMENT ======================== */

void markEmergencyPriority() {
    int token = readInt("\nEnter today's patient token: ",
                        1, 999999);

    Patient* p = findToken(token);

    if (p == nullptr || !isToday(*p)) {
        cout << "\nToday's patient token was not found.\n";
        return;
    }

    if (p->status != WAITING) {
        cout << "\nOnly waiting patients can be marked priority.\n";
        return;
    }

    if (p->emergency) {
        cout << "\nThis patient is already marked priority.\n";
        return;
    }

    cout << "\nConfirm staff assessment before changing priority.\n";

    int confirm = readInt(
        "Mark this patient as priority? (1=Yes, 0=No): ",
        0, 1
    );

    if (confirm == 1) {
        p->emergency = true;
        saveData();

        cout << "\nPriority successfully updated.\n";
        cout << "Token: " << p->token << '\n';
        cout << "Patient: " << p->name << '\n';
    } else {
        cout << "\nNo changes made.\n";
    }
}

/* ======================== CANCELLATION ======================== */

void cancelToken() {
    int token = readInt("\nEnter token number to cancel: ",
                        1, 999999);

    Patient* p = findToken(token);

    if (p == nullptr || !isToday(*p)) {
        cout << "\nToday's patient token was not found.\n";
        return;
    }

    if (p->status != WAITING) {
        cout << "\nOnly waiting tokens can be cancelled.\n";
        return;
    }

    p->status = CANCELLED;
    p->statusTime = time(nullptr);

    saveData();

    cout << "\nToken " << token << " cancelled successfully.\n";
}

/* ======================== STATISTICS ======================== */

void showStatistics() {
    int total = 0;
    int waiting = 0;
    int active = 0;
    int completed = 0;
    int cancelled = 0;
    int priority = 0;

    Node* temp = head;

    while (temp != nullptr) {
        Patient& p = temp->data;

        if (isToday(p)) {
            total++;

            if (p.status == WAITING) waiting++;
            else if (p.status == IN_CONSULTATION) active++;
            else if (p.status == COMPLETED) completed++;
            else if (p.status == CANCELLED) cancelled++;

            if (p.emergency) priority++;
        }

        temp = temp->next;
    }

    cout << "\n=============== TODAY'S STATISTICS ===============\n";
    cout << "Date                       : " << getTodayDate() << '\n';
    cout << "Total registered patients  : " << total << '\n';
    cout << "Waiting patients           : " << waiting << '\n';
    cout << "Active consultations       : " << active << '\n';
    cout << "Completed consultations    : " << completed << '\n';
    cout << "Cancelled tokens           : " << cancelled << '\n';
    cout << "Priority cases             : " << priority << '\n';
    cout << "Next token number          : " << nextToken << '\n';
}

void showHospitalStatistics() {
    cout << "\n=============== HOSPITAL-WISE STATISTICS ===============\n";

    for (int h = 0; h < HOSPITAL_COUNT; h++) {
        int total = 0;
        int waiting = 0;
        int completed = 0;

        Node* temp = head;

        while (temp != nullptr) {
            Patient& p = temp->data;

            if (isToday(p) && p.hospital == h) {
                total++;

                if (p.status == WAITING) waiting++;
                if (p.status == COMPLETED) completed++;
            }

            temp = temp->next;
        }

        cout << "\n" << hospitals[h] << '\n';
        cout << "Registered: " << total
             << " | Waiting: " << waiting
             << " | Completed: " << completed << '\n';
    }
}

/* ======================== PATIENT HISTORY ======================== */

void showPatientHistory() {
    int token = readInt(
        "\nEnter token number (0 to show all history): ",
        0, 999999
    );

    Node* temp = head;
    int found = 0;

    cout << "\n=============== PATIENT HISTORY ===============\n";

    while (temp != nullptr) {
        if (token == 0 || temp->data.token == token) {
            showPatient(temp->data);
            found++;
        }

        temp = temp->next;
    }

    if (found == 0) {
        cout << "No matching records found.\n";
    }

    cout << "\nTotal records displayed: " << found << '\n';
}

/* ======================== QUEUE MANAGEMENT MENU ======================== */

void selectQueue() {
    showHospitals();

    int hospital = readInt("Select hospital: ",
                           1, HOSPITAL_COUNT) - 1;

    showDepartments();

    int department = readInt("Select department: ",
                             1, DEPARTMENT_COUNT) - 1;

    showDoctors(department);

    int doctor = readInt("Select doctor: ",
                         1, DOCTORS_PER_DEPT) - 1;

    int choice;

    do {
        cout << "\n========== QUEUE MANAGEMENT ==========\n";

        cout << "Hospital   : " << hospitals[hospital] << '\n';
        cout << "Department : " << departments[department] << '\n';
        cout << "Doctor     : " << doctors[department][doctor] << '\n';

        cout << "\n1. Show waiting queue\n";
        cout << "2. Call next patient\n";
        cout << "3. Complete consultation\n";
        cout << "4. Return to receptionist menu\n";

        choice = readInt("Enter choice: ", 1, 4);

        switch (choice) {
            case 1:
                showQueue(hospital, department, doctor);
                break;

            case 2:
                callNextPatient(hospital, department, doctor);
                break;

            case 3:
                completeConsultation(hospital, department, doctor);
                break;

            case 4:
                cout << "Returning to receptionist menu.\n";
                break;
        }

        if (choice != 4) {
            pauseScreen();
        }

    } while (choice != 4);
}

/* ======================== DEPARTMENT SUGGESTION ======================== */

void symptomSuggestion() {
    string symptom = readLine(
        "\nEnter general reason for visit: "
    );

    for (char& ch : symptom) {
        ch = static_cast<char>(
            tolower(static_cast<unsigned char>(ch))
        );
    }

    cout << "\nPossible department information:\n";

    if (symptom.find("ear") != string::npos ||
        symptom.find("nose") != string::npos ||
        symptom.find("throat") != string::npos) {

        cout << "ENT may be relevant.\n";

    } else if (symptom.find("bone") != string::npos ||
               symptom.find("joint") != string::npos ||
               symptom.find("fracture") != string::npos) {

        cout << "Orthopaedics may be relevant.\n";

    } else if (symptom.find("skin") != string::npos ||
               symptom.find("rash") != string::npos ||
               symptom.find("acne") != string::npos) {

        cout << "Dermatology may be relevant.\n";

    } else {
        cout << "General Medicine may be a starting point.\n";
    }

    cout << "\nThis is an information helper, not a medical diagnosis.\n";
    cout << "For urgent concerns, seek immediate medical assistance.\n";
}

/* ======================== PATIENT HELP ======================== */

void patientHelp() {
    cout << "\n=============== PATIENT HELP / FAQ ===============\n";

    cout << "\n1. How do I book a token?\n";
    cout << "   Select Book Token and enter your details.\n";

    cout << "\n2. How do I check my queue position?\n";
    cout << "   Select Check Token Status and enter your token.\n";

    cout << "\n3. Can I cancel a token?\n";
    cout << "   Contact reception for assistance.\n";

    cout << "\n4. Is the waiting time exact?\n";
    cout << "   No. It is an approximate estimate.\n";

    cout << "\n5. What if I have an urgent medical concern?\n";
    cout << "   Seek immediate medical assistance.\n";

    cout << "\n6. Do tokens renew every day?\n";
    cout << "   Yes. The token counter starts from 1 on a new date.\n";

    cout << "\n7. Are old records deleted?\n";
    cout << "   No. Previous records remain in patient history.\n";
}

void patientGuidelines() {
    cout << "\n=============== PATIENT GUIDELINES ===============\n";

    cout << "1. Enter your correct name and booking details.\n";
    cout << "2. Select the appropriate hospital and department.\n";
    cout << "3. Keep your token number safe.\n";
    cout << "4. Check token status before visiting the counter.\n";
    cout << "5. Waiting time is an estimate and may change.\n";
    cout << "6. Contact reception if you need help.\n";
    cout << "7. Do not wait for a token if you need urgent care.\n";
}

/* ======================== PATIENT PORTAL ======================== */

void patientMenu() {
    int choice;

    do {
        cout << "\n========================================\n";
        cout << "             PATIENT PORTAL\n";
        cout << "========================================\n";

        cout << "1. Book a token\n";
        cout << "2. Check token status\n";
        cout << "3. View token details\n";
        cout << "4. Department information helper\n";
        cout << "5. Patient help / FAQ\n";
        cout << "6. Patient guidelines\n";
        cout << "7. Back to main menu\n";

        choice = readInt("Enter choice: ", 1, 7);

        switch (choice) {
            case 1:
                registerPatient(false);
                break;

            case 2:
                showPatientStatus();
                break;

            case 3:
                showTokenDetails();
                break;

            case 4:
                symptomSuggestion();
                break;

            case 5:
                patientHelp();
                break;

            case 6:
                patientGuidelines();
                break;

            case 7:
                cout << "Returning to main menu.\n";
                break;
        }

        if (choice != 7) {
            pauseScreen();
        }

    } while (choice != 7);
}

/* ======================== RECEPTIONIST LOGIN ======================== */

bool receptionistLogin() {
    cout << "\n========== RECEPTIONIST LOGIN ==========\n";

    for (int attempt = 1; attempt <= 3; attempt++) {
        string pin = readLine("Enter receptionist PIN: ");

        if (verifyPIN(pin)) {
            cout << "\nLogin successful.\n";
            return true;
        }

        cout << "Incorrect PIN. Attempts remaining: "
             << 3 - attempt << '\n';
    }

    cout << "\nAccess denied.\n";
    return false;
}

/* ======================== RECEPTIONIST PORTAL ======================== */

void receptionistMenu() {
    if (!receptionistLogin()) {
        return;
    }

    int choice;

    do {
        cout << "\n========================================\n";
        cout << "          RECEPTIONIST PORTAL\n";
        cout << "========================================\n";

        cout << "1. Register walk-in patient\n";
        cout << "2. Register multiple walk-in patients\n";
        cout << "3. Manage doctor queues\n";
        cout << "4. Search token details\n";
        cout << "5. Cancel waiting token\n";
        cout << "6. Mark patient as emergency priority\n";
        cout << "7. View today's statistics\n";
        cout << "8. View hospital-wise statistics\n";
        cout << "9. View patient history\n";
        cout << "10. Change receptionist PIN\n";
        cout << "11. Back to main menu\n";

        choice = readInt("Enter choice: ", 1, 11);

        switch (choice) {
            case 1:
                registerPatient(true);
                break;

            case 2:
                multipleWalkInRegistration();
                break;

            case 3:
                selectQueue();
                break;

            case 4:
                showTokenDetails();
                break;

            case 5:
                cancelToken();
                break;

            case 6:
                markEmergencyPriority();
                break;

            case 7:
                showStatistics();
                break;

            case 8:
                showHospitalStatistics();
                break;

            case 9:
                showPatientHistory();
                break;

            case 10:
                changePIN();
                break;

            case 11:
                cout << "Logging out.\n";
                break;
        }

        if (choice != 11) {
            pauseScreen();
        }

    } while (choice != 11);
}

/* ======================== MEMORY CLEANUP ======================== */

void destroySystem() {
    Node* temp = head;

    while (temp != nullptr) {
        Node* current = temp;
        temp = temp->next;
        delete current;
    }

    head = nullptr;
}

/* ======================== MAIN FUNCTION ======================== */

int main() {
    loadData();

    cout << "\n===============================================\n";
    cout << "    HOSPITAL TOKEN BOOKING AND QUEUE SYSTEM\n";
    cout << "===============================================\n";

    cout << "\nWelcome to the Hospital Token Management System.\n";
    cout << "Today's date: " << getTodayDate() << '\n';
    cout << "Next available token: " << nextToken << '\n';

    int choice;

    do {
        cout << "\n=============== MAIN MENU ===============\n";

        cout << "1. Patient Portal\n";
        cout << "2. Receptionist Portal\n";
        cout << "3. Exit\n";

        choice = readInt("Enter your choice: ", 1, 3);

        switch (choice) {
            case 1:
                patientMenu();
                break;

            case 2:
                receptionistMenu();
                break;

            case 3:
                saveData();
                cout << "\nThank you for using the system.\n";
                cout << "Patient records have been saved.\n";
                break;
        }

    } while (choice != 3);

    destroySystem();

    return 0;
}
