/* ==================== C_Version/hospital.h ==================== */
#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME_SIZE 31
#define SOURCE_SIZE 20
#define SYMPTOM_SIZE 200
#define INPUT_SIZE 256
#define MAX_TOKENS 1000
#define HOSPITAL_COUNT 8
#define DEPARTMENT_COUNT 4

typedef struct {
    char name[60];
    char type[80];
    int available[DEPARTMENT_COUNT];
} Hospital;

extern const char *departmentNames[DEPARTMENT_COUNT];
extern Hospital hospitals[HOSPITAL_COUNT];

typedef struct {
    int token;
    char name[NAME_SIZE];
    char source[SOURCE_SIZE];
    int hospital;
    int department;
    char symptoms[SYMPTOM_SIZE];
    char bookingDate[11];
} Patient;

typedef struct Node {
    Patient data;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *rear;
    int count;
} PatientQueue;

typedef struct {
    PatientQueue waitingQueue;
    Patient currentPatient;
    int consultationActive;
    int generatedCount;
    int completedCount;
    int cancelledCount;
} DepartmentState;

typedef struct {
    DepartmentState departments[HOSPITAL_COUNT][DEPARTMENT_COUNT];
    int lastToken;
    char currentDate[11];
} HospitalSystem;

void initializeQueue(PatientQueue *queue);
int isEmpty(const PatientQueue *queue);
int size(const PatientQueue *queue);
int enqueue(PatientQueue *queue, const Patient *patient);
int dequeue(PatientQueue *queue, Patient *patient);
void display(const PatientQueue *queue);
int findToken(const PatientQueue *queue, int token, Patient *patient);
int getQueuePosition(const PatientQueue *queue, int token);
int cancelToken(PatientQueue *queue, int token);
void destroyQueue(PatientQueue *queue);

int isNameWhitespace(char ch);
int validatePatientName(const char *input, char name[]);
int isDepartmentAvailable(int h, int d);
void initializeHospital(HospitalSystem *hospital);
void destroyHospital(HospitalSystem *hospital);
void printPatient(const Patient *patient);
void createBooking(HospitalSystem *hospital, const char *name,
                   const char *source, int h, int d, const char *symptoms);
void bookSelf(HospitalSystem *hospital, const char *name,
              int h, int d, const char *symptoms);
void bookWalkIn(HospitalSystem *hospital, const char *name,
                int h, int d, const char *symptoms);
void showStatus(const HospitalSystem *hospital, int h, int d);
void showWaitingQueue(const HospitalSystem *hospital, int h, int d);
void callNextPatient(HospitalSystem *hospital, int h, int d);
void completeConsultation(HospitalSystem *hospital, int h, int d);
int locateToken(const HospitalSystem *hospital, int token, int *h, int *d);
void showTokenDetails(const HospitalSystem *hospital, int token);
void cancelWaitingToken(HospitalSystem *hospital, int h, int d, int token);
void showStatistics(const HospitalSystem *hospital, int h, int d);
void showOverallStatistics(const HospitalSystem *hospital);

int readLine(char text[], int capacity);
int readChoice(int maximum);
int readPatientName(char name[]);
int readTokenNumber(void);
int confirmAction(const char *question);
int readSymptoms(char symptoms[]);
int selectHospital(void);
int selectDepartment(void);
int selectHospitalDepartment(int h);
int selectMatchingHospital(int department);
int suggestDepartment(void);
int selectBookingLocation(int *h, int *d);
void showBookingPreview(const char *name, const char *source,
                        int h, int d, const char *symptoms);
void bookPatientUI(HospitalSystem *hospital);
void viewPatientQueueStatusUI(const HospitalSystem *hospital);
void trackTokenUI(const HospitalSystem *hospital);
void showMyQueueOverviewUI(const HospitalSystem *hospital);
void showBookingHelp(void);
void showPatientFAQ(void);
void showPatientGuidelines(void);
void patientMenu(HospitalSystem *hospital);

int selectReceptionQueue(int *h, int *d);
void bookWalkInUI(HospitalSystem *hospital, int h, int d);
void registerMultipleWalkInsUI(HospitalSystem *hospital, int h, int d);
void showStatusUI(const HospitalSystem *hospital, int h, int d);
void showWaitingQueueUI(const HospitalSystem *hospital, int h, int d);
void callNextPatientUI(HospitalSystem *hospital, int h, int d);
void completeConsultationUI(HospitalSystem *hospital, int h, int d);
void showTokenDetailsUI(const HospitalSystem *hospital);
void cancelWaitingTokenUI(HospitalSystem *hospital, int h, int d);
void showStatisticsUI(const HospitalSystem *hospital, int h, int d);
void showShiftSummary(const HospitalSystem *hospital, int h, int d);
void showReceptionistHelp(void);
void receptionistMenu(HospitalSystem *hospital);

#endif

/* ==================== C_Version/main.c ==================== */
#include "hospital.h"

int main(void)
{
    HospitalSystem hospital;
    int choice;

    initializeHospital(&hospital);

    while (1)
    {
        printf("\n========================================\n");
        printf("      HOSPITAL TOKEN BOOKING SYSTEM\n");
        printf("      Booking date: %s\n", hospital.currentDate);
        printf("      Daily token sequence starts at 1\n");
        printf("========================================\n");
        printf("1. Patient portal\n");
        printf("2. Receptionist portal\n");
        printf("0. Exit\n");

        choice = readChoice(2);

        switch (choice)
        {
            case 1:
                patientMenu(&hospital);
                break;

            case 2:
                receptionistMenu(&hospital);
                break;

            case 0:
                destroyHospital(&hospital);
                printf("\nApplication closed.\n");
                return 0;
        }
    }
}

/* ==================== C_Version/backend1.c ==================== */
#include "hospital.h"

void initializeQueue(PatientQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

int isEmpty(const PatientQueue *queue)
{
    return queue->front == NULL;
}

int size(const PatientQueue *queue)
{
    return queue->count;
}

int enqueue(PatientQueue *queue, const Patient *patient)
{
    Node *p = malloc(sizeof(Node));

    if (p == NULL)
        return 0;

    p->data = *patient;
    p->next = NULL;

    if (isEmpty(queue))
    {
        queue->front = p;
        queue->rear = p;
    }
    else
    {
        queue->rear->next = p;
        queue->rear = p;
    }

    queue->count++;
    return 1;
}

int dequeue(PatientQueue *queue, Patient *patient)
{
    Node *p;

    if (isEmpty(queue))
        return 0;

    p = queue->front;
    *patient = p->data;

    queue->front = p->next;

    if (queue->front == NULL)
        queue->rear = NULL;

    free(p);
    queue->count--;

    return 1;
}

void display(const PatientQueue *queue)
{
    const Node *p = queue->front;
    int position = 1;

    if (isEmpty(queue))
    {
        printf("\nNo patients are waiting.\n");
        return;
    }

    printf("\n========== WAITING QUEUE ==========\n");

    while (p != NULL)
    {
        printf("Position: %d", position);
        printf(" | Token: %d", p->data.token);
        printf(" | Name: %s", p->data.name);
        printf(" | Source: %s\n", p->data.source);

        p = p->next;
        position++;
    }
}

int findToken(const PatientQueue *queue, int token, Patient *patient)
{
    const Node *p = queue->front;

    while (p != NULL)
    {
        if (p->data.token == token)
        {
            *patient = p->data;
            return 1;
        }

        p = p->next;
    }

    return 0;
}

int getQueuePosition(const PatientQueue *queue, int token)
{
    const Node *p = queue->front;
    int position = 1;

    while (p != NULL)
    {
        if (p->data.token == token)
            return position;

        p = p->next;
        position++;
    }

    return -1;
}

int cancelToken(PatientQueue *queue, int token)
{
    Node *p = queue->front;
    Node *previous = NULL;

    while (p != NULL)
    {
        if (p->data.token == token)
        {
            if (previous == NULL)
                queue->front = p->next;
            else
                previous->next = p->next;

            if (p == queue->rear)
                queue->rear = previous;

            free(p);
            queue->count--;

            return 1;
        }

        previous = p;
        p = p->next;
    }

    return 0;
}

void destroyQueue(PatientQueue *queue)
{
    Patient patient;

    while (dequeue(queue, &patient))
    {
        /* dequeue frees each node. */
    }
}

/* ==================== C_Version/backend2.c ==================== */
#include "hospital.h"

const char *departmentNames[DEPARTMENT_COUNT] = {
    "General Medicine",
    "ENT",
    "Orthopaedics",
    "Dermatology"
};

Hospital hospitals[HOSPITAL_COUNT] = {
    {"City General Hospital", "General / multispeciality hospital", {1, 1, 1, 1}},
    {"Community Clinic", "General medicine clinic", {1, 0, 0, 0}},
    {"Bone and Joint Hospital", "Orthopaedic hospital", {0, 0, 1, 0}},
    {"Sunrise Multispeciality Hospital", "Multispeciality hospital", {1, 1, 1, 1}},
    {"Green Valley Medical Centre", "General medicine and skin care", {1, 0, 0, 1}},
    {"Metro ENT Hospital", "Ear, nose and throat care", {1, 1, 0, 0}},
    {"Aarogya Skin and Wellness Centre", "Dermatology clinic", {1, 0, 0, 1}},
    {"Northside Family Hospital", "Family and general care", {1, 1, 1, 0}}
};

int isNameWhitespace(char ch)
{
    return ch == ' ' || ch == '\t' ||
           ch == '\r' || ch == '\n';
}

int validatePatientName(const char *input, char name[])
{
    size_t first = 0;
    size_t last = strlen(input);
    size_t length;
    size_t i;

    while (first < last && isNameWhitespace(input[first]))
        first++;

    while (last > first && isNameWhitespace(input[last - 1]))
        last--;

    length = last - first;

    if (length == 0)
    {
        printf("Name cannot be empty.\n");
        return 0;
    }

    if (length >= NAME_SIZE)
    {
        printf("Name must be at most 30 bytes long.\n");
        return 0;
    }

    for (i = first; i < last; i++)
    {
        unsigned char ch = (unsigned char)input[i];

        if (ch < 32 || ch == 127)
        {
            printf("Name contains an invalid control character.\n");
            return 0;
        }
    }

    memcpy(name, input + first, length);
    name[length] = '\0';

    return 1;
}

int isDepartmentAvailable(int h, int d)
{
    if (h < 0 || h >= HOSPITAL_COUNT ||
        d < 0 || d >= DEPARTMENT_COUNT)
        return 0;

    return hospitals[h].available[d];
}

void initializeHospital(HospitalSystem *hospital)
{
    int h, d;

    hospital->lastToken = 0;

    {
        time_t now = time(NULL);
        struct tm *local = localtime(&now);

        if (local != NULL)
            strftime(hospital->currentDate,
                     sizeof(hospital->currentDate),
                     "%Y-%m-%d", local);
        else
            strcpy(hospital->currentDate, "unknown-date");

        FILE *counterFile = fopen("daily_token_counter.txt", "r");
        char savedDate[11];
        int savedCount = 0;

        if (counterFile != NULL)
        {
            if (fscanf(counterFile, "%10s %d",
                       savedDate, &savedCount) == 2 &&
                strcmp(savedDate, hospital->currentDate) == 0 &&
                savedCount >= 0 && savedCount <= MAX_TOKENS)
            {
                hospital->lastToken = savedCount;
            }

            fclose(counterFile);
        }
    }

    for (h = 0; h < HOSPITAL_COUNT; h++)
    {
        for (d = 0; d < DEPARTMENT_COUNT; d++)
        {
            DepartmentState *state = &hospital->departments[h][d];

            initializeQueue(&state->waitingQueue);
            state->currentPatient = (Patient){0};
            state->consultationActive = 0;
            state->generatedCount = 0;
            state->completedCount = 0;
            state->cancelledCount = 0;
        }
    }
}

void destroyHospital(HospitalSystem *hospital)
{
    int h, d;

    for (h = 0; h < HOSPITAL_COUNT; h++)
    {
        for (d = 0; d < DEPARTMENT_COUNT; d++)
        {
            destroyQueue(&hospital->departments[h][d].waitingQueue);
        }
    }
}

void printPatient(const Patient *patient)
{
    printf("Token: %d\n", patient->token);
    printf("Booking date: %s\n", patient->bookingDate);
    printf("Patient: %s\n", patient->name);
    printf("Source: %s\n", patient->source);
    printf("Hospital: %s\n", hospitals[patient->hospital].name);
    printf("Department: %s\n", departmentNames[patient->department]);
    printf("Symptoms: %s\n", patient->symptoms);
}

void createBooking(
    HospitalSystem *hospital,
    const char *name,
    const char *source,
    int h,
    int d,
    const char *symptoms
)
{
    Patient patient = {0};
    DepartmentState *state;
    int waitingAhead;

    if (!isDepartmentAvailable(h, d))
    {
        printf("Selected hospital does not offer this department.\n");
        return;
    }

    if (!validatePatientName(name, patient.name))
        return;

    if (strlen(source) >= SOURCE_SIZE ||
        strlen(symptoms) == 0 ||
        strlen(symptoms) >= SYMPTOM_SIZE)
    {
        printf("Invalid booking details.\n");
        return;
    }

    if (hospital->lastToken >= MAX_TOKENS)
    {
        printf("Daily token limit reached.\n");
        return;
    }

    state = &hospital->departments[h][d];

    patient.token = hospital->lastToken + 1;
    patient.hospital = h;
    patient.department = d;
    strcpy(patient.bookingDate, hospital->currentDate);
    strcpy(patient.source, source);
    strcpy(patient.symptoms, symptoms);

    waitingAhead = size(&state->waitingQueue);

    if (!enqueue(&state->waitingQueue, &patient))
    {
        printf("Booking failed: insufficient memory.\n");
        return;
    }

    hospital->lastToken = patient.token;

    {
        FILE *counterFile = fopen("daily_token_counter.txt", "w");

        if (counterFile != NULL)
        {
            fprintf(counterFile, "%s %d\n",
                    hospital->currentDate, hospital->lastToken);
            fclose(counterFile);
        }
    }

    state->generatedCount++;

    printf("\n========== BOOKING CONFIRMED ==========\n");
    printPatient(&patient);
    printf("Waiting patients ahead: %d\n", waitingAhead);

    if (state->consultationActive)
        printf("One additional patient is in consultation.\n");

    printf("Please keep your token number.\n");
}

void bookSelf(
    HospitalSystem *hospital,
    const char *name,
    int h,
    int d,
    const char *symptoms
)
{
    createBooking(hospital, name, "Self-booking", h, d, symptoms);
}

void bookWalkIn(
    HospitalSystem *hospital,
    const char *name,
    int h,
    int d,
    const char *symptoms
)
{
    createBooking(hospital, name, "Walk-in", h, d, symptoms);
}

void showStatus(const HospitalSystem *hospital, int h, int d)
{
    const DepartmentState *state;

    if (!isDepartmentAvailable(h, d))
        return;

    state = &hospital->departments[h][d];

    printf("\n========== QUEUE STATUS ==========\n");
    printf("Date: %s\n", hospital->currentDate);
    printf("Hospital: %s\n", hospitals[h].name);
    printf("Department: %s\n", departmentNames[d]);

    if (state->consultationActive)
    {
        printf("Currently serving token: %d\n",
               state->currentPatient.token);
        printf("Patient: %s\n", state->currentPatient.name);
    }
    else
    {
        printf("No consultation is active.\n");
    }

    printf("Patients waiting: %d\n", size(&state->waitingQueue));
}

void showWaitingQueue(const HospitalSystem *hospital, int h, int d)
{
    if (!isDepartmentAvailable(h, d))
        return;

    display(&hospital->departments[h][d].waitingQueue);
}

void callNextPatient(HospitalSystem *hospital, int h, int d)
{
    DepartmentState *state;

    if (!isDepartmentAvailable(h, d))
        return;

    state = &hospital->departments[h][d];

    if (state->consultationActive)
    {
        printf("\nToken %d is already in consultation.\n",
               state->currentPatient.token);
        printf("Complete that consultation first.\n");
        return;
    }

    if (!dequeue(&state->waitingQueue, &state->currentPatient))
    {
        printf("No patients are waiting.\n");
        return;
    }

    state->consultationActive = 1;

    printf("\n========== PATIENT CALLED ==========\n");
    printPatient(&state->currentPatient);
    printf("Status: In consultation\n");
}

void completeConsultation(HospitalSystem *hospital, int h, int d)
{
    DepartmentState *state;

    if (!isDepartmentAvailable(h, d))
        return;

    state = &hospital->departments[h][d];

    if (!state->consultationActive)
    {
        printf("There is no active consultation.\n");
        return;
    }

    printf("Consultation completed for token %d.\n",
           state->currentPatient.token);

    state->completedCount++;
    state->consultationActive = 0;
    state->currentPatient = (Patient){0};
}

int locateToken(
    const HospitalSystem *hospital,
    int token,
    int *h,
    int *d
)
{
    int i, j;
    Patient patient;

    for (i = 0; i < HOSPITAL_COUNT; i++)
    {
        for (j = 0; j < DEPARTMENT_COUNT; j++)
        {
            const DepartmentState *state =
                &hospital->departments[i][j];

            if ((state->consultationActive &&
                 state->currentPatient.token == token) ||
                findToken(&state->waitingQueue, token, &patient))
            {
                *h = i;
                *d = j;
                return 1;
            }
        }
    }

    return 0;
}

void showTokenDetails(const HospitalSystem *hospital, int token)
{
    int h, d, position;
    Patient patient;
    const DepartmentState *state;

    if (token < 1 || token > MAX_TOKENS)
    {
        printf("Invalid token number.\n");
        return;
    }

    if (!locateToken(hospital, token, &h, &d))
    {
        printf("Token not found in active or waiting queues.\n");
        printf("It may be completed, cancelled, or incorrect.\n");
        return;
    }

    state = &hospital->departments[h][d];

    printf("\n========== TOKEN DETAILS ==========\n");

    if (state->consultationActive &&
        state->currentPatient.token == token)
    {
        printPatient(&state->currentPatient);
        printf("Status: Currently in consultation\n");
        return;
    }

    if (!findToken(&state->waitingQueue, token, &patient))
        return;

    position = getQueuePosition(&state->waitingQueue, token);

    printPatient(&patient);
    printf("Status: Waiting\n");
    printf("Queue position: %d\n", position);
    printf("Waiting patients ahead: %d\n", position - 1);

    if (state->consultationActive)
        printf("One additional patient is in consultation.\n");
}

void cancelWaitingToken(
    HospitalSystem *hospital,
    int h,
    int d,
    int token
)
{
    DepartmentState *state;

    if (!isDepartmentAvailable(h, d))
        return;

    if (token < 1 || token > MAX_TOKENS)
    {
        printf("Invalid token number.\n");
        return;
    }

    state = &hospital->departments[h][d];

    if (state->consultationActive &&
        state->currentPatient.token == token)
    {
        printf("Patient is already in consultation.\n");
        printf("The active token cannot be cancelled.\n");
        return;
    }

    if (cancelToken(&state->waitingQueue, token))
    {
        state->cancelledCount++;
        printf("Token %d cancelled successfully.\n", token);
    }
    else
    {
        printf("Waiting token not found in the selected queue.\n");
    }
}

void showStatistics(const HospitalSystem *hospital, int h, int d)
{
    const DepartmentState *state;

    if (!isDepartmentAvailable(h, d))
        return;

    state = &hospital->departments[h][d];

    printf("\n========== DAILY QUEUE STATISTICS ==========\n");
    printf("Date: %s\n", hospital->currentDate);
    printf("Hospital: %s\n", hospitals[h].name);
    printf("Department: %s\n", departmentNames[d]);
    printf("Tokens generated: %d\n", state->generatedCount);
    printf("Patients waiting: %d\n", size(&state->waitingQueue));
    printf("Active consultations: %d\n", state->consultationActive);
    printf("Completed consultations: %d\n", state->completedCount);
    printf("Cancelled tokens: %d\n", state->cancelledCount);
}

void showOverallStatistics(const HospitalSystem *hospital)
{
    int h, d;
    int waiting = 0;
    int active = 0;
    int completed = 0;
    int cancelled = 0;

    for (h = 0; h < HOSPITAL_COUNT; h++)
    {
        for (d = 0; d < DEPARTMENT_COUNT; d++)
        {
            const DepartmentState *state =
                &hospital->departments[h][d];

            waiting += size(&state->waitingQueue);
            active += state->consultationActive;
            completed += state->completedCount;
            cancelled += state->cancelledCount;
        }
    }

    printf("\n========== ALL HOSPITALS: DAILY TOTALS ==========\n");
    printf("Date: %s\n", hospital->currentDate);
    printf("Tokens generated today: %d\n", hospital->lastToken);
    printf("Waiting patients: %d\n", waiting);
    printf("Active consultations: %d\n", active);
    printf("Completed consultations: %d\n", completed);
    printf("Cancelled tokens: %d\n", cancelled);
}

/* ==================== C_Version/frontend1.c ==================== */
#include "hospital.h"

int readLine(char text[], int capacity)
{
    char *newline;
    int ch;

    if (fgets(text, capacity, stdin) == NULL)
        return 0;

    newline = strchr(text, '\n');

    if (newline != NULL)
    {
        *newline = '\0';
    }
    else
    {
        ch = getchar();

        if (ch != '\n' && ch != EOF)
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }

            return -1;
        }
    }

    text[strcspn(text, "\r")] = '\0';
    return 1;
}

int readChoice(int maximum)
{
    char line[INPUT_SIZE];
    char *end;
    long choice;
    int result;

    while (1)
    {
        printf("Enter your choice: ");

        result = readLine(line, sizeof(line));

        if (result == 0)
            return 0;

        if (result == 1)
        {
            choice = strtol(line, &end, 10);

            if (end != line)
            {
                while (*end == ' ' || *end == '\t')
                    end++;

                if (*end == '\0' &&
                    choice >= 0 &&
                    choice <= maximum)
                {
                    return (int)choice;
                }
            }
        }

        printf("Enter a number from 0 to %d.\n", maximum);
    }
}

int readPatientName(char name[])
{
    char input[INPUT_SIZE];
    int result;

    while (1)
    {
        printf("Enter patient name: ");

        result = readLine(input, sizeof(input));

        if (result == 0)
            return 0;

        if (result == -1)
        {
            printf("Input too long.\n");
            continue;
        }

        if (validatePatientName(input, name))
            return 1;
    }
}

int readTokenNumber(void)
{
    printf("Enter token number (1-1000), or 0 to go back.\n");
    return readChoice(MAX_TOKENS);
}

int confirmAction(const char *question)
{
    printf("\n%s\n", question);
    printf("1. Confirm\n");
    printf("0. Go back\n");

    return readChoice(1) == 1;
}

int readSymptoms(char symptoms[])
{
    int result;
    size_t i;
    int hasText;
    int invalid;

    while (1)
    {
        printf("Describe symptoms, or enter None: ");

        result = readLine(symptoms, SYMPTOM_SIZE);

        if (result == 0)
            return 0;

        if (result == -1)
        {
            printf("Use at most %d bytes.\n", SYMPTOM_SIZE - 1);
            continue;
        }

        hasText = 0;
        invalid = 0;

        for (i = 0; symptoms[i] != '\0'; i++)
        {
            unsigned char ch = (unsigned char)symptoms[i];

            if (!isNameWhitespace(symptoms[i]))
                hasText = 1;

            if ((ch < 32 && ch != '\t') || ch == 127)
                invalid = 1;
        }

        if (hasText && !invalid)
            return 1;

        printf("Enter readable, non-empty symptom details.\n");
    }
}

int selectHospital(void)
{
    int i;

    printf("\n========== SELECT HOSPITAL ==========\n");

    for (i = 0; i < HOSPITAL_COUNT; i++)
    {
        printf("%d. %s\n", i + 1, hospitals[i].name);
        printf("   Type: %s\n", hospitals[i].type);
    }

    printf("0. Back\n");

    return readChoice(HOSPITAL_COUNT) - 1;
}

int selectDepartment(void)
{
    int i;

    printf("\n========== SELECT DEPARTMENT ==========\n");

    for (i = 0; i < DEPARTMENT_COUNT; i++)
        printf("%d. %s\n", i + 1, departmentNames[i]);

    printf("0. Back\n");

    return readChoice(DEPARTMENT_COUNT) - 1;
}

int selectHospitalDepartment(int h)
{
    int mapping[DEPARTMENT_COUNT];
    int count = 0;
    int i, choice;

    if (h < 0 || h >= HOSPITAL_COUNT)
        return -1;

    printf("\nDepartments at %s:\n", hospitals[h].name);

    for (i = 0; i < DEPARTMENT_COUNT; i++)
    {
        if (hospitals[h].available[i])
        {
            mapping[count] = i;
            count++;

            printf("%d. %s\n", count, departmentNames[i]);
        }
    }

    if (count == 0)
    {
        printf("No departments are available.\n");
        return -1;
    }

    printf("0. Back\n");
    choice = readChoice(count);

    if (choice == 0)
        return -1;

    return mapping[choice - 1];
}

int selectMatchingHospital(int department)
{
    int mapping[HOSPITAL_COUNT];
    int count = 0;
    int i, choice;

    if (department < 0 || department >= DEPARTMENT_COUNT)
        return -1;

    printf("\nHospitals offering %s:\n",
           departmentNames[department]);

    for (i = 0; i < HOSPITAL_COUNT; i++)
    {
        if (hospitals[i].available[department])
        {
            mapping[count] = i;
            count++;

            printf("%d. %s\n", count, hospitals[i].name);
            printf("   Type: %s\n", hospitals[i].type);
        }
    }

    if (count == 0)
    {
        printf("No matching hospital in the demo directory.\n");
        return -1;
    }

    printf("0. Back\n");
    choice = readChoice(count);

    if (choice == 0)
        return -1;

    return mapping[choice - 1];
}

int suggestDepartment(void)
{
    int choice;
    int department;

    printf("\n========== SYMPTOM CATEGORY DEMO ==========\n");
    printf("This is an educational mapping, not a diagnosis.\n");
    printf("It does not assess urgency or rule out an emergency.\n");
    printf("For an emergency, leave routine token booking.\n");

    printf("\n1. Continue with routine booking demo\n");
    printf("0. Back\n");

    if (readChoice(1) == 0)
        return -1;

    printf("\nSelect a symptom category:\n");
    printf("1. Fever, cough or cold\n");
    printf("2. Ear pain or blocked nose\n");
    printf("3. Joint or bone pain\n");
    printf("4. Skin rash or itching\n");
    printf("5. Other / Not sure\n");
    printf("0. Back\n");

    choice = readChoice(5);

    if (choice == 0)
        return -1;

    department = (choice == 5) ? 0 : choice - 1;

    printf("\nDemo suggested department: %s\n",
           departmentNames[department]);

    switch (department)
    {
        case 0:
            printf("Hospital type: General hospital or clinic\n");
            break;

        case 1:
            printf("Hospital type: Hospital with ENT services\n");
            break;

        case 2:
            printf("Hospital type: Hospital with orthopaedic services\n");
            break;

        case 3:
            printf("Hospital type: Hospital with dermatology services\n");
            break;
    }

    printf("\n1. Use suggested department\n");
    printf("2. Choose department manually\n");
    printf("0. Back\n");

    choice = readChoice(2);

    if (choice == 0)
        return -1;

    if (choice == 2)
        return selectDepartment();

    return department;
}

int selectBookingLocation(int *h, int *d)
{
    int choice;

    printf("\n========== BOOKING OPTIONS ==========\n");
    printf("1. Choose hospital first\n");
    printf("2. Choose department first\n");
    printf("3. Get a symptom-category suggestion\n");
    printf("0. Back\n");

    choice = readChoice(3);

    if (choice == 0)
        return 0;

    if (choice == 1)
    {
        *h = selectHospital();

        if (*h == -1)
            return 0;

        *d = selectHospitalDepartment(*h);
    }
    else
    {
        if (choice == 2)
            *d = selectDepartment();
        else
            *d = suggestDepartment();

        if (*d == -1)
            return 0;

        *h = selectMatchingHospital(*d);
    }

    return isDepartmentAvailable(*h, *d);
}

void showBookingPreview(
    const char *name,
    const char *source,
    int h,
    int d,
    const char *symptoms
)
{
    printf("\n========== BOOKING PREVIEW ==========\n");
    printf("Patient: %s\n", name);
    printf("Booking source: %s\n", source);
    printf("Hospital: %s\n", hospitals[h].name);
    printf("Department: %s\n", departmentNames[d]);
    printf("Symptoms: %s\n", symptoms);
}

void bookPatientUI(HospitalSystem *hospital)
{
    char name[NAME_SIZE];
    char symptoms[SYMPTOM_SIZE];
    int h, d;

    if (!selectBookingLocation(&h, &d))
        return;

    if (!readPatientName(name))
        return;

    if (!readSymptoms(symptoms))
        return;

    showBookingPreview(name, "Self-booking", h, d, symptoms);

    if (confirmAction("Confirm and submit booking?"))
        bookSelf(hospital, name, h, d, symptoms);
    else
        printf("Booking not submitted.\n");
}

void viewPatientQueueStatusUI(const HospitalSystem *hospital)
{
    int h = selectHospital();
    int d;

    if (h == -1)
        return;

    d = selectHospitalDepartment(h);

    if (d == -1)
        return;

    showStatus(hospital, h, d);
    printf("Waiting count excludes the active consultation.\n");
}

void trackTokenUI(const HospitalSystem *hospital)
{
    int token = readTokenNumber();

    if (token != 0)
        showTokenDetails(hospital, token);
}

void showMyQueueOverviewUI(const HospitalSystem *hospital)
{
    int token = readTokenNumber();
    int h, d;

    if (token == 0)
        return;

    if (locateToken(hospital, token, &h, &d))
        showStatus(hospital, h, d);

    showTokenDetails(hospital, token);
}

void showBookingHelp(void)
{
    printf("\n========== HOW TO BOOK ==========\n");
    printf("1. Select Book a Token.\n");
    printf("2. Choose hospital, department, or symptom suggestion.\n");
    printf("3. Select an available hospital-department pair.\n");
    printf("4. Enter your name and detailed symptoms.\n");
    printf("5. Review and confirm your details.\n");
    printf("6. Keep the generated token number.\n");
    printf("7. Track your token to check its status.\n");
}

void showPatientFAQ(void)
{
    int choice;

    while (1)
    {
        printf("\n========== PATIENT FAQ ==========\n");
        printf("1. How is my token assigned?\n");
        printf("2. Do walk-ins use the same queue?\n");
        printf("3. How can I cancel my token?\n");
        printf("4. Why is my token not found?\n");
        printf("5. Does the system show exact waiting time?\n");
        printf("6. Why are some hospitals not shown?\n");
        printf("7. Is the symptom suggestion a diagnosis?\n");
        printf("0. Back\n");

        choice = readChoice(7);

        switch (choice)
        {
            case 1:
                printf("Tokens are assigned after successful booking.\n");
                printf("They are unique across all hospitals this day.\n");
                break;

            case 2:
                printf("Yes, for the same hospital and department.\n");
                printf("Other hospital-department pairs have separate queues.\n");
                break;

            case 3:
                printf("Contact the reception for your booked department.\n");
                break;

            case 4:
                printf("It may be incorrect, completed, or cancelled.\n");
                break;

            case 5:
                printf("No. The system displays queue position only.\n");
                break;

            case 6:
                printf("Only hospitals offering your department are shown.\n");
                break;

            case 7:
                printf("No. It is an educational category mapping.\n");
                printf("Detailed symptom text is stored, not diagnosed.\n");
                break;

            case 0:
                return;
        }
    }
}

void showPatientGuidelines(void)
{
    printf("\n========== PATIENT GUIDELINES ==========\n");
    printf("- Check your name before confirming.\n");
    printf("- Verify the selected hospital and department.\n");
    printf("- Enter your symptom details clearly.\n");
    printf("- Keep your token number safe.\n");
    printf("- Avoid duplicate bookings.\n");
    printf("- Contact reception for cancellation.\n");
    printf("- Routine patients are called in FIFO order per queue.\n");
    printf("- This demo does not perform emergency triage.\n");
    printf("- Patient records reset when the program closes.\n");
}

void patientMenu(HospitalSystem *hospital)
{
    int choice;

    while (1)
    {
        printf("\n========================================\n");
        printf("          HOSPITAL PATIENT PORTAL\n");
        printf("========================================\n");
        printf("1. Book a token\n");
        printf("2. View queue status\n");
        printf("3. Track my token\n");
        printf("4. View my queue overview\n");
        printf("5. How to book\n");
        printf("6. Frequently asked questions\n");
        printf("7. Patient guidelines\n");
        printf("0. Back\n");

        choice = readChoice(7);

        switch (choice)
        {
            case 1:
                bookPatientUI(hospital);
                break;

            case 2:
                viewPatientQueueStatusUI(hospital);
                break;

            case 3:
                trackTokenUI(hospital);
                break;

            case 4:
                showMyQueueOverviewUI(hospital);
                break;

            case 5:
                showBookingHelp();
                break;

            case 6:
                showPatientFAQ();
                break;

            case 7:
                showPatientGuidelines();
                break;

            case 0:
                return;
        }
    }
}

/* ==================== C_Version/frontend2.c ==================== */
#include "hospital.h"

int selectReceptionQueue(int *h, int *d)
{
    int selectedHospital = selectHospital();
    int selectedDepartment;

    if (selectedHospital == -1)
        return 0;

    selectedDepartment = selectHospitalDepartment(selectedHospital);

    if (selectedDepartment == -1)
        return 0;

    *h = selectedHospital;
    *d = selectedDepartment;

    return 1;
}

void bookWalkInUI(HospitalSystem *hospital, int h, int d)
{
    char name[NAME_SIZE];
    char symptoms[SYMPTOM_SIZE];

    if (!readPatientName(name))
        return;

    if (!readSymptoms(symptoms))
        return;

    showBookingPreview(name, "Walk-in", h, d, symptoms);

    if (confirmAction("Register this walk-in patient?"))
        bookWalkIn(hospital, name, h, d, symptoms);
    else
        printf("Registration cancelled.\n");
}

void registerMultipleWalkInsUI(
    HospitalSystem *hospital,
    int h,
    int d
)
{
    while (1)
    {
        bookWalkInUI(hospital, h, d);

        printf("\n1. Register another patient in this queue\n");
        printf("0. Return to reception menu\n");

        if (readChoice(1) == 0)
            return;
    }
}

void showStatusUI(const HospitalSystem *hospital, int h, int d)
{
    showStatus(hospital, h, d);
}

void showWaitingQueueUI(
    const HospitalSystem *hospital,
    int h,
    int d
)
{
    showStatus(hospital, h, d);
    showWaitingQueue(hospital, h, d);
}

void callNextPatientUI(HospitalSystem *hospital, int h, int d)
{
    showStatus(hospital, h, d);
    showWaitingQueue(hospital, h, d);

    if (confirmAction("Call the next waiting patient?"))
        callNextPatient(hospital, h, d);
}

void completeConsultationUI(
    HospitalSystem *hospital,
    int h,
    int d
)
{
    showStatus(hospital, h, d);

    if (confirmAction("Complete the current consultation?"))
        completeConsultation(hospital, h, d);
}

void showTokenDetailsUI(const HospitalSystem *hospital)
{
    int token = readTokenNumber();

    if (token != 0)
        showTokenDetails(hospital, token);
}

void cancelWaitingTokenUI(
    HospitalSystem *hospital,
    int h,
    int d
)
{
    int token = readTokenNumber();
    int tokenHospital, tokenDepartment;

    if (token == 0)
        return;

    if (!locateToken(hospital, token, &tokenHospital, &tokenDepartment))
    {
        printf("Active or waiting token not found.\n");
        return;
    }

    if (tokenHospital != h || tokenDepartment != d)
    {
        printf("This token belongs to another queue.\n");
        printf("Switch to its hospital and department first.\n");
        return;
    }

    showTokenDetails(hospital, token);

    if (confirmAction("Cancel this waiting token?"))
        cancelWaitingToken(hospital, h, d, token);
}

void showStatisticsUI(
    const HospitalSystem *hospital,
    int h,
    int d
)
{
    showStatistics(hospital, h, d);
}

void showShiftSummary(
    const HospitalSystem *hospital,
    int h,
    int d
)
{
    printf("\n========== SHIFT SUMMARY ==========\n");

    showStatistics(hospital, h, d);
    showStatus(hospital, h, d);
    showWaitingQueue(hospital, h, d);

    printf("Statistics cover the current day/session.\n");
}

void showReceptionistHelp(void)
{
    printf("\n========== RECEPTIONIST HELP ==========\n");
    printf("- Select your hospital and department first.\n");
    printf("- Register patients arriving without bookings.\n");
    printf("- Record symptoms for each walk-in patient.\n");
    printf("- Both booking types share the selected queue.\n");
    printf("- Other departments have independent queues.\n");
    printf("- Call routine patients in FIFO order.\n");
    printf("- Complete a consultation before calling another.\n");
    printf("- Only waiting tokens can be cancelled.\n");
    printf("- Switch queues before cancelling another queue's token.\n");
    printf("- Cancelled token numbers are not reused that day.\n");
    printf("- Patient records reset when the program closes.\n");
}

void receptionistMenu(HospitalSystem *hospital)
{
    int h, d;
    int choice;

    if (!selectReceptionQueue(&h, &d))
        return;

    while (1)
    {
        printf("\n========================================\n");
        printf("           HOSPITAL RECEPTION\n");
        printf("========================================\n");
        printf("Hospital: %s\n", hospitals[h].name);
        printf("Department: %s\n", departmentNames[d]);

        printf("\n1. Register walk-in patient\n");
        printf("2. Register multiple walk-in patients\n");
        printf("3. Display waiting queue\n");
        printf("4. Call next patient\n");
        printf("5. Complete consultation\n");
        printf("6. Search token\n");
        printf("7. Cancel waiting token\n");
        printf("8. View daily statistics\n");
        printf("9. View shift summary\n");
        printf("10. Receptionist help\n");
        printf("11. Change hospital / department\n");
        printf("12. View all-hospital statistics\n");
        printf("13. View current queue status\n");
        printf("0. Back\n");

        choice = readChoice(13);

        switch (choice)
        {
            case 1:
                bookWalkInUI(hospital, h, d);
                break;

            case 2:
                registerMultipleWalkInsUI(hospital, h, d);
                break;

            case 3:
                showWaitingQueueUI(hospital, h, d);
                break;

            case 4:
                callNextPatientUI(hospital, h, d);
                break;

            case 5:
                completeConsultationUI(hospital, h, d);
                break;

            case 6:
                showTokenDetailsUI(hospital);
                break;

            case 7:
                cancelWaitingTokenUI(hospital, h, d);
                break;

            case 8:
                showStatisticsUI(hospital, h, d);
                break;

            case 9:
                showShiftSummary(hospital, h, d);
                break;

            case 10:
                showReceptionistHelp();
                break;

            case 11:
                selectReceptionQueue(&h, &d);
                break;

            case 12:
                showOverallStatistics(hospital);
                break;

            case 13:
                showStatusUI(hospital, h, d);
                break;

            case 0:
                return;
        }
    }
}