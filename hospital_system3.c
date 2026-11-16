
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
#define SERVICE_MINUTES 15

#define DATA_FILE "patients_c.dat"
#define PIN_FILE "receptionist_pin_c.dat"

#ifdef _WIN32
#define CLEAR_SCREEN() system("cls")
#else
#define CLEAR_SCREEN() system("clear")
#endif

/* ============================================================
   COMMON DATA STRUCTURES AND CONSTANTS
   ============================================================ */

typedef enum {
    WAITING,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
} Status;

typedef struct Patient {
    int token;
    int order;
    char name[NAME_SIZE];
    char phone[PHONE_SIZE];
    int hospital;
    int department;
    int doctor;
    char symptoms[SYMPTOM_SIZE];
    char date[DATE_SIZE];
    Status status;
    int emergency;
    struct Patient *next;
} Patient;

Patient *head = NULL;

int nextToken = 1;
int nextOrder = 1;

unsigned long long receptionistPIN = 0;

const char *hospitals[HOSPITAL_COUNT] = {
    "City General Hospital",
    "Community Clinic",
    "Bone and Joint Hospital",
    "Sunrise Multispeciality Hospital",
    "Green Valley Medical Centre",
    "Metro ENT Hospital",
    "Aarogya Skin and Wellness Centre",
    "Northside Family Hospital"
};

const char *departments[DEPARTMENT_COUNT] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

const char *doctors[DEPARTMENT_COUNT][DOCTORS_PER_DEPT] = {
    {"Dr. A. Sharma", "Dr. R. Mehta"},
    {"Dr. S. Verma", "Dr. P. Kapoor"},
    {"Dr. K. Singh", "Dr. N. Joshi"},
    {"Dr. M. Gupta", "Dr. T. Rao"}
};

/* ============================================================
   COMMON HELPER FUNCTIONS
   ============================================================ */

void heading(const char *title) {
    printf("\n============================================================\n");
    printf("              %s\n", title);
    printf("============================================================\n");
}

void readLine(const char *message, char *buffer, int size) {
    int ch;

    printf("%s", message);

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        clearerr(stdin);
        return;
    }

    if (strchr(buffer, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    } else {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

int readInt(const char *message) {
    char buffer[INPUT_SIZE];
    char *end;
    long value;

    while (1) {
        readLine(message, buffer, sizeof(buffer));

        value = strtol(buffer, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (buffer[0] != '\0' && *end == '\0')
            return (int)value;

        printf("Invalid input. Enter a number.\n");
    }
}

void pauseScreen(void) {
    char temp[10];
    readLine("\nPress Enter to continue...", temp, sizeof(temp));
}

void copyText(char *destination, const char *source, int size) {
    strncpy(destination, source, size - 1);
    destination[size - 1] = '\0';
}

void sanitizeText(char *text) {
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '|')
            text[i] = '/';
    }
}

int validPhone(const char *phone) {
    int i;

    if (strlen(phone) != 10)
        return 0;

    for (i = 0; phone[i] != '\0'; i++) {
        if (!isdigit((unsigned char)phone[i]))
            return 0;
    }

    return 1;
}

void getTodayDate(char *buffer, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t == NULL) {
        copyText(buffer, "0000-00-00", size);
        return;
    }

    strftime(buffer, size, "%Y-%m-%d", t);
}

void getCurrentTime(char *buffer, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t == NULL) {
        copyText(buffer, "00:00:00", size);
        return;
    }

    strftime(buffer, size, "%H:%M:%S", t);
}

const char *getStatusName(Status status) {
    switch (status) {
        case WAITING: return "Waiting";
        case IN_CONSULTATION: return "In Consultation";
        case COMPLETED: return "Completed";
        case CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

void showHospitals(void) {
    int i;

    heading("AVAILABLE HOSPITALS");

    for (i = 0; i < HOSPITAL_COUNT; i++)
        printf("%d. %s\n", i + 1, hospitals[i]);
}

void showDepartments(void) {
    int i;

    heading("AVAILABLE DEPARTMENTS");

    for (i = 0; i < DEPARTMENT_COUNT; i++)
        printf("%d. %s\n", i + 1, departments[i]);
}

void showDoctors(int department) {
    int i;

    if (department < 0 || department >= DEPARTMENT_COUNT)
        return;

    heading("AVAILABLE DOCTORS");

    for (i = 0; i < DOCTORS_PER_DEPT; i++)
        printf("%d. %s\n", i + 1, doctors[department][i]);
}

/* ============================================================
   MEMBER 3 - BACKEND 1
   PATIENT RECORDS AND TOKEN MANAGEMENT
   ============================================================ */

/* Add a patient to the linked list */
void addPatientNode(Patient *p) {
    Patient *temp;

    p->next = NULL;

    if (head == NULL) {
        head = p;
        return;
    }

    temp = head;

    while (temp->next != NULL)
        temp = temp->next;

    temp->next = p;
}

/* Search patient using token */
Patient *findToken(int token) {
    Patient *temp = head;

    while (temp != NULL) {
        if (temp->token == token)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

/* Search patient using phone */
Patient *findPhone(const char *phone) {
    Patient *temp = head;

    while (temp != NULL) {
        if (strcmp(temp->phone, phone) == 0)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

/* Check duplicate active booking for today */
int hasActiveBooking(const char *phone) {
    Patient *temp = head;
    char today[DATE_SIZE];

    getTodayDate(today, sizeof(today));

    while (temp != NULL) {
        if (strcmp(temp->phone, phone) == 0 &&
            strcmp(temp->date, today) == 0 &&
            (temp->status == WAITING ||
             temp->status == IN_CONSULTATION)) {
            return 1;
        }

        temp = temp->next;
    }

    return 0;
}

/* Generate daily token number */
int generateToken(void) {
    return nextToken++;
}

/* Create and register a patient record */
Patient *createPatient(
    const char *name,
    const char *phone,
    int hospital,
    int department,
    int doctor,
    const char *symptoms,
    int emergency
) {
    Patient *p;

    if (name == NULL || name[0] == '\0')
        return NULL;

    if (!validPhone(phone))
        return NULL;

    if (hospital < 0 || hospital >= HOSPITAL_COUNT)
        return NULL;

    if (department < 0 || department >= DEPARTMENT_COUNT)
        return NULL;

    if (doctor < 0 || doctor >= DOCTORS_PER_DEPT)
        return NULL;

    if (hasActiveBooking(phone))
        return NULL;

    p = (Patient *)malloc(sizeof(Patient));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    p->token = generateToken();
    p->order = nextOrder++;

    copyText(p->name, name, NAME_SIZE);
    copyText(p->phone, phone, PHONE_SIZE);
    copyText(p->symptoms, symptoms, SYMPTOM_SIZE);

    sanitizeText(p->name);
    sanitizeText(p->phone);
    sanitizeText(p->symptoms);

    p->hospital = hospital;
    p->department = department;
    p->doctor = doctor;

    getTodayDate(p->date, sizeof(p->date));

    p->status = WAITING;
    p->emergency = emergency ? 1 : 0;
    p->next = NULL;

    addPatientNode(p);

    return p;
}

/* Cancel a waiting token */
int cancelToken(int token, const char *phone) {
    Patient *p = findToken(token);

    if (p == NULL)
        return 0;

    if (strcmp(p->phone, phone) != 0)
        return 0;

    if (p->status != WAITING)
        return 0;

    p->status = CANCELLED;

    return 1;
}

/* Simple PIN hashing for educational use */
unsigned long long hashPIN(const char *pin) {
    unsigned long long hash = 1469598103934665603ULL;
    int i;

    for (i = 0; pin[i] != '\0'; i++) {
        hash ^= (unsigned char)pin[i];
        hash *= 1099511628211ULL;
    }

    return hash;
}

void savePIN(const char *pin) {
    FILE *file = fopen(PIN_FILE, "w");

    if (file == NULL)
        return;

    fprintf(file, "%llu", hashPIN(pin));
    fclose(file);
}

void loadPIN(void) {
    FILE *file = fopen(PIN_FILE, "r");

    if (file == NULL) {
        copyText((char *)&receptionistPIN, "", 1);
        receptionistPIN = hashPIN("1234");
        savePIN("1234");
        return;
    }

    if (fscanf(file, "%llu", &receptionistPIN) != 1) {
        receptionistPIN = hashPIN("1234");
        savePIN("1234");
    }

    fclose(file);
}

int authenticatePIN(const char *pin) {
    return hashPIN(pin) == receptionistPIN;
}

int changePIN(const char *oldPIN, const char *newPIN) {
    if (!authenticatePIN(oldPIN))
        return 0;

    if (strlen(newPIN) < 4)
        return 0;

    savePIN(newPIN);
    receptionistPIN = hashPIN(newPIN);

    return 1;
}

/* Save patient records in a text file */
void saveData(void) {
    FILE *file = fopen(DATA_FILE, "w");
    Patient *p = head;

    if (file == NULL) {
        printf("Unable to save patient records.\n");
        return;
    }

    while (p != NULL) {
        fprintf(file,
            "%d|%d|%s|%s|%d|%d|%d|%s|%s|%d|%d\n",
            p->token,
            p->order,
            p->name,
            p->phone,
            p->hospital,
            p->department,
            p->doctor,
            p->symptoms,
            p->date,
            (int)p->status,
            p->emergency
        );

        p = p->next;
    }

    fclose(file);
}

/* Split a saved record into fields */
int splitRecord(char *line, char *fields[], int maxFields) {
    int count = 0;
    char *start = line;
    char *separator;

    while (count < maxFields) {
        separator = strchr(start, '|');

        if (separator == NULL) {
            fields[count++] = start;
            break;
        }

        *separator = '\0';
        fields[count++] = start;
        start = separator + 1;
    }

    return count;
}

/* Load saved patient records */
void loadData(void) {
    FILE *file = fopen(DATA_FILE, "r");
    char line[INPUT_SIZE * 2];

    if (file == NULL)
        return;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *fields[11];
        char *end;
        long values[7];
        int i;
        Patient *p;

        line[strcspn(line, "\r\n")] = '\0';

        if (splitRecord(line, fields, 11) != 11)
            continue;

        for (i = 0; i < 7; i++) {
            values[i] = strtol(fields[i], &end, 10);

            if (*end != '\0')
                break;
        }

        if (i != 7)
            continue;

        if (values[0] < 1 || values[1] < 1 ||
            values[4] < 0 || values[4] >= HOSPITAL_COUNT ||
            values[5] < 0 || values[5] >= DEPARTMENT_COUNT ||
            values[6] < 0 || values[6] >= DOCTORS_PER_DEPT)
            continue;

        p = (Patient *)malloc(sizeof(Patient));

        if (p == NULL)
            break;

        p->token = (int)values[0];
        p->order = (int)values[1];

        copyText(p->name, fields[2], NAME_SIZE);
        copyText(p->phone, fields[3], PHONE_SIZE);
        copyText(p->symptoms, fields[7], SYMPTOM_SIZE);
        copyText(p->date, fields[8], DATE_SIZE);

        p->hospital = (int)values[4];
        p->department = (int)values[5];
        p->doctor = (int)values[6];

        p->status = (Status)atoi(fields[9]);
        p->emergency = atoi(fields[10]) ? 1 : 0;

        if (p->status < WAITING || p->status > CANCELLED) {
            free(p);
            continue;
        }

        addPatientNode(p);

        if (p->order >= nextOrder)
            nextOrder = p->order + 1;

        if (strcmp(p->date, "") != 0) {
            char today[DATE_SIZE];
            getTodayDate(today, sizeof(today));

            if (strcmp(p->date, today) == 0 &&
                p->token >= nextToken) {
                nextToken = p->token + 1;
            }
        }
    }

    fclose(file);
}

/* Free all patient records */
void destroyRecords(void) {
    Patient *temp;

    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

/* ============================================================
   MEMBER 4 - BACKEND 2
   QUEUE AND SYSTEM MANAGEMENT
   ============================================================ */

/* Check whether two patients belong to the same doctor queue */
int sameDoctor(Patient *p, int hospital, int department, int doctor) {
    return p->hospital == hospital &&
           p->department == department &&
           p->doctor == doctor;
}

/* Count waiting patients for a selected doctor */
int countWaitingForDoctor(int hospital, int department, int doctor) {
    int count = 0;
    Patient *p = head;

    while (p != NULL) {
        if (sameDoctor(p, hospital, department, doctor) &&
            p->status == WAITING)
            count++;

        p = p->next;
    }

    return count;
}

/* Find next patient: emergency first, FIFO within each group */
Patient *getNextPatient(int hospital, int department, int doctor) {
    Patient *p = head;
    Patient *emergencyPatient = NULL;
    Patient *normalPatient = NULL;

    while (p != NULL) {
        if (sameDoctor(p, hospital, department, doctor) &&
            p->status == WAITING) {

            if (p->emergency) {
                if (emergencyPatient == NULL ||
                    p->order < emergencyPatient->order)
                    emergencyPatient = p;
            } else {
                if (normalPatient == NULL ||
                    p->order < normalPatient->order)
                    normalPatient = p;
            }
        }

        p = p->next;
    }

    if (emergencyPatient != NULL)
        return emergencyPatient;

    return normalPatient;
}

/* Count patients ahead in the selected doctor's queue */
int countAhead(Patient *target) {
    int count = 0;
    Patient *p = head;

    if (target == NULL)
        return 0;

    while (p != NULL) {
        if (p != target &&
            sameDoctor(p, target->hospital,
                       target->department, target->doctor) &&
            p->status == WAITING) {

            if (target->emergency) {
                if (p->emergency && p->order < target->order)
                    count++;
            } else {
                if (p->emergency ||
                    (!p->emergency && p->order < target->order))
                    count++;
            }
        }

        p = p->next;
    }

    return count;
}

/* Calculate estimated waiting time */
int estimatedWait(Patient *p) {
    return countAhead(p) * SERVICE_MINUTES;
}

/* Set emergency priority by staff */
int setEmergencyPriority(int token) {
    Patient *p = findToken(token);

    if (p == NULL || p->status != WAITING)
        return 0;

    p->emergency = 1;
    return 1;
}

/* Call next patient for a selected doctor */
Patient *callNextPatient(int hospital, int department, int doctor) {
    Patient *p = getNextPatient(hospital, department, doctor);
    Patient *temp;

    if (p == NULL)
        return NULL;

    /* Only one active consultation per doctor queue */
    temp = head;

    while (temp != NULL) {
        if (sameDoctor(temp, hospital, department, doctor) &&
            temp->status == IN_CONSULTATION) {
            return NULL;
        }

        temp = temp->next;
    }

    p->status = IN_CONSULTATION;

    return p;
}

/* Complete consultation */
int completeConsultation(int token) {
    Patient *p = findToken(token);

    if (p == NULL || p->status != IN_CONSULTATION)
        return 0;

    p->status = COMPLETED;
    return 1;
}

/* Statistics calculation */
int countByStatus(Status status) {
    int count = 0;
    Patient *p = head;

    while (p != NULL) {
        if (p->status == status)
            count++;

        p = p->next;
    }

    return count;
}

int countEmergencyWaiting(void) {
    int count = 0;
    Patient *p = head;

    while (p != NULL) {
        if (p->emergency && p->status == WAITING)
            count++;

        p = p->next;
    }

    return count;
}

/* ============================================================
   MEMBER 1 - FRONTEND 1
   COMPLETE PATIENT PORTAL
   All patient menu cases stay in this section.
   ============================================================ */

/* Display booking receipt */
void patientBookingReceipt(Patient *p) {
    char timeText[20];

    if (p == NULL)
        return;

    getCurrentTime(timeText, sizeof(timeText));

    heading("BOOKING CONFIRMATION");

    printf("Patient Name : %s\n", p->name);
    printf("Phone        : %s\n", p->phone);
    printf("Hospital     : %s\n", hospitals[p->hospital]);
    printf("Department   : %s\n", departments[p->department]);
    printf("Doctor       : %s\n", doctors[p->department][p->doctor]);
    printf("Token Number : %d\n", p->token);
    printf("Date         : %s\n", p->date);
    printf("Booking Time : %s\n", timeText);
    printf("Priority     : %s\n",
           p->emergency ? "Emergency" : "Normal");
    printf("Status       : %s\n", getStatusName(p->status));
    printf("Estimated Wait: %d minutes\n", estimatedWait(p));
}

/* Patient booking interface */
void patientBookToken(void) {
    char name[NAME_SIZE];
    char phone[PHONE_SIZE];
    char symptoms[SYMPTOM_SIZE];

    int h, d, doc;
    Patient *p;

    heading("BOOK YOUR TOKEN");

    readLine("Enter patient name: ", name, sizeof(name));
    readLine("Enter 10-digit phone number: ", phone, sizeof(phone));

    if (name[0] == '\0' || !validPhone(phone)) {
        printf("Invalid patient name or phone number.\n");
        return;
    }

    showHospitals();
    h = readInt("Select hospital: ");

    if (h < 1 || h > HOSPITAL_COUNT) {
        printf("Invalid hospital selection.\n");
        return;
    }

    showDepartments();
    d = readInt("Select department: ");

    if (d < 1 || d > DEPARTMENT_COUNT) {
        printf("Invalid department selection.\n");
        return;
    }

    showDoctors(d - 1);
    doc = readInt("Select doctor: ");

    if (doc < 1 || doc > DOCTORS_PER_DEPT) {
        printf("Invalid doctor selection.\n");
        return;
    }

    readLine("Enter symptoms or reason for visit: ",
             symptoms, sizeof(symptoms));

    p = createPatient(name, phone, h - 1, d - 1,
                      doc - 1, symptoms, 0);

    if (p == NULL) {
        printf("Booking failed. Check details or existing active booking.\n");
        return;
    }

    saveData();
    patientBookingReceipt(p);
}

/* Patient token details */
void patientTokenDetails(void) {
    int token;
    Patient *p;

    heading("TOKEN DETAILS");

    token = readInt("Enter token number: ");
    p = findToken(token);

    if (p == NULL) {
        printf("Token not found.\n");
        return;
    }

    patientBookingReceipt(p);
}

/* Patient status */
void patientCheckStatus(void) {
    int token;
    Patient *p;

    heading("CHECK TOKEN STATUS");

    token = readInt("Enter token number: ");
    p = findToken(token);

    if (p == NULL) {
        printf("Token not found.\n");
        return;
    }

    printf("Patient: %s\n", p->name);
    printf("Status: %s\n", getStatusName(p->status));

    if (p->status == WAITING) {
        printf("People ahead: %d\n", countAhead(p));
        printf("Estimated wait: %d minutes\n", estimatedWait(p));
    }
}

/* Patient booking history */
void patientHistory(void) {
    char phone[PHONE_SIZE];
    Patient *p;
    int found = 0;

    heading("PATIENT BOOKING HISTORY");

    readLine("Enter registered phone number: ",
             phone, sizeof(phone));

    p = head;

    while (p != NULL) {
        if (strcmp(p->phone, phone) == 0) {
            printf("\nToken: %d\n", p->token);
            printf("Date: %s\n", p->date);
            printf("Hospital: %s\n", hospitals[p->hospital]);
            printf("Doctor: %s\n",
                   doctors[p->department][p->doctor]);
            printf("Status: %s\n", getStatusName(p->status));

            found = 1;
        }

        p = p->next;
    }

    if (!found)
        printf("No booking history found.\n");
}

/* Patient cancellation interface */
void patientCancelBooking(void) {
    int token;
    char phone[PHONE_SIZE];

    heading("CANCEL TOKEN");

    token = readInt("Enter token number: ");
    readLine("Enter registered phone number: ",
             phone, sizeof(phone));

    if (cancelToken(token, phone)) {
        saveData();
        printf("Token cancelled successfully.\n");
    } else {
        printf("Cancellation failed. Only waiting tokens can be cancelled.\n");
    }
}

/* Department suggestion */
void patientDepartmentSuggestion(void) {
    heading("DEPARTMENT SUGGESTION");

    printf("This guide is for navigation only, not diagnosis.\n\n");

    printf("1. Fever, cold or general weakness - General Medicine\n");
    printf("2. Ear, nose or throat concerns - ENT\n");
    printf("3. Bone or joint concerns - Orthopaedics\n");
    printf("4. Skin concerns - Dermatology\n");

    printf("\nFor urgent or severe symptoms, seek immediate medical care.\n");
}

/* Patient guidelines */
void patientGuidelines(void) {
    heading("PATIENT GUIDELINES");

    printf("1. Keep your token number safe.\n");
    printf("2. Reach the hospital before your estimated turn.\n");
    printf("3. Carry relevant medical documents.\n");
    printf("4. Contact reception if details are incorrect.\n");
    printf("5. Emergency priority is assigned by hospital staff.\n");
}

/* Patient help */
void patientHelp(void) {
    heading("PATIENT HELP");

    printf("1. Book a token using Book Token.\n");
    printf("2. Check Status to see your queue position.\n");
    printf("3. Token Details displays your booking.\n");
    printf("4. Booking History displays previous records.\n");
    printf("5. Contact reception for assistance.\n");
}

/* Patient portal menu with all cases together */
void patientMenu(void) {
    int choice;

    do {
        heading("PATIENT PORTAL");

        printf("1. Book Token\n");
        printf("2. View Token Details\n");
        printf("3. Check Token Status\n");
        printf("4. Booking History\n");
        printf("5. Cancel Token\n");
        printf("6. Department Suggestion\n");
        printf("7. Hospital List\n");
        printf("8. Department List\n");
        printf("9. Patient Guidelines\n");
        printf("10. Help\n");
        printf("0. Back to Main Menu\n");

        choice = readInt("\nEnter choice: ");

        /* ALL FRONTEND 1 CASES ARE HERE */
        switch (choice) {
            case 1:
                patientBookToken();
                break;

            case 2:
                patientTokenDetails();
                break;

            case 3:
                patientCheckStatus();
                break;

            case 4:
                patientHistory();
                break;

            case 5:
                patientCancelBooking();
                break;

            case 6:
                patientDepartmentSuggestion();
                break;

            case 7:
                showHospitals();
                break;

            case 8:
                showDepartments();
                break;

            case 9:
                patientGuidelines();
                break;

            case 10:
                patientHelp();
                break;

            case 0:
                printf("Leaving Patient Portal.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

/* ============================================================
   MEMBER 2 - FRONTEND 2
   COMPLETE RECEPTIONIST PORTAL
   All receptionist menu cases stay in this section.
   ============================================================ */

/* Receptionist login interface */
int receptionistLogin(void) {
    char pin[INPUT_SIZE];

    heading("RECEPTIONIST LOGIN");

    readLine("Enter receptionist PIN: ", pin, sizeof(pin));

    if (authenticatePIN(pin)) {
        printf("Login successful.\n");
        return 1;
    }

    printf("Incorrect PIN.\n");
    return 0;
}

/* Select hospital, department and doctor */
int receptionistSelectDoctor(int *h, int *d, int *doc) {
    showHospitals();
    *h = readInt("Select hospital: ");

    if (*h < 1 || *h > HOSPITAL_COUNT)
        return 0;

    showDepartments();
    *d = readInt("Select department: ");

    if (*d < 1 || *d > DEPARTMENT_COUNT)
        return 0;

    showDoctors(*d - 1);
    *doc = readInt("Select doctor: ");

    if (*doc < 1 || *doc > DOCTORS_PER_DEPT)
        return 0;

    return 1;
}

/* Walk-in patient registration interface */
void receptionistWalkIn(void) {
    char name[NAME_SIZE];
    char phone[PHONE_SIZE];
    char symptoms[SYMPTOM_SIZE];

    int h, d, doc, emergency;
    Patient *p;

    heading("WALK-IN PATIENT REGISTRATION");

    readLine("Patient name: ", name, sizeof(name));
    readLine("Phone number: ", phone, sizeof(phone));

    if (name[0] == '\0' || !validPhone(phone)) {
        printf("Invalid name or phone number.\n");
        return;
    }

    if (!receptionistSelectDoctor(&h, &d, &doc)) {
        printf("Invalid selection.\n");
        return;
    }

    readLine("Symptoms or reason for visit: ",
             symptoms, sizeof(symptoms));

    emergency = readInt("Emergency marked by staff? (1=Yes, 0=No): ");

    if (emergency != 0 && emergency != 1)
        emergency = 0;

    p = createPatient(name, phone, h - 1, d - 1,
                      doc - 1, symptoms, emergency);

    if (p == NULL) {
        printf("Registration failed. Check details or active booking.\n");
        return;
    }

    saveData();
    patientBookingReceipt(p);
}

/* Multiple walk-in registration */
void receptionistMultipleWalkIn(void) {
    int total, i;

    heading("MULTIPLE WALK-IN REGISTRATION");

    total = readInt("Number of patients to register (1-20): ");

    if (total < 1 || total > 20) {
        printf("Enter a number between 1 and 20.\n");
        return;
    }

    for (i = 0; i < total; i++) {
        printf("\nPatient %d of %d\n", i + 1, total);
        receptionistWalkIn();
    }

    saveData();
}

/* Search patient interface */
void receptionistSearchPatient(void) {
    int token;
    Patient *p;

    heading("SEARCH PATIENT");

    token = readInt("Enter token number: ");
    p = findToken(token);

    if (p == NULL) {
        printf("Patient not found.\n");
        return;
    }

    patientBookingReceipt(p);
}

/* Receptionist cancellation */
void receptionistCancelToken(void) {
    int token;
    Patient *p;

    heading("RECEPTIONIST TOKEN CANCELLATION");

    token = readInt("Enter token number: ");
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

    printf("Token cancelled by receptionist.\n");
}

/* Doctor-wise queue display */
void receptionistQueueDisplay(void) {
    int h, d, doc;
    int position = 1;
    Patient *p;

    heading("DOCTOR-WISE WAITING QUEUE");

    if (!receptionistSelectDoctor(&h, &d, &doc)) {
        printf("Invalid selection.\n");
        return;
    }

    printf("\nQueue for %s at %s:\n",
           doctors[d - 1][doc - 1], hospitals[h - 1]);

    p = head;

    while (p != NULL) {
        if (sameDoctor(p, h - 1, d - 1, doc - 1) &&
            p->status == WAITING) {

            printf("%d. Token %d | %s | %s | Estimated wait: %d min\n",
                   position++,
                   p->token,
                   p->name,
                   p->emergency ? "EMERGENCY" : "NORMAL",
                   estimatedWait(p));
        }

        p = p->next;
    }

    if (position == 1)
        printf("No waiting patients.\n");
}

/* Emergency priority interface */
void receptionistEmergencyPriority(void) {
    int token;

    heading("EMERGENCY PRIORITY");

    token = readInt("Enter token number: ");

    if (setEmergencyPriority(token)) {
        saveData();
        printf("Emergency priority updated.\n");
    } else {
        printf("Unable to update priority. Check token status.\n");
    }
}

/* Call next patient interface */
void receptionistCallNext(void) {
    int h, d, doc;
    Patient *p;

    heading("CALL NEXT PATIENT");

    if (!receptionistSelectDoctor(&h, &d, &doc)) {
        printf("Invalid selection.\n");
        return;
    }

    p = callNextPatient(h - 1, d - 1, doc - 1);

    if (p == NULL) {
        printf("No waiting patient, or a consultation is already active.\n");
        return;
    }

    saveData();

    printf("Now calling Token %d - %s\n", p->token, p->name);
}

/* Complete consultation interface */
void receptionistCompleteConsultation(void) {
    int token;

    heading("COMPLETE CONSULTATION");

    token = readInt("Enter token number: ");

    if (completeConsultation(token)) {
        saveData();
        printf("Consultation marked as completed.\n");
    } else {
        printf("Invalid token or patient is not in consultation.\n");
    }
}

/* Statistics display */
void receptionistStatistics(void) {
    heading("SYSTEM STATISTICS");

    printf("Waiting patients        : %d\n", countByStatus(WAITING));
    printf("In consultation         : %d\n", countByStatus(IN_CONSULTATION));
    printf("Completed consultations : %d\n", countByStatus(COMPLETED));
    printf("Cancelled tokens        : %d\n", countByStatus(CANCELLED));
    printf("Emergency waiting       : %d\n", countEmergencyWaiting());
}

/* Change receptionist PIN interface */
void receptionistChangePIN(void) {
    char oldPIN[INPUT_SIZE];
    char newPIN[INPUT_SIZE];

    heading("CHANGE RECEPTIONIST PIN");

    readLine("Enter current PIN: ", oldPIN, sizeof(oldPIN));
    readLine("Enter new PIN (minimum 4 characters): ",
             newPIN, sizeof(newPIN));

    if (changePIN(oldPIN, newPIN))
        printf("PIN changed successfully.\n");
    else
        printf("PIN change failed. Check current PIN or new PIN length.\n");
}

/* Receptionist menu with all cases together */
void receptionistMenu(void) {
    int choice;

    if (!receptionistLogin())
        return;

    do {
        heading("RECEPTIONIST DASHBOARD");

        printf("1. Register Walk-in Patient\n");
        printf("2. Register Multiple Walk-in Patients\n");
        printf("3. Search Patient\n");
        printf("4. Cancel Token\n");
        printf("5. View Doctor-wise Queue\n");
        printf("6. Mark Emergency Priority\n");
        printf("7. Call Next Patient\n");
        printf("8. Complete Consultation\n");
        printf("9. View Statistics\n");
        printf("10. Change PIN\n");
        printf("0. Logout\n");

        choice = readInt("\nEnter choice: ");

        /* ALL FRONTEND 2 CASES ARE HERE */
        switch (choice) {
            case 1:
                receptionistWalkIn();
                break;

            case 2:
                receptionistMultipleWalkIn();
                break;

            case 3:
                receptionistSearchPatient();
                break;

            case 4:
                receptionistCancelToken();
                break;

            case 5:
                receptionistQueueDisplay();
                break;

            case 6:
                receptionistEmergencyPriority();
                break;

            case 7:
                receptionistCallNext();
                break;

            case 8:
                receptionistCompleteConsultation();
                break;

            case 9:
                receptionistStatistics();
                break;

            case 10:
                receptionistChangePIN();
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

/* ============================================================
   MAIN SYSTEM MENU
   ============================================================ */

void mainMenu(void) {
    int choice;
    char date[DATE_SIZE];
    char timeText[20];

    do {
        getTodayDate(date, sizeof(date));
        getCurrentTime(timeText, sizeof(timeText));

        heading("HOSPITAL TOKEN BOOKING AND QUEUE MANAGEMENT");

        printf("Date: %s\n", date);
        printf("Time: %s\n\n", timeText);

        printf("1. Patient Portal\n");
        printf("2. Receptionist Portal\n");
        printf("0. Exit System\n");

        choice = readInt("\nEnter choice: ");

        switch (choice) {
            case 1:
                patientMenu();
                break;

            case 2:
                receptionistMenu();
                break;

            case 0:
                saveData();
                printf("System data saved. Goodbye.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 0);
}

/* ============================================================
   PROGRAM ENTRY POINT
   ============================================================ */

int main(void) {
    loadPIN();
    loadData();

    mainMenu();

    saveData();
    destroyRecords();

    return 0;
}
