#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NAME_LENGTH 100
#define TEXT_LENGTH 180

const char student_id[] = "1234376";

const float minimum_temperature = 10.0f;
const float maximum_temperature = 45.0f;
const float normal_temperature_minimum = 18.0f;
const float normal_temperature_maximum = 30.0f;
const float temperature_drift_limit = 5.0f;

const int minimum_pressure = 80;
const int maximum_pressure = 130;
const int normal_pressure_minimum = 95;
const int normal_pressure_maximum = 110;
const float pressure_drift_limit = 10.0f;

const float minimum_voltage = 2.8f;
const float maximum_voltage = 4.5f;
const float normal_voltage_minimum = 3.4f;
const float normal_voltage_maximum = 4.2f;
const float voltage_drift_limit = 0.5f;

typedef enum {
    TEMPERATURE_SENSOR = 1,
    PRESSURE_SENSOR,
    BATTERY_MONITOR
} DeviceType;

typedef enum {
    STATUS_NORMAL,
    STATUS_ALERT
} DeviceStatus;

typedef union {
    float temperature;
    int pressure;
    float voltage;
} DeviceReading;

typedef struct {
    int id;
    char name[NAME_LENGTH];
    DeviceType type;
    DeviceReading reading;
    DeviceReading previous_reading;
    bool has_previous_reading;
    DeviceStatus status;
    bool drift_detected;
    int readings_processed;
    int normal_readings;
    int abnormal_readings;
    int drift_alerts;
} Device;

typedef struct {
    int device_id;
    char device_name[NAME_LENGTH];
    DeviceType device_type;
    float reading_value;
    char unit[20];
    char expected_condition[TEXT_LENGTH];
    char reason[TEXT_LENGTH];
    char callback_name[NAME_LENGTH];
} Alert;

typedef struct IoTSystem IoTSystem;

typedef void (*DeviceCallback)(Device *device, IoTSystem *system);
typedef void (*MenuOperation)(IoTSystem *system);

struct IoTSystem {
    Device *devices;
    int device_count;
    int device_capacity;
    Alert *alerts;
    int alert_count;
    int alert_capacity;
    int total_readings;
    int normal_readings;
    int abnormal_readings;
    int alerts_per_type[3];
    int custom_callback_alerts;
    bool program_running;
};

// clearing the remaining input after an invalid entry
void clear_input_buffer(void) {
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

// clearing the terminal before displaying a new screen
void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
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

// reading required text without allowing an empty value
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

// returning the readable name of a device type
const char *get_device_type_name(DeviceType type) {
    if (type == TEMPERATURE_SENSOR) {
        return "Temperature Sensor";
    }

    if (type == PRESSURE_SENSOR) {
        return "Pressure Sensor";
    }

    return "Battery Voltage Monitor";
}

// returning the unit used by the selected device type
const char *get_device_unit(DeviceType type) {
    if (type == TEMPERATURE_SENSOR) {
        return "degrees C";
    }

    if (type == PRESSURE_SENSOR) {
        return "kPa";
    }

    return "V";
}

// reading the active union member as one floating-point value
float get_reading_value(Device *device) {
    if (device->type == TEMPERATURE_SENSOR) {
        return device->reading.temperature;
    }

    if (device->type == PRESSURE_SENSOR) {
        return (float)device->reading.pressure;
    }

    return device->reading.voltage;
}

// reading the correct previous union member for drift comparison
float get_previous_reading_value(Device *device) {
    if (device->type == TEMPERATURE_SENSOR) {
        return device->previous_reading.temperature;
    }

    if (device->type == PRESSURE_SENSOR) {
        return (float)device->previous_reading.pressure;
    }

    return device->previous_reading.voltage;
}

// storing the current reading in the correct previous-reading member
void save_previous_reading(Device *device) {
    if (device->type == TEMPERATURE_SENSOR) {
        device->previous_reading.temperature = device->reading.temperature;
    }
    else if (device->type == PRESSURE_SENSOR) {
        device->previous_reading.pressure = device->reading.pressure;
    }
    else {
        device->previous_reading.voltage = device->reading.voltage;
    }

    device->has_previous_reading = true;
}

// finding a device using pointer traversal
Device *find_device_by_id(IoTSystem *system, int id) {
    Device *device = system->devices;
    Device *last_device = system->devices + system->device_count;

    while (device < last_device) {
        if (device->id == id) {
            return device;
        }

        device++;
    }

    return NULL;
}

// increasing the alert array whenever it becomes full
bool expand_alert_storage(IoTSystem *system) {
    Alert *resized_alerts;
    int new_capacity;

    if (system->alert_count < system->alert_capacity) {
        return true;
    }

    new_capacity = system->alert_capacity * 2;
    resized_alerts = realloc(system->alerts, new_capacity * sizeof(Alert));

    if (resized_alerts == NULL) {
        printf("\nError! Alert memory could not be expanded.\n");
        return false;
    }

    system->alerts = resized_alerts;
    system->alert_capacity = new_capacity;
    return true;
}

// saving one detailed alert for later display
void save_alert(
    IoTSystem *system,
    Device *device,
    char expected_condition[],
    char reason[],
    char callback_name[]
) {
    Alert *alert;

    if (!expand_alert_storage(system)) {
        return;
    }

    alert = system->alerts + system->alert_count;
    alert->device_id = device->id;
    snprintf(alert->device_name, NAME_LENGTH, "%s", device->name);
    alert->device_type = device->type;
    alert->reading_value = get_reading_value(device);
    snprintf(alert->unit, sizeof(alert->unit), "%s", get_device_unit(device->type));
    snprintf(alert->expected_condition, TEXT_LENGTH, "%s", expected_condition);
    snprintf(alert->reason, TEXT_LENGTH, "%s", reason);
    snprintf(alert->callback_name, NAME_LENGTH, "%s", callback_name);
    system->alert_count++;
}

// processing temperature readings against the safe operating range
void temperature_monitor(Device *device, IoTSystem *system) {
    char expected[TEXT_LENGTH];
    char reason[TEXT_LENGTH];

    if (
        device->reading.temperature >= normal_temperature_minimum &&
        device->reading.temperature <= normal_temperature_maximum
    ) {
        printf("\tProcessing Result: Normal temperature reading\n");
        return;
    }

    device->status = STATUS_ALERT;
    snprintf(
        expected,
        TEXT_LENGTH,
        "%.1f to %.1f degrees C",
        normal_temperature_minimum,
        normal_temperature_maximum
    );
    snprintf(
        reason,
        TEXT_LENGTH,
        "Temperature is outside the configured operating range"
    );
    save_alert(system, device, expected, reason, "temperature_monitor");

    printf("\tProcessing Result: ALERT - %s\n", reason);
    printf("\tExpected Range: %s\n", expected);
}

// processing pressure readings against the safe operating range
void pressure_monitor(Device *device, IoTSystem *system) {
    char expected[TEXT_LENGTH];
    char reason[TEXT_LENGTH];

    if (
        device->reading.pressure >= normal_pressure_minimum &&
        device->reading.pressure <= normal_pressure_maximum
    ) {
        printf("\tProcessing Result: Normal pressure reading\n");
        return;
    }

    device->status = STATUS_ALERT;
    snprintf(
        expected,
        TEXT_LENGTH,
        "%d to %d kPa",
        normal_pressure_minimum,
        normal_pressure_maximum
    );
    snprintf(
        reason,
        TEXT_LENGTH,
        "Pressure is outside the configured operating range"
    );
    save_alert(system, device, expected, reason, "pressure_monitor");

    printf("\tProcessing Result: ALERT - %s\n", reason);
    printf("\tExpected Range: %s\n", expected);
}

// processing battery readings against the safe voltage range
void battery_monitor(Device *device, IoTSystem *system) {
    char expected[TEXT_LENGTH];
    char reason[TEXT_LENGTH];

    if (
        device->reading.voltage >= normal_voltage_minimum &&
        device->reading.voltage <= normal_voltage_maximum
    ) {
        printf("\tProcessing Result: Normal battery voltage\n");
        return;
    }

    device->status = STATUS_ALERT;
    snprintf(
        expected,
        TEXT_LENGTH,
        "%.1f to %.1f V",
        normal_voltage_minimum,
        normal_voltage_maximum
    );
    snprintf(
        reason,
        TEXT_LENGTH,
        "Battery voltage is outside the configured operating range"
    );
    save_alert(system, device, expected, reason, "battery_monitor");

    printf("\tProcessing Result: ALERT - %s\n", reason);
    printf("\tExpected Range: %s\n", expected);
}

// detecting significant movement from a device's previous reading
void sensor_drift_detector(Device *device, IoTSystem *system) {
    float current_value;
    float previous_value;
    float difference;
    float drift_limit;
    char expected[TEXT_LENGTH];
    char reason[TEXT_LENGTH];

    if (!device->has_previous_reading) {
        printf("\tCustom Callback: No previous reading available for drift comparison\n");
        return;
    }

    current_value = get_reading_value(device);
    previous_value = get_previous_reading_value(device);
    difference = current_value - previous_value;

    if (difference < 0.0f) {
        difference = -difference;
    }

    if (device->type == TEMPERATURE_SENSOR) {
        drift_limit = temperature_drift_limit;
    }
    else if (device->type == PRESSURE_SENSOR) {
        drift_limit = pressure_drift_limit;
    }
    else {
        drift_limit = voltage_drift_limit;
    }

    if (difference <= drift_limit) {
        printf(
            "\tCustom Callback: Stable change of %.2f %s\n",
            difference,
            get_device_unit(device->type)
        );
        return;
    }

    device->status = STATUS_ALERT;
    device->drift_detected = true;
    device->drift_alerts++;
    system->custom_callback_alerts++;

    snprintf(
        expected,
        TEXT_LENGTH,
        "Change should not exceed %.2f %s between consecutive readings",
        drift_limit,
        get_device_unit(device->type)
    );
    snprintf(
        reason,
        TEXT_LENGTH,
        "Sensor drift detected: reading changed by %.2f %s",
        difference,
        get_device_unit(device->type)
    );
    save_alert(system, device, expected, reason, "sensor_drift_detector");

    printf("\tCustom Callback: ALERT - %s\n", reason);
}

// calling the selected monitoring callback through a function pointer
void process_device(Device *device, DeviceCallback callback, IoTSystem *system) {
    callback(device, system);
}

// selecting a required callback based on the active device type
DeviceCallback select_monitor_callback(DeviceType type) {
    DeviceCallback callbacks[3] = {
        temperature_monitor,
        pressure_monitor,
        battery_monitor
    };

    return callbacks[type - 1];
}

// increasing the device array when another device is registered
bool expand_device_storage(IoTSystem *system) {
    Device *resized_devices;
    int new_capacity;

    if (system->device_count < system->device_capacity) {
        return true;
    }

    if (system->device_capacity == 0) {
        new_capacity = 1;
    }
    else {
        new_capacity = system->device_capacity * 2;
    }
    resized_devices = realloc(system->devices, new_capacity * sizeof(Device));

    if (resized_devices == NULL) {
        printf("\nError! Device memory could not be expanded.\n");
        return false;
    }

    system->devices = resized_devices;
    system->device_capacity = new_capacity;
    return true;
}

// adding one device directly to the dynamic array
bool add_device(
    IoTSystem *system,
    int id,
    char name[],
    DeviceType type
) {
    Device *device;

    if (find_device_by_id(system, id) != NULL) {
        return false;
    }

    if (!expand_device_storage(system)) {
        return false;
    }

    device = system->devices + system->device_count;
    device->id = id;
    snprintf(device->name, NAME_LENGTH, "%s", name);
    device->type = type;
    device->reading.voltage = 0.0f;
    device->previous_reading.voltage = 0.0f;
    device->has_previous_reading = false;
    device->status = STATUS_NORMAL;
    device->drift_detected = false;
    device->readings_processed = 0;
    device->normal_readings = 0;
    device->abnormal_readings = 0;
    device->drift_alerts = 0;
    system->device_count++;
    return true;
}

// registering a new device from user input
void register_device(IoTSystem *system) {
    int id;
    int type;
    char name[NAME_LENGTH];

    printf("\n\t===== REGISTER A NEW DEVICE =====\n");

    do {
        if (!read_integer("\n\tEnter the unique device ID: ", &id)) {
            continue;
        }

        if (id <= 0) {
            printf("\nError! Device ID must be greater than zero.\n");
            continue;
        }

        if (find_device_by_id(system, id) != NULL) {
            printf("\nError! A device with ID %d already exists.\n", id);
            continue;
        }

        break;
    } while (true);

    read_required_text("\tEnter the device name: ", name, NAME_LENGTH);

    do {
        printf("\n\t1. Temperature Sensor\n");
        printf("\t2. Pressure Sensor\n");
        printf("\t3. Battery Voltage Monitor\n");

        if (!read_integer("\n\tSelect the device type: ", &type)) {
            continue;
        }

        if (type < TEMPERATURE_SENSOR || type > BATTERY_MONITOR) {
            printf("\nError! Select a device type from 1 to 3.\n");
            continue;
        }

        break;
    } while (true);

    if (add_device(system, id, name, (DeviceType)type)) {
        printf("\nDevice %s was registered successfully.\n", name);
    }
    else {
        printf("\nError! The device could not be registered.\n");
    }
}

// displaying all devices using pointer traversal
void display_devices(IoTSystem *system) {
    Device *device;
    Device *last_device;

    printf("\n\t================ REGISTERED DEVICES ================\n");

    if (system->device_count == 0) {
        printf("\nNo devices are currently registered.\n");
        return;
    }

    printf("\t%-8s %-28s %-26s %-10s\n", "ID", "NAME", "TYPE", "STATUS");
    printf("\t--------------------------------------------------------------\n");

    device = system->devices;
    last_device = system->devices + system->device_count;

    while (device < last_device) {
        printf(
            "\t%-8d %-28s %-26s %-10s\n",
            device->id,
            device->name,
            get_device_type_name(device->type),
            device->status == STATUS_NORMAL ? "Normal" : "Alert"
        );
        device++;
    }
}

// removing a device and shrinking the dynamic array
void remove_device(IoTSystem *system) {
    int id;
    int index;
    int move_index;
    Device *resized_devices;

    printf("\n\t===== REMOVE A DEVICE =====\n");

    if (system->device_count == 0) {
        printf("\nNo devices are available to remove.\n");
        return;
    }

    if (!read_integer("\n\tEnter the device ID to remove: ", &id)) {
        return;
    }

    index = -1;

    for (move_index = 0; move_index < system->device_count; move_index++) {
        if ((system->devices + move_index)->id == id) {
            index = move_index;
            break;
        }
    }

    if (index == -1) {
        printf("\nError! No device was found with ID %d.\n", id);
        return;
    }

    printf("\nDevice %s was removed successfully.\n", (system->devices + index)->name);

    for (move_index = index; move_index < system->device_count - 1; move_index++) {
        *(system->devices + move_index) = *(system->devices + move_index + 1);
    }

    system->device_count--;

    if (system->device_count == 0) {
        free(system->devices);
        system->devices = NULL;
        system->device_capacity = 0;
        return;
    }

    resized_devices = realloc(system->devices, system->device_count * sizeof(Device));

    if (resized_devices != NULL) {
        system->devices = resized_devices;
        system->device_capacity = system->device_count;
    }
}

// generating a random floating-point value inside a configured range
float random_float(float minimum, float maximum) {
    float random_fraction = (float)rand() / (float)RAND_MAX;

    return minimum + random_fraction * (maximum - minimum);
}

// generating a random whole number inside a configured range
int random_integer(int minimum, int maximum) {
    return minimum + rand() % (maximum - minimum + 1);
}

// generating a reading in the correct active union member
void generate_reading(Device *device, int reading_number) {
    if (device->type == TEMPERATURE_SENSOR) {
        if (reading_number == 0) {
            device->reading.temperature = 24.0f;
        }
        else if (reading_number == 3) {
            device->reading.temperature = 40.0f;
        }
        else {
            device->reading.temperature = random_float(
                minimum_temperature,
                maximum_temperature
            );
        }
    }
    else if (device->type == PRESSURE_SENSOR) {
        if (reading_number == 1) {
            device->reading.pressure = 125;
        }
        else if (reading_number == 4) {
            device->reading.pressure = 100;
        }
        else {
            device->reading.pressure = random_integer(
                minimum_pressure,
                maximum_pressure
            );
        }
    }
    else {
        if (reading_number == 2) {
            device->reading.voltage = 3.8f;
        }
        else if (reading_number == 5) {
            device->reading.voltage = 2.9f;
        }
        else {
            device->reading.voltage = random_float(
                minimum_voltage,
                maximum_voltage
            );
        }
    }
}

// finding the first registered device of a required type
Device *find_first_device_of_type(IoTSystem *system, DeviceType type) {
    Device *device = system->devices;
    Device *last_device = system->devices + system->device_count;

    while (device < last_device) {
        if (device->type == type) {
            return device;
        }

        device++;
    }

    return NULL;
}

// generating and processing at least ten readings through callbacks
void simulate_readings(IoTSystem *system) {
    Device *device;
    Device *required_device;
    DeviceCallback callback;
    DeviceType required_type;
    int reading_number;
    int device_index;
    int number_of_readings = 10;

    printf("\n\t========== SIMULATED DEVICE READINGS ==========\n");

    if (system->device_count == 0) {
        printf("\nNo devices are registered for simulation.\n");
        return;
    }

    for (reading_number = 0; reading_number < number_of_readings; reading_number++) {
        required_type = (DeviceType)(reading_number % 3 + 1);
        required_device = find_first_device_of_type(system, required_type);

        if (required_device != NULL && reading_number < 6) {
            device = required_device;
        }
        else {
            device_index = rand() % system->device_count;
            device = system->devices + device_index;
        }

        generate_reading(device, reading_number);
        device->status = STATUS_NORMAL;
        device->drift_detected = false;

        printf("\n\tReading %d of %d\n", reading_number + 1, number_of_readings);
        printf("\tDevice ID: %d\n", device->id);
        printf("\tDevice Name: %s\n", device->name);
        printf("\tDevice Type: %s\n", get_device_type_name(device->type));
        printf(
            "\tGenerated Value: %.2f %s\n",
            get_reading_value(device),
            get_device_unit(device->type)
        );

        callback = select_monitor_callback(device->type);
        process_device(device, callback, system);
        process_device(device, sensor_drift_detector, system);

        system->total_readings++;
        device->readings_processed++;

        if (device->status == STATUS_ALERT) {
            system->abnormal_readings++;
            system->alerts_per_type[device->type - 1]++;
            device->abnormal_readings++;
            printf("\tFinal Status: ALERT\n");
        }
        else {
            system->normal_readings++;
            device->normal_readings++;
            printf("\tFinal Status: NORMAL\n");
        }

        save_previous_reading(device);
    }
}

// displaying every alert stored during callback processing
void display_alerts(IoTSystem *system) {
    Alert *alert;
    Alert *last_alert;
    int number = 1;

    printf("\n\t================ DETECTED ALERTS ================\n");

    if (system->alert_count == 0) {
        printf("\nNo alerts have been detected.\n");
        return;
    }

    alert = system->alerts;
    last_alert = system->alerts + system->alert_count;

    while (alert < last_alert) {
        printf("\n\tAlert %d\n", number);
        printf("\tDevice: %d - %s\n", alert->device_id, alert->device_name);
        printf("\tType: %s\n", get_device_type_name(alert->device_type));
        printf("\tReading: %.2f %s\n", alert->reading_value, alert->unit);
        printf("\tExpected: %s\n", alert->expected_condition);
        printf("\tReason: %s\n", alert->reason);
        printf("\tCallback: %s\n", alert->callback_name);
        alert++;
        number++;
    }
}

// displaying the complete required monitoring summary
void generate_summary(IoTSystem *system) {
    Device *device;
    Device *last_device;

    printf("\n\t================ MONITORING SUMMARY ================\n");
    printf("\n\tStudent ID: %s\n", student_id);
    printf("\tTotal Registered Devices: %d\n", system->device_count);
    printf("\tTotal Readings Processed: %d\n", system->total_readings);
    printf("\tNormal Readings: %d\n", system->normal_readings);
    printf("\tAbnormal Readings: %d\n", system->abnormal_readings);
    printf("\tTemperature Alerts: %d\n", system->alerts_per_type[0]);
    printf("\tPressure Alerts: %d\n", system->alerts_per_type[1]);
    printf("\tBattery Voltage Alerts: %d\n", system->alerts_per_type[2]);
    printf("\tSensor Drift Alerts: %d\n", system->custom_callback_alerts);

    printf("\n\t===== CUSTOM CALLBACK RESULTS PER DEVICE =====\n");

    if (system->device_count == 0) {
        printf("\nNo registered devices are available.\n");
        return;
    }

    device = system->devices;
    last_device = system->devices + system->device_count;

    while (device < last_device) {
        printf(
            "\t%d - %-25s Drift Alerts: %d\n",
            device->id,
            device->name,
            device->drift_alerts
        );
        device++;
    }
}

// ending the menu loop
void exit_program(IoTSystem *system) {
    system->program_running = false;
    printf("\nExiting the Dynamic IoT Event Monitoring and Callback Engine...\n");
}

// asking whether to return to the menu or exit
void continue_or_exit(IoTSystem *system) {
    int choice;

    do {
        printf("\n\t1. Back to Main Menu\n");
        printf("\t2. Exit Program\n");

        if (!read_integer("\n\tSelect an option: ", &choice)) {
            continue;
        }

        if (choice == 1) {
            return;
        }

        if (choice == 2) {
            exit_program(system);
            return;
        }

        printf("\nError! Select either 1 or 2.\n");
    } while (true);
}

// printing the project identification
void print_header(void) {
    printf("\n\tStudent ID: %s\n", student_id);
    printf("\tProject: Dynamic IoT Event Monitoring and Callback Engine\n");
    printf("\tCustom Callback: Sensor Drift Detector\n");
}

// printing the complete function-pointer menu
void print_menu(void) {
    printf("\n\t================ MAIN MENU ================\n");
    printf("\t1. Register a New Device\n");
    printf("\t2. Display All Registered Devices\n");
    printf("\t3. Remove a Device\n");
    printf("\t4. Generate and Process 10 Readings\n");
    printf("\t5. Display Detected Alerts\n");
    printf("\t6. Generate Monitoring Summary\n");
    printf("\t7. Display Operating Limits\n");
    printf("\t8. Exit the Program\n");
}

// displaying the documented simulation and monitoring ranges
void display_operating_limits(IoTSystem *system) {
    (void)system;

    printf("\n\t================ OPERATING LIMITS ================\n");
    printf(
        "\n\tTemperature Generation Range: %.1f to %.1f degrees C\n",
        minimum_temperature,
        maximum_temperature
    );
    printf(
        "\tTemperature Normal Range: %.1f to %.1f degrees C\n",
        normal_temperature_minimum,
        normal_temperature_maximum
    );
    printf("\tTemperature Drift Limit: %.1f degrees C\n", temperature_drift_limit);

    printf(
        "\n\tPressure Generation Range: %d to %d kPa\n",
        minimum_pressure,
        maximum_pressure
    );
    printf(
        "\tPressure Normal Range: %d to %d kPa\n",
        normal_pressure_minimum,
        normal_pressure_maximum
    );
    printf("\tPressure Drift Limit: %.1f kPa\n", pressure_drift_limit);

    printf(
        "\n\tVoltage Generation Range: %.1f to %.1f V\n",
        minimum_voltage,
        maximum_voltage
    );
    printf(
        "\tVoltage Normal Range: %.1f to %.1f V\n",
        normal_voltage_minimum,
        normal_voltage_maximum
    );
    printf("\tVoltage Drift Limit: %.1f V\n", voltage_drift_limit);
}

// allocating the initial dynamic arrays and sample devices
bool initialize_system(IoTSystem *system) {
    int initial_device_capacity = 6;
    int initial_alert_capacity = 10;

    system->devices = malloc(initial_device_capacity * sizeof(Device));

    if (system->devices == NULL) {
        printf("\nError! Initial device memory allocation failed.\n");
        return false;
    }

    system->alerts = calloc(initial_alert_capacity, sizeof(Alert));

    if (system->alerts == NULL) {
        printf("\nError! Initial alert memory allocation failed.\n");
        free(system->devices);
        system->devices = NULL;
        return false;
    }

    system->device_count = 0;
    system->device_capacity = initial_device_capacity;
    system->alert_count = 0;
    system->alert_capacity = initial_alert_capacity;
    system->total_readings = 0;
    system->normal_readings = 0;
    system->abnormal_readings = 0;
    system->alerts_per_type[0] = 0;
    system->alerts_per_type[1] = 0;
    system->alerts_per_type[2] = 0;
    system->custom_callback_alerts = 0;
    system->program_running = true;

    add_device(system, 1001, "Server Room Temperature", TEMPERATURE_SENSOR);
    add_device(system, 1002, "Pipeline Pressure", PRESSURE_SENSOR);
    add_device(system, 1003, "Backup Battery", BATTERY_MONITOR);
    add_device(system, 1004, "Greenhouse Temperature", TEMPERATURE_SENSOR);
    add_device(system, 1005, "Water Tank Pressure", PRESSURE_SENSOR);
    add_device(system, 1006, "Solar Controller Battery", BATTERY_MONITOR);

    return true;
}

int main(void) {
    IoTSystem system = {0};
    int choice;

    MenuOperation menu_operations[8] = {
        register_device,
        display_devices,
        remove_device,
        simulate_readings,
        display_alerts,
        generate_summary,
        display_operating_limits,
        exit_program
    };

    srand((unsigned int)time(NULL));

    if (!initialize_system(&system)) {
        return 1;
    }

    while (system.program_running) {
        clear_screen();
        print_header();
        print_menu();

        if (!read_integer("\n\tSelect an option from 1 to 8: ", &choice)) {
            continue_or_exit(&system);
            continue;
        }

        if (choice < 1 || choice > 8) {
            printf("\nError! Select an option from 1 to 8.\n");
            continue_or_exit(&system);
            continue;
        }

        clear_screen();
        menu_operations[choice - 1](&system);

        if (system.program_running) {
            continue_or_exit(&system);
        }
    }

    // releasing every dynamically allocated section before exiting
    free(system.devices);
    system.devices = NULL;

    free(system.alerts);
    system.alerts = NULL;

    return 0;
}
