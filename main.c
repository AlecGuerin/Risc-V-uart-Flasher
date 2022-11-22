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
int Send_program(char *program_string, const unsigned int program_size);
int File_to_bytes(char* file_name, unsigned char* result, const int size);
int Get_data_size(char* file_name);
int String_to_byte(char* str, unsigned char* res, const int index);

FILE *settings_file;
FILE *programs_file;
FILE *results_file;

int main(void)
{
    int index=0;


    int file_size = 0;
    unsigned char* data;

    // Read init settings - Start //
    settings_file = fopen("settings.in", "r");
    if (settings_file == 0)
    {
        printf("Settings file not found\n\r");
        return 0;
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
    //serial_port_write("test\n\r");
    // Open serial port - End //

    while (index <= programs_number)
    {

        memset(program_file_name, 0, 20);
        sprintf(program_file_name, "./Tests/Test%u", index);
        memset(results_file_name, 0, 20);
        sprintf(results_file_name, "./Results/Result%u", index);
        //programs_file = fopen(program_file_name, "r");
        //results_file = fopen(results_file_name, "r");

        /*if (programs_file == 0)
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
        else*/
        if(1)
        {

            printf("Transmit program no %u ?  Y/N: ", index);
            memset(string, 0, 20);
            scanf("%s", string);
            if (string[0] == 'Y' || string[0] == 'y')
            {
                // send the program
                //fscanf(programs_file, "%s", program_string);
                //fscanf(results_file, "%s", program_string);

                file_size = Get_data_size( results_file_name);
                data = (unsigned char *) malloc(file_size);
                File_to_bytes(results_file_name, data, file_size);
                
                if (Send_expected_result(data) != 1)
                {
                    printf("Fail to send the expected result.\n\r");
                    free(data);
                    return 0;
                }

                free(data);
                file_size = Get_data_size( program_file_name);
                data = (unsigned char *) malloc(file_size);
                File_to_bytes(program_file_name, data, file_size);


                 if (Send_program(data, file_size) == 1)
                {
                    printf("Success\n\r");
                    index++;
                }
                else
                {
                    printf("Failure\n\r");
                }
               
                 free(data);
                
            }
            else if (string[0] == 'N')
            {
                printf("Aborted. Goodbye!\n\r");
                return 0;
            }
            else{
                printf("Command not recognized. Goodbye!\n\r");
                return 0;
            }
        }
    }
    fclose(programs_file);
    fclose(results_file);
    serial_port_close();
    return 1;
}

typedef enum
{
    idle,
    init,
    transmit,
    confirm,
    end
} comms_state_t;

int Send_program(char *program_string, const unsigned int program_size)
{
    comms_state_t comms_state = idle;
    char string[20] = {0};
    char input[5] = {0};
    unsigned char comms_success = 0;
    //unsigned int program_size = strlen(program_string);
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
        
        //    serial_port_write_data(program_string, program_size);
            for (int i = 0; i < program_size; i ++){
                serial_port_write_byte(program_string[i]); // transmit the program
            }
            //serial_port_write(program_string); // transmit the program
            comms_state = confirm;
            // Skip the checking
            //comms_success = 1;
            //comms_state = end;
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
            //serial_port_write(string);

            //serial_port_write_data(string, 4);

            for(int i = 0; i < 4 ; i ++){
                char tmp = result[i];
                serial_port_write_byte(tmp);
                //serial_port_write(tmp);
            }
            comms_state = confirm;

            // Skip the checking
            //comms_state = end; 
            //comms_success = 1;
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

int Get_data_size(char* file_name){
    FILE *results_file;
    const int maxSize = 2048;
    char str[maxSize];

    int tmp = 0;

    programs_file = fopen(file_name, "r");
    if (NULL == programs_file) {
        printf("file can't be opened \n");
        return 0;
    }


    while (fgets(str, maxSize, programs_file) != NULL) {
        printf("%s", str);
        tmp++;
    }

    fclose(programs_file);

    return tmp;

}

int String_to_byte(char* str, unsigned char* res, const int index){

    res[index] = 0x00;
    char tmp = 0x00;
    
    if (strlen(str) <= 8 ){
        printf("Unexpected string length for converting binary string to byte. \n");
        return 0;
    }

    for( int i =0; i < 8; i++){

        // Set '0'
        if(str[i] == 0x30){
            res[index] = res[index] << 1 & 0xFE;
        }
        // Set '1'
        else if(str[i] == 0x31){
            res[index] = res[index] << 1 | 0x01;
        }
        // Not happppppy :(
        else{
            printf("Bit %i in value %s isn't binary.\n", i, str );
            return 0;
        }
    }

    return 1;
}

// Return the byte number
int File_to_bytes(char* file_name, unsigned char* result, const int size){
    
    const int maxSize = 2048;
    char str[maxSize];

    programs_file = fopen(file_name, "r");
    if (NULL == programs_file) {
        printf("file can't be opened \n");
        return 0;
    }

    for (int i = 0; i < size; i++){
        if(fgets(str, maxSize, programs_file) != NULL){
            String_to_byte(str, result, i);
        }
    }
    

    fclose(programs_file);

    return 1;
}
