#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/utsname.h>

#include "main.h"
#include "prompt.h"
#include "inputs.h"
#include "helper_functions.h"
#include "db_config.h"

void check_environment_info(void) {
    fprintf(stdout, "Current Environment Info: \n");
    struct utsname buffer;

    errno = 0;
    if (uname(&buffer) < 0) {
        perror("uname");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "system name = %s\n", buffer.sysname);
    fprintf(stdout, "node name   = %s\n", buffer.nodename);
    fprintf(stdout, "release     = %s\n", buffer.release);
    fprintf(stdout, "version     = %s\n", buffer.version);
    fprintf(stdout, "machine     = %s\n", buffer.machine);
    fprintf(stdout, "==============================\n\n");
}

int main_app() {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (VERBOSE) {
        check_environment_info();
        check_current_path();
    }

    create_folder(WORKSPACE_PATH_FULL);

    query_state_t *query_state = query_state_construct();
    query_state->init(query_state);

    prompt_buf_t *prompt_buf = new_prompt_buf();
    while (1) {
        print_prompt();
        read_input(prompt_buf);

        check_commands(prompt_buf, query_state);
        if (VERBOSE) {
            fprintf(stdout, "Current query state: %s\n",
                    query_state_to_string(query_state->state));
        }

        if (query_state->state == EXIT) {
            fprintf(stdout, "Bye Bye \n");
            break;
        }
    }

    free_prompt_buf(prompt_buf);
    query_state->close(query_state);
    return EXIT_SUCCESS;
}


struct remote_write_packet_raw {
    uint32_t checksum;               // calc remote_write_packet_raw struct without `checksum` itself
    float bat_volt_low_threshold;    // config shutdown battery low threshold
    uint32_t pc_pwr_up_delay;        // microsecond
    uint32_t pc_pwr_down_delay;      // microsecond
    uint8_t do_value;                // GPIO Ouput
    uint8_t pc_pwr_on;               // turn on/off PC
    uint8_t reserve[2];              // data aligned
} ;

uint32_t crc32(const uint8_t* data, const uint32_t length) {
    uint8_t byte;
    uint32_t crc = 0xffffffff;
    uint32_t mask;

    for (uint32_t i = 0; i < length; i++) {
        byte = data[i];
        crc = crc ^ byte;

        for (uint8_t j = 0; j < 8; j++) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xedb88320 & mask);
        }
    }

    return ~crc;
}

int test_offset_crc32() {
    printf("start \n");

    struct remote_write_packet_raw pack = {0};
    pack.checksum = 0x0A0A0A0A;

    pack.bat_volt_low_threshold = 1234.0f;
    pack.pc_pwr_up_delay = 0x1234;
    pack.pc_pwr_down_delay = 0x5678;
    pack.do_value = 0xA;
    pack.pc_pwr_on = true;

    uint8_t offset = sizeof(pack.checksum);
    uint32_t checksum = crc32(&(((uint8_t*)&pack)[offset]), sizeof(pack)-offset);
    pack.checksum = checksum;
    return 0;
}


#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef struct canbus_message_t {
    uint32_t stdid;
    uint32_t extid;
    uint8_t ide;
    uint8_t rtr;
    uint8_t dlc;
    uint8_t data[8];
} canbus_message_t;

canbus_message_t canbus_init(uint8_t ide) {
    uint8_t data = 0;
    canbus_message_t msg = {.stdid=0x123, .extid=0x1ABCDEFA, .ide=ide, .rtr=1,.dlc=8};
    for (uint8_t i=0; i<8; i++) {
        data = (ide==0) ? i+1 : (i+1)*10;
        msg.data[i] = data;
    }
    return msg;
}

void canbus_struct_to_string(const canbus_message_t *msg, char *str) {
    uint32_t id_value = msg->ide ? msg->extid: msg->stdid;
    uint32_t offset = sprintf(str, "ID: 0x%08X, Ext: %u, ", id_value, msg->ide);
    offset += sprintf(str+offset, "RTR: %u, DLC: %u, Data: ", msg->rtr, msg->dlc);
    for (uint8_t i=0; i<8; i++) {
        offset += sprintf(str+offset, "%d ", msg->data[i]);
    }
}

int string_to_canbus_struct(const char *str, canbus_message_t *msg) {
    uint32_t id;
    uint8_t ide;
    uint8_t rtr;
    uint8_t dlc;

    int res = sscanf(str, "ID: 0x%X, Ext: %hhu, RTR: %hhu, DLC: %hhu, ", &id, &ide, &rtr, &dlc);
    if (res != 4) {
        assert(0);
    }

    msg->stdid = (ide==0) ? id : 0;
    msg->extid = (ide==1) ? id : 0;
    msg->ide = ide;
    msg->rtr = rtr;
    msg->dlc = dlc;

    const char *dataPart = strstr(str, "Data: ");
    if (dataPart == NULL) {
        assert(0);
    }

    dataPart += strlen("Data:") + 1;
    for (uint8_t i=0; i<msg->dlc; ++i) {
        uint8_t dataByte;
        int res1 = sscanf(dataPart, "%hhu", &dataByte);
        if (res1 != 1) {
            assert(0);
        }
        msg->data[i] = dataByte;

        // find next ' '
        dataPart = strchr(dataPart, ' ');
        if (dataPart == NULL) {
            break;
        }
        dataPart++;
    }

    return 0;
}

int test_canbus_convert() {
    // canbus -> string test
    char str_0[128] = "";
    canbus_message_t msg_ide0 = canbus_init(0);
    canbus_struct_to_string(&msg_ide0, str_0);
    printf("1. %s \n", str_0);
    // 1. ID: 0x00000123, Ext: 0, RTR: 1, DLC: 8, Data: 1 2 3 4 5 6 7 8

    char str_1[128] = "";
    canbus_message_t msg_ide1 = canbus_init(1);
    canbus_struct_to_string(&msg_ide1, str_1);
    printf("2. %s \n", str_1);
    // 2. ID: 0x1ABCDEFA, Ext: 1, RTR: 1, DLC: 8, Data: 10 20 30 40 50 60 70 80

    // string -> canbus test
    char str_2[] = "ID: 0x00000123, Ext: 0, RTR: 1, DLC: 8, Data: 1 2 3 4 5 6 7 8 ";
    canbus_message_t msg_ide2 = {0};
    string_to_canbus_struct(str_2, &msg_ide2);

    char str_3[] = "ID: 0x1ABCDEFA, Ext: 1, RTR: 1, DLC: 8, Data: 10 20 30 40 50 60 70 80 ";
    canbus_message_t msg_ide3 = {0};
    string_to_canbus_struct(str_3, &msg_ide3);
    return 0;
}

int main() {
    setvbuf(stdout, 0, _IONBF, 0);
    main_app();
//    test_offset_crc32();
//    test_canbus_convert();

    return 0;
}
