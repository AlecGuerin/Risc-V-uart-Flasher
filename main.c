#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Serial_port.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


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
int File_to_bytes(char *file_name, unsigned char *result, const int size);
int Get_data_size(char *file_name);
int String_to_byte(char *str, unsigned char *res, const int index);

FILE *settings_file;
FILE *programs_file;
FILE *results_file;

/*
  ________    ___           _______       ________      ________       _______           ________      ________      ________       _________        ________      ___  ___      ________      ________       ________      _______
|\   __  \  |\  \         |\  ___ \     |\   __  \    |\   ____\     |\  ___ \         |\   ___ \    |\   __  \    |\   ___  \    |\___   ___\     |\   ____\    |\  \|\  \    |\   __  \    |\   ___  \    |\   ____\    |\  ___ \
\ \  \|\  \ \ \  \        \ \   __/|    \ \  \|\  \   \ \  \___|_    \ \   __/|        \ \  \_|\ \   \ \  \|\  \   \ \  \\ \  \   \|___ \  \_|     \ \  \___|    \ \  \\\  \   \ \  \|\  \   \ \  \\ \  \   \ \  \___|    \ \   __/|
 \ \   ____\ \ \  \        \ \  \_|/__   \ \   __  \   \ \_____  \    \ \  \_|/__       \ \  \ \\ \   \ \  \\\  \   \ \  \\ \  \       \ \  \       \ \  \        \ \   __  \   \ \   __  \   \ \  \\ \  \   \ \  \  ___   \ \  \_|/__
  \ \  \___|  \ \  \____    \ \  \_|\ \   \ \  \ \  \   \|____|\  \    \ \  \_|\ \       \ \  \_\\ \   \ \  \\\  \   \ \  \\ \  \       \ \  \       \ \  \____    \ \  \ \  \   \ \  \ \  \   \ \  \\ \  \   \ \  \|\  \   \ \  \_|\ \
   \ \__\      \ \_______\   \ \_______\   \ \__\ \__\    ____\_\  \    \ \_______\       \ \_______\   \ \_______\   \ \__\\ \__\       \ \__\       \ \_______\   \ \__\ \__\   \ \__\ \__\   \ \__\\ \__\   \ \_______\   \ \_______\
    \|__|       \|_______|    \|_______|    \|__|\|__|   |\_________\    \|_______|        \|_______|    \|_______|    \|__| \|__|        \|__|        \|_______|    \|__|\|__|    \|__|\|__|    \|__| \|__|    \|_______|    \|_______|
                                                         \|_________|


 _________    ___  ___      _______           ________      ________      ________      _______               ___       __       ___      _________    ___  ___      ________      ___  ___      _________
|\___   ___\ |\  \|\  \    |\  ___ \         |\   ____\    |\   __  \    |\   ___ \    |\  ___ \             |\  \     |\  \    |\  \    |\___   ___\ |\  \|\  \    |\   __  \    |\  \|\  \    |\___   ___\
\|___ \  \_| \ \  \\\  \   \ \   __/|        \ \  \___|    \ \  \|\  \   \ \  \_|\ \   \ \   __/|            \ \  \    \ \  \   \ \  \   \|___ \  \_| \ \  \\\  \   \ \  \|\  \   \ \  \\\  \   \|___ \  \_|
     \ \  \   \ \   __  \   \ \  \_|/__       \ \  \        \ \  \\\  \   \ \  \ \\ \   \ \  \_|/__           \ \  \  __\ \  \   \ \  \       \ \  \   \ \   __  \   \ \  \\\  \   \ \  \\\  \       \ \  \
      \ \  \   \ \  \ \  \   \ \  \_|\ \       \ \  \____    \ \  \\\  \   \ \  \_\\ \   \ \  \_|\ \           \ \  \|\__\_\  \   \ \  \       \ \  \   \ \  \ \  \   \ \  \\\  \   \ \  \\\  \       \ \  \
       \ \__\   \ \__\ \__\   \ \_______\       \ \_______\   \ \y\   \ \_______\   \ \_______\           \ \____________\   \ \__\       \ \__\   \ \__\ \__\   \ \_______\   \ \_______\       \ \__\
        \|__|    \|__|\|__|    \|_______|        \|_______|    \|_______|    \|_______|    \|_______|            \|____________|    \|__|        \|__|    \|__|\|__|    \|_______|    \|_______|        \|__|



 ________      ________       ___  __        ___      ________       ________          ________  ___      ________      ________       _________    ___        ___        ___
|\   __  \    |\   ____\     |\  \|\  \     |\  \    |\   ___  \    |\   ____\        |\  _____\|\  \    |\   __  \    |\   ____\     |\___   ___\ |\  \      |\  \      |\  \
\ \  \|\  \   \ \  \___|_    \ \  \/  /|_   \ \  \   \ \  \\ \  \   \ \  \___|        \ \  \__/ \ \  \   \ \  \|\  \   \ \  \___|_    \|___ \  \_| \ \  \     \ \  \     \ \  \
 \ \   __  \   \ \_____  \    \ \   ___  \   \ \  \   \ \  \\ \  \   \ \  \  ___       \ \   __\ \ \  \   \ \   _  _\   \ \_____  \        \ \  \   \ \  \     \ \  \     \ \  \
  \ \  \ \  \   \|____|\  \    \ \  \\ \  \   \ \  \   \ \  \\ \  \   \ \  \|\  \       \ \  \_|  \ \  \   \ \  \\  \|   \|____|\  \        \ \  \   \ \__\     \ \__\     \ \__\
   \ \__\ \__\    ____\_\  \    \ \__\\ \__\   \ \__\   \ \__\\ \__\   \ \_______\       \ \__\    \ \__\   \ \__\\ _\     ____\_\  \        \ \__\   \|__|      \|__|      \|__|
    \|__|\|__|   |\_________\    \|__| \|__|    \|__|    \|__| \|__|    \|_______|        \|__|     \|__|    \|__|\|__|   |\_________\        \|__|       ___        ___        ___
                 \|_________|                                                                                             \|_________|                   |\__\      |\__\      |\__\
                                                                                                                                                         \|__|      \|__|      \|__|
 */

int main(void)
{
    int index = 0;

    int file_size = 0;
    unsigned char *data;

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
    index = starting_program_number;
    // Read init settings - End //

    // Open serial port - Start //
    serial_port_init(port_name, 9600, 0);
    // Open serial port - End //

    while (index <= programs_number)
    {

        printf("Transmit program no %u ?  Y/N/R: ", index);
        memset(string, 0, 20);
        scanf("%s", string);
        if ((string[0] == 'R' || string[0] == 'r') && index != 0){
            index --;
            string[0] = 'y';

        }
        if (string[0] == 'Y' || string[0] == 'y')
        {
            //Set the file names and directories
            memset(program_file_name, 0, 20);
            memset(results_file_name, 0, 20);
            sprintf(program_file_name, "./Tests/Test%u", index);
            sprintf(results_file_name, "./Results/Result%u", index);


        /**********************************************************Send result --- start**********************************************************/
            file_size = Get_data_size(results_file_name);
            if (file_size == -1)
            {
                printf("Results file %u not found!\n\r", index);
                printf("Aborted. Goodbye!\n\r");
                return 0;
            }
            data = (unsigned char *)malloc(file_size);
            File_to_bytes(results_file_name, data, file_size);
            
            if (Send_expected_result(data) == 1)
            {
                printf("Result no:%u transfer success\n\r", index);
            }
            else
            {
                printf("Result no:%u transfer failure\n\r", index);
            }
            free(data);
             /**********************************************************Send result --- end**********************************************************/


            /*******************************************************Send program --- start**********************************************************/
            file_size = Get_data_size(program_file_name);
            if (file_size == -1)
            {
                printf("Program file %u not found!\n\r", index);
                printf("Aborted. Goodbye!\n\r");
                return 0;
            }            
            data = (unsigned char *)malloc(file_size);
            File_to_bytes(program_file_name, data, file_size);

            if (Send_program(data, file_size) == 1)
            {
                printf("Program no:%u transfer success!\n\r", index);
                //index++;
            }
            else
            {
                printf("Program no:%u transfer failure!\n\r", index);
            }
            free(data);
              /**********************************************************Send program --- end**********************************************************/
            /*file_size = Get_data_size(results_file_name);
            if (file_size == -1)
            {
                printf("Results file %u not found!\n\r", index);
                printf("Aborted. Goodbye!\n\r");
                return 0;
            }
            data = (unsigned char *)malloc(file_size);
            File_to_bytes(results_file_name, data, file_size);
            
            if (Send_expected_result(data) == 1)
            {
                printf("Result no:%u transfer success\n\r", index);
            }
            else
            {
                printf("Result no:%u transfer failure\n\r", index);
            }
            free(data);*/
        }
        else if (string[0] == 'N')
        {
            printf("Aborted. Goodbye!\n\r");
            return 0;
        }
        else
        {
            printf("Command not recognized. Goodbye!\n\r");
            return 0;
        }
        index++;
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
    // unsigned int program_size = strlen(program_string);
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

            int program_size_small=program_size-1;

       //    sprintf(string, "%c%c%c%c", 0xFF & (program_size), 0xFF & (program_size >> 8), 0xFF & (program_size >> 16), 0xFF & (program_size>> 24));               
            printf("Prog. size TX: ");    
            
            for(int i=0; i<4; i++)
            {
                serial_port_write_byte(0xFF & program_size_small >> (i*8));
                printf("%u:",program_size_small >> (i*8));    
            }
        
            printf("\n\r");  
            comms_state = transmit;
            break;
        case transmit:

            //    serial_port_write_data(program_string, program_size);

            printf("Prog.data TX  -- start:\n\r");
            for (int i = 0; i < program_size; i++)
            {
                serial_port_write_byte(program_string[i]); // transmit the program
                printf(BYTE_TO_BINARY_PATTERN"\t%c\n\r", BYTE_TO_BINARY(program_string[i]),program_string[i]);
                /*serial_port_read(input, sizeof(input)); // wait for Y'A'
                 if (input[0] == 'A')
                {
                    printf("error&((((((((((((((((((((((((((((((((((((((((((((((((("); 
                }*/
            }
            timestamp=time(NULL); //remov dis
            printf("Prog.data TX  -- end.\n\r");
            comms_state = confirm;
            break;
        case confirm:
            serial_port_read(input, sizeof(input)); // wait for Y'A'
            if (input[0] == 'A')
            {
                comms_success = 1;
                comms_state = end;
                printf("rcvd input:%s",input);
                break;
            }
            if (time(NULL) - timestamp > 5)
            {
                comms_success = 0;
                comms_state = end;
                break;
            }
            /*Uncomment this line to keep sending the file until timeout */
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
    char string[4] = {0};
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
            printf("Result.data TX:  --  ");        
        
            //for (int i = 0; i < 4; i++)
            for (int i = 3; i >= 0; i--)
            {
                serial_port_write_byte(result[i]);
                printf("%u ",(unsigned char)result[i]); 
               
            }
             printf("  --\n\r");
            comms_state = confirm;
            break;
        case confirm:
            serial_port_read(input, sizeof(input)); // wait for Y'A'
            if (input[0] == 'A')
            {
                printf("rcvd input:%s",input);
                comms_success = 1;
                comms_state = end;
                break;
            }
            if (time(NULL) - timestamp > 10)
            {
                comms_success = 0;
                comms_state = end;
                break;
            }
            //uncomment to continue sending until timeout
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

int Get_data_size(char *file_name)
{
    FILE *results_file;
    const int maxSize = 2048;
    char str[maxSize];

    int tmp = 0;

    programs_file = fopen(file_name, "r");
    if (NULL == programs_file)
    {
        return -1;
    }

    while (fgets(str, maxSize, programs_file) != NULL)
    {
        tmp++;
    }

    fclose(programs_file);

    return tmp;
}

int String_to_byte(char *str, unsigned char *res, const int index)
{

    res[index] = 0x00;
    char tmp = 0x00;

    if (strlen(str) <= 8)
    {
        printf("Unexpected string length for converting binary string to byte. \n");
        return 0;
    }

    for (int i = 0; i < 8; i++)
    {

        // Set '0'
        if (str[i] == 0x30)
        {
            res[index] = res[index] << 1 & 0xFE;
        }
        // Set '1'
        else if (str[i] == 0x31)
        {
            res[index] = res[index] << 1 | 0x01;
        }
        // Not happppppy :(
        else
        {
            printf("Bit %i in value %s isn't binary.\n", i, str);
            return 0;
        }
    }

    return 1;
}

// Return the byte number
int File_to_bytes(char *file_name, unsigned char *result, const int size)
{

    const int maxSize = 2048;
    char str[maxSize];

    programs_file = fopen(file_name, "r");
    if (NULL == programs_file)
    {
        return 0;
    }
    for (int i = 0; i < size; i++)
    {
        if (fgets(str, maxSize, programs_file) != NULL)
        {
            String_to_byte(str, result, i);
        }
    }

    fclose(programs_file);

    return 1;
}
