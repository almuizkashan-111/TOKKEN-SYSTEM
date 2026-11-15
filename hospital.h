#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_SIZE 31
#define SOURCE_SIZE 20
#define SYMPTOM_SIZE 200
#define INPUT_SIZE 256
#define MAX_TOKENS 1000
#define HOSPITAL_COUNT 3
#define DEPARTMENT_COUNT 4

/* Shared declarations: all four members use this header. */
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
} HospitalSystem;

/* Backend 1: linked-list queue. */
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

/* Backend 2: validation, booking, state and reporting. */
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

/* Frontend 1: shared input helpers and patient portal. */
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

/* Frontend 2: receptionist portal. */
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
