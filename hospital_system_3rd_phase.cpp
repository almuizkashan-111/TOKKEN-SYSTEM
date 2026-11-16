/* ==================== CPP_Version/models.h ==================== */
#ifndef MODELS_H
#define MODELS_H

#include <string>

const int HOSPITAL_COUNT = 8;
const int DEPARTMENT_COUNT = 4;
const int MAX_TOKENS = 1000;
const int MAX_NAME_LENGTH = 50;
const int MAX_SYMPTOM_LENGTH = 200;

struct Patient {
    int token;
    std::string name;
    std::string source;
    std::string symptoms;
    int hospitalIndex;
    int departmentIndex;
    std::string bookingDate;
};

struct Hospital {
    std::string name;
    std::string type;
    bool available[DEPARTMENT_COUNT];
};

extern const std::string departmentNames[DEPARTMENT_COUNT];
extern const Hospital hospitals[HOSPITAL_COUNT];

#endif

/* ==================== CPP_Version/input_utils.h ==================== */
#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

inline std::string trimInput(const std::string& input) {
    std::size_t first = 0;

    while (first < input.size() &&
           std::isspace(static_cast<unsigned char>(input[first])))
        ++first;

    std::size_t last = input.size();

    while (last > first &&
           std::isspace(static_cast<unsigned char>(input[last - 1])))
        --last;

    return input.substr(first, last - first);
}

inline bool readLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, value));
}

inline bool hasControlCharacter(const std::string& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (std::iscntrl(static_cast<unsigned char>(value[i])))
            return true;
    }

    return false;
}

inline int readChoice(
    const std::string& prompt,
    int minimum,
    int maximum
) {
    std::string line;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, line))
            return -1;

        std::stringstream parser(line);
        int choice;
        char extra;

        if ((parser >> choice) && !(parser >> extra) &&
            choice >= minimum && choice <= maximum)
            return choice;

        std::cout << "Enter a whole number from " << minimum
                  << " to " << maximum << ".\n";
    }
}

inline bool readValidatedText(
    const std::string& prompt,
    std::string& value,
    std::size_t maximumLength
) {
    std::string input;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, input))
            return false;

        input = trimInput(input);

        if (input.empty()) {
            std::cout << "This field cannot be empty or spaces only.\n";
            continue;
        }

        if (input.size() > maximumLength) {
            std::cout << "Maximum length is "
                      << maximumLength << ".\n";
            continue;
        }

        if (hasControlCharacter(input)) {
            std::cout << "Control characters are not allowed.\n";
            continue;
        }

        value = input;
        return true;
    }
}

inline bool confirmAction(const std::string& message) {
    std::cout << "\n" << message
              << "\n1. Confirm\n0. Go back\n";

    return readChoice("Enter choice: ", 0, 1) == 1;
}

#endif

/* ==================== CPP_Version/selection.h ==================== */
#ifndef SELECTION_H
#define SELECTION_H

#include "hospital_system.h"
#include "input_utils.h"

#include <iostream>

inline int selectHospital() {
    std::cout << "\n========== SELECT HOSPITAL ==========\n";

    for (int i = 0; i < HOSPITAL_COUNT; ++i) {
        std::cout << i + 1 << ". " << hospitals[i].name
                  << " - " << hospitals[i].type << '\n';
    }

    std::cout << "0. Back\n";

    int choice = readChoice(
        "Select hospital: ", 0, HOSPITAL_COUNT
    );

    return choice <= 0 ? -1 : choice - 1;
}

inline int selectDepartment() {
    std::cout << "\n========== SELECT DEPARTMENT ==========\n";

    for (int i = 0; i < DEPARTMENT_COUNT; ++i)
        std::cout << i + 1 << ". "
                  << departmentNames[i] << '\n';

    std::cout << "0. Back\n";

    int choice = readChoice(
        "Select department: ", 0, DEPARTMENT_COUNT
    );

    return choice <= 0 ? -1 : choice - 1;
}

inline int selectAvailableDepartment(int hospitalIndex) {
    if (hospitalIndex < 0 || hospitalIndex >= HOSPITAL_COUNT)
        return -1;

    int mapping[DEPARTMENT_COUNT];
    int count = 0;

    std::cout << "\nDepartments available at "
              << hospitals[hospitalIndex].name << ":\n";

    for (int d = 0; d < DEPARTMENT_COUNT; ++d) {
        if (hospitals[hospitalIndex].available[d]) {
            mapping[count] = d;
            ++count;

            std::cout << count << ". "
                      << departmentNames[d] << '\n';
        }
    }

    if (count == 0)
        return -1;

    std::cout << "0. Back\n";

    int choice = readChoice(
        "Select department: ", 0, count
    );

    return choice <= 0 ? -1 : mapping[choice - 1];
}

inline int selectMatchingHospital(int departmentIndex) {
    if (departmentIndex < 0 ||
        departmentIndex >= DEPARTMENT_COUNT)
        return -1;

    int mapping[HOSPITAL_COUNT];
    int count = 0;

    std::cout << "\nHospitals offering "
              << departmentNames[departmentIndex] << ":\n";

    for (int h = 0; h < HOSPITAL_COUNT; ++h) {
        if (hospitals[h].available[departmentIndex]) {
            mapping[count] = h;
            ++count;

            std::cout << count << ". "
                      << hospitals[h].name << '\n';
        }
    }

    if (count == 0)
        return -1;

    std::cout << "0. Back\n";

    int choice = readChoice(
        "Select hospital: ", 0, count
    );

    return choice <= 0 ? -1 : mapping[choice - 1];
}

#endif

/* ==================== CPP_Version/queue.h ==================== */
#ifndef QUEUE_H
#define QUEUE_H

#include "models.h"

class PatientQueue {
private:
    struct Node {
        Patient data;
        Node* next;

        explicit Node(const Patient& patient)
            : data(patient), next(NULL) {}
    };

    Node* frontNode;
    Node* rearNode;
    int nodeCount;

public:
    PatientQueue();
    ~PatientQueue();

    PatientQueue(const PatientQueue&) = delete;
    PatientQueue& operator=(const PatientQueue&) = delete;

    bool isEmpty() const;
    int size() const;
    bool enqueue(const Patient& patient);
    bool dequeue(Patient& patient);
    bool peek(Patient& patient) const;
    bool find(int token, Patient& patient) const;
    int findNameToken(const std::string& patientName) const;
    int positionOf(int token) const;
    bool cancel(int token);
    void display() const;
    void clear();
};

#endif

/* ==================== CPP_Version/queue.cpp ==================== */
#include "queue.h"

#include <iostream>
#include <new>
#include <cctype>

using namespace std;

namespace {
string normalizeQueueName(const string& value) {
    string result;
    bool pendingSpace = false;

    for (size_t i = 0; i < value.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(value[i]);

        if (isspace(ch)) {
            if (!result.empty())
                pendingSpace = true;
        } else {
            if (pendingSpace)
                result += ' ';

            result += static_cast<char>(tolower(ch));
            pendingSpace = false;
        }
    }

    return result;
}
}

PatientQueue::PatientQueue()
    : frontNode(NULL), rearNode(NULL), nodeCount(0) {}

PatientQueue::~PatientQueue() {
    clear();
}

bool PatientQueue::isEmpty() const {
    return frontNode == NULL;
}

int PatientQueue::size() const {
    return nodeCount;
}

bool PatientQueue::enqueue(const Patient& patient) {
    Node* newNode = NULL;

    try {
        newNode = new Node(patient);
    } catch (const bad_alloc&) {
        return false;
    }

    if (isEmpty()) {
        frontNode = rearNode = newNode;
    } else {
        rearNode->next = newNode;
        rearNode = newNode;
    }

    ++nodeCount;
    return true;
}

bool PatientQueue::dequeue(Patient& patient) {
    if (isEmpty())
        return false;

    Node* temp = frontNode;
    patient = temp->data;
    frontNode = frontNode->next;

    if (frontNode == NULL)
        rearNode = NULL;

    delete temp;
    --nodeCount;

    return true;
}

bool PatientQueue::peek(Patient& patient) const {
    if (isEmpty())
        return false;

    patient = frontNode->data;
    return true;
}

bool PatientQueue::find(int token, Patient& patient) const {
    const Node* current = frontNode;

    while (current != NULL) {
        if (current->data.token == token) {
            patient = current->data;
            return true;
        }

        current = current->next;
    }

    return false;
}

int PatientQueue::findNameToken(const string& patientName) const {
    const string target = normalizeQueueName(patientName);
    const Node* current = frontNode;

    while (current != NULL) {
        if (normalizeQueueName(current->data.name) == target)
            return current->data.token;

        current = current->next;
    }

    return 0;
}

int PatientQueue::positionOf(int token) const {
    const Node* current = frontNode;
    int position = 1;

    while (current != NULL) {
        if (current->data.token == token)
            return position;

        current = current->next;
        ++position;
    }

    return -1;
}

bool PatientQueue::cancel(int token) {
    Node* current = frontNode;
    Node* previous = NULL;

    while (current != NULL) {
        if (current->data.token == token) {
            if (previous == NULL)
                frontNode = current->next;
            else
                previous->next = current->next;

            if (current == rearNode)
                rearNode = previous;

            delete current;
            --nodeCount;

            if (nodeCount == 0)
                frontNode = rearNode = NULL;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

void PatientQueue::display() const {
    if (isEmpty()) {
        cout << "No patients are waiting in this queue.\n";
        return;
    }

    cout << "\n--------------- WAITING QUEUE ---------------\n";

    const Node* current = frontNode;
    int position = 1;

    while (current != NULL) {
        cout << "Position: " << position
             << " | Token: " << current->data.token
             << " | Patient: " << current->data.name
             << " | Source: " << current->data.source << '\n';

        current = current->next;
        ++position;
    }
}

void PatientQueue::clear() {
    while (frontNode != NULL) {
        Node* temp = frontNode;
        frontNode = frontNode->next;
        delete temp;
    }

    rearNode = NULL;
    nodeCount = 0;
}

/* ==================== CPP_Version/hospital_system.h ==================== */
#ifndef HOSPITAL_SYSTEM_H
#define HOSPITAL_SYSTEM_H

#include "models.h"
#include "queue.h"

#include <string>
#include <vector>

enum class TokenStatus {
    WAITING,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

std::string statusText(TokenStatus status);

struct TokenRecord {
    Patient patient;
    TokenStatus status;
};

struct DepartmentState {
    PatientQueue waitingQueue;
    Patient currentPatient;
    bool consultationActive;
    int generatedCount;
    int completedCount;
    int cancelledCount;
    int averageConsultMinutes;
    int totalRating;
    int ratingCount;

    DepartmentState();
    DepartmentState(const DepartmentState&) = delete;
    DepartmentState& operator=(const DepartmentState&) = delete;
};

class HospitalSystem {
public:
    DepartmentState departments[HOSPITAL_COUNT][DEPARTMENT_COUNT];
    std::vector<TokenRecord> tokenHistory;
    int lastToken;
    std::string currentDate;
    std::string receptionistPin;

    HospitalSystem();
    HospitalSystem(const HospitalSystem&) = delete;
    HospitalSystem& operator=(const HospitalSystem&) = delete;

    bool isDepartmentAvailable(int hospitalIndex, int departmentIndex) const;
    int findPossibleDuplicate(int hospitalIndex, int departmentIndex,
                              const std::string& patientName) const;

    bool registerBooking(const std::string& name,
                         const std::string& source,
                         const std::string& symptoms,
                         int hospitalIndex,
                         int departmentIndex,
                         bool allowPossibleDuplicate,
                         int& createdToken,
                         std::string& errorMessage);

    int estimateWaitMinutes(int hospitalIndex, int departmentIndex,
                            int peopleAhead) const;

    const TokenRecord* findRecord(int token) const;
    TokenRecord* findRecord(int token);
    bool updateStatus(int token, TokenStatus newStatus);

    bool callNextPatient(int hospitalIndex, int departmentIndex,
                         std::string& errorMessage);
    bool completeConsultation(int hospitalIndex, int departmentIndex,
                              int rating, std::string& errorMessage);
    bool cancelWaitingToken(int hospitalIndex, int departmentIndex,
                            int token, std::string& errorMessage);
};

#endif

/* ==================== CPP_Version/hospital_system.cpp ==================== */
#include "hospital_system.h"

#include <new>
#include <cctype>
#include <ctime>
#include <fstream>

using namespace std;

namespace {
string localDateString() {
    std::time_t now = std::time(NULL);
    std::tm* local = std::localtime(&now);
    char buffer[11] = "0000-00-00";

    if (local != NULL)
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", local);

    return buffer;
}

string normalizeSystemName(const string& value) {
    string result;
    bool pendingSpace = false;

    for (size_t i = 0; i < value.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(value[i]);

        if (isspace(ch)) {
            if (!result.empty())
                pendingSpace = true;
        } else {
            if (pendingSpace)
                result += ' ';

            result += static_cast<char>(tolower(ch));
            pendingSpace = false;
        }
    }

    return result;
}
}

const string departmentNames[DEPARTMENT_COUNT] = {
    "General Medicine", "ENT", "Orthopaedics", "Dermatology"
};

const Hospital hospitals[HOSPITAL_COUNT] = {
    {"City General Hospital", "General / multispeciality hospital",
     {true, true, true, true}},
    {"Community Clinic", "General medicine clinic",
     {true, false, false, false}},
    {"Bone and Joint Hospital", "Orthopaedic hospital",
     {false, false, true, false}},
    {"Sunrise Multispeciality Hospital", "Multispeciality hospital",
     {true, true, true, true}},
    {"Green Valley Medical Centre", "General medicine and skin care",
     {true, false, false, true}},
    {"Metro ENT Hospital", "Ear, nose and throat care",
     {true, true, false, false}},
    {"Aarogya Skin and Wellness Centre", "Dermatology clinic",
     {true, false, false, true}},
    {"Northside Family Hospital", "Family and general care",
     {true, true, true, false}}
};

string statusText(TokenStatus status) {
    switch (status) {
        case TokenStatus::WAITING:
            return "Waiting";

        case TokenStatus::IN_CONSULTATION:
            return "In consultation";

        case TokenStatus::COMPLETED:
            return "Completed";

        case TokenStatus::CANCELLED:
            return "Cancelled";
    }

    return "Unknown";
}

DepartmentState::DepartmentState()
    : consultationActive(false),
      generatedCount(0),
      completedCount(0),
      cancelledCount(0),
      averageConsultMinutes(10),
      totalRating(0),
      ratingCount(0) {

    currentPatient.token = 0;
    currentPatient.hospitalIndex = -1;
    currentPatient.departmentIndex = -1;
}

HospitalSystem::HospitalSystem()
    : lastToken(0),
      currentDate(localDateString()),
      receptionistPin("2468") {

    tokenHistory.reserve(MAX_TOKENS);

    ifstream counterFile("daily_token_counter.txt");
    string savedDate;
    int savedCount = 0;

    if (counterFile >> savedDate >> savedCount &&
        savedDate == currentDate &&
        savedCount >= 0 &&
        savedCount <= MAX_TOKENS) {

        lastToken = savedCount;
    }
}

bool HospitalSystem::isDepartmentAvailable(int h, int d) const {
    return h >= 0 && h < HOSPITAL_COUNT &&
           d >= 0 && d < DEPARTMENT_COUNT &&
           hospitals[h].available[d];
}

int HospitalSystem::findPossibleDuplicate(
    int h,
    int d,
    const string& patientName
) const {

    if (!isDepartmentAvailable(h, d))
        return 0;

    const DepartmentState& state = departments[h][d];

    int token = state.waitingQueue.findNameToken(patientName);

    if (token != 0)
        return token;

    if (state.consultationActive) {
        if (normalizeSystemName(state.currentPatient.name) ==
            normalizeSystemName(patientName)) {

            return state.currentPatient.token;
        }
    }

    return 0;
}

bool HospitalSystem::registerBooking(
    const string& name,
    const string& source,
    const string& symptoms,
    int h,
    int d,
    bool allowPossibleDuplicate,
    int& createdToken,
    string& errorMessage
) {

    createdToken = 0;
    errorMessage.clear();

    if (!isDepartmentAvailable(h, d)) {
        errorMessage = "Selected hospital does not offer this department.";
        return false;
    }

    if (name.empty() || symptoms.empty() ||
        name.size() > MAX_NAME_LENGTH ||
        symptoms.size() > MAX_SYMPTOM_LENGTH) {

        errorMessage = "Booking details are invalid.";
        return false;
    }

    if (lastToken >= MAX_TOKENS) {
        errorMessage = "Daily token limit reached.";
        return false;
    }

    int duplicate = findPossibleDuplicate(h, d, name);

    if (duplicate != 0 && !allowPossibleDuplicate) {
        errorMessage = "Possible duplicate name found. Existing token: " +
                       to_string(duplicate);
        return false;
    }

    Patient patient;

    patient.token = lastToken + 1;
    patient.name = name;
    patient.source = source;
    patient.symptoms = symptoms;
    patient.hospitalIndex = h;
    patient.departmentIndex = d;
    patient.bookingDate = currentDate;

    TokenRecord record;
    record.patient = patient;
    record.status = TokenStatus::WAITING;

    try {
        tokenHistory.push_back(record);
    } catch (const bad_alloc&) {
        errorMessage = "Booking failed: unable to allocate token history.";
        return false;
    }

    if (!departments[h][d].waitingQueue.enqueue(patient)) {
        tokenHistory.pop_back();
        errorMessage = "Booking failed: unable to allocate queue node.";
        return false;
    }

    lastToken = patient.token;

    {
        ofstream counterFile(
            "daily_token_counter.txt",
            ios::trunc
        );

        if (counterFile)
            counterFile << currentDate << ' ' << lastToken << '\n';
    }

    departments[h][d].generatedCount++;
    createdToken = patient.token;

    return true;
}

int HospitalSystem::estimateWaitMinutes(
    int h,
    int d,
    int peopleAhead
) const {

    if (!isDepartmentAvailable(h, d) || peopleAhead < 0)
        return 0;

    const DepartmentState& state = departments[h][d];
    int active = state.consultationActive ? 1 : 0;

    return (peopleAhead + active) * state.averageConsultMinutes;
}

const TokenRecord* HospitalSystem::findRecord(int token) const {
    for (size_t i = 0; i < tokenHistory.size(); ++i) {
        if (tokenHistory[i].patient.token == token)
            return &tokenHistory[i];
    }

    return NULL;
}

TokenRecord* HospitalSystem::findRecord(int token) {
    for (size_t i = 0; i < tokenHistory.size(); ++i) {
        if (tokenHistory[i].patient.token == token)
            return &tokenHistory[i];
    }

    return NULL;
}

bool HospitalSystem::updateStatus(
    int token,
    TokenStatus newStatus
) {

    TokenRecord* record = findRecord(token);

    if (record == NULL)
        return false;

    record->status = newStatus;
    return true;
}

bool HospitalSystem::callNextPatient(
    int h,
    int d,
    string& errorMessage
) {

    errorMessage.clear();

    if (!isDepartmentAvailable(h, d)) {
        errorMessage = "Invalid hospital or department.";
        return false;
    }

    DepartmentState& state = departments[h][d];

    if (state.consultationActive) {
        errorMessage = "Complete the active consultation first.";
        return false;
    }

    Patient preview;

    if (!state.waitingQueue.peek(preview)) {
        errorMessage = "No patients are waiting. Queue underflow prevented.";
        return false;
    }

    const TokenRecord* previewRecord = findRecord(preview.token);

    if (previewRecord == NULL ||
        previewRecord->status != TokenStatus::WAITING) {

        errorMessage = "Queue and token history are inconsistent; call stopped.";
        return false;
    }

    Patient patient;

    if (!state.waitingQueue.dequeue(patient)) {
        errorMessage = "No patients are waiting. Queue underflow prevented.";
        return false;
    }

    state.currentPatient = patient;
    state.consultationActive = true;

    updateStatus(patient.token, TokenStatus::IN_CONSULTATION);

    return true;
}

bool HospitalSystem::completeConsultation(
    int h,
    int d,
    int rating,
    string& errorMessage
) {

    errorMessage.clear();

    if (!isDepartmentAvailable(h, d)) {
        errorMessage = "Invalid hospital or department.";
        return false;
    }

    DepartmentState& state = departments[h][d];

    if (!state.consultationActive) {
        errorMessage = "There is no active consultation.";
        return false;
    }

    if (rating < 0 || rating > 5) {
        errorMessage = "Rating must be between 0 and 5.";
        return false;
    }

    int token = state.currentPatient.token;

    if (!updateStatus(token, TokenStatus::COMPLETED)) {
        errorMessage = "Token history record is missing; completion stopped.";
        return false;
    }

    state.completedCount++;

    if (rating >= 1) {
        state.totalRating += rating;
        state.ratingCount++;
    }

    state.consultationActive = false;
    state.currentPatient = Patient();

    return true;
}

bool HospitalSystem::cancelWaitingToken(
    int h,
    int d,
    int token,
    string& errorMessage
) {

    errorMessage.clear();

    if (!isDepartmentAvailable(h, d)) {
        errorMessage = "Invalid hospital or department.";
        return false;
    }

    TokenRecord* record = findRecord(token);

    if (record == NULL) {
        errorMessage = "Token not found in session history.";
        return false;
    }

    if (record->status != TokenStatus::WAITING) {
        errorMessage = "Only waiting tokens can be cancelled.";
        return false;
    }

    if (record->patient.hospitalIndex != h ||
        record->patient.departmentIndex != d) {

        errorMessage = "Token belongs to another queue.";
        return false;
    }

    if (!departments[h][d].waitingQueue.cancel(token)) {
        errorMessage = "Token was not found in the waiting queue.";
        return false;
    }

    record->status = TokenStatus::CANCELLED;
    departments[h][d].cancelledCount++;

    return true;
}

/* ==================== CPP_Version/patient_portal.h ==================== */
#ifndef PATIENT_PORTAL_H
#define PATIENT_PORTAL_H

#include "hospital_system.h"

void patientMenu(HospitalSystem& system);

#endif

/* ==================== CPP_Version/patient_portal.cpp ==================== */
#include "patient_portal.h"
#include "input_utils.h"
#include "selection.h"

#include <iostream>

using namespace std;

namespace {

int selectBookingLocation(
    int& hospitalIndex,
    int& departmentIndex
) {

    cout << "\n========== BOOKING OPTIONS ==========\n";
    cout << "1. Choose hospital first\n";
    cout << "2. Choose department first\n";
    cout << "3. Symptom-category suggestion\n";
    cout << "0. Back\n";

    int choice = readChoice("Enter choice: ", 0, 3);

    if (choice <= 0)
        return 0;

    if (choice == 1) {
        int h = selectHospital();

        if (h < 0)
            return 0;

        int d = selectAvailableDepartment(h);

        if (d < 0)
            return 0;

        hospitalIndex = h;
        departmentIndex = d;
    } else {
        int d;

        if (choice == 2) {
            d = selectDepartment();
        } else {
            cout << "\n========== SYMPTOM CATEGORY DEMO ==========\n";
            cout << "This is a predefined category suggestion, not a diagnosis.\n";
            cout << "1. Fever, cough or cold\n";
            cout << "2. Ear pain or blocked nose\n";
            cout << "3. Joint or bone pain\n";
            cout << "4. Skin rash or itching\n";
            cout << "5. Other / Not sure (choose manually)\n";
            cout << "0. Back\n";

            int symptomChoice =
                readChoice("Select category: ", 0, 5);

            if (symptomChoice <= 0)
                return 0;

            if (symptomChoice == 5) {
                d = selectDepartment();
            } else {
                d = symptomChoice - 1;

                cout << "Suggested category: "
                     << departmentNames[d] << '\n';

                cout << "1. Use this category\n";
                cout << "2. Choose manually\n";
                cout << "0. Back\n";

                int option = readChoice("Enter choice: ", 0, 2);

                if (option == 0)
                    return 0;

                if (option == 2)
                    d = selectDepartment();
            }
        }

        if (d < 0)
            return 0;

        int h = selectMatchingHospital(d);

        if (h < 0)
            return 0;

        hospitalIndex = h;
        departmentIndex = d;
    }

    return 1;
}

void bookSelf(HospitalSystem& system) {
    int h = -1;
    int d = -1;

    if (!selectBookingLocation(h, d))
        return;

    string name, symptoms;

    if (!readValidatedText(
            "Enter patient name: ",
            name,
            MAX_NAME_LENGTH))
        return;

    if (!readValidatedText(
            "Enter symptoms: ",
            symptoms,
            MAX_SYMPTOM_LENGTH))
        return;

    cout << "\n========== BOOKING PREVIEW ==========\n";
    cout << "Patient: " << name << '\n';
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';
    cout << "Symptoms: " << symptoms << '\n';
    cout << "Booking source: Self-booking\n";
    cout << "Booking date: " << system.currentDate << '\n';

    if (!confirmAction("Confirm and submit this booking?")) {
        cout << "Booking not submitted.\n";
        return;
    }

    bool allowDuplicate = false;

    int possibleDuplicate =
        system.findPossibleDuplicate(h, d, name);

    if (possibleDuplicate != 0) {
        cout << "WARNING: A similar name already has token "
             << possibleDuplicate << " in this queue.\n";

        cout << "Names alone cannot verify patient identity.\n";

        if (!confirmAction("Continue with another booking anyway?"))
            return;

        allowDuplicate = true;
    }

    int token = 0;
    string error;

    if (!system.registerBooking(
            name,
            "Self-booking",
            symptoms,
            h,
            d,
            allowDuplicate,
            token,
            error)) {

        cout << "Booking failed: " << error << '\n';
        return;
    }

    const DepartmentState& state = system.departments[h][d];
    int position = state.waitingQueue.size();

    cout << "\n========== DIGITAL TOKEN SLIP ==========\n";
    cout << "Booking date: " << system.currentDate << '\n';
    cout << "Token number: " << token << '\n';
    cout << "Queue position: " << position << '\n';

    cout << "Estimated wait: "
         << system.estimateWaitMinutes(h, d, position - 1)
         << " minutes (approx.)\n";

    cout << "Keep your token number for tracking.\n";
}

void showPatientTokenStatus(
    const HospitalSystem& system,
    int token
) {

    const TokenRecord* record = system.findRecord(token);

    if (record == NULL) {
        cout << "Token not found. Check the number or contact reception.\n";
        return;
    }

    cout << "\n========== TOKEN STATUS ==========\n";
    cout << "Booking date: " << record->patient.bookingDate << '\n';
    cout << "Token: " << token << '\n';
    cout << "Status: " << statusText(record->status) << '\n';

    cout << "Hospital: "
         << hospitals[record->patient.hospitalIndex].name << '\n';

    cout << "Department: "
         << departmentNames[record->patient.departmentIndex] << '\n';

    if (record->status == TokenStatus::WAITING) {
        const DepartmentState& state =
            system.departments[record->patient.hospitalIndex]
                              [record->patient.departmentIndex];

        int position = state.waitingQueue.positionOf(token);

        if (position > 0) {
            cout << "Queue position: " << position << '\n';

            cout << "Estimated wait: "
                 << system.estimateWaitMinutes(
                        record->patient.hospitalIndex,
                        record->patient.departmentIndex,
                        position - 1)
                 << " minutes (approx.)\n";
        }
    }

    cout << "For privacy, this portal does not display patient names or symptoms.\n";
}

void viewQueueStatus(const HospitalSystem& system) {
    int h = selectHospital();

    if (h < 0)
        return;

    int d = selectAvailableDepartment(h);

    if (d < 0)
        return;

    const DepartmentState& state = system.departments[h][d];

    cout << "\n========== DAILY QUEUE STATUS ==========\n";
    cout << "Date: " << system.currentDate << '\n';
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';

    cout << "Patients waiting: "
         << state.waitingQueue.size() << '\n';

    cout << "Consultation active: "
         << (state.consultationActive ? "Yes" : "No") << '\n';

    cout << "Queue load: ";

    if (state.waitingQueue.size() <= 3)
        cout << "LOW\n";
    else if (state.waitingQueue.size() <= 7)
        cout << "MODERATE\n";
    else
        cout << "HIGH\n";

    cout << "Waiting-time estimates are approximate.\n";
}

}

void patientMenu(HospitalSystem& system) {
    while (true) {
        cout << "\n========================================\n";
        cout << "             PATIENT PORTAL\n";
        cout << "========================================\n";
        cout << "Booking date: " << system.currentDate << '\n';
        cout << "1. Book a token\n";
        cout << "2. Track token status\n";
        cout << "3. View queue status\n";
        cout << "4. Help and guidelines\n";
        cout << "0. Back\n";

        int choice = readChoice("Enter choice: ", 0, 4);

        if (choice <= 0)
            return;

        switch (choice) {
            case 1:
                bookSelf(system);
                break;

            case 2: {
                int token = readChoice(
                    "Enter token number: ", 1, MAX_TOKENS
                );

                if (token < 0)
                    return;

                showPatientTokenStatus(system, token);
                break;
            }

            case 3:
                viewQueueStatus(system);
                break;

            case 4:
                cout << "\n1. Choose a hospital and available department.\n";
                cout << "2. Enter your name and symptom description.\n";
                cout << "3. Review and confirm the booking.\n";
                cout << "4. Keep the token and use it to track status.\n";
                cout << "5. Contact reception for cancellation.\n";
                cout << "Symptom suggestions are not diagnoses or emergency triage.\n";
                cout << "Records are temporary and reset when the program closes.\n";
                break;
        }
    }
}

/* ==================== CPP_Version/receptionist_portal.h ==================== */
#ifndef RECEPTIONIST_PORTAL_H
#define RECEPTIONIST_PORTAL_H

#include "hospital_system.h"

void receptionistMenu(HospitalSystem& system);

#endif

/* ==================== CPP_Version/receptionist_portal.cpp ==================== */
#include "receptionist_portal.h"
#include "input_utils.h"
#include "selection.h"

#include <iostream>
#include <cctype>
#include <iomanip>

using namespace std;

namespace {

bool receptionistLogin(HospitalSystem& system) {
    cout << "\n========== RECEPTIONIST LOGIN ==========\n";

    for (int attempt = 1; attempt <= 3; ++attempt) {
        string pin;

        if (!readLine("Enter receptionist PIN: ", pin))
            return false;

        if (pin == system.receptionistPin) {
            cout << "Login successful.\n";
            return true;
        }

        cout << "Incorrect PIN. Attempts remaining: "
             << 3 - attempt << '\n';
    }

    cout << "Too many incorrect attempts. Returning to main menu.\n";
    return false;
}

bool selectReceptionQueue(int& h, int& d) {
    int selectedHospital = selectHospital();

    if (selectedHospital < 0)
        return false;

    int selectedDepartment =
        selectAvailableDepartment(selectedHospital);

    if (selectedDepartment < 0)
        return false;

    h = selectedHospital;
    d = selectedDepartment;

    return true;
}

void printReceptionistTokenDetails(
    const HospitalSystem& system,
    int token
) {

    const TokenRecord* record = system.findRecord(token);

    if (record == NULL) {
        cout << "Token not found in this session.\n";
        return;
    }

    const Patient& p = record->patient;

    cout << "\n========== TOKEN DETAILS ==========\n";
    cout << "Booking date: " << p.bookingDate << '\n';
    cout << "Token: " << p.token << '\n';
    cout << "Patient: " << p.name << '\n';
    cout << "Source: " << p.source << '\n';
    cout << "Hospital: " << hospitals[p.hospitalIndex].name << '\n';
    cout << "Department: " << departmentNames[p.departmentIndex] << '\n';
    cout << "Symptoms: " << p.symptoms << '\n';
    cout << "Status: " << statusText(record->status) << '\n';

    if (record->status == TokenStatus::WAITING) {
        const DepartmentState& state =
            system.departments[p.hospitalIndex][p.departmentIndex];

        int position = state.waitingQueue.positionOf(token);

        if (position > 0) {
            cout << "Queue position: " << position << '\n';

            cout << "Estimated wait: "
                 << system.estimateWaitMinutes(
                        p.hospitalIndex,
                        p.departmentIndex,
                        position - 1)
                 << " minutes (approx.)\n";
        }
    }
}

void showDepartmentStatistics(
    const HospitalSystem& system,
    int h,
    int d
) {

    const DepartmentState& state = system.departments[h][d];

    cout << "\n========== DAILY DEPARTMENT STATISTICS ==========\n";
    cout << "Date: " << system.currentDate << '\n';
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';

    cout << "Generated: " << state.generatedCount << '\n';
    cout << "Waiting: " << state.waitingQueue.size() << '\n';
    cout << "Active consultation: "
         << (state.consultationActive ? 1 : 0) << '\n';
    cout << "Completed: " << state.completedCount << '\n';
    cout << "Cancelled: " << state.cancelledCount << '\n';

    if (state.ratingCount > 0) {
        cout << fixed << setprecision(2)
             << "Average feedback: "
             << static_cast<double>(state.totalRating) /
                state.ratingCount
             << "/5 (" << state.ratingCount << " responses)\n";

        cout.unsetf(ios::floatfield);
    } else {
        cout << "Feedback: No ratings yet\n";
    }
}

void showOverallStatistics(const HospitalSystem& system) {
    int waiting = 0;
    int active = 0;
    int completed = 0;
    int cancelled = 0;

    for (int h = 0; h < HOSPITAL_COUNT; ++h) {
        for (int d = 0; d < DEPARTMENT_COUNT; ++d) {
            const DepartmentState& state = system.departments[h][d];

            waiting += state.waitingQueue.size();
            active += state.consultationActive ? 1 : 0;
            completed += state.completedCount;
            cancelled += state.cancelledCount;
        }
    }

    cout << "\n========== OVERALL DAILY STATISTICS ==========\n";
    cout << "Date: " << system.currentDate << '\n';
    cout << "Tokens generated: " << system.lastToken << '\n';
    cout << "Waiting: " << waiting << '\n';
    cout << "Active: " << active << '\n';
    cout << "Completed: " << completed << '\n';
    cout << "Cancelled: " << cancelled << '\n';
}

void showTokenHistory(const HospitalSystem& system) {
    cout << "\n========== COMPLETE TOKEN HISTORY ==========\n";

    if (system.tokenHistory.empty()) {
        cout << "No token records generated.\n";
        return;
    }

    for (size_t i = 0; i < system.tokenHistory.size(); ++i) {
        const TokenRecord& record = system.tokenHistory[i];

        cout << "Date: " << record.patient.bookingDate
             << " | Token: " << record.patient.token
             << " | Name: " << record.patient.name
             << " | Department: "
             << departmentNames[record.patient.departmentIndex]
             << " | Status: " << statusText(record.status) << '\n';
    }
}

void changeReceptionistPin(HospitalSystem& system) {
    string oldPin, newPin, confirmPin;

    if (!readLine("Enter current PIN: ", oldPin))
        return;

    if (oldPin != system.receptionistPin) {
        cout << "Current PIN is incorrect.\n";
        return;
    }

    if (!readValidatedText(
            "Enter new PIN (4-12 digits): ",
            newPin,
            12))
        return;

    if (newPin.size() < 4) {
        cout << "PIN must contain at least 4 digits.\n";
        return;
    }

    for (size_t i = 0; i < newPin.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(newPin[i]))) {
            cout << "PIN must contain digits only.\n";
            return;
        }
    }

    if (!readLine("Confirm new PIN: ", confirmPin))
        return;

    if (newPin != confirmPin) {
        cout << "PIN confirmation does not match.\n";
        return;
    }

    system.receptionistPin = newPin;
    cout << "PIN changed for this session. It resets on restart.\n";
}

void configureAverageTime(
    HospitalSystem& system,
    int h,
    int d
) {

    DepartmentState& state = system.departments[h][d];

    cout << "Current setting: "
         << state.averageConsultMinutes << " minutes\n";

    int value = readChoice(
        "New average duration (1-60): ", 1, 60
    );

    if (value < 0)
        return;

    state.averageConsultMinutes = value;

    cout << "Configured estimate updated. It is not measured automatically.\n";
}

void showQueueInsights(
    const HospitalSystem& system,
    int h,
    int d
) {

    const DepartmentState& state = system.departments[h][d];
    int waiting = state.waitingQueue.size();

    cout << "\n========== DAILY QUEUE INSIGHTS ==========\n";
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';
    cout << "Waiting patients: " << waiting << '\n';

    cout << "Queue load: ";

    if (waiting <= 3)
        cout << "LOW\n";
    else if (waiting <= 7)
        cout << "MODERATE\n";
    else
        cout << "HIGH\n";

    cout << "Estimated wait for a new patient: "
         << system.estimateWaitMinutes(h, d, waiting)
         << " minutes\n";

    cout << "Configured consultation duration: "
         << state.averageConsultMinutes << " minutes\n";

    cout << "Assumption: one consultation at a time in this queue.\n";
    cout << "This is a rough mathematical estimate, not AI.\n";
}

}

void receptionistMenu(HospitalSystem& system) {
    if (!receptionistLogin(system))
        return;

    int h = -1;
    int d = -1;

    if (!selectReceptionQueue(h, d))
        return;

    while (true) {
        cout << "\n========================================\n";
        cout << "           RECEPTIONIST PORTAL\n";
        cout << "========================================\n";

        cout << "Booking date: " << system.currentDate << '\n';
        cout << "Hospital: " << hospitals[h].name << '\n';
        cout << "Department: " << departmentNames[d] << '\n';

        cout << "1. Register walk-in patient\n";
        cout << "2. Register multiple walk-in patients\n";
        cout << "3. Display waiting queue\n";
        cout << "4. Call next patient\n";
        cout << "5. Complete consultation\n";
        cout << "6. Search token details\n";
        cout << "7. Cancel waiting token\n";
        cout << "8. Department statistics\n";
        cout << "9. Shift summary\n";
        cout << "10. All-hospital statistics\n";
        cout << "11. Change hospital / department\n";
        cout << "12. Configure average consultation time\n";
        cout << "13. Smart queue insights\n";
        cout << "14. Complete token history\n";
        cout << "15. Change receptionist PIN\n";
        cout << "0. Logout\n";

        int choice = readChoice("Enter choice: ", 0, 15);

        if (choice <= 0)
            return;

        switch (choice) {
            case 1: {
                string name, symptoms;

                if (!readValidatedText(
                        "Enter patient name: ",
                        name,
                        MAX_NAME_LENGTH))
                    return;

                if (!readValidatedText(
                        "Enter symptoms: ",
                        symptoms,
                        MAX_SYMPTOM_LENGTH))
                    return;

                if (!confirmAction("Register this walk-in patient?"))
                    break;

                bool allowDuplicate = false;

                int duplicate =
                    system.findPossibleDuplicate(h, d, name);

                if (duplicate != 0) {
                    cout << "Similar name already has token "
                         << duplicate << ".\n";

                    if (!confirmAction("Continue anyway?"))
                        break;

                    allowDuplicate = true;
                }

                int token = 0;
                string error;

                if (system.registerBooking(
                        name,
                        "Walk-in",
                        symptoms,
                        h,
                        d,
                        allowDuplicate,
                        token,
                        error)) {

                    cout << "Walk-in registered. Token: "
                         << token << '\n';

                    const DepartmentState& state =
                        system.departments[h][d];

                    cout << "Estimated wait: "
                         << system.estimateWaitMinutes(
                                h, d,
                                state.waitingQueue.size() - 1)
                         << " minutes (approx.)\n";
                } else {
                    cout << "Registration failed: "
                         << error << '\n';
                }

                break;
            }

            case 2: {
                while (true) {
                    string name, symptoms;

                    if (!readValidatedText(
                            "Enter patient name: ",
                            name,
                            MAX_NAME_LENGTH))
                        return;

                    if (!readValidatedText(
                            "Enter symptoms: ",
                            symptoms,
                            MAX_SYMPTOM_LENGTH))
                        return;

                    if (confirmAction("Register this walk-in patient?")) {
                        bool allowDuplicate = false;

                        int duplicate =
                            system.findPossibleDuplicate(h, d, name);

                        if (duplicate != 0) {
                            cout << "Similar name already has token "
                                 << duplicate << ".\n";

                            allowDuplicate =
                                confirmAction("Continue anyway?");
                        }

                        if (duplicate == 0 || allowDuplicate) {
                            int token = 0;
                            string error;

                            if (system.registerBooking(
                                    name,
                                    "Walk-in",
                                    symptoms,
                                    h,
                                    d,
                                    allowDuplicate,
                                    token,
                                    error)) {

                                cout << "Registered. Token: "
                                     << token << '\n';
                            } else {
                                cout << "Registration failed: "
                                     << error << '\n';
                            }
                        }
                    }

                    int again = readChoice(
                        "1. Register another\n0. Return\nChoice: ",
                        0,
                        1
                    );

                    if (again <= 0)
                        break;
                }

                break;
            }

            case 3:
                system.departments[h][d].waitingQueue.display();
                break;

            case 4: {
                if (!confirmAction("Call the next patient?"))
                    break;

                string error;

                if (system.callNextPatient(h, d, error)) {
                    const Patient& p =
                        system.departments[h][d].currentPatient;

                    cout << "Patient called. Token: "
                         << p.token
                         << " | Name: " << p.name << '\n';
                } else {
                    cout << error << '\n';
                }

                break;
            }

            case 5: {
                if (!system.departments[h][d].consultationActive) {
                    cout << "No active consultation.\n";
                    break;
                }

                cout << "Completing token: "
                     << system.departments[h][d].currentPatient.token
                     << '\n';

                int rating = readChoice(
                    "Optional rating (0 to skip, 1-5): ",
                    0,
                    5
                );

                if (rating < 0)
                    return;

                if (!confirmAction("Confirm consultation completion?"))
                    break;

                string error;

                if (system.completeConsultation(h, d, rating, error))
                    cout << "Consultation completed and history updated.\n";
                else
                    cout << error << '\n';

                break;
            }

            case 6: {
                int token = readChoice(
                    "Enter token number: ", 1, MAX_TOKENS
                );

                if (token < 0)
                    return;

                printReceptionistTokenDetails(system, token);
                break;
            }

            case 7: {
                int token = readChoice(
                    "Enter token number to cancel: ",
                    1,
                    MAX_TOKENS
                );

                if (token < 0)
                    return;

                printReceptionistTokenDetails(system, token);

                if (!confirmAction("Confirm cancellation?"))
                    break;

                string error;

                if (system.cancelWaitingToken(h, d, token, error))
                    cout << "Token cancelled. History retained.\n";
                else
                    cout << "Cancellation failed: " << error << '\n';

                break;
            }

            case 8:
                showDepartmentStatistics(system, h, d);
                break;

            case 9:
                showDepartmentStatistics(system, h, d);
                showQueueInsights(system, h, d);
                system.departments[h][d].waitingQueue.display();
                break;

            case 10:
                showOverallStatistics(system);
                break;

            case 11: {
                int newH, newD;

                if (selectReceptionQueue(newH, newD)) {
                    h = newH;
                    d = newD;
                }

                break;
            }

            case 12:
                configureAverageTime(system, h, d);
                break;

            case 13:
                showQueueInsights(system, h, d);
                break;

            case 14:
                showTokenHistory(system);
                break;

            case 15:
                changeReceptionistPin(system);
                break;
        }
    }
}

/* ==================== CPP_Version/main.cpp ==================== */
#include "hospital_system.h"
#include "patient_portal.h"
#include "receptionist_portal.h"
#include "input_utils.h"

#include <iostream>
#include <new>
#include <exception>

using namespace std;

void showProjectScope() {
    cout << "\n========== PROJECT SCOPE AND LIMITATIONS ==========\n";

    cout << "IMPLEMENTED IN THIS CONSOLE PROTOTYPE:\n";
    cout << "1. Patient self-booking and receptionist walk-in registration.\n";
    cout << "2. Separate linked-list FIFO queues for hospital departments.\n";
    cout << "3. Daily token sequence and date-stamped token history.\n";
    cout << "4. Waiting, in-consultation, completed and cancelled statuses.\n";
    cout << "5. Token search, queue cancellation and queue statistics.\n";
    cout << "6. Configurable waiting-time estimate and queue load indicator.\n";
    cout << "7. Basic receptionist PIN gate and optional patient feedback.\n";

    cout << "\nLIMITATIONS:\n";
    cout << "1. Sample hospital records only; no live hospital integration.\n";
    cout << "2. Data is held in RAM and resets when the program closes.\n";
    cout << "3. Demo PIN is stored in source code; not production authentication.\n";
    cout << "4. No database, SMS, email, multi-user networking or mobile app.\n";
    cout << "5. Symptom suggestions are not diagnosis or emergency triage.\n";
    cout << "6. Waiting-time estimate uses a configured assumption, not live measurements.\n";
    cout << "7. Duplicate-name checking cannot verify a person's identity.\n";

    cout << "\nFUTURE UPGRADES:\n";
    cout << "Database persistence, secure user accounts, audit logs, private tracking codes,\n";
    cout << "multi-doctor scheduling, measured consultation times and approved hospital APIs.\n";
}

int main() {
    try {
        HospitalSystem system;

        cout << "\n============================================\n";
        cout << "     HOSPITAL TOKEN BOOKING SYSTEM\n";
        cout << "     Booking date: " << system.currentDate << "\n";
        cout << "     Daily token sequence starts at 1\n";
        cout << "============================================\n";

        while (true) {
            cout << "\n================ MAIN MENU ================\n";
            cout << "1. Patient portal\n";
            cout << "2. Receptionist portal\n";
            cout << "3. Project scope and limitations\n";
            cout << "0. Exit\n";

            int choice = readChoice("Enter choice: ", 0, 3);

            if (choice <= 0)
                break;

            if (choice == 1)
                patientMenu(system);
            else if (choice == 2)
                receptionistMenu(system);
            else if (choice == 3)
                showProjectScope();
        }

        cout << "\nApplication closed. Session data will be cleared.\n";
        return 0;
    }
    catch (const bad_alloc&) {
        cerr << "Fatal error: memory allocation failed.\n";
        return 1;
    }
    catch (const exception& error) {
        cerr << "Unexpected error: " << error.what() << '\n';
        return 1;
    }
}