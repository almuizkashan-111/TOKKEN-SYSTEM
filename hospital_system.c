
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define HOSPITAL_COUNT 8
#define DEPARTMENT_COUNT 4
#define DOCTORS_PER_DEPT 2
#define NAME_SIZE 60
#define SYMPTOM_SIZE 200
#define DATE_SIZE 30
#define DATA_FILE "patients.dat"
#define PIN_FILE "receptionist_pin.dat"
#define SERVICE_MINUTES 15

enum {
    WAITING = 1,
    IN_CONSULTATION,
    COMPLETED,
    CANCELLED
};

typedef struct Patient {
    int token;
    char name[NAME_SIZE];
    char source[20];
    int hospital;
    int department;
    int doctor;
    char symptoms[SYMPTOM_SIZE];
    char bookingDate[DATE_SIZE];
    int emergency;
    int status;
    time_t bookingTime;
    time_t statusTime;
    struct Patient *next;
} Patient;

/* ======================== GLOBAL DATA ======================== */

char hospitals[HOSPITAL_COUNT][80] = {
    "City General Hospital",
    "Community Clinic",
    "Bone and Joint Hospital",
    "Sunrise Multispeciality Hospital",
    "Green Valley Medical Centre",
    "Metro ENT Hospital",
    "Aarogya Skin and Wellness Centre",
    "Northside Family Hospital"
};

char departments[DEPARTMENT_COUNT][40] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

char doctors[DEPARTMENT_COUNT][DOCTORS_PER_DEPT][40] = {
    {"Dr. A. Sharma", "Dr. R. Mehta"},
    {"Dr. P. Verma", "Dr. S. Kapoor"},
    {"Dr. K. Singh", "Dr. M. Joshi"},
    {"Dr. N. Gupta", "Dr. T. Rao"}
};

Patient *head = NULL;
int nextToken = 1;

/* ======================== INPUT FUNCTIONS ======================== */

void readLine(const char *message, char *buffer, int size) {
    printf("%s", message);

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

int readInt(const char *message, int min, int max) {
    char buffer[100];
    char *end;
    long value;

    while (1) {
        readLine(message, buffer, sizeof(buffer));

        value = strtol(buffer, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (buffer[0] != '\0' && *end == '\0' &&
            value >= min && value <= max) {
            return (int)value;
        }

        printf("Invalid input. Enter a number between %d and %d.\n",
               min, max);
    }
}

void pauseScreen(void) {
    char temp[10];
    printf("\nPress ENTER to continue...");
    fgets(temp, sizeof(temp), stdin);
}

/* ======================== DATE FUNCTIONS ======================== */

void getTodayDate(char *buffer, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t != NULL) {
        strftime(buffer, size, "%d-%m-%Y", t);
    } else {
        strcpy(buffer, "Unknown");
    }
}

void getCurrentDateTime(char *buffer, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t != NULL) {
        strftime(buffer, size, "%d-%m-%Y %H:%M", t);
    } else {
        strcpy(buffer, "Unknown");
    }
}

/* Supports records saved by the earlier version too:
   "16-11-2026" and "16-11-2026 10:30". */
int isToday(Patient *p) {
    char today[DATE_SIZE];

    getTodayDate(today, sizeof(today));

    return strncmp(p->bookingDate, today, 10) == 0;
}

/* ======================== PATIENT FILE STORAGE ======================== */

void saveData(void) {
    FILE *file = fopen(DATA_FILE, "wb");

    if (file == NULL) {
        printf("\nWarning: Unable to save patient records.\n");
        return;
    }

    fwrite(&nextToken, sizeof(int), 1, file);

    Patient *p = head;

    while (p != NULL) {
        fwrite(p, sizeof(Patient), 1, file);
        p = p->next;
    }

    fclose(file);
}

void loadData(void) {
    FILE *file = fopen(DATA_FILE, "rb");

    if (file == NULL) {
        nextToken = 1;
        return;
    }

    int savedCounter = 1;

    if (fread(&savedCounter, sizeof(int), 1, file) != 1) {
        fclose(file);
        nextToken = 1;
        return;
    }

    Patient temp;

    while (fread(&temp, sizeof(Patient), 1, file) == 1) {
        Patient *p = (Patient *)malloc(sizeof(Patient));

        if (p == NULL) {
            printf("Memory allocation failed while loading records.\n");
            break;
        }

        *p = temp;
        p->next = NULL;

        if (head == NULL) {
            head = p;
        } else {
            Patient *last = head;

            while (last->next != NULL) {
                last = last->next;
            }

            last->next = p;
        }
    }

    fclose(file);

    /*
       DAILY TOKEN RENEWAL:
       Find the highest token number issued today.
       Old records remain stored but do not affect today's counter.
    */

    int maximumToday = 0;
    Patient *p = head;

    while (p != NULL) {
        if (isToday(p) && p->token > maximumToday) {
            maximumToday = p->token;
        }

        p = p->next;
    }

    nextToken = maximumToday + 1;

    if (nextToken < 1) {
        nextToken = 1;
    }

    (void)savedCounter;
}

/* ======================== RECEPTIONIST PIN ======================== */

unsigned long hashPIN(const char *pin) {
    unsigned long hash = 0;

    for (int i = 0; pin[i] != '\0'; i++) {
        hash = hash * 31 + (unsigned char)pin[i];
    }

    return hash;
}

void saveDefaultPIN(void) {
    FILE *file = fopen(PIN_FILE, "wb");

    if (file == NULL) {
        return;
    }

    unsigned long hash = hashPIN("1234");

    fwrite(&hash, sizeof(unsigned long), 1, file);
    fclose(file);
}

int verifyPIN(const char *pin) {
    FILE *file = fopen(PIN_FILE, "rb");

    if (file == NULL) {
        saveDefaultPIN();
        file = fopen(PIN_FILE, "rb");

        if (file == NULL) {
            return 0;
        }
    }

    unsigned long savedHash = 0;

    if (fread(&savedHash, sizeof(unsigned long), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    fclose(file);

    return savedHash == hashPIN(pin);
}

void changePIN(void) {
    char oldPIN[30];
    char newPIN[30];
    char confirmPIN[30];

    printf("\n========== CHANGE RECEPTIONIST PIN ==========\n");

    readLine("Enter current PIN: ", oldPIN, sizeof(oldPIN));

    if (!verifyPIN(oldPIN)) {
        printf("Incorrect PIN.\n");
        return;
    }

    readLine("Enter new PIN: ", newPIN, sizeof(newPIN));

    if (strlen(newPIN) < 4) {
        printf("PIN must contain at least 4 characters.\n");
        return;
    }

    readLine("Confirm new PIN: ", confirmPIN, sizeof(confirmPIN));

    if (strcmp(newPIN, confirmPIN) != 0) {
        printf("PIN confirmation does not match.\n");
        return;
    }

    FILE *file = fopen(PIN_FILE, "wb");

    if (file == NULL) {
        printf("Unable to update PIN.\n");
        return;
    }

    unsigned long hash = hashPIN(newPIN);

    fwrite(&hash, sizeof(unsigned long), 1, file);
    fclose(file);

    printf("PIN changed successfully.\n");
}

/* ======================== DISPLAY FUNCTIONS ======================== */

void showHospitals(void) {
    printf("\n=============== AVAILABLE HOSPITALS ===============\n");

    for (int i = 0; i < HOSPITAL_COUNT; i++) {
        printf("%d. %s\n", i + 1, hospitals[i]);
    }
}

void showDepartments(void) {
    printf("\n=============== AVAILABLE DEPARTMENTS ===============\n");

    for (int i = 0; i < DEPARTMENT_COUNT; i++) {
        printf("%d. %s\n", i + 1, departments[i]);
    }
}

void showDoctors(int department) {
    printf("\n=============== AVAILABLE DOCTORS ===============\n");

    for (int i = 0; i < DOCTORS_PER_DEPT; i++) {
        printf("%d. %s\n", i + 1, doctors[department][i]);
    }
}

void showPatient(Patient *p) {
    if (p == NULL) {
        return;
    }

    printf("\n---------------------------------------------\n");
    printf("Token Number      : %d\n", p->token);
    printf("Patient Name      : %s\n", p->name);
    printf("Registration Type : %s\n", p->source);
    printf("Hospital          : %s\n", hospitals[p->hospital]);
    printf("Department        : %s\n", departments[p->department]);
    printf("Assigned Doctor   : %s\n",
           doctors[p->department][p->doctor]);
    printf("Symptoms          : %s\n", p->symptoms);
    printf("Booking Date      : %s\n", p->bookingDate);
    printf("Priority          : %s\n",
           p->emergency ? "Staff-marked priority" : "Normal");

    printf("Status            : ");

    switch (p->status) {
        case WAITING:
            printf("Waiting\n");
            break;
        case IN_CONSULTATION:
            printf("In Consultation\n");
            break;
        case COMPLETED:
            printf("Completed\n");
            break;
        case CANCELLED:
            printf("Cancelled\n");
            break;
        default:
            printf("Unknown\n");
    }

    printf("---------------------------------------------\n");
}

/* ======================== TOKEN SEARCH ======================== */

/*
   Token numbers renew daily. Therefore, search today's record first.
   If it is not found, search older records and return the latest match.
*/

Patient *findToken(int token) {
    Patient *p = head;
    Patient *oldRecord = NULL;

    while (p != NULL) {
        if (p->token == token) {
            if (isToday(p)) {
                return p;
            }

            oldRecord = p;
        }

        p = p->next;
    }

    return oldRecord;
}

/* ======================== QUEUE CALCULATIONS ======================== */

int countWaiting(int hospital, int department, int doctor) {
    int count = 0;
    Patient *p = head;

    while (p != NULL) {
        if (isToday(p) &&
            p->hospital == hospital &&
            p->department == department &&
            p->doctor == doctor &&
            p->status == WAITING) {
            count++;
        }

        p = p->next;
    }

    return count;
}

int countAhead(Patient *target) {
    if (target == NULL || !isToday(target)) {
        return 0;
    }

    int count = 0;
    Patient *p = head;

    while (p != NULL) {
        if (p != target &&
            isToday(p) &&
            p->hospital == target->hospital &&
            p->department == target->department &&
            p->doctor == target->doctor &&
            p->status == WAITING) {

            if (target->emergency == 0 || p->emergency == 1) {
                count++;
            }
        }

        p = p->next;
    }

    return count;
}

int getWaitingPosition(Patient *target) {
    if (target == NULL || target->status != WAITING ||
        !isToday(target)) {
        return 0;
    }

    return countAhead(target) + 1;
}

/* ======================== PRIORITY QUEUE ======================== */

/*
   Emergency-priority patients are selected before normal patients.
   Within each priority group, the earlier token is called first.
*/

Patient *getNextPatient(int hospital, int department, int doctor) {
    Patient *p = head;
    Patient *selected = NULL;

    while (p != NULL) {
        if (isToday(p) &&
            p->hospital == hospital &&
            p->department == department &&
            p->doctor == doctor &&
            p->status == WAITING) {

            if (selected == NULL ||
                p->emergency > selected->emergency ||
                (p->emergency == selected->emergency &&
                 p->token < selected->token)) {
                selected = p;
            }
        }

        p = p->next;
    }

    return selected;
}

void showQueue(int hospital, int department, int doctor) {
    printf("\n=============== WAITING QUEUE ===============\n");
    printf("Hospital   : %s\n", hospitals[hospital]);
    printf("Department : %s\n", departments[department]);
    printf("Doctor     : %s\n", doctors[department][doctor]);

    if (countWaiting(hospital, department, doctor) == 0) {
        printf("\nNo patients are waiting today.\n");
        return;
    }

    printf("\n%-10s %-22s %-15s %-10s\n",
           "Token", "Patient", "Priority", "Position");

    int position = 1;

    for (int priority = 1; priority >= 0; priority--) {
        Patient *p = head;

        while (p != NULL) {
            if (isToday(p) &&
                p->hospital == hospital &&
                p->department == department &&
                p->doctor == doctor &&
                p->status == WAITING &&
                p->emergency == priority) {

                printf("%-10d %-22.22s %-15s %-10d\n",
                       p->token,
                       p->name,
                       p->emergency ? "Priority" : "Normal",
                       position);

                position++;
            }

            p = p->next;
        }
    }
}

/* ======================== CONSULTATION MANAGEMENT ======================== */

void callNextPatient(int hospital, int department, int doctor) {
    Patient *p = head;

    while (p != NULL) {
        if (isToday(p) &&
            p->hospital == hospital &&
            p->department == department &&
            p->doctor == doctor &&
            p->status == IN_CONSULTATION) {
            printf("\nThis doctor already has an active consultation.\n");
            return;
        }

        p = p->next;
    }

    p = getNextPatient(hospital, department, doctor);

    if (p == NULL) {
        printf("\nNo waiting patients for this doctor.\n");
        return;
    }

    p->status = IN_CONSULTATION;
    p->statusTime = time(NULL);

    saveData();

    printf("\n========== PATIENT CALLED ==========\n");
    printf("Token Number : %d\n", p->token);
    printf("Patient      : %s\n", p->name);
    printf("Doctor       : %s\n", doctors[department][doctor]);
    printf("Priority     : %s\n",
           p->emergency ? "Priority" : "Normal");
}

void completeConsultation(int hospital, int department, int doctor) {
    Patient *p = head;

    while (p != NULL) {
        if (isToday(p) &&
            p->hospital == hospital &&
            p->department == department &&
            p->doctor == doctor &&
            p->status == IN_CONSULTATION) {

            p->status = COMPLETED;
            p->statusTime = time(NULL);

            saveData();

            printf("\nConsultation completed successfully.\n");
            printf("Patient: %s\n", p->name);
            printf("Token: %d\n", p->token);
            return;
        }

        p = p->next;
    }

    printf("\nNo active consultation found for this doctor.\n");
}

/* ======================== PATIENT REGISTRATION ======================== */

void registerPatient(int walkIn) {
    if (nextToken > 999999) {
        printf("\nDaily token limit reached.\n");
        return;
    }

    Patient *p = (Patient *)calloc(1, sizeof(Patient));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    printf("\n========== PATIENT REGISTRATION ==========\n");

    readLine("Enter patient name: ", p->name, sizeof(p->name));

    if (strlen(p->name) == 0) {
        printf("Patient name cannot be empty.\n");
        free(p);
        return;
    }

    showHospitals();

    p->hospital = readInt("Select hospital: ",
                          1, HOSPITAL_COUNT) - 1;

    showDepartments();

    p->department = readInt("Select department: ",
                            1, DEPARTMENT_COUNT) - 1;

    showDoctors(p->department);

    p->doctor = readInt("Select doctor: ",
                        1, DOCTORS_PER_DEPT) - 1;

    readLine("Enter symptoms or reason for visit: ",
             p->symptoms, sizeof(p->symptoms));

    if (strlen(p->symptoms) == 0) {
        strcpy(p->symptoms, "Not specified");
    }

    strcpy(p->source, walkIn ? "Walk-in" : "Online");

    p->emergency = 0;

    /*
       Only reception staff can mark a walk-in registration
       as priority. Patient self-booking cannot set priority.
    */

    if (walkIn) {
        p->emergency = readInt(
            "Mark as staff-priority case? (1=Yes, 0=No): ", 0, 1
        );
    }

    p->token = nextToken++;
    p->status = WAITING;
    p->bookingTime = time(NULL);
    p->statusTime = p->bookingTime;

    getTodayDate(p->bookingDate, sizeof(p->bookingDate));

    p->next = NULL;

    if (head == NULL) {
        head = p;
    } else {
        Patient *last = head;

        while (last->next != NULL) {
            last = last->next;
        }

        last->next = p;
    }

    saveData();

    printf("\n========== REGISTRATION SUCCESSFUL ==========\n");
    printf("Token Number : %d\n", p->token);
    printf("Patient      : %s\n", p->name);
    printf("Hospital     : %s\n", hospitals[p->hospital]);
    printf("Department   : %s\n", departments[p->department]);
    printf("Doctor       : %s\n", doctors[p->department][p->doctor]);
    printf("Priority     : %s\n",
           p->emergency ? "Staff-marked priority" : "Normal");

    printf("\nPlease keep your token number safe.\n");
}

/* ======================== MULTIPLE WALK-IN REGISTRATION ======================== */

void multipleWalkInRegistration(void) {
    int number = readInt(
        "\nHow many patients do you want to register? ", 1, 50);

    for (int i = 0; i < number; i++) {
        printf("\n========== PATIENT %d OF %d ==========\n",
               i + 1, number);

        registerPatient(1);
    }
}

/* ======================== TOKEN DETAILS ======================== */

void showTokenDetails(void) {
    int token = readInt("\nEnter token number: ", 1, 999999);

    Patient *p = findToken(token);

    if (p == NULL) {
        printf("\nToken not found.\n");
        return;
    }

    showPatient(p);

    if (p->status == WAITING && isToday(p)) {
        int ahead = countAhead(p);

        printf("Waiting position: %d\n", ahead + 1);
        printf("Patients ahead: %d\n", ahead);
        printf("Estimated waiting time: %d minutes\n",
               ahead * SERVICE_MINUTES);

        printf("\nWaiting time is only an estimate.\n");
    }
}

void showPatientStatus(void) {
    int token = readInt("\nEnter your token number: ", 1, 999999);

    Patient *p = findToken(token);

    if (p == NULL) {
        printf("\nPatient token not found.\n");
        return;
    }

    printf("\n========== TOKEN STATUS ==========\n");

    printf("Token: %d\n", p->token);

    if (!isToday(p)) {
        printf("This token belongs to an earlier date.\n");
    }

    if (p->status == WAITING) {
        printf("Status: Waiting\n");

        if (isToday(p)) {
            int ahead = countAhead(p);
            printf("Queue position: %d\n", ahead + 1);
            printf("Estimated waiting time: %d minutes\n",
                   ahead * SERVICE_MINUTES);
        }
    } else if (p->status == IN_CONSULTATION) {
        printf("Status: In consultation\n");
    } else if (p->status == COMPLETED) {
        printf("Status: Consultation completed\n");
    } else if (p->status == CANCELLED) {
        printf("Status: Token cancelled\n");
    }
}

/* ======================== EMERGENCY PRIORITY MANAGEMENT ======================== */

void markEmergencyPriority(void) {
    int token = readInt("\nEnter today's patient token: ",
                        1, 999999);

    Patient *p = findToken(token);

    if (p == NULL || !isToday(p)) {
        printf("\nToday's patient token was not found.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("\nOnly waiting patients can be marked priority.\n");
        return;
    }

    if (p->emergency == 1) {
        printf("\nThis patient is already marked priority.\n");
        return;
    }

    printf("\nConfirm staff assessment before changing priority.\n");

    int confirm = readInt(
        "Mark this patient as priority? (1=Yes, 0=No): ", 0, 1
    );

    if (confirm == 1) {
        p->emergency = 1;
        saveData();

        printf("\nPriority successfully updated.\n");
        printf("Token: %d\n", p->token);
        printf("Patient: %s\n", p->name);
        printf("The patient will be considered before normal queue entries.\n");
    } else {
        printf("\nNo changes made.\n");
    }
}

/* ======================== CANCELLATION ======================== */

void cancelToken(void) {
    int token = readInt("\nEnter token number to cancel: ",
                        1, 999999);

    Patient *p = findToken(token);

    if (p == NULL || !isToday(p)) {
        printf("\nToday's patient token was not found.\n");
        return;
    }

    if (p->status != WAITING) {
        printf("\nOnly waiting tokens can be cancelled.\n");
        return;
    }

    p->status = CANCELLED;
    p->statusTime = time(NULL);

    saveData();

    printf("\nToken %d cancelled successfully.\n", token);
}

/* ======================== STATISTICS ======================== */

void showStatistics(void) {
    int total = 0;
    int waiting = 0;
    int active = 0;
    int completed = 0;
    int cancelled = 0;
    int priority = 0;

    Patient *p = head;

    while (p != NULL) {
        if (isToday(p)) {
            total++;

            if (p->status == WAITING) {
                waiting++;
            } else if (p->status == IN_CONSULTATION) {
                active++;
            } else if (p->status == COMPLETED) {
                completed++;
            } else if (p->status == CANCELLED) {
                cancelled++;
            }

            if (p->emergency) {
                priority++;
            }
        }

        p = p->next;
    }

    printf("\n=============== TODAY'S STATISTICS ===============\n");
    printf("Date                       : ");

    char today[DATE_SIZE];
    getTodayDate(today, sizeof(today));
    printf("%s\n", today);

    printf("Total registered patients : %d\n", total);
    printf("Waiting patients          : %d\n", waiting);
    printf("Active consultations      : %d\n", active);
    printf("Completed consultations   : %d\n", completed);
    printf("Cancelled tokens          : %d\n", cancelled);
    printf("Priority cases             : %d\n", priority);
    printf("Next token number          : %d\n", nextToken);
}

void showHospitalStatistics(void) {
    printf("\n=============== HOSPITAL-WISE STATISTICS ===============\n");

    for (int h = 0; h < HOSPITAL_COUNT; h++) {
        int total = 0;
        int waiting = 0;
        int completed = 0;

        Patient *p = head;

        while (p != NULL) {
            if (isToday(p) && p->hospital == h) {
                total++;

                if (p->status == WAITING) {
                    waiting++;
                }

                if (p->status == COMPLETED) {
                    completed++;
                }
            }

            p = p->next;
        }

        printf("\n%s\n", hospitals[h]);
        printf("Registered: %d | Waiting: %d | Completed: %d\n",
               total, waiting, completed);
    }
}

/* ======================== PATIENT HISTORY ======================== */

void showPatientHistory(void) {
    int token = readInt(
        "\nEnter token number (0 to show all history): ",
        0, 999999
    );

    Patient *p = head;
    int found = 0;

    printf("\n=============== PATIENT HISTORY ===============\n");

    while (p != NULL) {
        if (token == 0 || p->token == token) {
            showPatient(p);
            found++;
        }

        p = p->next;
    }

    if (found == 0) {
        printf("No matching records found.\n");
    }

    printf("\nTotal records displayed: %d\n", found);
}

/* ======================== QUEUE MANAGEMENT MENU ======================== */

void selectQueue(void) {
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
        printf("\n========== QUEUE MANAGEMENT ==========\n");
        printf("Hospital   : %s\n", hospitals[hospital]);
        printf("Department : %s\n", departments[department]);
        printf("Doctor     : %s\n", doctors[department][doctor]);

        printf("\n1. Show waiting queue\n");
        printf("2. Call next patient\n");
        printf("3. Complete consultation\n");
        printf("4. Return to receptionist menu\n");

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
                printf("Returning to receptionist menu.\n");
                break;
        }

        if (choice != 4) {
            pauseScreen();
        }

    } while (choice != 4);
}

/* ======================== DEPARTMENT SUGGESTION ======================== */

void symptomSuggestion(void) {
    char symptom[SYMPTOM_SIZE];

    printf("\n========== DEPARTMENT INFORMATION HELPER ==========\n");

    readLine("Enter general reason for visit: ",
             symptom, sizeof(symptom));

    for (int i = 0; symptom[i] != '\0'; i++) {
        symptom[i] = (char)tolower((unsigned char)symptom[i]);
    }

    printf("\nPossible department information:\n");

    if (strstr(symptom, "ear") ||
        strstr(symptom, "nose") ||
        strstr(symptom, "throat")) {

        printf("ENT may be relevant.\n");

    } else if (strstr(symptom, "bone") ||
               strstr(symptom, "joint") ||
               strstr(symptom, "fracture")) {

        printf("Orthopaedics may be relevant.\n");

    } else if (strstr(symptom, "skin") ||
               strstr(symptom, "rash") ||
               strstr(symptom, "acne")) {

        printf("Dermatology may be relevant.\n");

    } else {
        printf("General Medicine may be a starting point.\n");
    }

    printf("\nThis is an information helper, not a medical diagnosis.\n");
    printf("For urgent concerns, seek immediate medical assistance.\n");
}

/* ======================== PATIENT HELP AND FAQ ======================== */

void patientHelp(void) {
    printf("\n=============== PATIENT HELP / FAQ ===============\n");

    printf("\n1. How do I book a token?\n");
    printf("   Select Book Token and enter your details.\n");

    printf("\n2. How do I check my queue position?\n");
    printf("   Select Check Token Status and enter your token.\n");

    printf("\n3. Can I cancel a token?\n");
    printf("   Contact reception for assistance.\n");

    printf("\n4. Is the waiting time exact?\n");
    printf("   No. It is an approximate estimate.\n");

    printf("\n5. What if I have an urgent medical concern?\n");
    printf("   Seek immediate medical assistance. Do not wait for a token.\n");

    printf("\n6. Do tokens renew every day?\n");
    printf("   Yes. The token counter starts from 1 on a new date.\n");

    printf("\n7. Are previous records deleted?\n");
    printf("   No. Previous records remain available in patient history.\n");
}

/* ======================== PATIENT GUIDELINES ======================== */

void patientGuidelines(void) {
    printf("\n=============== PATIENT GUIDELINES ===============\n");

    printf("1. Enter your correct name and booking details.\n");
    printf("2. Select the appropriate hospital and department.\n");
    printf("3. Keep your token number safe.\n");
    printf("4. Check your token status before visiting the counter.\n");
    printf("5. Waiting time is an estimate and may change.\n");
    printf("6. Contact reception if you need help.\n");
    printf("7. Do not wait for a token if you need urgent medical care.\n");
}

/* ======================== PATIENT PORTAL ======================== */

void patientMenu(void) {
    int choice;

    do {
        printf("\n========================================\n");
        printf("             PATIENT PORTAL\n");
        printf("========================================\n");

        printf("1. Book a token\n");
        printf("2. Check token status\n");
        printf("3. View token details\n");
        printf("4. Department information helper\n");
        printf("5. Patient help / FAQ\n");
        printf("6. Patient guidelines\n");
        printf("7. Back to main menu\n");

        choice = readInt("Enter choice: ", 1, 7);

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
                symptomSuggestion();
                break;

            case 5:
                patientHelp();
                break;

            case 6:
                patientGuidelines();
                break;

            case 7:
                printf("Returning to main menu.\n");
                break;
        }

        if (choice != 7) {
            pauseScreen();
        }

    } while (choice != 7);
}

/* ======================== RECEPTIONIST LOGIN ======================== */

int receptionistLogin(void) {
    char pin[30];

    printf("\n========== RECEPTIONIST LOGIN ==========\n");

    for (int attempt = 1; attempt <= 3; attempt++) {
        readLine("Enter receptionist PIN: ", pin, sizeof(pin));

        if (verifyPIN(pin)) {
            printf("\nLogin successful.\n");
            return 1;
        }

        printf("Incorrect PIN. Attempts remaining: %d\n",
               3 - attempt);
    }

    printf("\nAccess denied.\n");
    return 0;
}

/* ======================== RECEPTIONIST PORTAL ======================== */

void receptionistMenu(void) {
    if (!receptionistLogin()) {
        return;
    }

    int choice;

    do {
        printf("\n========================================\n");
        printf("          RECEPTIONIST PORTAL\n");
        printf("========================================\n");

        printf("1. Register walk-in patient\n");
        printf("2. Register multiple walk-in patients\n");
        printf("3. Manage doctor queues\n");
        printf("4. Search token details\n");
        printf("5. Cancel waiting token\n");
        printf("6. Mark patient as emergency priority\n");
        printf("7. View today's statistics\n");
        printf("8. View hospital-wise statistics\n");
        printf("9. View patient history\n");
        printf("10. Change receptionist PIN\n");
        printf("11. Back to main menu\n");

        choice = readInt("Enter choice: ", 1, 11);

        switch (choice) {
            case 1:
                registerPatient(1);
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
                printf("Logging out.\n");
                break;
        }

        if (choice != 11) {
            pauseScreen();
        }

    } while (choice != 11);
}

/* ======================== MEMORY CLEANUP ======================== */

void destroySystem(void) {
    Patient *p = head;

    while (p != NULL) {
        Patient *temp = p;
        p = p->next;
        free(temp);
    }

    head = NULL;
}

/* ======================== MAIN FUNCTION ======================== */

int main(void) {
    loadData();

    printf("\n");
    printf("===============================================\n");
    printf("    HOSPITAL TOKEN BOOKING AND QUEUE SYSTEM\n");
    printf("===============================================\n");

    printf("\nWelcome to the Hospital Token Management System.\n");

    char today[DATE_SIZE];
    getTodayDate(today, sizeof(today));

    printf("Today's date: %s\n", today);
    printf("Next available token: %d\n", nextToken);

    int choice;

    do {
        printf("\n=============== MAIN MENU ===============\n");

        printf("1. Patient Portal\n");
        printf("2. Receptionist Portal\n");
        printf("3. Exit\n");

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
                printf("\nThank you for using the system.\n");
                printf("Patient records have been saved.\n");
                break;
        }

    } while (choice != 3);

    destroySystem();

    return 0;
}
