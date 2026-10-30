#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME_LENGTH 100
#define COURSE_LENGTH 100
#define INITIAL_STUDENT_COUNT 10
#define MENU_OPTIONS 10

#define MY_STUDENT_ID "1234376"
#define PASS_GRADE 50.0

typedef struct {
    int id;
    char name[NAME_LENGTH];
    char course[COURSE_LENGTH];
    float grade;
} Student;

typedef struct {
    Student *students;
    int count;
    bool program_running;
} StudentDatabase;

// creating a structure for courses comparison
typedef struct {
    char course[COURSE_LENGTH];
    float total_grade;
    int student_count;
} CoursePerformance;

// creating one function pointer type for all menu operations
typedef void (*MenuOperation)(StudentDatabase *database);

// clearing the console before showing a new screen
void clear_screen(void) {
    printf("\033c");
    fflush(stdout);
}

// clearing input buffer for invalid inputs
void clear_input_buffer(void) {
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

// creating one function to safely read integer values
bool read_integer(char message[], int *value) {
    int scan_result;
    int next_character;

    printf("%s", message);
    scan_result = scanf("%d", value);
    next_character = getchar();

    if (scan_result != 1 || next_character != '\n') {
        if (next_character != '\n') {
            clear_input_buffer();
        }

        printf("\nError! Enter a valid whole number.\n");
        return false;
    }

    return true;
}

// creating one function to safely read float values
bool read_float(char message[], float *value) {
    int scan_result;
    int next_character;

    printf("%s", message);
    scan_result = scanf("%f", value);
    next_character = getchar();

    if (scan_result != 1 || next_character != '\n') {
        if (next_character != '\n') {
            clear_input_buffer();
        }

        printf("\nError! Enter a valid number.\n");
        return false;
    }

    return true;
}

// asking the user to either return to the menu or exit the program
void back_to_menu_or_exit(StudentDatabase *database) {
    int navigation_choice;

    while (database->program_running) {
        printf("\n\t1. Back to Menu\n");
        printf("\t2. Exit the Program\n");

        if (
            !read_integer("\n\tSelect an option: ",&navigation_choice)) {
            continue;
        }

        if (navigation_choice == 1) {
            return;
        }

        if (navigation_choice == 2) {
            database->program_running = false;
            printf("\nExiting Project 1...\n");
            return;
        }

        printf("\nError! Select either 1 or 2.\n");
    }
}

// creating one function to accept text including spaces
void read_text(char message[], char value[], int size) {
    printf("%s", message);

    if (fgets(value, size, stdin) != NULL) {
        value[strcspn(value, "\n")] = '\0';
    }
}

// checking if the student ID already exists before adding a new record
bool student_id_exists(StudentDatabase *database, int student_id) {
    Student *current_student;
    Student *last_student;

    current_student = database->students;
    last_student = database->students + database->count;

    while (current_student < last_student) {
        if (current_student->id == student_id) {
            return true;
        }

        current_student++;
    }

    return false;
}

// function for allocating memory and loading my initial student dataset
bool load_initial_students(StudentDatabase *database) {
    int i;
    // creating the initial dataset
    Student initial_students[INITIAL_STUDENT_COUNT] = {
        {1001, "Helen Okereke", "C Programming", 78.5},
        {1002, "Jefferson Achaia", "Data Structures", 64.0},
        {1003, "Wisdom Okechukwu", "C Programming", 89.5},
        {1004, "Ishami Irene", "Computer Systems", 46.0},
        {1005, "Robert Niyosengi", "Data Structures", 72.5},
        {1006, "Grace Njunge", "Computer Systems", 55.0},
        {1007, "Chipo Hamza", "C Programming", 93.0},
        {1008, "Henry Nsengimana", "Data Structures", 39.5},
        {1009, "Irene Uwamahoro", "Computer Systems", 81.0},
        {1010, "John Ishimwe", "C Programming", 67.5}
    };

    database->students = malloc(INITIAL_STUDENT_COUNT * sizeof(Student));

    if (database->students == NULL) {
        printf("\nError! Initial memory allocation failed.\n");
        return false;
    }

    database->count = INITIAL_STUDENT_COUNT;
    database->program_running = true;

    for (i = 0; i < database->count; i++) {
        database->students[i] = initial_students[i];
    }

    return true;
}

// printing the required project identification before the menu
void print_project_header(void) {
    printf("\n");
    printf("\tStudent ID: %s\n", MY_STUDENT_ID);
    printf("\tProject: Dynamic Academic Performance and Risk Analysis Engine\n");
    printf("\tCustom Analysis: Course Performance Comparison\n");
}

// printing the application menu
void print_menu(void) {
    printf("\n");
    printf("\t=======ACADEMIC PERFORMANCE MENU=======\n");
    printf("\t1.  Add a New Student Record\n");
    printf("\t2.  Display All Student Records\n");
    printf("\t3.  Sort Student Records\n");
    printf("\t4.  Calculate the Average Grade\n");
    printf("\t5.  Find the Top-Performing Student\n");
    printf("\t6.  Search for a Student\n");
    printf("\t7.  Remove a Student Record\n");
    printf("\t8.  Run Custom Academic Analysis\n");
    printf("\t9.  Display Summary Report\n");
    printf("\t10. Exit the Program\n");
}

// adding a new student and expanding the allocated memory
void add_student(StudentDatabase *database) {
    Student new_student;
    Student *resized_students;
    bool valid_input;

    printf("\n\t===== ADD A NEW STUDENT =====\n");

    valid_input = false;

    while (!valid_input) {
        valid_input = read_integer(
            "Enter the student ID: ",
            &new_student.id
        );

        if (valid_input && new_student.id < 1) {
            printf("\nError! Student ID must be above zero.\n");
            valid_input = false;
        }

        if (valid_input && student_id_exists(database, new_student.id)) {
            printf("\nError! This student ID already exists.\n");
            valid_input = false;
        }
    }

    read_text(
        "Enter the student's full name: ",
        new_student.name,
        NAME_LENGTH
    );

    read_text(
        "Enter the course or module name: ",
        new_student.course,
        COURSE_LENGTH
    );

    valid_input = false;

    while (!valid_input) {
        valid_input = read_float(
            "Enter the student's grade from 0 to 100: ",
            &new_student.grade
        );

        if (
            valid_input &&
            (new_student.grade < 0 || new_student.grade > 100)
        ) {
            printf("\nError! Grade must be from 0 to 100.\n");
            valid_input = false;
        }
    }

    resized_students = realloc(
        database->students,
        (database->count + 1) * sizeof(Student)
    );

    if (resized_students == NULL) {
        printf("\nError! Memory allocation failed.\n");
        return;
    }

    database->students = resized_students;
    database->students[database->count] = new_student;
    database->count++;

    printf("\nStudent record added successfully.\n");
}

// displaying records using pointer traversal instead of array indexing
void display_students(StudentDatabase *database) {
    Student *current_student;
    Student *last_student;

    printf("\n");
    printf("===================================================================="
           "============\n");
    printf("%-10s %-25s %-25s %-10s\n",
           "ID", "STUDENT NAME", "COURSE", "GRADE");
    printf("===================================================================="
           "============\n");

    if (database->count == 0) {
        printf("No student records are available.\n");
        printf("================================================================"
               "================\n");
        return;
    }

    current_student = database->students;
    last_student = database->students + database->count;

    while (current_student < last_student) {
        printf(
            "%-10d %-25s %-25s %-10.2f\n",
            current_student->id,
            current_student->name,
            current_student->course,
            current_student->grade
        );

        current_student++;
    }

    printf("===================================================================="
           "============\n");
    printf("Total records: %d\n", database->count);
}

// implementing sorting with either Grade, name or Student ID
void sort_students(StudentDatabase *database) {
    int sort_choice;
    int order_choice;
    int i;
    int j;
    bool should_swap;
    Student temporary_student;

    if (database->count == 0) {
        printf("\nThere are no student records to sort.\n");
        return;
    }

    if (database->count == 1) {
        printf("\nThere is only one student record.\n");
        display_students(database);
        return;
    }

    printf("\n\t===== SORT STUDENT RECORDS =====\n");
    printf("\t1. Sort by Grade\n");
    printf("\t2. Sort by Name\n");
    printf("\t3. Sort by Student ID\n");

    if (
        !read_integer(
            "\nSelect how you want to sort: ",
            &sort_choice
        )
    ) {
        return;
    }

    if (sort_choice < 1 || sort_choice > 3) {
        printf("\nError! Select an option from 1 to 3.\n");
        return;
    }

    printf("\n\t1. Ascending Order\n");
    printf("\t2. Descending Order\n");

    if (
        !read_integer(
            "\nSelect the sorting order: ",
            &order_choice
        )
    ) {
        return;
    }

    if (order_choice < 1 || order_choice > 2) {
        printf("\nError! Select either 1 or 2.\n");
        return;
    }

    // comparing two students and moving them based on the selected option
    for (i = 0; i < database->count - 1; i++) {
        for (j = 0; j < database->count - i - 1; j++) {
            should_swap = false;

            if (sort_choice == 1) {
                if (order_choice == 1 && database->students[j].grade > database->students[j + 1].grade) {
                    should_swap = true;
                }

                if (
                    order_choice == 2 &&
                    database->students[j].grade <
                    database->students[j + 1].grade
                ) {
                    should_swap = true;
                }
            }
            else if (sort_choice == 2) {
                if (order_choice == 1 &&
                    strcmp(
                        database->students[j].name,
                        database->students[j + 1].name) > 0) {
                    should_swap = true;
                }

                if (order_choice == 2 && strcmp(database->students[j].name,database->students[j + 1].name) < 0) {
                    should_swap = true;
                }
            }
            else {
                if (order_choice == 1 && database->students[j].id > database->students[j + 1].id) {
                    should_swap = true;
                }

                if (order_choice == 2 && database->students[j].id < database->students[j + 1].id) {
                    should_swap = true;
                }
            }

            if (should_swap) {
                temporary_student = database->students[j];
                database->students[j] =
                    database->students[j + 1];
                database->students[j + 1] =
                    temporary_student;
            }
        }
    }

    printf("\nStudent records sorted successfully.\n");
    display_students(database);
}

// calculating and returning the average for function reuse
float get_average_grade(StudentDatabase *database) {
    Student *current_student;
    Student *last_student;
    float total_grade = 0;

    if (database->count == 0) {
        return 0;
    }

    current_student = database->students;
    last_student = database->students + database->count;

    while (current_student < last_student) {
        total_grade += current_student->grade;
        current_student++;
    }

    return total_grade / database->count;
}

// calculating the class average using pointer traversal
void calculate_average(StudentDatabase *database) {
    float average_grade;

    if (database->count == 0) {
        printf("\nThere are no student records available.\n");
        return;
    }
    average_grade = get_average_grade(database);
    printf("\n\t===== AVERAGE GRADE =====\n");
    printf("\tTotal Students: %d\n", database->count);
    printf("\tAverage Grade: %.2f\n", average_grade);
}

// finding and returning the student with the highest grade
Student *get_top_student(StudentDatabase *database) {
    Student *top_student;
    Student *current_student;
    Student *last_student;

    if (database->count == 0) {
        return NULL;
    }

    top_student = database->students;
    current_student = database->students + 1;
    last_student = database->students + database->count;

    while (current_student < last_student) {
        if (current_student->grade > top_student->grade) {
            top_student = current_student;
        }
        current_student++;
    }
    return top_student;
}

// finding the student with the highest grade
void find_top_student(StudentDatabase *database) {
    Student *top_student;

    if (database->count == 0) {
        printf("\nThere are no student records available.\n");
        return;
    }
    top_student = get_top_student(database);

    printf("\n\t===== TOP-PERFORMING STUDENT =====\n");
    printf("\tStudent ID: %d\n", top_student->id);
    printf("\tName: %s\n", top_student->name);
    printf("\tCourse: %s\n", top_student->course);
    printf("\tGrade: %.2f\n", top_student->grade);
}

// checking if a name contains the searched text without checking letter case
bool contains_ignore_case(char text[], char search[]) {
    char *text_position;
    char *current_text;
    char *current_search;

    text_position = text;

    while (*text_position != '\0') {
        current_text = text_position;
        current_search = search;

        while (
            *current_text != '\0' &&
            *current_search != '\0' &&
            tolower((unsigned char)*current_text) ==
            tolower((unsigned char)*current_search)
        ) {
            current_text++;
            current_search++;
        }

        if (*current_search == '\0') {
            return true;
        }

        text_position++;
    }

    return false;
}

// searching using either the student ID or part of the student's name
void search_student(StudentDatabase *database) {
    int search_choice;
    int student_id;
    char student_name[NAME_LENGTH];
    Student *current_student;
    Student *last_student;
    bool student_found = false;

    if (database->count == 0) {
        printf("\nThere are no student records available.\n");
        return;
    }

    printf("\n\t===== SEARCH FOR A STUDENT =====\n");
    printf("\t1. Search using Student ID\n");
    printf("\t2. Search using Student Name\n");

    if (
        !read_integer(
            "\nSelect how you want to search: ",
            &search_choice
        )
    ) {
        return;
    }

    if (search_choice < 1 || search_choice > 2) {
        printf("\nError! Select either 1 or 2.\n");
        return;
    }

    current_student = database->students;
    last_student = database->students + database->count;

    if (search_choice == 1) {
        if (
            !read_integer("Enter the student ID: ",&student_id)) {
            return;
        }

        while (current_student < last_student) {
            if (current_student->id == student_id) {
                student_found = true;

                printf("\n\t===== STUDENT FOUND =====\n");
                printf("\tStudent ID: %d\n", current_student->id);
                printf("\tName: %s\n", current_student->name);
                printf("\tCourse: %s\n", current_student->course);
                printf("\tGrade: %.2f\n", current_student->grade);

                break;
            }

            current_student++;
        }

        if (!student_found) {
            printf(
                "\nNo student was found with ID %d.\n",
                student_id
            );
        }
    }
    else {
        read_text(
            "Type the any letter to start searching: ",
            student_name,
            NAME_LENGTH
        );

        if (strlen(student_name) == 0) {
            printf("\nError! The search name cannot be empty.\n");
            return;
        }

        printf("\n\tSearching for a student...\n");
        sleep(2);
        while (current_student < last_student) {
            if (
                contains_ignore_case(
                    current_student->name,
                    student_name
                )
            ) {
                if (!student_found) {
                    printf("\n\t===== SEARCH RESULTS =====\n");
                    printf(
                        "%-10s %-25s %-25s %-10s\n",
                        "ID",
                        "STUDENT NAME",
                        "COURSE",
                        "GRADE"
                    );
                }

                student_found = true;

                printf(
                    "%-10d %-25s %-25s %-10.2f\n",
                    current_student->id,
                    current_student->name,
                    current_student->course,
                    current_student->grade
                );
            }

            current_student++;
        }

        if (!student_found) {
            printf(
                "\nNo student name contains \"%s\".\n",
                student_name
            );
        }
    }
}

// removing a record and shrinking the allocated memory
void remove_student(StudentDatabase *database) {
    int student_id;
    int student_index;
    int i;
    Student *resized_students;

    if (database->count == 0) {
        printf("\nThere are no student records to remove.\n");
        return;
    }

    if (
        !read_integer(
            "\nEnter the ID of the student to remove: ",
            &student_id
        )
    ) {
        return;
    }

    student_index = -1;

    for (i = 0; i < database->count; i++) {
        if (database->students[i].id == student_id) {
            student_index = i;
            break;
        }
    }

    if (student_index == -1) {
        printf("\nNo student was found with ID %d.\n", student_id);
        return;
    }

    for (i = student_index; i < database->count - 1; i++) {
        database->students[i] = database->students[i + 1];
    }

    database->count--;

    if (database->count == 0) {
        free(database->students);
        database->students = NULL;

        printf("\nStudent record removed successfully.\n");
        return;
    }

    resized_students = realloc(
        database->students,
        database->count * sizeof(Student)
    );

    if (resized_students != NULL) {
        database->students = resized_students;
    }

    printf("\nStudent record removed successfully.\n");
}

// comparing the average performance of the different courses
void custom_analysis(StudentDatabase *database) {
    CoursePerformance *course_results;
    Student *current_student;
    Student *last_student;
    int course_count = 0;
    int course_index;
    int i;
    int highest_course_index = 0;
    int lowest_course_index = 0;
    float course_average;
    float highest_course_average;
    float lowest_course_average;
    float overall_average;
    float average_difference;
    bool course_found;

    if (database->count == 0) {
        printf("\nThere are no student records available.\n");
        return;
    }

    course_results = calloc(
        database->count,
        sizeof(CoursePerformance)
    );

    if (course_results == NULL) {
        printf("\nError! Memory allocation failed.\n");
        return;
    }

    current_student = database->students;
    last_student = database->students + database->count;

    // grouping students who are taking the same course
    while (current_student < last_student) {
        course_found = false;

        for (i = 0; i < course_count; i++) {
            if (
                strcmp(
                    course_results[i].course,
                    current_student->course
                ) == 0
            ) {
                course_results[i].total_grade +=
                    current_student->grade;

                course_results[i].student_count++;
                course_found = true;
                break;
            }
        }

        if (!course_found) {
            strcpy(
                course_results[course_count].course,
                current_student->course
            );

            course_results[course_count].total_grade =
                current_student->grade;

            course_results[course_count].student_count = 1;
            course_count++;
        }

        current_student++;
    }

    overall_average = get_average_grade(database);

    highest_course_average =
        course_results[0].total_grade /
        course_results[0].student_count;

    lowest_course_average = highest_course_average;

    printf("\n\t===== COURSE PERFORMANCE COMPARISON =====\n");
    printf("\tOverall Student Average: %.2f\n\n", overall_average);

    printf(
        "%-25s %-12s %-15s %-15s %-20s\n",
        "COURSE",
        "STUDENTS",
        "COURSE AVG",
        "DIFFERENCE",
        "COMPARISON"
    );

    printf(
        "===================================================================="
        "====================\n"
    );

    for (course_index = 0;
         course_index < course_count;
         course_index++) {
        course_average =
            course_results[course_index].total_grade /
            course_results[course_index].student_count;

        average_difference = course_average - overall_average;

        printf(
            "%-25s %-12d %-15.2f %-+15.2f ",
            course_results[course_index].course,
            course_results[course_index].student_count,
            course_average,
            average_difference
        );

        if (average_difference > 0) {
            printf("%-20s\n", "Above Overall Average");
        }
        else if (average_difference < 0) {
            printf("%-20s\n", "Below Overall Average");
        }
        else {
            printf("%-20s\n", "Equal to Overall Average");
        }

        if (course_average > highest_course_average) {
            highest_course_average = course_average;
            highest_course_index = course_index;
        }

        if (course_average < lowest_course_average) {
            lowest_course_average = course_average;
            lowest_course_index = course_index;
        }
    }

    printf(
        "===================================================================="
        "====================\n"
    );

    printf(
        "\n\tHighest-Performing Course: %s with %.2f\n",
        course_results[highest_course_index].course,
        highest_course_average
    );

    printf(
        "\tCourse Requiring More Support: %s with %.2f\n",
        course_results[lowest_course_index].course,
        lowest_course_average
    );

    free(course_results);
}

// producing the complete required statistical summary
void generate_summary_report(StudentDatabase *database) {
    Student *current_student;
    Student *last_student;
    Student *top_student;
    Student *lowest_student;
    float average_grade;
    int passed_students = 0;
    int at_risk_students = 0;

    if (database->count == 0) {
        printf("\nThere are no student records available.\n");
        return;
    }

    average_grade = get_average_grade(database);
    top_student = get_top_student(database);
    lowest_student = database->students;

    current_student = database->students;
    last_student = database->students + database->count;

    // counting passed and at-risk students while finding the lowest grade
    while (current_student < last_student) {
        if (current_student->grade >= PASS_GRADE) {
            passed_students++;
        }
        else {
            at_risk_students++;
        }

        if (current_student->grade < lowest_student->grade) {
            lowest_student = current_student;
        }

        current_student++;
    }

    printf("\n\t===== ACADEMIC PERFORMANCE SUMMARY =====\n");
    printf("\tTotal Student Records: %d\n", database->count);
    printf("\tOverall Average Grade: %.2f\n", average_grade);
    printf("\tPass Threshold: %.2f\n", PASS_GRADE);

    printf(
        "\tHighest Grade: %.2f - %s\n",
        top_student->grade,
        top_student->name
    );

    printf(
        "\tLowest Grade: %.2f - %s\n",
        lowest_student->grade,
        lowest_student->name
    );

    printf(
        "\tStudents Meeting the Pass Threshold: %d\n",
        passed_students
    );

    printf(
        "\tStudents Below the Pass Threshold: %d\n",
        at_risk_students
    );

    printf("\n\t===== CUSTOM ANALYSIS RESULTS =====\n");

    custom_analysis(database);
}

// changing the program state so that the main loop can finish
void exit_program(StudentDatabase *database) {
    database->program_running = false;

    printf("\nExiting the Academic Performance Analysis Engine...\n");
}

int main(void) {
    StudentDatabase database;
    int choice;

    MenuOperation menu_operations[MENU_OPTIONS] = {
        add_student,
        display_students,
        sort_students,
        calculate_average,
        find_top_student,
        search_student,
        remove_student,
        custom_analysis,
        generate_summary_report,
        exit_program
    };

    database.students = NULL;
    database.count = 0;
    database.program_running = true;

    if (!load_initial_students(&database)) {
        return 1;
    }

    while (database.program_running) {
        clear_screen();
        print_project_header();
        print_menu();

        if (!read_integer("\tSelect an option from 1 to 10: ", &choice)) {
            continue;
        }

        if (choice < 1 || choice > MENU_OPTIONS) {
            printf("\nError! Select an option from 1 to 10.\n");
            continue;
        }

        // calling the operation selected by the user
        clear_screen();
        menu_operations[choice - 1](&database);

        if (database.program_running) {
            back_to_menu_or_exit(&database);
        }
    }

    // releasing all student memory before the program ends
    free(database.students);
    database.students = NULL;

    return 0;
}
