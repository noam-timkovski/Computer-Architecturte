#pragma once

//------------------------------------define-------------------------------------------//#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS_GLOBALS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define VALUE 32
#define INST_FIFO_SIZE 16
#define NUM_OF_OPCODES 7
#define READY 1
#define NOT_READY 0 
#define YES 1
#define NO 0 
#define length_of_registers 8
#define dmemin_depth 4096
#define dmemin_length 32

//------------------------------------------include--------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include<stdbool.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>

//---------------------------------struture define-----------------------------------------//

typedef struct opcode {
    const char* key;
    int value;
} opcode;

typedef union {
    float f;
    struct
    {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
    } raw;
} myfloat;

// Registers declaration
typedef struct {
    int index;
    int d;
    int q;
    float f;
    char  unit_name[20];
    int just_freed; // to make sure that if write_back happens at cycle X so read_operands of other instruction will happen at cycle X+1
} myreg;

typedef struct parameter
{
    char* name;
    int amount;
    int delay;
} parameter;



typedef struct unit {
    char type[20];
    int stage; // 0 = issue, 1 = read operands, 2 = execute complete, 3 = write back
    int busy;
    int latency;
    int index;
    opcode op;
    myreg fi;
    myreg fj;
    myreg fk;
    bool  rj;
    bool  rk;
    char qj[20];
    char qk[20];
    char imm[4];
    int cycles_left;
    int instruction_id;
    int trace;
    int first_execute;

    struct unit* next;
}unit;


typedef struct imemin_line {

    opcode opcode;
    char dst;
    char src0;
    char src1;
    char imm[4];
    struct imemin_line* next;


}imemin_line;

//typedef struct fifo {
//    imemin_line* front;
//    imemin_line* rear;
//    int count;
//}fifo;

typedef struct fifo {
    char** queue[INST_FIFO_SIZE];
    int count;
    char* name;
}fifo;
//----------------------------------global tables------------------------------------------//
char opcodes[NUM_OF_OPCODES][5] = {
    "LD" , "ST" , "ADD" , "SUB " , "MULT" , "DIV" , "HALT"
};



/* build lookup table for instructions */

opcode opcode_table[] = {
    {"LD",0}, {"ST",1} , {"ADD",2} , {"SUB",3} , {"MULT",4} , {"DIV",5} , {"HALT",6}
};



// Global variable initiation

parameter scoreboard_params[6] = {
    {"ADD", 0,0},
    {"SUB", 0,0},
    {"MUL", 0,0},
    {"DIV", 0,0},
    {"LD", 0,0},
    {"ST", 0,0},

};

//sb_params sb_paramerts;
//--------------------------------------units-------------------------------------------------------//




//char *fifo[INST_FIFO_SIZE];
//int fifo_count = 0;

unsigned char MEMORY[dmemin_depth][dmemin_length] = { 0 }; /// Maybe need to change the size to [8][4096]

char INSTRUCTION_TRACE[dmemin_depth][1000] = { 0 };

char UNIT_TRACE[4096][500] = { 0 };
int unit_trace_index = 0;
char trace_unit[20];

myreg regfile[17] = { 0 };

int clock = 1;

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

void sign_extension(char arr[length_of_registers + 1], int x) {
    char x_hexa_string[length_of_registers + 1];
    char register_sign_extend_x[length_of_registers + 1] = { 0 };
    int bit = 0, countr = 0;
    if (x == 0) {
        strcpy(arr, "00000000");
        return;
    }
    if (x > 0) {// positive int need to zeropadding
        _itoa(x, x_hexa_string, 16); // change int to hexa
        while (bit < length_of_registers) {
            if (bit < length_of_registers - strlen(x_hexa_string)) {
                register_sign_extend_x[bit] = '0';
                bit++;
            }
            else {
                register_sign_extend_x[bit] = x_hexa_string[countr];
                countr++;
                bit++;
            }
        }
        strcpy(arr, register_sign_extend_x);
    }
    if (x < 0) {// negetive int need to add ones

        _itoa(x, x_hexa_string, 16); //change int to hexa. if it was a negetive int the function add ones until getting eight hexa string
        strcpy(arr, x_hexa_string);
    }
}


void printIEEE(float value)
{
    // Prints the IEEE 754 representation
    // of a float value (32 bits)
    myfloat var;
    var.f = value;
    printf("%d | ", var.raw.sign);
    printBinary(var.raw.exponent, 8);
    printf(" | ");
    printBinary(var.raw.mantissa, 23);
    printf("\n");
}


void init_regfile(myreg* regfile) {
    for (int i = 0; i < 16; i++) {
        regfile[i].index = i;
        regfile[i].f = i;
        regfile[i].d = 0;
        regfile[i].q = 0;
        regfile[i].just_freed = 0;
        strcpy(regfile[i].unit_name, "");
    }
    // Null register
    regfile[16].index = 0;
    regfile[16].f = 0;
    regfile[16].d = 0;
    regfile[16].q = 0;
    strcpy(regfile[16].unit_name, "");













}



// One-time writing
int write_regout(FILE* regout) {

    printf("\nWriting F0 - F15 to regout.txt . .\n");
    for (int i = 0; i < 16; i++) {
        printf("F%d : %f\n", i, regfile[i].f);
    }
    for (int i = 0; i < 16; i++)
    {
        fprintf(regout, "%\n", regfile[i].f);
    }
    fclose(regout);
    return 0;
}

void create_dmemout(FILE* dmemout)
{
    for (int i = 0; i < dmemin_depth; i++)
    {
        fprintf(dmemout, "%s\n", MEMORY[i]);
    }
    fclose(dmemout);
}

// read and save the the memin.txt input file 
void read_memin(FILE* fp_memin) {
    char buffer[500];
    char* line;

    int i = 0;
    while (fgets(buffer, 500, fp_memin)) {
        //printf("%s\n", buffer);
        line = strtok(buffer, "\n");
        strcpy(MEMORY[i], line);
        i += 1;
    }
    // Create Instruction Trace for traceinst.txt file. copy all the instructions up to HALT 
    char buf[12];
    char pc_buf[12]; // up to 4096 rows
    for (int j = 0; j < i; j++) {
        if (MEMORY[j][1] == '6') break;
        strcpy(buf, MEMORY[j]);
        sprintf(pc_buf, " %d ", j);
        strcat(buf, pc_buf);
        strcpy(INSTRUCTION_TRACE[j], buf);
    }
    return;
}

void init_scoreboard_params(FILE* fp_cfg) {

    char buffer[500] = { 0 };
    char param[20] = { 0 };
    char value[2] = { 0 };
    char line;
    int value_integer;
    int i = 0;
    while (fgets(buffer, 500, fp_cfg)) {
        //printf("%s\n", buffer);
        if (sscanf(buffer, "%[^=]=%d", param, &value_integer) > 0) {
        };
        if (strcmp(param, "trace_unit ") == 0) {
            const char s[4] = "=";
            char* tok;
            char* unit;
            tok = strtok(buffer, s);
            tok = strtok(0, s);
            unit = strtok(tok, " \n");
            strcpy(trace_unit, unit);
            return;
        }
        if (i < 6) {
            scoreboard_params[i].amount = value_integer;
        }
        else if (i < 12) {
            scoreboard_params[i - 6].delay = value_integer;
        }
        i += 1;
    }
    return;
}


//void init(fifo* q) {
//    q->front = NULL;
//    q->rear = NULL;
//    q->count = 0;
//
//}

/*
*  check if the fifo is empty.
*/
int fifo_is_empty(fifo fifo) {
    
    return(fifo.count==0);       
    
}

char* empty_fifo(fifo fifo) {
    // return the first var in the queue , removes it and promote the rest of the queue forwoard .
    if (fifo.count == 0) {
        printf("no elements to extract from queue \n");
        return "error";
    }
    char* res = fifo.queue[0];
    for (int i = 0; i < fifo.count - 1; i++) {
        fifo.queue[i] = fifo.queue[i + 1];
    }
    fifo.count--;
    return res;

}

void add_to_fifo(char **instruction[32], fifo fifo) {
    if (fifo.count == INST_FIFO_SIZE) {
        printf("fifo is full");
        return;
    }
    *fifo.queue[fifo.count] = *instruction;
    fifo.count++;
}

// Add an element to the queue

//int fifo_is_full(fifo* q) {
//    return (q->count == INST_FIFO_SIZE);
//}
//void enqueue(fifo* q, imemin_line instruction) {
//    if (fifo_is_full(q)) {
//        printf("Error: queue is full\n");
//        return;
//    }
//    imemin_line* new_node = malloc(sizeof(imemin_line));
//    new_node->dst=instruction.dst;
//    strcpy(new_node->imm ,instruction.imm);
//
//    new_node->opcode= instruction.opcode;
//    new_node->src0 = instruction.src0;
//    new_node->src1 = instruction.src1;
//
//    new_node->next = NULL;
//    if (fifo_is_empty(q)) {
//        q->front = new_node;
//        q->rear = new_node;
//    }
//    else {
//        q->rear->next = new_node;
//        q->rear = new_node;
//    }
//    q->count++;
//}
//
//// Remove an element from the queue
//imemin_line dequeue(fifo* q) {
//    if (fifo_is_empty(q)) {
//        printf("Error: queue is empty\n");
//        return -1;
//    }
//    imemin_line* front_node = q->front;
//    int element = front_node->data;
//    q->front = front_node->next;
//    if (q->front == NULL) {
//        q->rear = NULL;
//    }
//    free(front_node);
//    q->size--;
//    return element;
//}

opcode find_opcode(opcode* opcode_table, int opcode_value) {
    for (int i = 0; i <= NUM_OF_OPCODES; i++) {
        if (opcode_table[i].value == opcode_value) {
            return opcode_table[i];
            break;
        }
    }
}
imemin_line decrypt_instruction(char* mem_line) {
    // returns which instruction it is (add/sub/mul/div/halt)
    char res, opcode_str[2], dst, src0, src1, imm[4];
    imemin_line current_instruction;
    int opcode_int = 0;
    opcode opcode;

    res = *mem_line;
    mem_line++;
    opcode_str[0] = *mem_line;
    opcode_str[1] = '\0';
    mem_line++;
    dst = *mem_line;
    mem_line++;
    src0 = *mem_line;
    mem_line++;
    src1 = *mem_line;
    mem_line++;
    imm[0] = *mem_line;
    mem_line++;
    imm[1] = *mem_line;
    mem_line++;
    imm[2] = *mem_line;
    mem_line++;
    imm[3] = '\0';

    opcode_int = (int)strtoll(opcode_str, (char**)NULL, 16); // maybe need to add a checker to illeagel opcode number
    opcode = find_opcode(opcode_table, opcode_int);
    current_instruction.opcode = opcode;
    current_instruction.dst = dst;
    current_instruction.src0 = src0;
    current_instruction.src1 = src1;
    strcpy(current_instruction.imm, imm);

    return current_instruction;



}
int check_war() {

}
void read_registers() {

}

/*  This function checks for structural hazard
    Returns 0 if no hazard
*/
int iterate_list(unit** head) {
    unit* iter = *head;

    while (iter != NULL) {
        if (iter->busy == 0) {
            return 0;
        }
        iter = iter->next;
    }
    return 1;
}

// return 1 if a structural hazard exists.
int structural_hazard(opcode opcode, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    unit* iter_add, * iter_sub, * iter_mul, * iter_div, * iter_ld, * iter_st;

    if (opcode.value == 0) {
        if (iterate_list(ld_units) == 0) return 0;
    }
    else if (opcode.value == 1) {
        if (iterate_list(st_units) == 0) return 0;
    }
    else if (opcode.value == 2) {
        if (iterate_list(add_units) == 0) return 0;
    }
    else if (opcode.value == 3) {
        if (iterate_list(sub_units) == 0) return 0;
    }
    else if (opcode.value == 4) {
        if (iterate_list(mul_units) == 0) return 0;
    }
    else if (opcode.value == 5) {
        if (iterate_list(div_units) == 0) return 0;
    }

    return 1;
}
void write_result(unit* unit_i) {
    unit_i->rj = YES;
    unit_i->rk = YES;
    unit_i->busy = NO;
    int st_memory_index = 0;
    char st_memory_value[10];
    char hex_str[17];
    if (strcmp(unit_i->type, "ADD") == 0) {
        //unit_i->fi.d = unit_i->fj.f + unit_i->fk.f;
        regfile[unit_i->fi.index].f = (int)regfile[unit_i->fj.index].f + (int)regfile[unit_i->fk.index].f;
    }
    else if (strcmp(unit_i->type, "SUB") == 0) {

        //unit_i->fi.d = unit_i->fj.f - unit_i->fk.f;
        regfile[unit_i->fi.index].f = regfile[unit_i->fj.index].f - regfile[unit_i->fk.index].f;
    }
    else if (strcmp(unit_i->type, "MUL") == 0) {

        //unit_i->fi.d = unit_i->fj.f * unit_i->fk.f;
        regfile[unit_i->fi.index].f = regfile[unit_i->fj.index].f * regfile[unit_i->fk.index].f;
    }
    else if (strcmp(unit_i->type, "DIV") == 0) {

        //unit_i->fi.d = unit_i->fj.f / unit_i->fk.f;
        regfile[unit_i->fi.index].f = regfile[unit_i->fj.index].f / regfile[unit_i->fk.index].f;
    }
    else if (strcmp(unit_i->type, "LD") == 0) {
        int imm;
        imm = atoi(unit_i->imm);
        char* memory_float = { 0 };
        char mantissa[24];
        char exponent[9];
        char sign[2];

        memory_float = &MEMORY[imm];
        sign[0] = *memory_float;
        sign[1] = '\0';
        memory_float++;
        for (int i = 0; i < 8; i++) {
            exponent[7 - i] = *memory_float;
            memory_float++;
        }
        exponent[8] = '\0';
        for (int i = 0; i < 24; i++) {
            mantissa[23 - i] = *memory_float;
            memory_float++;
        }
        mantissa[23] = '\0';

        //unit_i->fi.f;


        int memory_index = 0;
        char* ptr;
        // Convert immediate to integer

 /*       int len = strlen(unit_i->imm);
        for (int i = 0; i < len; i++) {
            memory_index = memory_index * 10 + (unit_i->imm[i] - '0');
        }*/
        mem_index = strtol(unit_i->imm, &ptr, 16);
        char memory_value[9];
        strcpy(memory_value, MEMORY[mem_index]);



        float new_reg_value = 0;
        
        long hex_value = strtol(memory_value, &ptr, 16);
        float float_value ;
        memcpy(&float_value, &hex_value, sizeof(float));
        printf("Floating point value: %f\n", float_value);
        //char binary_string[33];
        //sprintf(binary_string, "%032lx", hex_value);
        ////printf("Hexadecimal string: %s\n", hex_string);
        //printf("Long integer: %ld\n", hex_value);
        //printf("Binary string: %s\n", binary_string);

        //sscanf(MEMORY[memory_index], "%f", &new_reg_value);
        //printf("%.6f\n", new_reg_value);
        //// Convert new register value to integer and then cast to integer
        //int new_reg_len = strlen(memory_value);
        //for (int i = 0; i < new_reg_len; i++) {
        //    new_reg_value = new_reg_value * 10 + (memory_value[i] - '0');
        //}
        //// convert from hex to binary 
        regfile[unit_i->fi.index].f = float_value;

    }
    else if (strcmp(unit_i->type, "ST") == 0) {
        char* ptr;
        // Convert immediate to integer
        //int len = strlen(unit_i->imm);
        //for (int i = 0; i < len; i++) {
        //    st_memory_index = st_memory_index * 10 + (unit_i->imm[i] - '0');
        //}

        long hex_value;
        float float_value;
        memcpy(&hex_value, &regfile[unit_i->fk.index].f, sizeof(long));
        //printf("Floating point value: %f\n", float_value);  
        st_memory_index = strtol(unit_i->imm, &ptr, 16);
        
        sprintf(hex_str, "%lx", hex_value);
        strcpy(MEMORY[st_memory_index], hex_str);

        //int mem_temp = (int)regfile[unit_i->fk.index].f;
        //char hex_string[9];
        //char dest[9];
        ////sign_extension(hex_str, mem_temp);
        //sprintf(hex_str, "%x", mem_temp);
        //memcpy(dest, hex_str, sizeof(hex_str));
        //snprintf(st_memory_value, 10, "%d", mem_temp);
        

        //unit_i->fi.d = unit_i->fj.f + unit_i->fk.f;
        //strcpy(regfile[unit_i->fk.index].unit_name, "");
    }

    // update INSTRUCTION_TRACE 
    char buf_traceinst[1000];
    char cycle_write_result[10];
    strcpy(buf_traceinst, INSTRUCTION_TRACE[unit_i->instruction_id]);
    snprintf(cycle_write_result, sizeof(cycle_write_result), "%d", clock);
    strcat(buf_traceinst, cycle_write_result);
    strcpy(INSTRUCTION_TRACE[unit_i->instruction_id], buf_traceinst);

    // update unit trace 




    // FIX HERE THE REGFILE UPDATE CLEANING
    int dst_index = unit_i->fi.index;
    int src0_index = unit_i->fj.index;
    int src1_index = unit_i->fk.index;

    if (unit_i->op.value == 1) { // ST instruction
        strcpy(regfile[src1_index].unit_name, "");
        regfile[src1_index].just_freed = 1;
    }
    else {
        strcpy(regfile[dst_index].unit_name, "");
        regfile[dst_index].just_freed = 1;
    }
    if (unit_i->op.value == 1) {
        printf("unit %s%d loaded F%d = %s to MEM[%d]\n", unit_i->type, unit_i->index, regfile[src1_index].index, hex_str, st_memory_index);
    }
    else {
        printf("unit %s%d at write_result stage, new F%d : %f\n", unit_i->type, unit_i->index, unit_i->fi.index, regfile[unit_i->fi.index].f);
    }

}

void execute(unit* unit_i) {
    //  unit_i.stage += 1;
    unit_i->cycles_left -= 1;
    if (unit_i->cycles_left == 0) {
        unit_i->stage += 1;

        // update INSTRUCTION_TRACE 
        char buf_traceinst[1000];
        char cycle_execution_end[10];
        strcpy(buf_traceinst, INSTRUCTION_TRACE[unit_i->instruction_id]);
        snprintf(cycle_execution_end, sizeof(cycle_execution_end), "%d ", clock);
        strcat(buf_traceinst, cycle_execution_end);
        strcpy(INSTRUCTION_TRACE[unit_i->instruction_id], buf_traceinst);

    }
    printf("unit %s%d during execute stage, cycles left : %d, rj=%d, rk=%d\n", unit_i->type, unit_i->index, unit_i->cycles_left, unit_i->rj, unit_i->rk);
}
void read_operands(unit* unit_i) {
    // reading the rj and rk values 
   /* if (check_raw(unit_i) != 1) {

    }*/
    unit_i->fi.q = unit_i->fi.d;
    unit_i->fj.q = unit_i->fj.d;
    if (unit_i->op.value == 0) { // LD instruction
        unit_i->rk = NO;
    }
    else {
        unit_i->rj = NO;
        unit_i->rk = NO;
    }

    unit_i->stage += 1;
    unit_i->cycles_left = unit_i->latency;

    // update INSTRUCTION_TRACE

    char buf_traceinst[1000];
    char cycle_read_operands[10];
    strcpy(buf_traceinst, INSTRUCTION_TRACE[unit_i->instruction_id]);
    snprintf(cycle_read_operands, sizeof(cycle_read_operands), "%d ", clock);
    strcat(buf_traceinst, cycle_read_operands);
    strcpy(INSTRUCTION_TRACE[unit_i->instruction_id], buf_traceinst);


    printf("unit %s%d finished read operands stage, cycles left : %d, rj=%d, rk=%d\n", unit_i->type, unit_i->index, unit_i->cycles_left, unit_i->rj, unit_i->rk);
    return;

}


void update_unit(unit** head, imemin_line instruction) {


    unit* iter;
    //printf("iter . busy = %d", iter->busy);
    for (iter = *head; iter->next != NULL; iter = iter->next) {
        if (iter->busy != 1) {
            break;
        }
    }
    int dst_index = (int)(instruction.dst) - '0';
    int src0_index = (int)(instruction.src0) - '0';
    int src1_index = (int)(instruction.src1) - '0';

    iter->stage = 1;
    iter->busy = 1;
    iter->op = instruction.opcode;





    strcpy(iter->imm, instruction.imm);

    if (instruction.opcode.value == 1) { // ST instruction : MEM[IMM] = F[SRC1], // LD instruction: F[DST] = MEM[IMM]
        strcpy(iter->qj, "");
        strcpy(iter->qk, "");
    }
    else {
        strcpy(iter->qj, regfile[src0_index].unit_name); // need to think how to initiate this field 
        strcpy(iter->qk, regfile[src1_index].unit_name);// need to think how to initiate this field
    }

    iter->fi = regfile[dst_index];
    iter->fj = regfile[src0_index];
    iter->fk = regfile[src1_index];

    if ((strcmp(iter->qj, "") == 0)) { // then the register rj is ready to use
        iter->rj = YES;
    }
    else {
        iter->rj = NO;
    }
    if ((strcmp(iter->qk, "") == 0)) { // then the register rk is ready to use
        iter->rk = YES;

    }
    else {
        iter->rk = NO;
    }


    //update regfile 

    char new_unit_name[20];
    char index_str[4];
    strcpy(new_unit_name, iter->type);
    snprintf(index_str, 4, "%d", iter->index);
    strcat(new_unit_name, index_str);

    //update instruction trace

    iter->instruction_id = mem_index;
    char buf_traceinst[1000];
    char cycle_issued[10];
    strcpy(buf_traceinst, INSTRUCTION_TRACE[iter->instruction_id]);
    strcat(buf_traceinst, new_unit_name);
    snprintf(cycle_issued, sizeof(cycle_issued), " %d ", clock);
    strcat(buf_traceinst, cycle_issued);
    strcpy(INSTRUCTION_TRACE[iter->instruction_id], buf_traceinst);

    //update trace unit trace
    if (strcmp(trace_unit, new_unit_name) == 0) {
        iter->trace = 1;
    }
    if (instruction.opcode.value != 1) { // All instructions except for store.
        strcpy(regfile[dst_index].unit_name, new_unit_name);
    }


    printf("unit %s%d issue stage, cycles left : %d, rj=%d, rk=%d\n", iter->type, iter->index, iter->latency, iter->rj, iter->rk);

}
void issue(imemin_line instruction, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {

    //check raw(); ??

    if (strcmp(instruction.opcode.key, "ADD") == 0) {
        update_unit(add_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "SUB") == 0) {
        update_unit(sub_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "DIV") == 0) {
        update_unit(div_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "MUL") == 0) {
        update_unit(mul_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "LD") == 0) {
        update_unit(ld_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "ST") == 0) {
        update_unit(st_units, instruction);
    }

}

void handle_traceunit(unit* unit_i, char* new_unit_name, int dst_index, int src0_index, int src1_index) {

    char reg_dst_index[4];
    char reg_src0_index[4];
    char reg_src1_index[4];

    char reg_dst[10];
    char reg_src0[10];
    char reg_src1[10];

    snprintf(reg_dst_index, sizeof(reg_dst_index), "%d ", regfile[dst_index].index);
    snprintf(reg_src0_index, sizeof(reg_src0_index), "%d ", regfile[src0_index].index);
    snprintf(reg_src1_index, sizeof(reg_src1_index), "%d ", regfile[src1_index].index);

    strcpy(reg_dst, " F");
    strcat(reg_dst, reg_dst_index);

    strcpy(reg_src0, "F");
    strcat(reg_src0, reg_src0_index);

    strcpy(reg_src1, "F");
    strcat(reg_src1, reg_src1_index);


    char buff[1000];
    char curr_cycle[100];

    snprintf(curr_cycle, sizeof(curr_cycle), "%d ", clock);
    strcpy(buff, curr_cycle);
    strcat(buff, new_unit_name);


    strcat(buff, reg_dst);
    strcat(buff, reg_src0);
    strcat(buff, reg_src1);

    char qj[10];
    char qk[10];
    char rj[10];
    char rk[10];

    if (unit_i->stage == 1) { // read operands
        if ((strcmp(regfile[src0_index].unit_name, "") == 0)) {
            strcpy(qj, "- ");
            strcpy(rj, " Yes ");
        }
        else {
            if (strcmp(regfile[src0_index].unit_name, new_unit_name) == 0) { // this is the same unit
                strcpy(qj, "- ");
                strcpy(rj, " Yes ");
            }
            else { // qj is taken 
                strcpy(qj, regfile[src0_index].unit_name);
                strcpy(rj, " Yes ");
            }
        }

        if ((strcmp(regfile[src1_index].unit_name, "") == 0)) {
            strcpy(qk, "- ");
            strcpy(rk, "Yes ");
        }
        else {
            if (strcmp(regfile[src1_index].unit_name, new_unit_name) == 0) { // this is the same unit
                strcpy(qk, "- ");
                strcpy(rk, "Yes ");
            }
            else {
                strcpy(qk, regfile[src1_index].unit_name);
                strcpy(rk, "No");
            }
        }
        strcat(buff, qj); strcat(buff, qk); strcat(buff, rj); strcat(buff, rk);
    }
    else { // all other stages
        if (unit_i->first_execute) {
            strcpy(qj, "- ");
            strcpy(qk, "- ");
            strcpy(rj, "Yes ");
            strcpy(rk, "Yes ");
            strcat(buff, qj); strcat(buff, qk); strcat(buff, rj); strcat(buff, rk);
            unit_i->first_execute = 0;
        }
        else {
            strcpy(qj, "- ");
            strcpy(qk, "- ");
            strcpy(rj, "No ");
            strcpy(rk, "No ");
            strcat(buff, qj); strcat(buff, qk); strcat(buff, rj); strcat(buff, rk);
        }
    }
    strcpy(UNIT_TRACE[unit_trace_index], buff);
    unit_trace_index += 1;

}

void continue_execution(unit* unit_i) {

    char new_unit_name[20];
    char index_str[4];
    strcpy(new_unit_name, unit_i->type);
    snprintf(index_str, 4, "%d", unit_i->index);
    strcat(new_unit_name, index_str);

    int dst_index = (unit_i->fi.index);
    int src0_index = (unit_i->fj.index);
    int src1_index = (unit_i->fk.index);

    if (unit_i->stage == 1) { // read operands
        if (unit_i->op.value == 0) { // LD
            if (strcmp(regfile[dst_index].unit_name, new_unit_name) == 0) { // The dst register is up to date
                read_operands(unit_i);
                execute(unit_i);
            }
            else {
                printf("Could not read operands of %s%d because reg F%d is taken by %s\n", unit_i->type, unit_i->index, regfile[dst_index].index, unit_i->fi.unit_name);
            }
        }
        else if (unit_i->op.value == 1) { // ST
            if ((strcmp(regfile[src1_index].unit_name, new_unit_name) == 0) ||
                (strcmp(regfile[src1_index].unit_name, "") == 0 && regfile[src1_index].just_freed == 0)) { // src1 reg is up to date
                read_operands(unit_i);
                execute(unit_i);
            }
            else {
                printf("Could not read operands of %s%d because reg F%d is taken by %s\n", unit_i->type, unit_i->index, regfile[src1_index].index, unit_i->fk.unit_name);
            }
        }
        else { // all the rest of the instructions
            if ((strcmp(regfile[src0_index].unit_name, new_unit_name) == 0 && strcmp(regfile[src1_index].unit_name, "") == 0 && regfile[src1_index].just_freed == 0) ||
                (strcmp(regfile[src0_index].unit_name, "") == 0 && regfile[src0_index].just_freed == 0 && strcmp(regfile[src1_index].unit_name, new_unit_name) == 0)) { // src0 and src1 reg is up to date
                read_operands(unit_i);
                execute(unit_i);
            }
            else {
                printf("Could not read operands of %s%d because reg F%d is taken by %s or reg F%d is taken by %s\n", unit_i->type, unit_i->index, regfile[src0_index].index, regfile[src0_index].unit_name, regfile[src1_index].index, regfile[src1_index].unit_name);
            }
        }
        if (unit_i->trace == 1) { // so need to trace this unit
            handle_traceunit(unit_i, new_unit_name, dst_index, src0_index, src1_index);
        }
        return;
    }
    else if (unit_i->stage == 2) { // execute
        execute(unit_i);
        if (unit_i->trace == 1) { // so need to trace this unit
            handle_traceunit(unit_i, new_unit_name, dst_index, src0_index, src1_index);
        }
        return;
    }
    else if (unit_i->stage == 3) { // write result
        if (check_war()) { // no hazard
            write_result(unit_i);
            if (unit_i->trace == 1) { // so need to trace this unit
                handle_traceunit(unit_i, new_unit_name, dst_index, src0_index, src1_index);
            }
            return;
        }
        else {
            if (unit_i->trace == 1) { // so need to trace this unit
                handle_traceunit(unit_i, new_unit_name, dst_index, src0_index, src1_index);
            }
            return;
        }
    }
};

int unit_is_busy(unit unit_i) {
    if (unit_i.busy) {
        return 1;
    }
    return 0;
}


int can_issue_instruction() {

    return 1;
}

int instruction_in_fifo(char* instruction) {

}

//int check_raw(unit *unit_i) {
//    if ((unit_i->rj.ready == READY) && (unit_i->rk.ready == READY)) {
//        return 1;
//    }
//    return 0;
//}

int check_waw(imemin_line inst) {
    if (strcmp(regfile[inst.dst].unit_name, inst.opcode.key) == 0) { // the instruction dst register is waiting for another unit to finish 
        return 1;
    }
    return 0;

}

int run_if_busy(unit** head) {
    unit* iter = *head;
    int busy = 0;
    while (iter != NULL) {
        if (iter->busy == 1) {
            if (iter->op.value == 0 && iter->cycles_left == 0) {
                printf("LD finish\n");
            }
            continue_execution(iter);
            busy = 1;
        }
        iter = iter->next;
    }
    return busy;
}
void run_scoreboard(FILE* dmemout, FILE* regout,fifo inst_fifo, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    imemin_line instruction_decoded;
    bool halt_flag = FALSE;
    while (true) {
        char* instruction[20];
        opcode current_opcode = { "",0 };

        // This loop is to handle if write_back happens at cycle X so read_operands of other instruction will happen at cycle X+1
        for (int i = 0; i < 16; i++) {
            regfile[i].just_freed = 0;
        }

        if (halt_flag == FALSE) {
            run_if_busy(add_units);
            run_if_busy(sub_units);
            run_if_busy(mul_units);
            run_if_busy(div_units);
            run_if_busy(ld_units);
            run_if_busy(st_units);
        }


        if (!fifo_is_empty(inst_fifo) && !structural_hazard(current_opcode, add_units, sub_units, mul_units, div_units, ld_units, st_units)
            && check_waw(instruction_decoded)) {
            // issue(inst_fifo.queue[0]); should be imemin_line type
            empty_fifo(inst_fifo);
            clock += 1;
            mem_index += 1;
            continue;

        }
        else if (!fifo_is_empty(inst_fifo)) { // there is still a pending instruction in the fifo, but we need to wait for the previous to complete
            clock += 1;
            continue;
        }
        else if (halt_flag == FALSE) { // can decode instructions

            instruction_decoded = decrypt_instruction(MEMORY[mem_index]);
            current_opcode = instruction_decoded.opcode;
        }


        if (current_opcode.value == 6) { // TODO: let the previous instructions run to complete before exiting.
            printf("opcode value is :%d ! Finishing the current commands and exiting. .\n", current_opcode.value);
            halt_flag = TRUE; // halt == 6

            printf("cycle: %d\n", clock);
            clock += 1;
        }
        if (halt_flag == FALSE &&(structural_hazard(current_opcode, add_units, sub_units, mul_units, div_units, ld_units, st_units) || check_waw(instruction_decoded)) ) {
            add_to_fifo(&MEMORY[mem_index], inst_fifo);
            clock += 1;
            continue;
        }
        if (halt_flag == FALSE && can_issue_instruction(instruction_decoded) ) {
            issue(instruction_decoded, add_units, sub_units, mul_units, div_units, ld_units, st_units);
        }

        if (halt_flag == FALSE) {

            printf("cycle: %d\n", clock);
            clock += 1;
            mem_index += 1;

        }
        else {
            if ((run_if_busy(add_units) + run_if_busy(sub_units) + run_if_busy(mul_units) + run_if_busy(div_units) + run_if_busy(ld_units) + run_if_busy(st_units)) == 0) {
                break;
            }
            else {
                clock += 1;
                printf("cycle: %d\n", clock);
            }
        }

    }

    create_dmemout(dmemout);
    write_regout(regout);
}




void print_list(unit** head) {

    unit* iter = *head;
    while (iter != NULL) {
        printf("unit type: %s, index: %d, latency: %d, stage: %d, busy: %d\n", iter->type, iter->index, iter->latency, iter->stage, iter->busy);
        iter = iter->next;
    }
}

void print_units_vals(unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    printf("\n");
    print_list(add_units);
    print_list(sub_units);
    print_list(mul_units);
    print_list(div_units);
    print_list(ld_units);
    print_list(st_units);
    printf("\n");
}

unit* create_unit(char* type, int delay, int index) {
    unit* new_unit = (unit*)malloc(sizeof(unit));
    if (new_unit != NULL) {
        strcpy(new_unit->type, type);
        new_unit->stage = 0;
        new_unit->busy = 0;
        new_unit->latency = delay;
        new_unit->index = index;
        new_unit->next = NULL;
        new_unit->trace = 0;
        new_unit->first_execute = 1;
    }
    return new_unit;
}


void add_first(unit** head, char* type, int delay, int index) {
    unit* new_unit = create_unit(type, delay, index);
    if (new_unit == NULL) return NULL;
    new_unit->next = *head;
    *head = new_unit;
}


void init_units(unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {

    // init all the units one by one according to scoreboard_params:
    for (int i = scoreboard_params[0].amount; i > 0; i--) { // add_units
        add_first(add_units, "ADD", scoreboard_params[0].delay, i - 1);
    }
    for (int i = scoreboard_params[1].amount; i > 0; i--) { // sub_units
        add_first(sub_units, "SUB", scoreboard_params[1].delay, i - 1);
    }
    for (int i = scoreboard_params[2].amount; i > 0; i--) { // mul_units
        add_first(mul_units, "MUL", scoreboard_params[2].delay, i - 1);
    }
    for (int i = scoreboard_params[3].amount; i > 0; i--) { // div_units
        add_first(div_units, "DIV", scoreboard_params[3].delay, i - 1);
    }
    for (int i = scoreboard_params[4].amount; i > 0; i--) { // ld_units
        add_first(ld_units, "LD", scoreboard_params[4].delay, i - 1);
    }
    for (int i = scoreboard_params[5].amount; i > 0; i--) { // st_units
        add_first(st_units, "ST", scoreboard_params[5].delay, i - 1);
    }

}

void free_list(unit* head) {
    if (head == NULL) return;
    free_list(head->next);
    free(head);
}

void free_units_list(unit* add_units, unit* sub_units, unit* mul_units, unit* div_units, unit* ld_units, unit* st_units) {
    free_list(add_units);
    free_list(sub_units);
    free_list(mul_units);
    free_list(div_units);
    free_list(ld_units);
    free_list(st_units);
    printf("freed all units!\n");
}

void write_traceinst() {
    FILE* fp = fopen("traceinst.txt", "a");
    printf("\nwriting to traceinst.txt : \n");
    for (int i = 0; i < mem_index; i++) {
        printf("%s\n", INSTRUCTION_TRACE[i]);
        fprintf(fp, "%s\n", INSTRUCTION_TRACE[i]);
    }
    fclose(fp);
}

void write_traceunit() {
    FILE* fp = fopen("traceunit.txt", "a");
    printf("\nwriting to traceunit.txt : \n");
    for (int i = 0; i < unit_trace_index; i++) {
        printf("%s\n", UNIT_TRACE[i]);
        fprintf(fp, "%s\n", UNIT_TRACE[i]);
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {

    // initiate instructions fifo
    fifo inst_fifo = { {0},0 , "instructions" };
    parameter* scoreboard_params_p;
    scoreboard_params_p = &scoreboard_params;


    /*  for (int i = 0; i < argc; i++) {
          printf("argv[%d] : %s\n", i, argv[i]);
      }*/


      // Check num of arguments 

      /*
      if (argc != 7) {
          printf("Number of arguemnts is wrong");
          exit(EXIT_FAILURE);
      }
      */

      // Initiate the regfile
    printf("Welcome to scoreboard project !\n\n");


    init_regfile(regfile);

    //Sanity check for regfile
    for (int i = 0; i < 16; i++) {
        printf("Reg #%d: d = %d, q = %d representation of %f is ", i, regfile[i].d, regfile[i].q, regfile[i].f);
        printIEEE(regfile[i].f);
    }

    /// Handle input files
    FILE* fp_cfg, * fp_memin, * dmemout, * regout, * traceint, * traceunit;

    // Read config file
    fp_cfg = fopen(argv[1], "r"); // argv[1]/cfg.txt
    fp_memin = fopen(argv[2], "r"); //argv[2]/memin.txt
    dmemout = fopen(argv[3], "w"); //create and open dmemout
    regout = fopen(argv[4], "w"); //create and open regout
    traceint = fopen(argv[5], "w");
    traceunit = fopen(argv[6], "w");
    if (fp_cfg == NULL) {
        printf("\nMissing config file, exiting");
        exit(EXIT_FAILURE);
    }

    init_scoreboard_params(fp_cfg, &scoreboard_params_p);

    fclose(fp_cfg);

    // Initiate the processor units according to scoreboard params.
    unit* add_units = NULL;
    unit* sub_units = NULL;
    unit* mul_units = NULL;
    unit* div_units = NULL;
    unit* ld_units = NULL;
    unit* st_units = NULL;

    init_units(&add_units, &sub_units, &mul_units, &div_units, &ld_units, &st_units);

    // Sanity check for units instantiation

    print_units_vals(&add_units, &sub_units, &mul_units, &div_units, &ld_units, &st_units);


    // Read memin file
   
    if (fp_memin == NULL) {
        printf("\nMissing memin file, exiting");
        exit(EXIT_FAILURE);
    }

    read_memin(fp_memin);
    fclose(fp_memin);

    //Sanity check for scoreboard params and memory
    for (int i = 0; i < 5; i++) {
        printf("%s system param equals %d with delay %d\n", scoreboard_params[i].name, scoreboard_params[i].amount, scoreboard_params[i].delay);
    }
    printf("trace unit: %s\n", trace_unit);

    for (int i = 0; i < 11; i++) {
        printf("Mem val: %s\n", MEMORY[i]);
    }

    // Handle output files

    FILE* fp_memout, * fp_regout, * fp_traceinst, * fp_traceunit;

    // Create output files
    /*fp_memout = fopen("memout.txt", "w");
    fp_regout = fopen("regout.txt", "w");
    fp_traceinst = fopen("traceinst.txt", "w");
    fp_traceunit = fopen("traceunit.txt", "w");


    if (fp_memout == NULL || fp_regout == NULL ||
        fp_traceinst == NULL || fp_traceunit == NULL) {
        printf("\nError creating output files, exiting");
        exit(EXIT_FAILURE);
    }
    */
    run_scoreboard(dmemout,regout, inst_fifo, &add_units, &sub_units, &mul_units, &div_units, &ld_units, &st_units);

    write_traceinst();
    write_traceunit();


    free_units_list(add_units, sub_units, mul_units, div_units, ld_units, st_units);

    return 0;
}