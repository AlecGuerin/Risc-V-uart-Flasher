#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Serial_port.h"

int counter = 0;

char string[20] = {0};
char program_string[1024] = {0};
char result_string[32] = {0};
char port_name[20] = {0};
char program_file_name[20] = {0};
char results_file_name[20] = {0};
char int_string[10] = {0};
int programs_number = 0;
int starting_program_number = 0;
int final_program_index = 0;

int Send_expected_result(char result[4]);
int Send_program(char *program_string);

FILE *settings_file;
FILE *programs_file;
FILE *results_file;

void main(void)
{
    int index=0;
    // Read init settings - Start //
    settings_file = fopen("settings.in", "r");
    if (settings_file == 0)
    {
        printf("Settings file not found\n\r");
        return;
    }
    fscanf(settings_file, "Port: %s\n", port_name);
    fscanf(settings_file, "Programs number: %s\n", int_string);
    programs_number = atoi(int_string);
    fscanf(settings_file, "Starting from: %s\n", int_string);
    fclose(settings_file);

    starting_program_number = atoi(int_string);
    // Read init settings - End //

    // Open serial port - Start //
    serial_port_init(port_name, 9600, 0);
    serial_port_write("test\n\r");
    // Open serial port - End //

    while (index <= programs_number)
    {

        memset(program_file_name, 0, 20);
        sprintf(program_file_name, "./Tests/Test%u", index);
        memset(results_file_name, 0, 20);
        sprintf(results_file_name, "./Results/Result%u", index);
        programs_file = fopen(program_file_name, "r");
        results_file = fopen(results_file_name, "r");

        if (programs_file == 0)
        {
            printf("Program file %u not found!\n\r", index);
            printf("Aborted. Goodbye!\n\r");
            return;
        }
        else if (results_file == 0)
        {
            printf("Results file %u not found!\n\r", index);
            printf("Aborted. Goodbye!\n\r");
            return;
        }
        else
        {
            printf("Transmit program no %u ?  Y/N: ", index);
            memset(string, 0, 20);
            scanf("%s", string);
            if (string[0] == 'Y')
            {
                // send the program
                fscanf(programs_file, "%s", program_string);
                fscanf(results_file, "%s", program_string);

                if (Send_expected_result(result_string) == 1 & Send_program(program_string) == 1)
                {
                    printf("Success\n\r");
                    index++;
                }
                else
                {
                    printf("Failure\n\r");
                }
            }
            else if (string[0] == 'N')
            {
                printf("Aborted. Goodbye!\n\r");
                return;
            }
        }
    }
    fclose(programs_file);
    fclose(results_file);
    serial_port_close();
}

typedef enum
{
    idle,
    init,
    transmit,
    confirm,
    end
} comms_state_t;

int Send_program(char *program_string)
{
    comms_state_t comms_state = idle;
    char string[20] = {0};
    char input[5] = {0};
    unsigned char comms_success = 0;
    unsigned int program_size = strlen(program_string);
    unsigned long timestamp = 0;

    while (comms_state != end)
    {
        switch (comms_state)
        {
        case idle:
            timestamp = time(NULL);
            comms_state = init;
        case init:
            /*Send the number of bytes of the program in 4 bytes */
            serial_port_write("$"); // start the process
            sprintf(string, "%c%c%c%c", 0xFF & (program_size >> 3), 0xFF & (program_size >> 2), 0xFF & (program_size >> 1), 0xFF & (program_size));
            serial_port_write(string);
            comms_state = transmit;
            break;
        case transmit:
            serial_port_write(program_string); // transmit the program
            comms_state = confirm;
            break;
        case confirm:
            serial_port_read(input, sizeof(input)); // wait for Y'A'
            if (input[0] == 'A')
            {
                comms_success = 1;
                comms_state = end;
                break;
            }
            if (time(NULL) - timestamp > 2)
            {
                comms_success = 0;
                comms_state = end;
                break;
            }
            comms_state = init;
            break;
        case end:
            break;
        default:
            break;
        }
    }
    return comms_success;
}

int Send_expected_result(char result[4])
{
    comms_state_t comms_state = idle;
    char string[20] = {0};
    char input[5] = {0};
    unsigned char comms_success = 0;
    unsigned long timestamp = 0;

    while (comms_state != end)
    {
        switch (comms_state)
        {
        case idle:
            timestamp = time(NULL);
            comms_state = transmit;
            break;
        case transmit:
            serial_port_write("="); // start the process
            /*Send the number of bytes of the program in 4 bytes */
            sprintf(string, "%c%c%c%c", result[3], result[2], result[1], result[0]);
            serial_port_write(string);
            comms_state = confirm;
            break;
        case confirm:
            serial_port_read(input, sizeof(input)); // wait for Y'A'
            if (input[0] == 'A')
            {
                comms_success = 1;
                comms_state = end;
                break;
            }
            if (time(NULL) - timestamp > 2)
            {
                comms_success = 0;
                comms_state = end;
                break;
            }
            comms_state = transmit;
            break;
        case end:
            break;
        default:
            break;
        }
    }
    return comms_success;
}