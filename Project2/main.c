#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOURCE_LENGTH 150
#define INPUT_CHUNK 256
#define MINIMUM_WORDS 150

typedef struct {
    char *text;
    char source_title[SOURCE_LENGTH];
    char source_details[SOURCE_LENGTH];
} TextData;


#define WORD_LENGTH 40
#define INITIAL_WORD_CAPACITY 10
#define MENU_OPTIONS 10
#define MY_STUDENT_ID "1234376"

typedef struct {
    char name[WORD_LENGTH];
    int count;
} WordStat;

typedef struct {
    TextData text_data;
    WordStat *word_statistics;
    int total_words;
    int distinct_words;
    int statistics_capacity;
    int total_letters;
    char longest_word[WORD_LENGTH];
    int longest_word_length;
    bool program_running;
} TextAnalyzer;

typedef void (*AnalysisOperation)(TextAnalyzer *analyzer);

// clearing the terminal before displaying a new screen
void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

// pausing before returning to the main menu
void wait_for_menu(void) {
    printf("\n\tPress Enter to return to the main menu...");
    getchar();
}

// the default paragraph from John 3:16-22 in the King James Version
const char DEFAULT_TEXT[] =
    "For God so loved the world, that he gave his only begotten Son, "
    "that whosoever believeth in him should not perish, but have "
    "everlasting life. For God sent not his Son into the world to "
    "condemn the world; but that the world through him might be saved. "
    "He that believeth on him is not condemned: but he that believeth "
    "not is condemned already, because he hath not believed in the name "
    "of the only begotten Son of God. And this is the condemnation, "
    "that light is come into the world, and men loved darkness rather "
    "than light, because their deeds were evil. For every one that "
    "doeth evil hateth the light, neither cometh to the light, lest his "
    "deeds should be reproved. But he that doeth truth cometh to the "
    "light, that his deeds may be made manifest, that they are wrought "
    "in God. After these things came Jesus and his disciples into the "
    "land of Judaea; and there he tarried with them, and baptized.";

// clearing the remaining input after an invalid entry
void clear_input_buffer(void) {
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

// safely reading one whole number
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

// reading required text such as the title or publication details
void read_required_text(char message[], char value[], int size) {
    do {
        printf("%s", message);

        if (fgets(value, size, stdin) == NULL) {
            value[0] = '\0';
        }
        else {
            value[strcspn(value, "\n")] = '\0';
        }

        if (strlen(value) == 0) {
            printf("\nError! This field cannot be empty.\n");
        }
    } while (strlen(value) == 0);
}

// counting words using pointer traversal
int count_words_in_text(char *text) {
    char *current_character;
    int total_words = 0;
    bool inside_word = false;

    current_character = text;

    while (*current_character != '\0') {
        if (isalpha((unsigned char)*current_character)) {
            if (!inside_word) {
                total_words++;
                inside_word = true;
            }
        }
        else if (
            *current_character == '\'' &&
            inside_word &&
            isalpha((unsigned char)*(current_character + 1))
        ) {
            // making sure I keep an apostrophe inside words such as don't
        }
        else {
            inside_word = false;
        }
        current_character++;
    }

    return total_words;
}

// creating a dynamic copy of the built-in paragraph
char *copy_default_text(void) {
    char *text;
    size_t required_size;

    required_size = strlen(DEFAULT_TEXT) + 1;
    text = malloc(required_size);

    if (text == NULL) {
        printf("\nError! Text memory allocation failed.\n");
        return NULL;
    }

    strcpy(text, DEFAULT_TEXT);
    return text;
}

// allowing the user to paste text without assuming a fixed length
char *read_dynamic_text(void) {
    char input_line[INPUT_CHUNK];
    char *text;
    char *resized_text;
    size_t capacity = INPUT_CHUNK;
    size_t text_length = 0;
    size_t line_length;
    size_t required_size;

    text = malloc(capacity);

    if (text == NULL) {
        printf("\nError! Text memory allocation failed.\n");
        return NULL;
    }

    text[0] = '\0';

    printf("\nPaste or type the paragraph below.\n");
    printf("Type END on a separate line when you are finished.\n\n");

    while (fgets(input_line, sizeof(input_line), stdin) != NULL) {
        if (strcmp(input_line, "END\n") == 0 || strcmp(input_line, "END") == 0) {
            break;
        }

        line_length = strlen(input_line);
        required_size = text_length + line_length + 1;

        if (required_size > capacity) {
            while (capacity < required_size) {
                capacity *= 2;
            }

            resized_text = realloc(text, capacity);

            if (resized_text == NULL) {
                printf("\nError! Text memory expansion failed.\n");
                free(text);
                return NULL;
            }
            text = resized_text;
        }

        strcpy(text + text_length, input_line);
        text_length += line_length;
    }

    if (text_length == 0) {
        printf("\nError! The paragraph cannot be empty.\n");
        free(text);
        return NULL;
    }
    return text;
}

// asking whether to use the default paragraph or enter another one
bool select_text_dataset(TextData *text_data) {
    int text_choice;
    int total_words;

    while (true) {
        printf("\n\t===== SELECT TEXT DATASET =====\n");
        printf("\t1. Use John 3:16-22 from the King James Version\n");
        printf("\t2. Enter or paste another paragraph\n");

        if (
            !read_integer("\n\tSelect an option: ", &text_choice)) {
            continue;
        }

        if (text_choice == 1) {
            text_data->text = copy_default_text();

            if (text_data->text == NULL) {
                return false;
            }
            strcpy(text_data->source_title, "The Holy Bible");

            strcpy(
                text_data->source_details,
                "King James Version, John 3:16-22, Public Domain"
            );
            break;
        }

        if (text_choice == 2) {
            read_required_text(
                "Enter the source title: ",
                text_data->source_title,
                SOURCE_LENGTH
            );
            read_required_text(
                "Enter the author or publication details: ",
                text_data->source_details,
                SOURCE_LENGTH
            );

            while (true) {
                text_data->text = read_dynamic_text();

                if (text_data->text == NULL) {
                    printf("\nPlease enter the paragraph again.\n");
                    continue;
                }

                total_words = count_words_in_text(text_data->text);

                if (total_words < MINIMUM_WORDS) {
                    printf("\nError! The paragraph contains %d words.\n", total_words);
                    printf("A minimum of %d words is required.\n", MINIMUM_WORDS);

                    free(text_data->text);
                    text_data->text = NULL;
                    continue;
                }
                break;
            }
            break;
        }

        printf("\nError! Select either 1 or 2.\n");
    }

    total_words = count_words_in_text(text_data->text);

    printf("\n\t===== TEXT DATASET LOADED =====\n");
    printf("\tSource: %s\n", text_data->source_title);
    printf("\tDetails: %s\n", text_data->source_details);
    printf("\tWords loaded: %d\n", total_words);

    return true;
}

// checking whether a character belongs inside a word
bool is_word_character(char *position, char *word_start) {
    if (isalpha((unsigned char)*position)) {
        return true;
    }

    if (
        *position == '\'' &&
        position > word_start &&
        isalpha((unsigned char)*(position - 1)) &&
        isalpha((unsigned char)*(position + 1))
    ) {
        return true;
    }

    return false;
}

// expanding the word-statistics array when another word is needed
bool expand_word_statistics(TextAnalyzer *analyzer) {
    WordStat *resized_statistics;
    int new_capacity;

    if (analyzer->distinct_words < analyzer->statistics_capacity) {
        return true;
    }

    new_capacity = analyzer->statistics_capacity * 2;
    resized_statistics = realloc(
        analyzer->word_statistics,
        new_capacity * sizeof(WordStat)
    );

    if (resized_statistics == NULL) {
        printf("\nError! Word-statistics memory expansion failed.\n");
        return false;
    }

    analyzer->word_statistics = resized_statistics;
    analyzer->statistics_capacity = new_capacity;
    return true;
}

// adding one normalized word or increasing its existing frequency
bool add_word_statistic(TextAnalyzer *analyzer, char word[]) {
    int i;

    for (i = 0; i < analyzer->distinct_words; i++) {
        if (strcmp(analyzer->word_statistics[i].name, word) == 0) {
            analyzer->word_statistics[i].count++;
            return true;
        }
    }

    if (!expand_word_statistics(analyzer)) {
        return false;
    }

    strcpy(
        analyzer->word_statistics[analyzer->distinct_words].name,
        word
    );
    analyzer->word_statistics[analyzer->distinct_words].count = 1;
    analyzer->distinct_words++;
    return true;
}

// extracting and normalizing every word using pointer traversal
bool process_text(TextAnalyzer *analyzer) {
    char *current_character;
    char *word_start;
    char normalized_word[WORD_LENGTH];
    int stored_length;
    int actual_length;

    analyzer->statistics_capacity = INITIAL_WORD_CAPACITY;
    analyzer->word_statistics = calloc(
        analyzer->statistics_capacity,
        sizeof(WordStat)
    );

    if (analyzer->word_statistics == NULL) {
        printf("\nError! Word-statistics memory allocation failed.\n");
        return false;
    }

    current_character = analyzer->text_data.text;

    while (*current_character != '\0') {
        while (
            *current_character != '\0' &&
            !isalpha((unsigned char)*current_character)
        ) {
            current_character++;
        }

        if (*current_character == '\0') {
            break;
        }

        word_start = current_character;
        stored_length = 0;
        actual_length = 0;

        while (
            *current_character != '\0' &&
            is_word_character(current_character, word_start)
        ) {
            if (stored_length < WORD_LENGTH - 1) {
                normalized_word[stored_length] =
                    (char)tolower((unsigned char)*current_character);
                stored_length++;
            }

            if (isalpha((unsigned char)*current_character)) {
                analyzer->total_letters++;
                actual_length++;
            }

            current_character++;
        }

        normalized_word[stored_length] = '\0';
        analyzer->total_words++;

        if (actual_length > analyzer->longest_word_length) {
            analyzer->longest_word_length = actual_length;
            strcpy(analyzer->longest_word, normalized_word);
        }

        if (!add_word_statistic(analyzer, normalized_word)) {
            return false;
        }
    }

    return true;
}

// returning the most frequent word-statistics entry
WordStat *get_most_frequent(TextAnalyzer *analyzer) {
    WordStat *most_frequent_word;
    WordStat *current_word;
    WordStat *last_word;

    if (analyzer->distinct_words == 0) {
        return NULL;
    }

    most_frequent_word = analyzer->word_statistics;
    current_word = analyzer->word_statistics + 1;
    last_word = analyzer->word_statistics + analyzer->distinct_words;

    while (current_word < last_word) {
        if (current_word->count > most_frequent_word->count) {
            most_frequent_word = current_word;
        }
        current_word++;
    }

    return most_frequent_word;
}

// displaying the total number of words
void count_words(TextAnalyzer *analyzer) {
    printf("\n\t===== TOTAL WORD COUNT =====\n");
    printf("\tTotal Words: %d\n", analyzer->total_words);
}

// displaying the longest extracted word
void longest_word(TextAnalyzer *analyzer) {
    printf("\n\t===== LONGEST WORD =====\n");
    printf("\tWord: %s\n", analyzer->longest_word);
    printf("\tLength: %d alphabetic characters\n", analyzer->longest_word_length);
}

// displaying the word with the highest frequency
void most_frequent(TextAnalyzer *analyzer) {
    WordStat *frequent_word;

    frequent_word = get_most_frequent(analyzer);

    printf("\n\t===== MOST FREQUENT WORD =====\n");
    printf("\tWord: %s\n", frequent_word->name);
    printf("\tFrequency: %d\n", frequent_word->count);
}

// displaying all distinct words and their frequencies
void display_word_statistics(TextAnalyzer *analyzer) {
    WordStat *current_word;
    WordStat *last_word;

    printf("\n\t===== WORD STATISTICS =====\n");
    printf("%-8s %-40s %-10s\n", "NUMBER", "WORD", "FREQUENCY");
    printf("============================================================\n");

    current_word = analyzer->word_statistics;
    last_word = analyzer->word_statistics + analyzer->distinct_words;

    while (current_word < last_word) {
        printf(
            "%-8ld %-40s %-10d\n",
            (long)(current_word - analyzer->word_statistics + 1),
            current_word->name,
            current_word->count
        );
        current_word++;
    }

    printf("============================================================\n");
    printf("Distinct words: %d\n", analyzer->distinct_words);
}

// sorting the word statistics alphabetically or by frequency
void sort_word_statistics(TextAnalyzer *analyzer) {
    int sort_choice;
    int order_choice;
    int i;
    int j;
    bool should_swap;
    WordStat temporary_word;

    printf("\n\t===== SORT WORD STATISTICS =====\n");
    printf("\t1. Sort Alphabetically\n");
    printf("\t2. Sort by Frequency\n");

    if (!read_integer("\n\tSelect a sorting method: ", &sort_choice)) {
        return;
    }

    if (sort_choice < 1 || sort_choice > 2) {
        printf("\nError! Select either 1 or 2.\n");
        return;
    }

    printf("\n\t1. Ascending Order\n");
    printf("\t2. Descending Order\n");

    if (!read_integer("\n\tSelect the sorting order: ", &order_choice)) {
        return;
    }

    if (order_choice < 1 || order_choice > 2) {
        printf("\nError! Select either 1 or 2.\n");
        return;
    }

    for (i = 0; i < analyzer->distinct_words - 1; i++) {
        for (j = 0; j < analyzer->distinct_words - i - 1; j++) {
            should_swap = false;

            if (sort_choice == 1) {
                if (
                    order_choice == 1 &&
                    strcmp(
                        analyzer->word_statistics[j].name,
                        analyzer->word_statistics[j + 1].name
                    ) > 0
                ) {
                    should_swap = true;
                }

                if (
                    order_choice == 2 &&
                    strcmp(
                        analyzer->word_statistics[j].name,
                        analyzer->word_statistics[j + 1].name
                    ) < 0
                ) {
                    should_swap = true;
                }
            }
            else {
                if (
                    order_choice == 1 &&
                    analyzer->word_statistics[j].count >
                    analyzer->word_statistics[j + 1].count
                ) {
                    should_swap = true;
                }

                if (
                    order_choice == 2 &&
                    analyzer->word_statistics[j].count <
                    analyzer->word_statistics[j + 1].count
                ) {
                    should_swap = true;
                }
            }

            if (should_swap) {
                temporary_word = analyzer->word_statistics[j];
                analyzer->word_statistics[j] =
                    analyzer->word_statistics[j + 1];
                analyzer->word_statistics[j + 1] = temporary_word;
            }
        }
    }

    printf("\nWord statistics sorted successfully.\n");
    display_word_statistics(analyzer);
}

// normalizing one searched word to match the stored words
bool normalize_search_word(char input[], char normalized[]) {
    char *current_character;
    int length = 0;

    current_character = input;

    while (
        *current_character != '\0' &&
        !isalpha((unsigned char)*current_character)
    ) {
        current_character++;
    }

    while (
        *current_character != '\0' &&
        (isalpha((unsigned char)*current_character) ||
         *current_character == '\'')
    ) {
        if (length < WORD_LENGTH - 1) {
            normalized[length] =
                (char)tolower((unsigned char)*current_character);
            length++;
        }
        current_character++;
    }

    normalized[length] = '\0';
    return length > 0;
}

// searching for a word and reporting its frequency
void search_word(TextAnalyzer *analyzer) {
    char input_word[100];
    char normalized_word[WORD_LENGTH];
    WordStat *current_word;
    WordStat *last_word;

    read_required_text(
        "\nEnter the word to search for: ",
        input_word,
        sizeof(input_word)
    );

    if (!normalize_search_word(input_word, normalized_word)) {
        printf("\nError! Enter a word containing letters.\n");
        return;
    }

    current_word = analyzer->word_statistics;
    last_word = analyzer->word_statistics + analyzer->distinct_words;

    while (current_word < last_word) {
        if (strcmp(current_word->name, normalized_word) == 0) {
            printf("\n\t===== WORD FOUND =====\n");
            printf("\tWord: %s\n", current_word->name);
            printf("\tFrequency: %d\n", current_word->count);
            return;
        }
        current_word++;
    }

    printf("\nThe word \"%s\" was not found.\n", normalized_word);
}

// displaying the required addresses and explaining each one
void inspect_memory(TextAnalyzer *analyzer) {
    int selected_entry;

    printf("\n\t===== MEMORY INSPECTION =====\n");
    printf(
        "\tText buffer address: %p\n",
        (void *)analyzer->text_data.text
    );
    printf("\tThis is where the dynamically allocated paragraph begins.\n\n");

    printf(
        "\tWord-statistics array address: %p\n",
        (void *)analyzer->word_statistics
    );
    printf("\tThis is where the dynamic frequency array begins.\n\n");

    while (true) {
        if (
            !read_integer(
                "\tSelect a word entry number to inspect: ",
                &selected_entry
            )
        ) {
            continue;
        }

        if (
            selected_entry >= 1 &&
            selected_entry <= analyzer->distinct_words
        ) {
            break;
        }

        printf(
            "\nError! Select a number from 1 to %d.\n",
            analyzer->distinct_words
        );
    }

    printf(
        "\n\tSelected entry address: %p\n",
        (void *)&analyzer->word_statistics[selected_entry - 1]
    );
    printf(
        "\tThis address stores the word \"%s\" and its frequency.\n",
        analyzer->word_statistics[selected_entry - 1].name
    );
}

// calculating the ratio of distinct words to total words
float get_lexical_diversity(TextAnalyzer *analyzer) {
    if (analyzer->total_words == 0) {
        return 0;
    }

    return (
        (float)analyzer->distinct_words /
        analyzer->total_words
    ) * 100;
}

// running the original lexical diversity analysis
void custom_analysis(TextAnalyzer *analyzer) {
    float lexical_diversity;

    lexical_diversity = get_lexical_diversity(analyzer);

    printf("\n\t===== LEXICAL DIVERSITY ANALYSIS =====\n");
    printf("\tDistinct Words: %d\n", analyzer->distinct_words);
    printf("\tTotal Words: %d\n", analyzer->total_words);
    printf("\tLexical Diversity Score: %.2f%%\n", lexical_diversity);
    printf(
        "\tFormula: Distinct Words / Total Words x 100\n"
    );

    if (lexical_diversity >= 70) {
        printf("\tInterpretation: The text has high vocabulary variety.\n");
    }
    else if (lexical_diversity >= 50) {
        printf("\tInterpretation: The text has moderate vocabulary variety.\n");
    }
    else {
        printf("\tInterpretation: The text contains frequent repetition.\n");
    }
}

// generating all required summary statistics
void generate_text_summary(TextAnalyzer *analyzer) {
    WordStat *frequent_word;
    float average_word_length;

    frequent_word = get_most_frequent(analyzer);
    average_word_length =
        (float)analyzer->total_letters /
        analyzer->total_words;

    printf("\n\t===== TEXT ANALYSIS SUMMARY =====\n");
    printf("\tSource: %s\n", analyzer->text_data.source_title);
    printf("\tDetails: %s\n", analyzer->text_data.source_details);
    printf("\tTotal Words: %d\n", analyzer->total_words);
    printf("\tDistinct Words: %d\n", analyzer->distinct_words);
    printf("\tAverage Word Length: %.2f letters\n", average_word_length);
    printf(
        "\tLongest Word: %s (%d letters)\n",
        analyzer->longest_word,
        analyzer->longest_word_length
    );
    printf(
        "\tMost Frequent Word: %s (%d occurrences)\n",
        frequent_word->name,
        frequent_word->count
    );
    printf(
        "\tLexical Diversity Score: %.2f%%\n",
        get_lexical_diversity(analyzer)
    );
}

// ending the main menu loop
void exit_program(TextAnalyzer *analyzer) {
    analyzer->program_running = false;
    printf("\nExiting the Dynamic Text Mining Analyzer...\n");
}

// printing the required project identification
void print_project_header(TextAnalyzer *analyzer) {
    printf("\n");
    printf("\tStudent ID: %s\n", MY_STUDENT_ID);
    printf("\tProject: Dynamic Text Mining and Linguistic Pattern Analyzer\n");
    printf("\tSource: %s\n", analyzer->text_data.source_title);
    printf("\tDetails: %s\n", analyzer->text_data.source_details);
    printf("\tCustom Analysis: Lexical Diversity Score\n");
}

// printing the menu for all analysis operations
void print_menu(void) {
    printf("\n\t========== TEXT ANALYSIS MENU ==========\n");
    printf("\t1.  Count Total Words\n");
    printf("\t2.  Find the Longest Word\n");
    printf("\t3.  Find the Most Frequent Word\n");
    printf("\t4.  Display Word Statistics\n");
    printf("\t5.  Sort Word Statistics\n");
    printf("\t6.  Search for a Word\n");
    printf("\t7.  Inspect Memory Addresses\n");
    printf("\t8.  Run Lexical Diversity Analysis\n");
    printf("\t9.  Generate Text Summary\n");
    printf("\t10. Exit the Program\n");
}

int main(void) {
    TextAnalyzer analyzer = {0};
    int choice;

    AnalysisOperation menu_operations[MENU_OPTIONS] = {
        count_words,
        longest_word,
        most_frequent,
        display_word_statistics,
        sort_word_statistics,
        search_word,
        inspect_memory,
        custom_analysis,
        generate_text_summary,
        exit_program
    };

    analyzer.program_running = true;

    clear_screen();

    if (!select_text_dataset(&analyzer.text_data)) {
        return 1;
    }

    if (!process_text(&analyzer)) {
        free(analyzer.text_data.text);
        free(analyzer.word_statistics);
        return 1;
    }

    while (analyzer.program_running) {
        clear_screen();
        print_project_header(&analyzer);
        print_menu();

        if (!read_integer("\n\tSelect an option from 1 to 10: ", &choice)) {
            wait_for_menu();
            continue;
        }

        if (choice < 1 || choice > MENU_OPTIONS) {
            printf("\nError! Select an option from 1 to 10.\n");
            wait_for_menu();
            continue;
        }

        clear_screen();
        menu_operations[choice - 1](&analyzer);

        if (analyzer.program_running) {
            wait_for_menu();
        }
    }

    // releasing every dynamically allocated section before exiting
    free(analyzer.text_data.text);
    analyzer.text_data.text = NULL;

    free(analyzer.word_statistics);
    analyzer.word_statistics = NULL;

    return 0;
}

