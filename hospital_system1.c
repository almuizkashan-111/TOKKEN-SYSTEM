
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define HOSPITAL_COUNT 8
#define DEPARTMENT_COUNT 4
#define DOCTORS_PER_DEPT 2

#define NAME_SIZE 60
#define PHONE_SIZE 20
#define SYMPTOM_SIZE 200
#define DATE_SIZE 20
#define INPUT_SIZE 256

#define DATA_FILE "patients_c.dat"
#define PIN_FILE "receptionist_pin_c.dat"

#define SERVICE_MINUTES 15

/* =====================================================
   HOSPITAL TOKEN BOOKING AND QUEUE MANAGEMENT SYSTEM
   Complete Single-File C Project
   C99 Compatible
   ===================================================== */

/* ==================== PATIENT STATUS ==================== */

enum {
    WAITING = 1,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

/* ==================== HOSPITAL DATA ==================== */

const char hospitals[HOSPITAL_COUNT][NAME_SIZE] = {
    "City General Hospital",
    "Community Clinic",
    "Bone and Joint Hospital",
    "Sunrise Multispeciality Hospital",
    "Green Valley Medical Centre",
    "Metro ENT Hospital",
    "Aarogya Skin and Wellness Centre",
    "Northside Family Hospital"
};

const char departments[DEPARTMENT_COUNT][NAME_SIZE] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

const char doctors[DEPARTMENT_COUNT][DOCTORS_PER_DEPT][NAME_SIZE] = {
    {"Dr. Amit Sharma", "Dr. Neha Verma"},
    {"Dr. Rahul Mehta", "Dr. Priya Singh"},
    {"Dr. Arjun Kapoor", "Dr. Simran Kaur"},
    {"Dr. Rohan Gupta", "Dr. Ananya Joshi"}
};

/* ==================== PATIENT STRUCTURE ==================== */

typedef struct Patient {
    int token;
    long long order;

    char name[NAME_SIZE];
    char phone[PHONE_SIZE];
    char hospital[NAME_SIZE];
    char department[NAME_SIZE];
    char doctor[NAME_SIZE];
    char symptoms[SYMPTOM_SIZE];
    char bookingDate[DATE_SIZE];

    int status;
    int emergency;

    struct Patient *next;
} Patient;

/* ==================== GLOBAL VARIABLES ==================== */

Patient *head = NULL;

int nextToken = 1;
long long nextOrder = 1;

unsigned long long receptionistPIN = 0;

/* =====================================================
   INPUT HANDLING
   ===================================================== */

void readLine(const char *message, char *buffer, int size)
{
    int ch;

    printf("%s", message);

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    if (strchr(buffer, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    buffer[strcspn(buffer, "\n")] = '\0';
}

int readInt(const char *message)
{
    char input[INPUT_SIZE];
    char extra;
    int value;

    while (1) {
        readLine(message, input, sizeof(input));

        if (sscanf(input, "%d %c", &value, &extra) == 1)
            return value;

        printf("Invalid input! Enter a valid number.\n");
    }
}

void pauseScreen(void)
{
    char temp[10];

    printf("\nPress Enter to continue...");
    fgets(temp, sizeof(temp), stdin);
}

void copyText(char *destination, const char *source, int size)
{
    snprintf(destination, size, "%s", source);
}

void sanitizeText(char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '|')
            text[i] = '/';
    }
}

/* =====================================================
   DATE AND TIME
   ===================================================== */

void getTodayDate(char *date)
{
    time_t now = time(NULL);
    struct tm *current = localtime(&now);

    strftime(date, DATE_SIZE, "%Y-%m-%d", current);
}

void getCurrentTime(char *buffer)
{
    time_t now = time(NULL);
    struct tm *current = localtime(&now);

    strftime(buffer, 20, "%H:%M:%S", current);
}

void showDate(void)
{
    char date[DATE_SIZE];

    getTodayDate(date);

    printf("Date: %s\n", date);
}

/* =====================================================
   VALIDATION
   ===================================================== */

int isValidPhone(const char *phone)
{
    int i;
    int digits = 0;

    for (i = 0; phone[i] != '\0'; i++) {
        if (isdigit((unsigned char)phone[i])) {
            digits++;
        }
        else if (phone[i] == '+' && i == 0) {
            continue;
        }
        else {
            return 0;
        }
    }

    return digits >= 10 && digits <= 15;
}

/* =====================================================
   LINKED LIST OPERATIONS
   ===================================================== */

void addPatient(Patient *newPatient)
{
    Patient *temp;

    newPatient->next = NULL;

    if (head == NULL) {
        head = newPatient;
        return;
    }

    temp = head;

    while (temp->next != NULL)
        temp = temp->next;

    temp->next = newPatient;
}

Patient *findToken(int token)
{
    char today[DATE_SIZE];
    Patient *temp = head;

    getTodayDate(today);

    while (temp != NULL) {
        if (temp->token == token &&
            strcmp(temp->bookingDate, today) == 0) {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}

Patient *findPatientByPhone(const char *phone)
{
    Patient *temp = head;

    while (temp != NULL) {
        if (strcmp(temp->phone, phone) == 0)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

/* =====================================================
   PIN MANAGEMENT
   ===================================================== */

unsigned long long hashPIN(const char *pin)
{
    unsigned long long hash = 1469598103934665603ULL;
    int i;

    for (i = 0; pin[i] != '\0'; i++) {
        hash ^= (unsigned char)pin[i];
        hash *= 1099511628211ULL;
    }

    return hash;
}

void savePIN(void)
{
    FILE *fp = fopen(PIN_FILE, "w");

    if (fp == NULL) {
        printf("Unable to save PIN.\n");
        return;
    }

    fprintf(fp, "%llu", receptionistPIN);

    fclose(fp);
}

void loadPIN(void)
{
    FILE *fp = fopen(PIN_FILE, "r");

    if (fp == NULL) {
        receptionistPIN = hashPIN("1234");
        savePIN();
        return;
    }

    if (fscanf(fp, "%llu", &receptionistPIN) != 1)
        receptionistPIN = hashPIN("1234");

    fclose(fp);
}

/* =====================================================
   FILE HANDLING - BACKEND 1
   ===================================================== */

void saveData(void)
{
    FILE *fp;
    Patient *temp;

    fp = fopen(DATA_FILE, "w");

    if (fp == NULL) {
        printf("Error saving patient records.\n");
        return;
    }

    temp = head;

    while (temp != NULL) {
        fprintf(fp,
            "%d|%lld|%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
            temp->token,
            temp->order,
            temp->name,
            temp->phone,
            temp->hospital,
            temp->department,
            temp->doctor,
            temp->symptoms,
            temp->bookingDate,
            temp->status,
            temp->emergency
        );

        temp = temp->next;
    }

    fclose(fp);
}

void loadData(void)
{
    FILE *fp;
    char line[1000];
    char *fields[11];
    char *token;
    int i;

    fp = fopen(DATA_FILE, "r");

    if (fp == NULL)
        return;

    while (fgets(line, sizeof(line), fp) != NULL) {
        Patient *p;

        line[strcspn(line, "\n")] = '\0';

        fields[0] = strtok(line, "|");

        if (fields[0] == NULL)
            continue;

        for (i = 1; i < 11; i++) {
            fields[i] = strtok(NULL, "|");

            if (fields[i] == NULL)
                break;
        }

        if (i != 11)
            continue;

        p = (Patient *)malloc(sizeof(Patient));

        if (p == NULL) {
            printf("Memory allocation failed.\n");
            break;
        }

        memset(p, 0, sizeof(Patient));

        p->token = atoi(fields[0]);
        p->order = atoll(fields[1]);

        copyText(p->name, fields[2], NAME_SIZE);
        copyText(p->phone, fields[3], PHONE_SIZE);
        copyText(p->hospital, fields[4], NAME_SIZE);
        copyText(p->department, fields[5], NAME_SIZE);
        copyText(p->doctor, fields[6], NAME_SIZE);
        copyText(p->symptoms, fields[7], SYMPTOM_SIZE);
        copyText(p->bookingDate, fields[8], DATE_SIZE);

        p->status = atoi(fields[9]);
        p->emergency = atoi(fields[10]);

        if (p->status < WAITING || p->status > CANCELLED) {
            free(p);
            continue;
        }

        if (p->emergency != 0 && p->emergency != 1) {
            free(p);
            continue;
        }

        p->next = NULL;

        addPatient(p);

        if (p->order >= nextOrder)
            nextOrder = p->order + 1;
    }

    fclose(fp);
}

/* =====================================================
   DAILY TOKEN RENEWAL
   ===================================================== */

void updateDailyToken(void)
{
    int maximum = 0;
    char today[DATE_SIZE];
    Patient *temp = head;

    getTodayDate(today);

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0 &&
            temp->token > maximum) {
            maximum = temp->token;
        }

        temp = temp->next;
    }

    nextToken = maximum + 1;
}

/* =====================================================
   HOSPITAL INFORMATION
   ===================================================== */

void showHospitals(void)
{
    int i;

    printf("\n========== AVAILABLE HOSPITALS ==========\n");

    for (i = 0; i < HOSPITAL_COUNT; i++)
        printf("%d. %s\n", i + 1, hospitals[i]);
}

void showDepartments(void)
{
    int i;

    printf("\n========== AVAILABLE DEPARTMENTS ==========\n");

    for (i = 0; i < DEPARTMENT_COUNT; i++)
        printf("%d. %s\n", i + 1, departments[i]);
}

void showDoctors(int departmentIndex)
{
    int i;

    printf("\n========== AVAILABLE DOCTORS ==========\n");

    for (i = 0; i < DOCTORS_PER_DEPT; i++)
        printf("%d. %s\n", i + 1,
               doctors[departmentIndex][i]);
}

int selectHospital(void)
{
    int choice;

    showHospitals();

    choice = readInt("Select hospital: ");

    if (choice < 1 || choice > HOSPITAL_COUNT) {
        printf("Invalid hospital selection.\n");
        return -1;
    }

    return choice - 1;
}

int selectDepartment(void)
{
    int choice;

    showDepartments();

    choice = readInt("Select department: ");

    if (choice < 1 || choice > DEPARTMENT_COUNT) {
        printf("Invalid department selection.\n");
        return -1;
    }

    return choice - 1;
}

int selectDoctor(int departmentIndex)
{
    int choice;

    showDoctors(departmentIndex);

    choice = readInt("Select doctor: ");

    if (choice < 1 || choice > DOCTORS_PER_DEPT) {
        printf("Invalid doctor selection.\n");
        return -1;
    }

    return choice - 1;
}

/* =====================================================
   STATUS DISPLAY
   ===================================================== */

const char *getStatusName(int status)
{
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

void showPatientDetails(Patient *p)
{
    if (p == NULL)
        return;

    printf("\n========== PATIENT DETAILS ==========\n");

    printf("Token Number : %d\n", p->token);
    printf("Name         : %s\n", p->name);
    printf("Phone        : %s\n", p->phone);
    printf("Hospital     : %s\n", p->hospital);
    printf("Department   : %s\n", p->department);
    printf("Doctor       : %s\n", p->doctor);
    printf("Symptoms     : %s\n", p->symptoms);
    printf("Booking Date : %s\n", p->bookingDate);
    printf("Status       : %s\n", getStatusName(p->status));

    printf("Priority     : %s\n",
           p->emergency ? "EMERGENCY" : "NORMAL");
}

/* =====================================================
   BOOKING RECEIPT
   ===================================================== */

void showBookingReceipt(Patient *p)
{
    char currentTime[20];

    getCurrentTime(currentTime);

    printf("\n========================================\n");
    printf("          BOOKING CONFIRMATION\n");
    printf("========================================\n");

    printf("Patient Name : %s\n", p->name);
    printf("Token Number : %d\n", p->token);
    printf("Hospital     : %s\n", p->hospital);
    printf("Department   : %s\n", p->department);
    printf("Doctor       : %s\n", p->doctor);
    printf("Booking Date : %s\n", p->bookingDate);
    printf("Booking Time : %s\n", currentTime);
    printf("Status       : WAITING\n");

    printf("========================================\n");
    printf("Please keep your token number safe.\n");
}

/* =====================================================
   BACKEND 1 - PATIENT REGISTRATION
   ===================================================== */

void registerPatient(int walkIn)
{
    Patient *p;
    Patient *temp;

    int h, d, doc;

    char input[INPUT_SIZE];

    p = (Patient *)malloc(sizeof(Patient));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    memset(p, 0, sizeof(Patient));

    printf("\n========== PATIENT REGISTRATION ==========\n");

    readLine("Enter patient name: ", input, sizeof(input));
    sanitizeText(input);

    if (strlen(input) == 0) {
        printf("Name cannot be empty.\n");
        free(p);
        return;
    }

    copyText(p->name, input, NAME_SIZE);

    readLine("Enter phone number: ", input, sizeof(input));

    if (!isValidPhone(input)) {
        printf("Invalid phone number.\n");
        free(p);
        return;
    }

    copyText(p->phone, input, PHONE_SIZE);

    /* Prevent duplicate active booking */

    temp = head;

    while (temp != NULL) {
        char today[DATE_SIZE];

        getTodayDate(today);

        if (strcmp(temp->phone, p->phone) == 0 &&
            strcmp(temp->bookingDate, today) == 0 &&
            (temp->status == WAITING ||
             temp->status == IN_CONSULTATION)) {

            printf("An active booking already exists.\n");
            printf("Existing token: %d\n", temp->token);

            free(p);
            return;
        }

        temp = temp->next;
    }

    h = selectHospital();

    if (h == -1) {
        free(p);
        return;
    }

    d = selectDepartment();

    if (d == -1) {
        free(p);
        return;
    }

    doc = selectDoctor(d);

    if (doc == -1) {
        free(p);
        return;
    }

    copyText(p->hospital, hospitals[h], NAME_SIZE);
    copyText(p->department, departments[d], NAME_SIZE);
    copyText(p->doctor, doctors[d][doc], NAME_SIZE);

    readLine("Enter symptoms or reason for visit: ",
             input, sizeof(input));

    sanitizeText(input);

    if (strlen(input) == 0)
        copyText(p->symptoms, "Not specified", SYMPTOM_SIZE);
    else
        copyText(p->symptoms, input, SYMPTOM_SIZE);

    getTodayDate(p->bookingDate);

    p->token = nextToken++;
    p->order = nextOrder++;

    p->status = WAITING;
    p->emergency = 0;
    p->next = NULL;

    addPatient(p);

    saveData();

    printf("\nRegistration successful.\n");

    if (walkIn)
        printf("Walk-in patient registered successfully.\n");

    showBookingReceipt(p);
}

/* =====================================================
   TOKEN DETAILS
   ===================================================== */

void showTokenDetails(void)
{
    int token;
    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Token not found for today.\n");
        return;
    }

    showPatientDetails(p);
}

/* =====================================================
   QUEUE CALCULATIONS
   ===================================================== */

int sameDoctor(Patient *p,
               const char *hospital,
               const char *department,
               const char *doctor)
{
    return strcmp(p->hospital, hospital) == 0 &&
           strcmp(p->department, department) == 0 &&
           strcmp(p->doctor, doctor) == 0;
}

int countWaitingForDoctor(const char *hospital,
                          const char *department,
                          const char *doctor)
{
    int count = 0;
    char today[DATE_SIZE];

    Patient *temp = head;

    getTodayDate(today);

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0 &&
            temp->status == WAITING &&
            sameDoctor(temp, hospital, department, doctor)) {
            count++;
        }

        temp = temp->next;
    }

    return count;
}

Patient *getNextPatient(const char *hospital,
                        const char *department,
                        const char *doctor)
{
    Patient *emergencyPatient = NULL;
    Patient *normalPatient = NULL;

    char today[DATE_SIZE];

    Patient *temp = head;

    getTodayDate(today);

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0 &&
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

int countAhead(Patient *target)
{
    int ahead = 0;

    Patient *temp = head;

    if (target == NULL || target->status != WAITING)
        return 0;

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, target->bookingDate) == 0 &&
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

/* =====================================================
   TOKEN STATUS
   ===================================================== */

void showPatientStatus(void)
{
    int token;
    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Token not found for today.\n");
        return;
    }

    showPatientDetails(p);

    if (p->status == WAITING) {
        int ahead = countAhead(p);

        printf("\nPatients ahead: %d\n", ahead);

        printf("Estimated waiting time: %d minutes\n",
               ahead * SERVICE_MINUTES);
    }
}

/* =====================================================
   PATIENT HISTORY
   ===================================================== */

void showPatientHistory(void)
{
    char phone[PHONE_SIZE];

    int found = 0;

    Patient *temp = head;

    readLine("\nEnter registered phone number: ",
             phone, sizeof(phone));

    if (!isValidPhone(phone)) {
        printf("Invalid phone number.\n");
        return;
    }

    printf("\n========== PATIENT HISTORY ==========\n");

    while (temp != NULL) {
        if (strcmp(temp->phone, phone) == 0) {
            showPatientDetails(temp);
            found = 1;
        }

        temp = temp->next;
    }

    if (!found)
        printf("No booking history found.\n");
}

/* =====================================================
   PATIENT TOKEN CANCELLATION
   ===================================================== */

void cancelPatientToken(void)
{
    int token;

    char phone[PHONE_SIZE];

    Patient *p;

    token = readInt("\nEnter token number: ");

    readLine("Enter registered phone number: ",
             phone, sizeof(phone));

    p = findToken(token);

    if (p == NULL || strcmp(p->phone, phone) != 0) {
        printf("Token or patient details do not match.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("Only waiting tokens can be cancelled.\n");
        return;
    }

    p->status = CANCELLED;

    saveData();

    printf("Token cancelled successfully.\n");
}

/* =====================================================
   SYMPTOM-BASED DEPARTMENT SUGGESTION
   ===================================================== */

void symptomSuggestion(void)
{
    char symptoms[SYMPTOM_SIZE];
    int i;

    readLine("\nDescribe your symptoms: ",
             symptoms, sizeof(symptoms));

    for (i = 0; symptoms[i] != '\0'; i++) {
        symptoms[i] = (char)tolower(
            (unsigned char)symptoms[i]
        );
    }

    printf("\nSuggested department: ");

    if (strstr(symptoms, "ear") != NULL ||
        strstr(symptoms, "nose") != NULL ||
        strstr(symptoms, "throat") != NULL) {

        printf("ENT\n");
    }
    else if (strstr(symptoms, "bone") != NULL ||
             strstr(symptoms, "joint") != NULL ||
             strstr(symptoms, "fracture") != NULL) {

        printf("Orthopaedics\n");
    }
    else if (strstr(symptoms, "skin") != NULL ||
             strstr(symptoms, "rash") != NULL ||
             strstr(symptoms, "itch") != NULL) {

        printf("Dermatology\n");
    }
    else {
        printf("General Medicine\n");
    }

    printf("This is a basic navigation suggestion, not a diagnosis.\n");
    printf("For urgent or severe symptoms, seek immediate medical care.\n");
}

/* =====================================================
   PATIENT HELP AND FAQ
   ===================================================== */

void patientHelp(void)
{
    printf("\n========== PATIENT HELP AND FAQ ==========\n");

    printf("\n1. How to book a token?\n");
    printf("   Select Register / Book New Token.\n");

    printf("\n2. How to check token status?\n");
    printf("   Select Check Token Status.\n");

    printf("\n3. How to cancel a token?\n");
    printf("   Select Cancel Token and verify your phone number.\n");

    printf("\n4. How to view previous bookings?\n");
    printf("   Select Patient History.\n");

    printf("\n5. What if I lose my token number?\n");
    printf("   Contact hospital reception for assistance.\n");
}

/* =====================================================
   PATIENT GUIDELINES
   ===================================================== */

void patientGuidelines(void)
{
    printf("\n========== PATIENT GUIDELINES ==========\n");

    printf("1. Enter correct personal information.\n");
    printf("2. Keep your token number safe.\n");
    printf("3. Reach the hospital on time.\n");
    printf("4. Follow queue instructions.\n");
    printf("5. Contact reception if you need help.\n");
    printf("6. Do not share personal information unnecessarily.\n");
    printf("7. Follow instructions given by hospital staff.\n");
}

/* =====================================================
   PATIENT HOSPITAL INFORMATION
   ===================================================== */

void patientHospitalInfo(void)
{
    showHospitals();
}

/* =====================================================
   PATIENT DOCTOR INFORMATION
   ===================================================== */

void patientDoctorInfo(void)
{
    int i, j;

    printf("\n========== DOCTOR DIRECTORY ==========\n");

    for (i = 0; i < DEPARTMENT_COUNT; i++) {
        printf("\nDepartment: %s\n", departments[i]);

        for (j = 0; j < DOCTORS_PER_DEPT; j++) {
            printf("%s\n", doctors[i][j]);
        }
    }
}

/* =====================================================
   FRONTEND 1 - PATIENT WELCOME SCREEN
   ===================================================== */

void patientWelcome(void)
{
    printf("\n");
    printf("========================================\n");
    printf("     HOSPITAL TOKEN BOOKING SYSTEM\n");
    printf("             PATIENT PORTAL\n");
    printf("========================================\n");

    showDate();

    printf("Welcome! Manage your hospital bookings here.\n");
}

/* =====================================================
   FRONTEND 1 - PATIENT PORTAL
   ===================================================== */

void patientMenu(void)
{
    int choice;

    do {
        patientWelcome();

        printf("\n1. Register / Book New Token");
        printf("\n2. Check Token Status");
        printf("\n3. View Token Details");
        printf("\n4. View Patient History");
        printf("\n5. Cancel Waiting Token");
        printf("\n6. Symptom-Based Department Suggestion");
        printf("\n7. Hospital Information");
        printf("\n8. Doctor Information");
        printf("\n9. Help and FAQ");
        printf("\n10. Patient Guidelines");
        printf("\n0. Return to Main Menu");

        choice = readInt("\n\nEnter your choice: ");

        switch (choice) {
            case 1:
                registerPatient(0);
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
                printf("Returning to main menu...\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

/* =====================================================
   FRONTEND 2 - RECEPTIONIST LOGIN
   ===================================================== */

int receptionistLogin(void)
{
    char pin[INPUT_SIZE];
    int attempt;

    printf("\n========== RECEPTIONIST LOGIN ==========\n");

    for (attempt = 1; attempt <= 3; attempt++) {
        readLine("Enter receptionist PIN: ",
                 pin, sizeof(pin));

        if (hashPIN(pin) == receptionistPIN) {
            printf("Login successful.\n");
            return 1;
        }

        printf("Incorrect PIN. Attempts remaining: %d\n",
               3 - attempt);
    }

    printf("Access denied.\n");

    return 0;
}

/* =====================================================
   RECEPTIONIST PIN CHANGE
   ===================================================== */

void changePIN(void)
{
    char oldPIN[INPUT_SIZE];
    char newPIN[INPUT_SIZE];
    char confirmPIN[INPUT_SIZE];

    readLine("\nEnter current PIN: ",
             oldPIN, sizeof(oldPIN));

    if (hashPIN(oldPIN) != receptionistPIN) {
        printf("Incorrect current PIN.\n");
        return;
    }

    readLine("Enter new PIN (minimum 4 characters): ",
             newPIN, sizeof(newPIN));

    if (strlen(newPIN) < 4) {
        printf("PIN must contain at least 4 characters.\n");
        return;
    }

    readLine("Confirm new PIN: ",
             confirmPIN, sizeof(confirmPIN));

    if (strcmp(newPIN, confirmPIN) != 0) {
        printf("PIN confirmation does not match.\n");
        return;
    }

    receptionistPIN = hashPIN(newPIN);

    savePIN();

    printf("PIN changed successfully.\n");
}

/* =====================================================
   MULTIPLE WALK-IN REGISTRATION
   ===================================================== */

void multipleWalkInRegistration(void)
{
    int count;
    int i;

    count = readInt("\nHow many walk-in patients? ");

    if (count < 1 || count > 50) {
        printf("Enter a number between 1 and 50.\n");
        return;
    }

    for (i = 1; i <= count; i++) {
        printf("\n========== WALK-IN PATIENT %d ==========\n", i);

        registerPatient(1);
    }
}

/* =====================================================
   RECEPTIONIST SEARCH PATIENT
   ===================================================== */

void searchPatient(void)
{
    int choice;
    int token;
    int found = 0;

    char phone[PHONE_SIZE];

    Patient *temp;

    printf("\n========== SEARCH PATIENT ==========\n");

    printf("1. Search by today's token\n");
    printf("2. Search by phone number\n");

    choice = readInt("Enter choice: ");

    if (choice == 1) {
        token = readInt("Enter token number: ");

        temp = findToken(token);

        if (temp == NULL)
            printf("Patient not found.\n");
        else
            showPatientDetails(temp);
    }
    else if (choice == 2) {
        readLine("Enter phone number: ",
                 phone, sizeof(phone));

        if (!isValidPhone(phone)) {
            printf("Invalid phone number.\n");
            return;
        }

        temp = head;

        while (temp != NULL) {
            if (strcmp(temp->phone, phone) == 0) {
                showPatientDetails(temp);
                found = 1;
            }

            temp = temp->next;
        }

        if (!found)
            printf("No matching patient found.\n");
    }
    else {
        printf("Invalid choice.\n");
    }
}

/* =====================================================
   BACKEND 2 - VIEW DOCTOR-WISE QUEUE
   ===================================================== */

void showQueue(void)
{
    int h, d, doc;
    int count = 0;

    char today[DATE_SIZE];

    Patient *temp;

    h = selectHospital();
    if (h == -1) return;

    d = selectDepartment();
    if (d == -1) return;

    doc = selectDoctor(d);
    if (doc == -1) return;

    getTodayDate(today);

    printf("\n========== DOCTOR-WISE WAITING QUEUE ==========\n");

    printf("Hospital: %s\n", hospitals[h]);
    printf("Department: %s\n", departments[d]);
    printf("Doctor: %s\n", doctors[d][doc]);

    printf("\n--- EMERGENCY PATIENTS ---\n");

    temp = head;

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0 &&
            temp->status == WAITING &&
            temp->emergency &&
            sameDoctor(temp, hospitals[h],
                       departments[d], doctors[d][doc])) {

            printf("Token %d | %s\n",
                   temp->token, temp->name);

            count++;
        }

        temp = temp->next;
    }

    printf("\n--- NORMAL PATIENTS ---\n");

    temp = head;

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0 &&
            temp->status == WAITING &&
            !temp->emergency &&
            sameDoctor(temp, hospitals[h],
                       departments[d], doctors[d][doc])) {

            printf("Token %d | %s\n",
                   temp->token, temp->name);

            count++;
        }

        temp = temp->next;
    }

    if (count == 0)
        printf("No waiting patients.\n");

    printf("\nTotal waiting patients: %d\n", count);
}

/* =====================================================
   EMERGENCY PRIORITY
   ===================================================== */

void markEmergencyPriority(void)
{
    int token;
    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Patient not found.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("Only waiting patients can be marked as emergency.\n");
        return;
    }

    if (p->emergency) {
        printf("Patient already has emergency priority.\n");
        return;
    }

    p->emergency = 1;

    saveData();

    printf("Emergency priority assigned by receptionist.\n");
    printf("Patient will be considered before normal waiting patients.\n");
}

/* =====================================================
   CALL NEXT PATIENT
   ===================================================== */

void callNextPatient(void)
{
    int h, d, doc;

    Patient *p;

    h = selectHospital();
    if (h == -1) return;

    d = selectDepartment();
    if (d == -1) return;

    doc = selectDoctor(d);
    if (doc == -1) return;

    p = getNextPatient(
        hospitals[h],
        departments[d],
        doctors[d][doc]
    );

    if (p == NULL) {
        printf("No waiting patients for this doctor.\n");
        return;
    }

    p->status = IN_CONSULTATION;

    saveData();

    printf("\n========== PATIENT CALLED ==========\n");

    showPatientDetails(p);
}

/* =====================================================
   COMPLETE CONSULTATION
   ===================================================== */

void completeConsultation(void)
{
    int token;
    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Patient not found.\n");
        return;
    }

    if (p->status != IN_CONSULTATION) {
        printf("Patient is not currently in consultation.\n");
        return;
    }

    p->status = COMPLETED;

    saveData();

    printf("Consultation completed successfully.\n");
}

/* =====================================================
   WAITING POSITION
   ===================================================== */

void showWaitingPosition(void)
{
    int token;
    int ahead;

    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Token not found.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("Patient is not in the waiting queue.\n");
        printf("Current status: %s\n", getStatusName(p->status));
        return;
    }

    ahead = countAhead(p);

    printf("\n========== WAITING POSITION ==========\n");

    printf("Token Number: %d\n", p->token);
    printf("Patients Ahead: %d\n", ahead);

    printf("Estimated Waiting Time: %d minutes\n",
           ahead * SERVICE_MINUTES);
}

/* =====================================================
   DAILY STATISTICS
   ===================================================== */

void showStatistics(void)
{
    int waiting = 0;
    int consultation = 0;
    int completed = 0;
    int cancelled = 0;
    int emergency = 0;

    char today[DATE_SIZE];

    Patient *temp = head;

    getTodayDate(today);

    while (temp != NULL) {
        if (strcmp(temp->bookingDate, today) == 0) {
            if (temp->status == WAITING)
                waiting++;

            else if (temp->status == IN_CONSULTATION)
                consultation++;

            else if (temp->status == COMPLETED)
                completed++;

            else if (temp->status == CANCELLED)
                cancelled++;

            if (temp->emergency && temp->status == WAITING)
                emergency++;
        }

        temp = temp->next;
    }

    printf("\n========== DAILY STATISTICS ==========\n");

    printf("Date: %s\n", today);
    printf("Waiting Patients        : %d\n", waiting);
    printf("Emergency Waiting       : %d\n", emergency);
    printf("In Consultation         : %d\n", consultation);
    printf("Completed Consultations : %d\n", completed);
    printf("Cancelled Tokens        : %d\n", cancelled);

    printf("Total Bookings Today    : %d\n",
           waiting + consultation + completed + cancelled);
}

/* =====================================================
   HOSPITAL-WISE STATISTICS
   ===================================================== */

void showHospitalStatistics(void)
{
    int i;
    char today[DATE_SIZE];

    getTodayDate(today);

    printf("\n========== HOSPITAL-WISE STATISTICS ==========\n");

    for (i = 0; i < HOSPITAL_COUNT; i++) {
        int total = 0;
        int waiting = 0;
        int completed = 0;

        Patient *temp = head;

        while (temp != NULL) {
            if (strcmp(temp->bookingDate, today) == 0 &&
                strcmp(temp->hospital, hospitals[i]) == 0) {

                total++;

                if (temp->status == WAITING)
                    waiting++;

                if (temp->status == COMPLETED)
                    completed++;
            }

            temp = temp->next;
        }

        printf("\n%s\n", hospitals[i]);

        printf("Total: %d | Waiting: %d | Completed: %d\n",
               total, waiting, completed);
    }
}

/* =====================================================
   DOCTOR-WISE STATISTICS
   ===================================================== */

void showDoctorStatistics(void)
{
    int h, d, doc;

    char today[DATE_SIZE];

    getTodayDate(today);

    printf("\n========== DOCTOR-WISE STATISTICS ==========\n");

    for (h = 0; h < HOSPITAL_COUNT; h++) {
        printf("\nHospital: %s\n", hospitals[h]);

        for (d = 0; d < DEPARTMENT_COUNT; d++) {
            for (doc = 0; doc < DOCTORS_PER_DEPT; doc++) {
                int waiting = 0;
                int completed = 0;

                Patient *temp = head;

                while (temp != NULL) {
                    if (strcmp(temp->bookingDate, today) == 0 &&
                        sameDoctor(temp, hospitals[h],
                                   departments[d], doctors[d][doc])) {

                        if (temp->status == WAITING)
                            waiting++;

                        if (temp->status == COMPLETED)
                            completed++;
                    }

                    temp = temp->next;
                }

                printf("%s | Waiting: %d | Completed: %d\n",
                       doctors[d][doc], waiting, completed);
            }
        }
    }
}

/* =====================================================
   RECEPTIONIST TOKEN CANCELLATION
   ===================================================== */

void receptionistCancelToken(void)
{
    int token;
    Patient *p;

    token = readInt("\nEnter token number: ");

    p = findToken(token);

    if (p == NULL) {
        printf("Token not found.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("Only waiting tokens can be cancelled.\n");
        return;
    }

    p->status = CANCELLED;

    saveData();

    printf("Token cancelled successfully by receptionist.\n");
}

/* =====================================================
   FRONTEND 2 - RECEPTIONIST DASHBOARD
   ===================================================== */

void receptionistMenu(void)
{
    int choice;

    if (!receptionistLogin())
        return;

    do {
        printf("\n========================================\n");
        printf("       RECEPTIONIST DASHBOARD\n");
        printf("========================================\n");

        printf("\n1. Register Walk-in Patient");
        printf("\n2. Multiple Walk-in Registration");
        printf("\n3. Search Patient");
        printf("\n4. View Doctor-wise Queue");
        printf("\n5. Call Next Patient");
        printf("\n6. Complete Consultation");
        printf("\n7. Mark Emergency Priority");
        printf("\n8. Show Waiting Position");
        printf("\n9. Cancel Patient Token");
        printf("\n10. Daily Statistics");
        printf("\n11. Hospital-wise Statistics");
        printf("\n12. Doctor-wise Statistics");
        printf("\n13. Change Receptionist PIN");
        printf("\n0. Logout");

        choice = readInt("\nEnter your choice: ");

        switch (choice) {
            case 1:
                registerPatient(1);
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
                printf("Receptionist logged out.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

/* =====================================================
   MEMORY CLEANUP
   ===================================================== */

void destroySystem(void)
{
    Patient *temp = head;

    while (temp != NULL) {
        Patient *nextNode = temp->next;

        free(temp);

        temp = nextNode;
    }

    head = NULL;
}

/* =====================================================
   MAIN MENU
   ===================================================== */

void mainMenu(void)
{
    int choice;

    do {
        printf("\n");
        printf("============================================\n");
        printf("  HOSPITAL TOKEN BOOKING AND QUEUE SYSTEM\n");
        printf("============================================\n");

        printf("\n1. Patient Portal");
        printf("\n2. Receptionist Portal");
        printf("\n0. Exit Program");

        choice = readInt("\n\nEnter your choice: ");

        switch (choice) {
            case 1:
                patientMenu();
                break;

            case 2:
                receptionistMenu();
                break;

            case 0:
                printf("\nThank you for using the system.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 0);
}

/* =====================================================
   MAIN FUNCTION
   ===================================================== */

int main(void)
{
    loadPIN();

    loadData();

    updateDailyToken();

    mainMenu();

    saveData();

    destroySystem();

    return 0;
}
