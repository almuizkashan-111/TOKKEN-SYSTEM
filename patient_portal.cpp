#include "patient_portal.h"
#include "input_utils.h"
#include "selection.h"

#include <iostream>

using namespace std;

namespace {
int selectBookingLocation(int& hospitalIndex, int& departmentIndex) {
    cout << "\n========== BOOKING OPTIONS ==========\n";
    cout << "1. Choose hospital first\n";
    cout << "2. Choose department first\n";
    cout << "3. Symptom-category suggestion\n";
    cout << "0. Back\n";

    int choice = readChoice("Enter choice: ", 0, 3);
    if (choice <= 0) return 0;

    if (choice == 1) {
        int h = selectHospital();
        if (h < 0) return 0;
        int d = selectAvailableDepartment(h);
        if (d < 0) return 0;
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
            int symptomChoice = readChoice("Select category: ", 0, 5);
            if (symptomChoice <= 0) return 0;
            if (symptomChoice == 5) {
                d = selectDepartment();
            } else {
                d = symptomChoice - 1;
                cout << "Suggested category: " << departmentNames[d] << '\n';
                cout << "1. Use this category\n2. Choose manually\n0. Back\n";
                int option = readChoice("Enter choice: ", 0, 2);
                if (option == 0) return 0;
                if (option == 2) d = selectDepartment();
            }
        }
        if (d < 0) return 0;
        int h = selectMatchingHospital(d);
        if (h < 0) return 0;
        hospitalIndex = h;
        departmentIndex = d;
    }

    return 1;
}

void bookSelf(HospitalSystem& system) {
    int h = -1, d = -1;
    if (!selectBookingLocation(h, d)) return;

    string name, symptoms;
    if (!readValidatedText("Enter patient name: ", name, MAX_NAME_LENGTH)) return;
    if (!readValidatedText("Enter symptoms: ", symptoms, MAX_SYMPTOM_LENGTH)) return;

    cout << "\n========== BOOKING PREVIEW ==========\n";
    cout << "Patient: " << name << '\n';
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';
    cout << "Symptoms: " << symptoms << '\n';
    cout << "Booking source: Self-booking\n";

    if (!confirmAction("Confirm and submit this booking?")) {
        cout << "Booking not submitted.\n";
        return;
    }

    bool allowDuplicate = false;
    int possibleDuplicate = system.findPossibleDuplicate(h, d, name);
    if (possibleDuplicate != 0) {
        cout << "WARNING: A similar name already has token "
             << possibleDuplicate << " in this queue.\n";
        cout << "Names alone cannot verify patient identity.\n";
        if (!confirmAction("Continue with another booking anyway?")) return;
        allowDuplicate = true;
    }

    int token = 0;
    string error;
    if (!system.registerBooking(name, "Self-booking", symptoms,
                                h, d, allowDuplicate, token, error)) {
        cout << "Booking failed: " << error << '\n';
        return;
    }

    const DepartmentState& state = system.departments[h][d];
    int position = state.waitingQueue.size();

    cout << "\n========== DIGITAL TOKEN SLIP ==========\n";
    cout << "Token number: " << token << '\n';
    cout << "Queue position: " << position << '\n';
    cout << "Estimated wait: "
         << system.estimateWaitMinutes(h, d, position - 1)
         << " minutes (approx.)\n";
    cout << "Keep your token number for tracking.\n";
}

void showPatientTokenStatus(const HospitalSystem& system, int token) {
    const TokenRecord* record = system.findRecord(token);
    if (record == NULL) {
        cout << "Token not found. Check the number or contact reception.\n";
        return;
    }

    cout << "\n========== TOKEN STATUS ==========" << '\n';
    cout << "Token: " << token << '\n';
    cout << "Status: " << statusText(record->status) << '\n';
    cout << "Hospital: " << hospitals[record->patient.hospitalIndex].name << '\n';
    cout << "Department: " << departmentNames[record->patient.departmentIndex] << '\n';

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
    if (h < 0) return;
    int d = selectAvailableDepartment(h);
    if (d < 0) return;

    const DepartmentState& state = system.departments[h][d];
    cout << "\n========== QUEUE STATUS ==========" << '\n';
    cout << "Hospital: " << hospitals[h].name << '\n';
    cout << "Department: " << departmentNames[d] << '\n';
    cout << "Patients waiting: " << state.waitingQueue.size() << '\n';
    cout << "Consultation active: "
         << (state.consultationActive ? "Yes" : "No") << '\n';
    cout << "Queue load: ";
    if (state.waitingQueue.size() <= 3) cout << "LOW\n";
    else if (state.waitingQueue.size() <= 7) cout << "MODERATE\n";
    else cout << "HIGH\n";
    cout << "Waiting-time estimates are approximate.\n";
}
}

void patientMenu(HospitalSystem& system) {
    while (true) {
        cout << "\n========================================\n";
        cout << "             PATIENT PORTAL\n";
        cout << "========================================\n";
        cout << "1. Book a token\n";
        cout << "2. Track token status\n";
        cout << "3. View queue status\n";
        cout << "4. Help and guidelines\n";
        cout << "0. Back\n";

        int choice = readChoice("Enter choice: ", 0, 4);
        if (choice <= 0) return;

        switch (choice) {
            case 1:
                bookSelf(system);
                break;
            case 2: {
                int token = readChoice("Enter token number: ", 1, MAX_TOKENS);
                if (token < 0) return;
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
