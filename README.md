# Formative 2 — C Programming

This repository contains three C programming projects demonstrating structures, unions, pointers, dynamic memory allocation, string processing, function pointers, and callback functions.

## Projects

### Project 1: Dynamic Academic Performance and Risk Analysis

Manages student academic records and supports:

- Adding and removing student records
- Sorting by grade, name, or student ID
- Searching by ID or partial name
- Calculating average grades
- Identifying the top-performing student
- Course Performance Comparison
- Generating an academic summary report

### Project 2: Dynamic Text Mining and Linguistic Pattern Analyzer

Processes a published or user-provided paragraph and supports:

- Dynamic text input
- Word counting and frequency analysis
- Longest and most frequent word identification
- Alphabetical and frequency-based sorting
- Word searching
- Memory-address inspection
- Lexical Diversity Analysis
- Text summary generation

Words are converted to lowercase. Surrounding punctuation is ignored, internal apostrophes are retained, and hyphens separate words. Words longer than 39 characters are safely truncated to their first 39 characters.

### Project 3: Dynamic IoT Event Monitoring and Callback Engine

Simulates and monitors IoT devices using callback functions. It supports:

- Temperature, pressure, and battery-voltage devices
- Dynamic device registration and removal
- Simulated sensor readings
- Device-specific callback dispatch
- Normal and abnormal reading detection
- Detailed alert storage
- Sensor Drift Detection
- Monitoring summary generation

## Requirements

Install one of the following:

- GCC
- Clang
- CMake with a C compiler

The programs use the C11 standard.

## Clone the Repository

```bash
git clone https://github.com/helen751/Formative2-C_Programming.git
cd Formative2-C_Programming
```

## Compile and Run from the Terminal

### Project 1

```bash
gcc -std=c11 -Wall -Wextra -pedantic Project1/main.c -o project1
./project1
```

### Project 2

```bash
gcc -std=c11 -Wall -Wextra -pedantic Project2/main.c -o project2
./project2
```

### Project 3

```bash
gcc -std=c11 -Wall -Wextra -pedantic Project3/main.c -o project3
./project3
```

If `gcc` is unavailable but Clang is installed, replace `gcc` with `clang`.

## Compile and Run with CMake

From the repository’s root directory, run:

```bash
cmake -S . -B build
cmake --build build
```

Run the compiled programs:

```bash
./build/Project1_academic_analysis
./build/Project2_text_analyzer
./build/Project3_iot_monitoring
```

## Run with Visual Studio Code

1. Open Visual Studio Code.
2. Select **File → Open Folder**.
3. Open the cloned `Formative2-C_Programming` folder.
4. Open **Terminal → New Terminal**.
5. Run the compile and execution commands for the required project from the sections above.

## Run with CLion

1. Open CLion.
2. Select **Open** and choose the `Formative2-C_Programming` folder.
3. Allow CLion to load the `CMakeLists.txt` file.
4. Select one of these targets:

    - `Project1_academic_analysis`
    - `Project2_text_analyzer`
    - `Project3_iot_monitoring`

5. Click **Run**.