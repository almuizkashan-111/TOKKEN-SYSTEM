#include "hospital.h"

/* FRONTEND 1: patient portal and input helpers shared by both portals. */

/* 1 = input read; 0 = end of input; -1 = input too long. */
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
                printf("They are unique across all hospitals this session.\n");
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
    printf("- Data resets when the program closes.\n");
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
