#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS_GLOBALS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include<stdbool.h>
#include <errno.h>
#include <inttypes.h>

#define VALUE 32;

typedef union {
    int d;
    int q;
    float f;
    struct
    {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;

    } raw;

} myfloat;
typedef struct parameter
{
    char name[20];
    int value;
} parameter;

// Global variable initiation

parameter scoreboard_params[13] = {
    {"add_nr_units", 0},
    {"sub_nr_units", 0},
    {"mul_nr_units", 0},
    {"div_nr_units", 0},
    {"ld_nr_units", 0},
    {"st_nr_units", 0},
    {"add_delay", 0},
    {"sub_delay", 0},
    {"mul_delay", 0},
    {"div_delay", 0},
    {"ld_delay", 0},
    {"st_delay", 0}
};

typedef struct unit {
    int stage; // 0 = issue, 1 = read operands, 2 = execute complete, 3 = write back
    int busy;
}unit;

unit units[100] = { 0 }; // TODO: think about how many units to define.
char *fifo[4096];
unsigned char MEMORY[32][4096] = { 0 };

char trace_unit[20];

myfloat regfile[16] = { 0 };

int clock = 0;

int mem_index = 0;
void printBinary(int n, int i)
{

    // Prints the binary representation
    // of a number n up to i-bits.
    int k;
    for (k = i - 1; k >= 0; k--) {

        if ((n >> k) & 1)
            printf("1");
        else
            printf("0");
    }
}



void printIEEE(myfloat var)
{
    // Prints the IEEE 754 representation
    // of a float value (32 bits)
    printf("%d | ", var.raw.sign);
    printBinary(var.raw.exponent, 8);
    printf(" | ");
    printBinary(var.raw.mantissa, 23);
    printf("\n");
}


void init_regfile(myfloat* regfile) {
    for (int i=0; i < 16; i++){
    regfile[i].f = i;
    regfile[i].d = 0;
    regfile[i].q = 0;
    }
}




void create_traceinst() {

}

void create_traceunit(char *unit_name) {

}

void add_to_traceunit(FILE * fp) {

}

void add_to_traceinst(FILE* fp) {

}



// One-time writing
int write_regout() {

    for (int i = 0; i < 16; i++) {
        printf("%f", regfile[i].f);
    }
    return 0;
}

int write_memout() {
    return 0;
}

// read and save the the memin.txt input file 
void read_memin(FILE* fp_memin) {
    char buffer[500];
    char* line;

    int i = 0;
    while (fgets(buffer, 500, fp_memin)) {
        printf("%s\n", buffer);
        line = strtok(buffer, "\n");
        strcpy(MEMORY[i], line);
        i += 1;
    }
    return ;
}

void init_scoreboard_params(FILE * fp_cfg) {
    char buffer[500]="add_nr_units = 2";
    char* param;
    char* value;
    char* line;
    int value_integer;
    int i = 0;
    while (fgets(buffer, 500, fp_cfg)) {
        printf("%s\n", buffer);
        line = strtok(buffer, "\n");
        param = strtok(buffer, " = "); // TODO : consider handle spaces in input file.
        value = strtok(NULL, " = ");
        //value[strlen(value)] = '\0';
        
        strcpy(scoreboard_params[i].name, param);

        if (strcmp(param, "trace_unit") == 0) {
            strcpy(trace_unit, value);
            if (i == 12) return; // then this is the last line in the input file.
        }
        sscanf(value, "%d", &value_integer);
        scoreboard_params[i].value = value_integer;
        i += 1;
    }
    return;
}

int fifo_is_empty() {
// check if the fifo is empty.
}

int empty_fifo() {
    // empty the fifo from instructions.
}

void add_to_fifo() {

}

char * decrypt_instruction(char * mem_line, char * instruction) {
    // returns which instruction it is (add/sub/mul/div/halt)
}
int check_war() {

}
void read_registers() {

}

int check_structural(int opcode, char * instruction) {
    
    return 0;
}
void write_result(unit unit_i) {
    unit_i.stage = 0;
}

void execute(unit unit_i) {
    unit_i.stage += 1;

}
void read_operands(unit unit_i) {
    unit_i.stage += 1;
}
void issue(int instruction) {
    

    //unit_i.stage += 1;

}



void continue_execution(unit unit_i) {
    //TODO: check the cases where hazards can occur.

    if (unit_i.stage == 1) { // read operands
        read_operands(unit_i);
        return;
    }
    else if (unit_i.stage == 2) { // execute
        execute(unit_i);
        return;
    }
    else if (unit_i.stage == 3) { // write result
        if (check_war()) { // no hazard
            write_result(unit_i);
            return;
        }
        else {
            return;
        }
    }
};

int unit_is_busy(unit unit_i) {

}


int can_issue_instruction() {

}

int get_opcode(char* instruction) {

}
int instruction_in_fifo(char* instruction) {

}

void run_scoreboard() {

    while (true) {
        int opcode = -1, is_structural = 0;
        char* instruction[20];

        int TOTAL_NUM_OF_UNITS = 5; // TODO: after generating the functional units, edit this. maybe need some malloc :(

        for (int i = 0; i < TOTAL_NUM_OF_UNITS; i++) {
            if (units[i].busy) {
                continue_execution(units[i]);
            }
        }

        if (!fifo_is_empty() && can_issue_instruction()) {
            issue(fifo[0]);
            empty_fifo();
            clock += 1;
            mem_index += 1;
            continue;

        }

        if (fifo_is_empty()) {
            // to ask Gadi - what if getting halt and there are still commands to execute that 
            // didn't finish yet?
            decrypt_instruction(MEMORY[mem_index], instruction);
            opcode = get_opcode(instruction);
        }
        
        if (opcode == 6) break; // halt == 6
        if ((is_structural=check_structural(opcode, instruction)) == 1) { // All units full
            if (!instruction_in_fifo(instruction)) {
                add_to_fifo(instruction);
            }
            clock += 1;
            continue;
        }
        issue(instruction); // assuming that stalled ops wait for issue in fifo
        
        clock += 1;
        mem_index += 1;
    }

    write_memout();
    write_regout();
}

int main(int argc, char* argv[]) {
    
    // Check num of arguments 
    
    /*
    if (argc != 7) {
        printf("Number of arguemnts is wrong");
        return -1;
    }
    */

    // Initiate the regfile
    printf("Welcome to scoreboard project !\n\n");
    
    
	init_regfile(regfile);

    //Sanity check for regfile
    for (int i = 0; i < 16; i++) {
        printf(" d = %d, q = %d representation of %f is ", regfile[i].d, regfile[i].q, regfile[i].f);
        printIEEE(regfile[i]);
    }

    // Handle input files
    FILE* fp_cfg;
    FILE* fp_memin;

    fp_cfg = fopen("cfg.txt", "r"); //argv[1]
    if (fp_cfg == NULL) return -1;

    init_scoreboard_params(fp_cfg);

    fclose(fp_cfg);

    fp_memin = fopen("memin.txt", "r"); //argv[2]
    if (fp_memin == NULL) return -1;
    read_memin(fp_memin);

    fclose(fp_memin);

    //Sanity check for scoreboard params and memory
    for (int i = 0; i < 12; i++) {
        printf("%s system param equals %d \n", scoreboard_params[i].name, scoreboard_params[i].value);
    }
    printf("trace unit: %s\n", trace_unit);

    for (int i = 0; i < 20; i++) {
        printf("Mem val: %s\n", MEMORY[i]);
    }

    run_scoreboard();

	return 0;
}