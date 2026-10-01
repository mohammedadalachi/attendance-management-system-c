#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//files to store system data
#define LECTURERS_FILE  "lecturers.txt"
#define SUBJECTS_FILE   "subjects.txt"
#define STUDENTS_FILE   "students.txt"
#define ENROLL_FILE     "enrollments.txt"
#define ATTENDANCE_FILE "attendance.txt"
#define WARNINGS_FILE   "warnings.txt"

//standard auto warning message (for automatic warnings below 80%)
#define AUTO_WARNING_MESSAGE "Attendance is below 80%. Please improve your attendance to avoid academic action."

//Stores logged lecturer ID
char loggedLecturer[25] = "";

//Stores logged student ID
char loggedStudent[25]  = "";

//function prototypes
void academicMenu(void);
void lecturerMenu(void);
void studentMenu(void);

void admin_RegisterLecturer(void);
void admin_RegisterSubject(void);
void admin_ViewReports(void);
void admin_ViewStudentAttendancePercentage(void);
void admin_IssueWarning(void);
void admin_ViewWarningsHistory(void);

int lecturer_Login(void);
void lecturer_EnrollStudent(void);
void lecturer_MarkAttendance(void);
void lecturer_UpdateAttendance(void);
void lecturer_ViewAttendanceList(void);

void student_Dashboard(void);
void student_ViewAttendancePercentage(void);
void student_ViewWarningMessage(void);

//auto warning helper prototypes
void getTodayDate(char *out, int outSize);
int autoWarningExists(const char *studentId, const char *subjectCode);
void writeAutoWarning(const char *studentId, const char *subjectCode, float pct);
void autoGenerateWarningsForSubject(const char *subjectCode);

//Input functions
//readLine function reads a full line safely from the user
int readLine(char *out, int size) {
    //checks for input errors
    if (!fgets(out, size, stdin)) return 0;

    //removes the newline character
    out[strcspn(out, "\n")] = '\0';

    return 1;
}


//askString function to ask the user for text input
void askString(const char *prompt, char *out, int outSize) {
    char buffer[300];

    //prints the prompt message
    printf("%s", prompt);

    //checks for input errors
    if (!readLine(buffer, (int)sizeof(buffer))) {
        out[0] = '\0';
        return;
    }

    //copies the input from buffer into out safely
    strncpy(out, buffer, outSize - 1);
    out[outSize - 1] = '\0';
}


//askIntRange asks the user until a valid integer within a given range is entered
int askIntRange(const char *prompt, int min, int max) {
    char line[64];
    int x;

    //infinite loop to keep asking until input is valid
    while (1) {
        askString(prompt, line, (int)sizeof(line));

        //checks that the input is a number and within the required range
        if (sscanf(line, "%d", &x) == 1 && x >= min && x <= max)
            return x;

        //error message for invalid input
        printf("Please enter a number between %d and %d.\n", min, max);
    }
}

//askStatusPA function to record attendance status
//returns 1 for Present and 0 for Absent
int askStatusPA(const char *prompt) {
    char line[32];

    //infinite loop to keep asking until input is valid
    while (1) {
        //asks the user for attendance input
        askString(prompt, line, (int)sizeof(line));

        //checks if input is P or p (Present)
        if (line[0] == 'P' || line[0] == 'p') return 1;

        //checks if input is A or a (Absent)
        if (line[0] == 'A' || line[0] == 'a') return 0;

        //also accepts numeric input for Present
        if (strcmp(line, "1") == 0) return 1;

        //also accepts numeric input for Absent
        if (strcmp(line, "0") == 0) return 0;

        //error message for invalid input
        printf("Enter P/A or 1/0 only.\n");
    }
}

//pressEnter function pauses the program until the user presses Enter
void pressEnter(void) {
    char enterspace[8];

    //prints message to inform the user
    printf("\nPress Enter to continue...");

    //waits for the user to press Enter
    readLine(enterspace, sizeof(enterspace));
}

//setupSystem function creates all required data files if they do not exist
void setupSystem(void) {

    //array holding all file names used in the system
    const char *files[] = {
        LECTURERS_FILE, SUBJECTS_FILE, STUDENTS_FILE,
        ENROLL_FILE, ATTENDANCE_FILE, WARNINGS_FILE
    };

    int i;

    //loops through each file name
    for (i = 0; i < 6; i++) {

        //opens file in append mode (creates file if it does not exist)
        FILE *f = fopen(files[i], "a");

        //closes the file if opened successfully
        if (f) fclose(f);
    }
}

//getLecturerName function finds a lecturer name using lecturer ID
//returns 1 if found and 0 if not found
int getLecturerName(const char *lecturerId, char *outName, int outSize) {

    //opens lecturers file for reading
    FILE *f = fopen(LECTURERS_FILE, "r");

    char id[20], name[50];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each lecturer record from the file
    while (fscanf(f, " %19[^|]|%49[^\n]\n", id, name) == 2) {

        //checks if lecturer ID matches
        if (strcmp(id, lecturerId) == 0) {

            //copies lecturer name into output variable
            strncpy(outName, name, outSize - 1);
            outName[outSize - 1] = '\0';

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if lecturer ID was not found
    fclose(f);
    return 0;
}

//getStudentName function finds a student name using student ID
//returns 1 if found and 0 if not found
int getStudentName(const char *studentId, char *outName, int outSize) {

    //opens students file for reading
    FILE *f = fopen(STUDENTS_FILE, "r");

    char id[20], name[50];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each student record from the file
    while (fscanf(f, " %19[^|]|%49[^\n]\n", id, name) == 2) {

        //checks if student ID matches
        if (strcmp(id, studentId) == 0) {

            //copies student name into output variable
            strncpy(outName, name, outSize - 1);
            outName[outSize - 1] = '\0';

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if student ID was not found
    fclose(f);
    return 0;
}

//getSubjectNameAndLecturer function finds subject name and lecturer ID using subject code
//returns 1 if found and 0 if not found
int getSubjectNameAndLecturer(const char *subjectCode,
                             char *outName, int outSize,
                             char *outLecturerId, int lidSize) {

    //opens subjects file for reading
    FILE *f = fopen(SUBJECTS_FILE, "r");

    char code[20], name[50], lecturerId[20];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each subject record from the file
    while (fscanf(f, " %19[^|]|%49[^|]|%19[^\n]\n",
                  code, name, lecturerId) == 3) {

        //checks if subject code matches
        if (strcmp(code, subjectCode) == 0) {

            //copies subject name into output variable
            strncpy(outName, name, outSize - 1);
            outName[outSize - 1] = '\0';

            //copies lecturer ID into output variable
            strncpy(outLecturerId, lecturerId, lidSize - 1);
            outLecturerId[lidSize - 1] = '\0';

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if subject code was not found
    fclose(f);
    return 0;
}

//lecturerExists function checks if a lecturer ID exists
//returns 1 if found and 0 if not found
int lecturerExists(const char *lecturerId) {

    //opens lecturers file for reading
    FILE *f = fopen(LECTURERS_FILE, "r");

    char id[20], name[50];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each lecturer record from the file
    while (fscanf(f, " %19[^|]|%49[^\n]\n", id, name) == 2) {

        //checks if lecturer ID matches
        if (strcmp(id, lecturerId) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if lecturer ID was not found
    fclose(f);
    return 0;
}

//subjectExists function checks if a subject code exists
//returns 1 if found and 0 if not found
int subjectExists(const char *subjectCode) {

    //opens subjects file for reading
    FILE *f = fopen(SUBJECTS_FILE, "r");

    char code[20], name[50], lecturerId[20];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each subject record from the file
    while (fscanf(f, " %19[^|]|%49[^|]|%19[^\n]\n",
                  code, name, lecturerId) == 3) {

        //checks if subject code matches
        if (strcmp(code, subjectCode) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if subject code was not found
    fclose(f);
    return 0;
}

//studentExists function checks if a student ID exists
//returns 1 if found and 0 if not found
int studentExists(const char *studentId) {

    //opens students file for reading
    FILE *f = fopen(STUDENTS_FILE, "r");

    char id[20], name[50];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each student record from the file
    while (fscanf(f, " %19[^|]|%49[^\n]\n", id, name) == 2) {

        //checks if student ID matches
        if (strcmp(id, studentId) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if student ID was not found
    fclose(f);
    return 0;
}

//enrollmentExists function checks if a student is enrolled in a subject
//returns 1 if found and 0 if not found
int enrollmentExists(const char *studentId, const char *subjectCode) {

    //opens enrollments file for reading
    FILE *f = fopen(ENROLL_FILE, "r");

    char sid[20], sub[20];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each enrollment record from the file
    while (fscanf(f, " %19[^|]|%19[^\n]\n", sid, sub) == 2) {

        //checks if student ID and subject code match
        if (strcmp(sid, studentId) == 0 && strcmp(sub, subjectCode) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if enrollment was not found
    fclose(f);
    return 0;
}

//subjectBelongsToLecturer function checks if a subject belongs to a lecturer
//used to control lecturer permissions
int subjectBelongsToLecturer(const char *subjectCode, const char *lecturerId) {

    //opens subjects file for reading
    FILE *f = fopen(SUBJECTS_FILE, "r");

    char code[20], name[50], lid[20];

    //checks if file opened successfully
    if (!f) return 0;

    //reads each subject record from the file
    while (fscanf(f, " %19[^|]|%49[^|]|%19[^\n]\n",
                  code, name, lid) == 3) {

        //checks if subject code and lecturer ID match
        if (strcmp(code, subjectCode) == 0 && strcmp(lid, lecturerId) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if subject does not belong to lecturer
    fclose(f);
    return 0;
}

//attendanceExists function checks if an attendance record already exists
//used to prevent duplicate attendance entries
int attendanceExists(const char *studentId, const char *subjectCode, const char *date) {

    //opens attendance file for reading
    FILE *f = fopen(ATTENDANCE_FILE, "r");

    char sid[20], sub[20], fdate[15];
    int status;

    //checks if file opened successfully
    if (!f) return 0;

    //reads each attendance record from the file
    while (fscanf(f, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                  sid, sub, fdate, &status) == 4) {

        //checks if student ID, subject code, and date all match
        if (strcmp(sid, studentId) == 0 &&
            strcmp(sub, subjectCode) == 0 &&
            strcmp(fdate, date) == 0) {

            //closes file and returns success
            fclose(f);
            return 1;
        }
    }

    //closes file if attendance record was not found
    fclose(f);
    return 0;
}

//calculatePercentage function calculates attendance percentage for a student in a subject
//returns the percentage value as a float
float calculatePercentage(const char *studentId, const char *subjectCode) {

    //opens attendance file for reading
    FILE *f = fopen(ATTENDANCE_FILE, "r");

    char sid[20], sub[20], date[15];
    int status;

    //stores total number of attendance records
    int total = 0;

    //stores number of present records
    int present = 0;

    //checks if file opened successfully
    if (!f) return 0.0f;

    //reads each attendance record from the file
    while (fscanf(f, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                  sid, sub, date, &status) == 4) {

        //checks if record belongs to the student and subject
        if (strcmp(sid, studentId) == 0 && strcmp(sub, subjectCode) == 0) {

            //increments total attendance count
            total++;

            //increments present count if status is present
            if (status == 1) present++;
        }
    }

    //closes attendance file
    fclose(f);

    //avoids division by zero if no records exist
    if (total == 0) return 0.0f;

    //calculates and returns attendance percentage
    return (present * 100.0f) / (float)total;
}

//getTodayDate function gets today's date automatically in YYYY-MM-DD format
void getTodayDate(char *out, int outSize) {

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    //formats date into out
    strftime(out, outSize, "%Y-%m-%d", tm_info);
}

//autoWarningExists function checks if an AUTO warning already exists
//prevents duplicate AUTO warnings for the same student and subject
int autoWarningExists(const char *studentId, const char *subjectCode) {

    FILE *f = fopen(WARNINGS_FILE, "r");

    char sid[20], sub[20], date[15], type[10], msg[120];
    float pct;

    //if file does not exist, then no warning exists
    if (!f) return 0;

    //reads warnings file (NEW FORMAT: sid|sub|pct|date|type|message)
    while (fscanf(f, " %19[^|]|%19[^|]|%f|%14[^|]|%9[^|]|%119[^\n]\n",
                  sid, sub, &pct, date, type, msg) == 6) {

        //checks if it matches same student and subject and is AUTO
        if (strcmp(sid, studentId) == 0 &&
            strcmp(sub, subjectCode) == 0 &&
            strcmp(type, "AUTO") == 0) {

            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

//writeAutoWarning function writes an AUTO warning record using standard message
//it writes only one AUTO warning per student per subject (no duplicates)
void writeAutoWarning(const char *studentId, const char *subjectCode, float pct) {

    char today[15];

    //prevents duplicate AUTO warnings
    if (autoWarningExists(studentId, subjectCode)) return;

    //gets today's date automatically
    getTodayDate(today, (int)sizeof(today));

    //opens warnings file in append mode
    {
        FILE *wf = fopen(WARNINGS_FILE, "a");

        //checks if file opened successfully
        if (!wf) return;

        //writes warning record (NEW FORMAT includes type)
        fprintf(wf, "%s|%s|%.2f|%s|AUTO|%s\n",
                studentId, subjectCode, pct, today, AUTO_WARNING_MESSAGE);

        fclose(wf);
    }
}

//autoGenerateWarningsForSubject function automatically issues warnings
//for all enrolled students below 80% in the given subject
void autoGenerateWarningsForSubject(const char *subjectCode) {

    FILE *ef = fopen(ENROLL_FILE, "r");
    char sid[20], sub[20];

    //checks if enrollment file exists
    if (!ef) return;

    //reads each enrollment record
    while (fscanf(ef, " %19[^|]|%19[^\n]\n", sid, sub) == 2) {

        //skips different subjects
        if (strcmp(sub, subjectCode) != 0) continue;

        //calculates current attendance percentage
        {
            float pct = calculatePercentage(sid, subjectCode);

            //issues AUTO warning if below 80%
            if (pct < 80.0f) {
                writeAutoWarning(sid, subjectCode, pct);
            }
        }
    }

    fclose(ef);
}

//admin_RegisterLecturer function registers a new lecturer
//adds lecturer ID and name to the lecturers file
void admin_RegisterLecturer(void) {

    char lecturerId[20], lecturerName[50];

    //prints section title
    printf("\n--- Register New Lecturer ---\n");

    //asks user to enter lecturer ID
    askString("ID: ", lecturerId, (int)sizeof(lecturerId));

    //checks if lecturer ID is empty
    if (lecturerId[0] == '\0') {
        printf("ID cannot be empty.\n");
        pressEnter();
        return;
    }

    //checks if lecturer already exists
    if (lecturerExists(lecturerId)) {
        printf("Lecturer already exists.\n");
        pressEnter();
        return;
    }

    //asks user to enter lecturer name
    askString("Name: ", lecturerName, (int)sizeof(lecturerName));

    //checks if lecturer name is empty
    if (lecturerName[0] == '\0') {
        printf("Name cannot be empty.\n");
        pressEnter();
        return;
    }

    //opens lecturers file in append mode to save new lecturer
    {
        FILE *f = fopen(LECTURERS_FILE, "a");

        //checks if file opened successfully
        if (!f) {
            printf("Cannot open lecturers file.\n");
            pressEnter();
            return;
        }

        //writes lecturer record to file
        fprintf(f, "%s|%s\n", lecturerId, lecturerName);

        //closes the file
        fclose(f);
    }

    //prints success message
    printf("Success.\n");
    pressEnter();
}

//admin_RegisterSubject function registers a new subject
//assigns the subject to an existing lecturer
void admin_RegisterSubject(void) {

    char subjectCode[20], subjectName[50], lecturerId[20];

    //prints section title
    printf("\n--- Register New Subject ---\n");

    //asks user to enter subject code
    askString("Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if subject code is empty
    if (subjectCode[0] == '\0') {
        printf("Code cannot be empty.\n");
        pressEnter();
        return;
    }

    //checks if subject already exists
    if (subjectExists(subjectCode)) {
        printf("Subject already exists.\n");
        pressEnter();
        return;
    }

    //asks user to enter subject name
    askString("Name: ", subjectName, (int)sizeof(subjectName));

    //checks if subject name is empty
    if (subjectName[0] == '\0') {
        printf("Name cannot be empty.\n");
        pressEnter();
        return;
    }

    //asks user to enter lecturer ID
    askString("Lecturer ID: ", lecturerId, (int)sizeof(lecturerId));

    //checks if lecturer exists
    if (!lecturerExists(lecturerId)) {
        printf("Lecturer not found. Register lecturer first.\n");
        pressEnter();
        return;
    }

    //opens subjects file in append mode to save new subject
    {
        FILE *f = fopen(SUBJECTS_FILE, "a");

        //checks if file opened successfully
        if (!f) {
            printf("Cannot open subjects file.\n");
            pressEnter();
            return;
        }

        //writes subject record to file
        fprintf(f, "%s|%s|%s\n", subjectCode, subjectName, lecturerId);

        //closes the file
        fclose(f);
    }

    //prints success message
    printf("Success.\n");
    pressEnter();
}

//admin_ViewReports function allows admin to view attendance reports
//reports can be viewed by subject or by lecturer
void admin_ViewReports(void) {

    //opens attendance file for reading
    FILE *f = fopen(ATTENDANCE_FILE, "r");

    char studentId[20], subjectCode[20], date[15];
    int status;

    //checks if attendance file exists
    if (!f) {
        printf("No attendance data.\n");
        pressEnter();
        return;
    }

    //prints report menu
    printf("\n--- View Attendance Report ---\n");
    printf("1) By Subject\n");
    printf("2) By Lecturer\n");

    {
        //asks admin to select report type
        int mode = askIntRange("Choice: ", 1, 2);

        //*****************
        //REPORT BY SUBJECT
        //*****************
        if (mode == 1) {

            char filterSub[20];
            char subjectName[50], lecturerId[20], lecturerName[50];

            //asks for subject code
            askString("Enter Subject Code: ", filterSub, (int)sizeof(filterSub));

            //checks if subject exists
            if (!subjectExists(filterSub)) {
                printf("Subject not found.\n");
                fclose(f);
                pressEnter();
                return;
            }

            //gets subject name and lecturer ID
            getSubjectNameAndLecturer(filterSub, subjectName, sizeof(subjectName),
                                      lecturerId, sizeof(lecturerId));

            //gets lecturer name
            if (!getLecturerName(lecturerId, lecturerName, sizeof(lecturerName)))
                strcpy(lecturerName, "Unknown");

            //prints report header
            printf("\nSubject: %s (%s) | Lecturer: %s (%s)\n",
                   filterSub, subjectName, lecturerId, lecturerName);

            printf("\nSTUDENT ID     | STUDENT NAME               | DATE          | STATUS\n");
            printf("---------------------------------------------------------------------\n");

            //reads and prints attendance records for this subject
            while (fscanf(f, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                          studentId, subjectCode, date, &status) == 4) {

                //checks if subject matches
                if (strcmp(filterSub, subjectCode) == 0) {

                    char studentName[50];

                    //gets student name
                    if (!getStudentName(studentId, studentName, sizeof(studentName)))
                        strcpy(studentName, "Unknown");

                    //prints attendance record
                    printf("%-13s | %-25s | %-13s | %s\n",
                           studentId, studentName, date,
                           status ? "Present" : "Absent");
                }
            }

        //******************
        //REPORT BY LECTURER
        //******************
        } else {

            char lecturerId[20];
            char lecturerName[50];

            //asks admin to enter lecturer ID
            askString("Enter Lecturer ID: ", lecturerId, (int)sizeof(lecturerId));

            //checks if lecturer exists
            if (!lecturerExists(lecturerId)) {
                printf("Lecturer not found.\n");
                fclose(f);
                pressEnter();
                return;
            }

            //gets lecturer name
            if (!getLecturerName(lecturerId, lecturerName, sizeof(lecturerName)))
                strcpy(lecturerName, "Unknown");

            //prints lecturer header
            printf("\nLecturer: %s (%s)\n", lecturerId, lecturerName);

            //prints table header
            printf("\nSUBJECT        | SUBJECT NAME               | STUDENT ID     | STUDENT NAME               | DATE          | STATUS\n");
            printf("---------------------------------------------------------------------------------------------------------------\n");

            //reads each attendance record
            while (fscanf(f, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                          studentId, subjectCode, date, &status) == 4) {

                //checks if subject belongs to this lecturer
                if (subjectBelongsToLecturer(subjectCode, lecturerId)) {

                    char studentName[50], subjectName[50], lidDummy[20];

                    //gets student name
                    if (!getStudentName(studentId, studentName, sizeof(studentName)))
                        strcpy(studentName, "Unknown");

                    //gets subject name
                    if (!getSubjectNameAndLecturer(subjectCode, subjectName, sizeof(subjectName),
                                                   lidDummy, sizeof(lidDummy)))
                        strcpy(subjectName, "Unknown");

                    //prints attendance record
                    printf("%-13s | %-25s | %-13s | %-25s | %-13s | %s\n",
                           subjectCode, subjectName,
                           studentId, studentName,
                           date,
                           status ? "Present" : "Absent");
                }
            }
        }
    }

    //closes attendance file
    fclose(f);

    //waits for user before returning to menu
    pressEnter();
}


//admin_ViewStudentAttendancePercentage function displays attendance percentage
//for all subjects enrolled by a student
void admin_ViewStudentAttendancePercentage(void) {

    char studentId[20];

    //prints section title
    printf("\n--- View Student Attendance Percentage ---\n");

    //asks admin to enter student ID
    askString("Student ID: ", studentId, (int)sizeof(studentId));

    //checks if student exists
    if (!studentExists(studentId)) {
        printf("Student not found.\n");
        pressEnter();
        return;
    }

    {
        //opens enrollments file for reading
        FILE *ef = fopen(ENROLL_FILE, "r");

        char sid[20], sub[20];
        int found = 0;

        //checks if enrollments file exists
        if (!ef) {
            printf("No enrollments found.\n");
            pressEnter();
            return;
        }

        //prints report header
        printf("\nStudent: %s\n", studentId);
        printf("Subject        | Percentage\n");
        printf("---------------------------\n");

        //reads each enrollment record
        while (fscanf(ef, " %19[^|]|%19[^\n]\n", sid, sub) == 2) {

            //checks if enrollment belongs to the student
            if (strcmp(sid, studentId) == 0) {

                //prints subject code and attendance percentage
                printf("%-13s | %.2f%%\n",
                       sub, calculatePercentage(studentId, sub));

                found = 1;
            }
        }

        //closes enrollments file
        fclose(ef);

        //prints message if student has no enrolled subjects
        if (!found) printf("Student has no subjects.\n");
    }

    //waits for user before returning to menu
    pressEnter();
}


//admin_IssueWarning function issues a warning to students below 80% attendance
//students below 80% are automatically listed by subject
void admin_IssueWarning(void) {

    char subjectCode[20];

    //arrays to store students below 80%
    char listIds[200][20];
    float listPct[200];
    int count = 0;

    //prints section title
    printf("\n--- Issue Reminder / Warning (<80%%) [AUTO LIST] ---\n");

    //asks admin to enter subject code
    askString("Subject Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if subject exists
    if (!subjectExists(subjectCode)) {
        printf("Subject not found.\n");
        pressEnter();
        return;
    }

    //*******************************
    //STEP 1: FIND STUDENTS BELOW 80%
    //*******************************
    {
        //opens enrollments file for reading
        FILE *ef = fopen(ENROLL_FILE, "r");
        char sid[20], sub[20];

        //checks if enrollments file exists
        if (!ef) {
            printf("No enrollments found.\n");
            pressEnter();
            return;
        }

        //reads each enrollment record
        while (fscanf(ef, " %19[^|]|%19[^\n]\n", sid, sub) == 2) {

            //skips records that do not match the subject
            if (strcmp(sub, subjectCode) != 0) continue;

            //calculates current attendance percentage
            {
                float pct = calculatePercentage(sid, subjectCode);

                //stores student only if attendance is below 80%
                if (pct < 80.0f && count < 200) {
                    strcpy(listIds[count], sid);
                    listPct[count] = pct;
                    count++;
                }
            }
        }

        //closes enrollments file
        fclose(ef);
    }

    //checks if no students are below 80%
    if (count == 0) {
        printf("No students are below 80%% for subject %s.\n", subjectCode);
        pressEnter();
        return;
    }

    //******************************************
    //STEP 2: DISPLAY LIST OF STUDENTS BELOW 80%
    //******************************************
    printf("\nStudents below 80%% in subject %s:\n", subjectCode);
    printf("-------------------------------------------------------------\n");
    printf("No. | Student ID     | Student Name               | Percentage\n");
    printf("-------------------------------------------------------------\n");

    {
        int i;
        for (i = 0; i < count; i++) {

            char studentName[50];

            //gets student name
            if (!getStudentName(listIds[i], studentName, (int)sizeof(studentName)))
                strcpy(studentName, "Unknown");

            //prints student details
            printf("%-3d | %-13s | %-25s | %.2f%%\n",
                   i + 1, listIds[i], studentName, listPct[i]);
        }
    }

    //************************************************
    //STEP 3: ADMIN SELECTS STUDENT AND ISSUES WARNING
    //************************************************
    {
        //asks admin to choose a student from the list
        int pick = askIntRange("\nSelect student number to issue warning: ", 1, count);
        int idx = pick - 1;

        char date[15], message[120];

        //asks for warning date
        askString("Date (YYYY-MM-DD): ", date, (int)sizeof(date));
        if (date[0] == '\0') {
            printf("Date cannot be empty.\n");
            pressEnter();
            return;
        }

        //asks for warning message
        askString("Message: ", message, (int)sizeof(message));
        if (message[0] == '\0') {
            printf("Message cannot be empty.\n");
            pressEnter();
            return;
        }

        //opens warnings file to save warning record
        {
            FILE *wf = fopen(WARNINGS_FILE, "a");

            //checks if warnings file opened successfully
            if (!wf) {
                printf("Cannot open warnings file.\n");
                pressEnter();
                return;
            }

            //writes warning record to file (NEW FORMAT includes type)
            fprintf(wf, "%s|%s|%.2f|%s|MANUAL|%s\n",
                    listIds[idx], subjectCode, listPct[idx], date, message);

            //closes warnings file
            fclose(wf);
        }

        //prints confirmation message
        printf("Warning issued to %s for %s.\n", listIds[idx], subjectCode);
        pressEnter();
    }
}

//admin_ViewWarningsHistory function displays all warning and reminder records
//shows student name and subject name for better readability
void admin_ViewWarningsHistory(void) {

    //opens warnings file for reading
    FILE *f = fopen(WARNINGS_FILE, "r");

    char studentId[20], subjectCode[20], date[15], type[10], message[120];
    float pct;

    //prints section title
    printf("\n--- Warning/Reminder History ---\n");

    //checks if warnings file exists
    if (!f) {
        printf("No warnings found.\n");
        pressEnter();
        return;
    }

    //reads each warning record from the file (NEW FORMAT includes type)
    while (fscanf(f, " %19[^|]|%19[^|]|%f|%14[^|]|%9[^|]|%119[^\n]\n",
                  studentId, subjectCode, &pct, date, type, message) == 6) {

        char studentName[50], subjectName[50], lidDummy[20];

        //gets student name
        if (!getStudentName(studentId, studentName, sizeof(studentName)))
            strcpy(studentName, "Unknown");

        //gets subject name
        if (!getSubjectNameAndLecturer(subjectCode, subjectName, sizeof(subjectName),
                                       lidDummy, sizeof(lidDummy)))
            strcpy(subjectName, "Unknown");

        //prints warning details
        printf("Student: %s (%s) | Subject: %s (%s) | %.2f%% | %s | %s | %s\n",
               studentId, studentName,
               subjectCode, subjectName,
               pct, date, type, message);
    }

    //closes warnings file
    fclose(f);

    //waits for user before returning to menu
    pressEnter();
}

//lecturer_Login function allows lecturer to log in using lecturer ID
//stores the logged lecturer ID for session use
int lecturer_Login(void) {

    char lecturerId[20];

    //prints login title
    printf("\n--- Lecturer Login ---\n");

    //asks lecturer to enter ID
    askString("Lecturer ID: ", lecturerId, (int)sizeof(lecturerId));

    //checks if lecturer ID exists
    if (!lecturerExists(lecturerId)) {
        printf("Invalid ID.\n");
        pressEnter();
        return 0;
    }

    //stores logged-in lecturer ID
    strcpy(loggedLecturer, lecturerId);

    //prints success message
    printf("Login successful.\n");
    pressEnter();

    return 1;
}

//lecturer_EnrollStudent function allows lecturer to enroll students into a subject
//lecturer can only enroll students for subjects they teach
void lecturer_EnrollStudent(void) {

    char subjectCode[20], studentId[20], studentName[50];

    //prints section title
    printf("\n--- Register/Enroll Student (By Subject) ---\n");

    //asks lecturer to enter subject code
    askString("Subject Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if lecturer teaches this subject
    if (!subjectBelongsToLecturer(subjectCode, loggedLecturer)) {
        printf("Permission denied: you do not teach this subject.\n");
        pressEnter();
        return;
    }

    //asks lecturer to enter student ID
    askString("Student ID: ", studentId, (int)sizeof(studentId));

    //checks if student ID is empty
    if (studentId[0] == '\0') {
        printf("Student ID cannot be empty.\n");
        pressEnter();
        return;
    }

    //checks if student exists
    if (!studentExists(studentId)) {

        //asks for new student name
        askString("New student. Enter Student Name: ", studentName, (int)sizeof(studentName));

        //checks if student name is empty
        if (studentName[0] == '\0') {
            printf("Name cannot be empty.\n");
            pressEnter();
            return;
        }

        //opens students file to save new student
        {
            FILE *sf = fopen(STUDENTS_FILE, "a");

            //checks if file opened successfully
            if (!sf) {
                printf("Cannot open students file.\n");
                pressEnter();
                return;
            }

            //writes student record to file
            fprintf(sf, "%s|%s\n", studentId, studentName);

            //closes students file
            fclose(sf);
        }
    }

    //checks if student is already enrolled in this subject
    if (enrollmentExists(studentId, subjectCode)) {
        printf("Student already enrolled.\n");
        pressEnter();
        return;
    }

    //opens enrollments file to save enrollment
    {
        FILE *ef = fopen(ENROLL_FILE, "a");

        //checks if file opened successfully
        if (!ef) {
            printf("Cannot open enrollments file.\n");
            pressEnter();
            return;
        }

        //writes enrollment record to file
        fprintf(ef, "%s|%s\n", studentId, subjectCode);

        //closes enrollments file
        fclose(ef);
    }

    //prints success message
    printf("Enrolled.\n");
    pressEnter();
}


//lecturer_MarkAttendance function records daily attendance for a subject
//lecturer can only mark attendance for subjects they teach
void lecturer_MarkAttendance(void) {

    char subjectCode[20], date[15];
    FILE *enf, *atf;
    char studentId[20], sub[20];
    int any = 0;

    //prints section title
    printf("\n--- Record Daily Attendance ---\n");

    //asks lecturer to enter subject code
    askString("Subject Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if lecturer has permission for this subject
    if (!subjectBelongsToLecturer(subjectCode, loggedLecturer)) {
        printf("Invalid subject or permission denied.\n");
        pressEnter();
        return;
    }

    //asks lecturer to enter attendance date
    askString("Date (YYYY-MM-DD): ", date, (int)sizeof(date));

    //checks if date is empty
    if (date[0] == '\0') {
        printf("Date cannot be empty.\n");
        pressEnter();
        return;
    }

    //opens enrollments file for reading
    //opens attendance file for appending new records
    enf = fopen(ENROLL_FILE, "r");
    atf = fopen(ATTENDANCE_FILE, "a");

    //checks if files opened successfully
    if (!enf || !atf) {
        printf("Error opening files.\n");
        if (enf) fclose(enf);
        if (atf) fclose(atf);
        pressEnter();
        return;
    }

    //prints recording information
    printf("\nRecording attendance for %s on %s\n", subjectCode, date);

    //reads each enrollment record
    while (fscanf(enf, " %19[^|]|%19[^\n]\n", studentId, sub) == 2) {

        //skips students not enrolled in this subject
        if (strcmp(sub, subjectCode) != 0) continue;

        any = 1;

        //checks if attendance was already recorded for this date
        if (attendanceExists(studentId, subjectCode, date)) {
            printf("Student %s already recorded. Skipping.\n", studentId);
            continue;
        }

        //asks lecturer to mark present or absent
        {
            int status;
            printf("Student %s: ", studentId);
            status = askStatusPA("Status (P/A or 1/0): ");

            //writes attendance record to file
            fprintf(atf, "%s|%s|%s|%d\n",
                    studentId, subjectCode, date, status);
        }
    }

    //closes both files
    fclose(enf);
    fclose(atf);

    //prints message if no students were found
    if (!any) printf("No students enrolled yet.\n");
    else {
        printf("Attendance recorded.\n");

        //automatically generates warnings for students below 80%
        autoGenerateWarningsForSubject(subjectCode);
    }

    //waits for user before returning to menu
    pressEnter();
}


//lecturer_UpdateAttendance function updates an existing attendance record
//uses a temporary file to safely update the attendance file
void lecturer_UpdateAttendance(void) {

    char studentId[20], subjectCode[20], date[15];
    int newStatus;

    FILE *f, *t;

    char fsid[20], fsub[20], fdate[15];
    int fstatus;

    int updated = 0;

    //prints section title
    printf("\n--- Update Attendance ---\n");

    //asks lecturer to enter subject code
    askString("Subject Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if lecturer has permission for this subject
    if (!subjectBelongsToLecturer(subjectCode, loggedLecturer)) {
        printf("Permission denied.\n");
        pressEnter();
        return;
    }

    //asks for date and student ID
    askString("Date (YYYY-MM-DD): ", date, (int)sizeof(date));
    askString("Student ID: ", studentId, (int)sizeof(studentId));

    //checks if attendance record exists
    if (!attendanceExists(studentId, subjectCode, date)) {
        printf("No record found to update.\n");
        pressEnter();
        return;
    }

    //asks lecturer to enter new attendance status
    newStatus = askStatusPA("New status (P/A or 1/0): ");

    //opens attendance file for reading
    //opens temporary file for writing updated records
    f = fopen(ATTENDANCE_FILE, "r");
    t = fopen("attendance_temp.txt", "w");

    //checks if files opened successfully
    if (!f || !t) {
        printf("Error opening files.\n");
        if (f) fclose(f);
        if (t) fclose(t);
        pressEnter();
        return;
    }

    //reads each attendance record
    while (fscanf(f, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                  fsid, fsub, fdate, &fstatus) == 4) {

        //checks if this is the record to update
        if (strcmp(fsid, studentId) == 0 &&
            strcmp(fsub, subjectCode) == 0 &&
            strcmp(fdate, date) == 0) {

            //updates attendance status
            fstatus = newStatus;
            updated = 1;
        }

        //writes record to temporary file
        fprintf(t, "%s|%s|%s|%d\n", fsid, fsub, fdate, fstatus);
    }

    //closes both files
    fclose(f);
    fclose(t);

    //replaces original attendance file with updated file
    remove(ATTENDANCE_FILE);
    rename("attendance_temp.txt", ATTENDANCE_FILE);

    //prints update result
    if (updated) {
        printf("Updated.\n");

        //automatically generates warnings for students below 80%
        autoGenerateWarningsForSubject(subjectCode);
    } else {
        printf("No update happened.\n");
    }

    //waits for user before returning to menu
    pressEnter();
}

//lecturer_ViewAttendanceList function displays attendance status for a subject on a specific date
//shows Present, Absent, or Not Recorded for each enrolled student
void lecturer_ViewAttendanceList(void) {

    char subjectCode[20], date[15];
    FILE *enf, *atf;
    char studentId[20], sub[20];
    int any = 0;

    //prints section title
    printf("\n--- View Attendance List ---\n");

    //asks lecturer to enter subject code
    askString("Subject Code: ", subjectCode, (int)sizeof(subjectCode));

    //checks if lecturer has permission for this subject
    if (!subjectBelongsToLecturer(subjectCode, loggedLecturer)) {
        printf("Permission denied.\n");
        pressEnter();
        return;
    }

    //asks lecturer to enter date
    askString("Date (YYYY-MM-DD): ", date, (int)sizeof(date));

    //opens enrollments file for reading
    //opens attendance file for reading
    enf = fopen(ENROLL_FILE, "r");
    atf = fopen(ATTENDANCE_FILE, "r");

    //checks if required files exist
    if (!enf || !atf) {
        printf("Data files missing.\n");
        if (enf) fclose(enf);
        if (atf) fclose(atf);
        pressEnter();
        return;
    }

    //prints attendance list header
    printf("\nAttendance List for %s on %s\n", subjectCode, date);
    printf("----------------------------------------\n");

    //reads each enrollment record
    while (fscanf(enf, " %19[^|]|%19[^\n]\n", studentId, sub) == 2) {

        int status = -1;

        //skips students not enrolled in this subject
        if (strcmp(sub, subjectCode) != 0) continue;

        any = 1;

        //rewinds attendance file to search from beginning
        rewind(atf);

        {
            char asid[20], asub[20], adate[15];
            int ast;

            //searches attendance records for matching student, subject, and date
            while (fscanf(atf, " %19[^|]|%19[^|]|%14[^|]|%d\n",
                          asid, asub, adate, &ast) == 4) {

                if (strcmp(asid, studentId) == 0 &&
                    strcmp(asub, subjectCode) == 0 &&
                    strcmp(adate, date) == 0) {

                    status = ast;
                    break;
                }
            }
        }

        //prints attendance status
        printf("Student: %s | %s\n",
               studentId,
               (status == 1) ? "Present" :
               (status == 0) ? "Absent" : "Not Recorded");
    }

    //prints message if no students are enrolled
    if (!any) printf("No students enrolled.\n");

    //closes both files
    fclose(enf);
    fclose(atf);

    //waits for user before returning to menu
    pressEnter();
}

//student_ViewAttendancePercentage function displays attendance percentage
//for all subjects enrolled by the logged-in student
void student_ViewAttendancePercentage(void) {

    FILE *ef = fopen(ENROLL_FILE, "r");
    char sid[20], sub[20];
    int found = 0;

    //prints section title
    printf("\n--- View Attendance Percentage by Subjects ---\n");

    //checks if enrollments file exists
    if (!ef) {
        printf("No enrollments found.\n");
        pressEnter();
        return;
    }

    //reads each enrollment record
    while (fscanf(ef, " %19[^|]|%19[^\n]\n", sid, sub) == 2) {

        //checks if enrollment belongs to logged student
        if (strcmp(sid, loggedStudent) == 0) {

            //prints subject code and attendance percentage
            printf("Subject: %-10s | %.2f%%\n",
                   sub, calculatePercentage(loggedStudent, sub));

            found = 1;
        }
    }

    //closes enrollments file
    fclose(ef);

    //prints message if student has no enrolled subjects
    if (!found) printf("You are not enrolled in any subject.\n");

    //waits for user before returning to menu
    pressEnter();
}

//student_ViewWarningMessage function displays warning messages
//only shows warnings if student's CURRENT attendance is below 80%
void student_ViewWarningMessage(void) {

    FILE *wf = fopen(WARNINGS_FILE, "r");

    char wid[20], wsub[20], wdate[15], wtype[10], wmsg[120];
    float pctSaved;
    int found = 0;

    //prints section title
    printf("\n--- View Reminder / Warning Message ---\n");

    //checks if warnings file exists
    if (!wf) {
        printf("No warnings.\n");
        pressEnter();
        return;
    }

    //reads each warning record (NEW FORMAT includes type)
    while (fscanf(wf, " %19[^|]|%19[^|]|%f|%14[^|]|%9[^|]|%119[^\n]\n",
                  wid, wsub, &pctSaved, wdate, wtype, wmsg) == 6) {

        //checks if warning belongs to logged student
        if (strcmp(wid, loggedStudent) == 0) {

            //calculates current attendance percentage
            float currentPct = calculatePercentage(loggedStudent, wsub);

            //shows warning only if attendance is below 80%
            if (currentPct < 80.0f) {

                //prints warning details
                printf("[%s] %s (%s): %s (Current: %.2f%%)\n",
                       wdate, wsub, wtype, wmsg, currentPct);

                found = 1;
            }
        }
    }

    //closes warnings file
    fclose(wf);

    //prints message if no active warnings exist
    if (!found) printf("No warnings (you are 80%% or above).\n");

    //waits for user before returning to menu
    pressEnter();
}


//student_Dashboard function logs in the student and opens the student menu
void student_Dashboard(void) {

    char studentId[20];

    //prints login title
    printf("\n--- Student Login ---\n");

    //asks student to enter student ID
    askString("Enter Student ID: ", studentId, (int)sizeof(studentId));

    //checks if student exists
    if (!studentExists(studentId)) {
        printf("Student not found.\n");
        pressEnter();
        return;
    }

    //stores logged-in student ID
    strcpy(loggedStudent, studentId);

    //prints success message
    printf("Login successful.\n");

    //waits before opening menu
    pressEnter();

    //opens student menu
    studentMenu();
}


//academicMenu function displays admin menu options in a loop
//admin can keep choosing options until selecting Back
void academicMenu(void) {

    int c;

    //a do while loop to keep showing the menu until user selects back
    do {

        //prints academic/admin menu options
        printf("\n--- Academic Menu ---\n");
        printf("1. Register Lecturer\n");
        printf("2. Register Subject\n");
        printf("3. View Attendance Report (By Subject / By Lecturer)\n");
        printf("4. View Student Attendance Percentage (All Subjects)\n");
        printf("5. Issue Reminder/Warning (<80%%) [AUTO LIST]\n");
        printf("6. View Warning/Reminder History\n");
        printf("7. Back\n");

        //asks user to choose a valid option from 1 to 7
        c = askIntRange("Choice: ", 1, 7);

        //runs the selected function based on user choice
        switch (c) {
            case 1: admin_RegisterLecturer(); break;
            case 2: admin_RegisterSubject(); break;
            case 3: admin_ViewReports(); break;
            case 4: admin_ViewStudentAttendancePercentage(); break;
            case 5: admin_IssueWarning(); break;
            case 6: admin_ViewWarningsHistory(); break;
            default: break; //7 Back
        }

    } while (c != 7);
}


//lecturerMenu function displays lecturer menu options in a loop
//lecturer can keep choosing options until selecting Back
void lecturerMenu(void) {

    int c;

    //a do while loop to keep showing the menu until user selects back
    do {

        //prints lecturer menu options
        printf("\n--- Lecturer Menu (%s) ---\n", loggedLecturer);
        printf("1. Register Students based on Subject\n");
        printf("2. Record Daily Attendance\n");
        printf("3. Update Attendance Records\n");
        printf("4. View Attendance List (by subject)\n");
        printf("5. Back\n");

        //asks lecturer to choose a valid option from 1 to 5
        c = askIntRange("Choice: ", 1, 5);

        //runs the selected function based on lecturer choice
        switch (c) {
            case 1: lecturer_EnrollStudent(); break;
            case 2: lecturer_MarkAttendance(); break;
            case 3: lecturer_UpdateAttendance(); break;
            case 4: lecturer_ViewAttendanceList(); break;
            default: break; //5 Back
        }

    } while (c != 5);
}

//studentMenu function displays student menu options in a loop
//student can keep choosing options until selecting Back
void studentMenu(void) {

    int c;

    //a do while loop to keep showing the menu until user selects back
    do {

        //prints student menu options
        printf("\n--- Student Menu (%s) ---\n", loggedStudent);
        printf("1. View Attendance Percentage by Subjects\n");
        printf("2. View Reminder / Warning Message\n");
        printf("3. Back\n");

        //asks student to choose a valid option
        c = askIntRange("Choice: ", 1, 3);

        //runs the selected function based on user choice
        switch (c) {
            case 1: student_ViewAttendancePercentage(); break;
            case 2: student_ViewWarningMessage(); break;
            default: break; //3 Back
        }

    } while (c != 3);
}


//main function starts the program and shows the role menu
//controls the full system flow until user exits
int main(void) {

    int choice;

    //calls setupSystem to make sure all required files exist
    setupSystem();

    //an infinite loop to keep showing the main role menu
    while (1) {

        //prints system title and role options
        printf("\n============================\n");
        printf("   ATTENDANCE SYSTEM\n");
        printf("============================\n");
        printf("1. Academic Staff\n");
        printf("2. Lecturer\n");
        printf("3. Student\n");
        printf("4. Exit\n");

        //asks user to pick a valid role from 1 to 4
        choice = askIntRange("Choice: ", 1, 4);

        //checks which role was chosen
        if (choice == 1) {

            //opens academic menu
            academicMenu();

        } else if (choice == 2) {

            //lecturer must login first before accessing lecturer menu
            if (lecturer_Login()) lecturerMenu();

        } else if (choice == 3) {

            //opens student dashboard
            student_Dashboard();

        } else {

            //breaks loop and exits program
            break;
        }
    }

    //program ends successfully
    return 0;
}