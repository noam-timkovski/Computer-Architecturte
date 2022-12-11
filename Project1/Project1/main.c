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


typedef struct {
    int d;
    int q;
    float f;
    char  unit_name[20] ;

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
    char index;
    opcode op;
    myreg fi;
    myreg fj;
    myreg fk;
    bool  rj;
    bool  rk;
    char qj [20];
    char qk [20];
    int cycles_left;

    struct unit* next;
}unit;


typedef struct imemin_line {

    opcode opcode;
    char dst;
    char src0;
    char src1;
    char imm[4];


}imemin_line;

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
    {"add", 0,0},
    {"sub", 0,0},
    {"mul", 0,0},
    {"div", 0,0},
    {"ld", 0,0},
    {"st", 0,0},

};

//sb_params sb_paramerts;
//--------------------------------------units-------------------------------------------------------//




//char *fifo[INST_FIFO_SIZE];
//int fifo_count = 0;

unsigned char MEMORY[32][4096] = { 0 }; /// Maybe need to change the size to [8][4096]

char trace_unit[20];

myreg regfile[96] = { 0 };

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
        regfile[i].f = i;
        regfile[i].d = 0;
        regfile[i].q = 0;
        strcpy(regfile[i].unit_name,"0");
    }
}




void create_traceinst() {

}

void create_traceunit(char* unit_name) {

}

void add_to_traceunit(FILE* fp) {

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
        //printf("%s\n", buffer);
        line = strtok(buffer, "\n");
        strcpy(MEMORY[i], line);
        i += 1;
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
            strcpy(trace_unit, param);
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



int fifo_is_empty(fifo fifo) {
    // check if the fifo is empty.
    if (fifo.count == 0) {
        printf("%s fifo is empty \n",fifo.name);
        return 1;
    }

}

char* empty_fifo(fifo fifo) {
    // return the first var in the queue , removes it and promote the rest of the queue forwoard .
    if (fifo.count == 0) {
        printf("No elements to extract from queue \n");
        return "Error";
    }
    char* res = fifo.queue[0];
    for (int i = 0; i < fifo.count - 1; i++) {
        fifo.queue[i] = fifo.queue[i + 1];
    }
    fifo.count--;
    return res;

}

void add_to_fifo(char* instruction[20], fifo fifo) {
    if (fifo.count == INST_FIFO_SIZE) {
        printf("FIFO IS FULL");
        return;
    }
    fifo.queue[fifo.count] = instruction;
    fifo.count++;
}

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

int check_structural(opcode opcode, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    unit* iter_add, * iter_sub, * iter_mul, * iter_div, * iter_ld, * iter_st;
    iter_add = *add_units;
    iter_sub = *sub_units;
    iter_mul = *mul_units;
    iter_div = *div_units;
    iter_ld = *ld_units;
    iter_st = *st_units;

    if (opcode.value == 0) {
        while (iter_ld != NULL) {
            if (iter_ld->busy == 0) {
                return 0;
                iter_ld = iter_ld->next;
            }
        }
    }
    else if (opcode.value == 1) {
        while (iter_st != NULL) {
            if (iter_st->busy == 0) {
                return 0;
                iter_st = iter_st->next;
            }
        }
    }
    else if (opcode.value == 2) {
        while (iter_add != NULL) {
            if (iter_add->busy == 0) {
                return 0;
                iter_add = iter_add->next;
            }
        }
    }
    else if (opcode.value == 3) {
        while (iter_sub != NULL) {
            if (iter_sub->busy == 0) {
                return 0;
                iter_sub = iter_sub->next;
            }
        }
    }
    else if (opcode.value == 4) {
        while (iter_mul != NULL) {
            if (iter_mul->busy == 0) {
                return 0;
                iter_mul = iter_mul->next;
            }
        }
    }
    else if (opcode.value == 5) {
        while (iter_div != NULL) {
            if (iter_div->busy == 0) {
                return 0;
                iter_div = iter_div->next;
            }
        }
    }
    return 1;
}
void write_result(unit* unit_i) {
    unit_i->rj = YES;
    unit_i->rk = YES;
    unit_i->busy = NO;
    if (strcmp(unit_i->type, "add") == 0) {
        unit_i->fi.d = unit_i->fj.f + unit_i->fk.f;
    }
        
    
    
        

}

void execute(unit* unit_i) {
    //  unit_i.stage += 1;
    unit_i->cycles_left -= 1;
    if (unit_i->cycles_left == 0) {
        unit_i->stage += 1;
    }

}
void read_operands(unit* unit_i) {
    // reading the rj and rk values 
   /* if (check_raw(unit_i) != 1) {

    }*/
    unit_i->rj = NO;
    unit_i->rk = NO;
    unit_i->stage += 1;
    unit_i->cycles_left = unit_i->latency;
    return;

}

unit* find_not_busy_unit();

void update_unit(unit** head, imemin_line instruction) {
    unit* iter = *head;
    for (iter = head; iter != NULL; iter = iter->next) {
        if (iter->busy) {
            continue;
        }
    }
    iter->stage = 1;
    iter->op =  instruction.opcode;
    iter->fi = regfile[instruction.dst];
    iter->fj = regfile[instruction.src0];
    iter->fk = regfile[instruction.src1];
    strcpy(iter->qj,regfile[instruction.src0].unit_name); // need to think how to initiate this field 
    strcpy(iter->qk, regfile[instruction.src1].unit_name);// need to think how to initiate this field 
    iter->rj = YES;
    iter->rk = NO;

    //update regfile
    
    strcpy(regfile[instruction.dst].unit_name, strcat(iter->type,iter->index));

}
void issue(imemin_line instruction, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    unit* iter_add  = *add_units;
    unit* iter_sub  = *sub_units;
    unit* iter_mul  = *mul_units;
    unit* iter_div  = *div_units;
    unit* iter_ld   = *ld_units;
    unit* iter_st   = *st_units;
    //check raw();

    if (strcmp(instruction.opcode.key, "add") == 0) {
        update_unit(&sub_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "sub") == 0) {
        update_unit(add_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "div") == 0) {
        update_unit(div_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "mul") == 0) {
        update_unit(mul_units, instruction);
    }
    else if (strcmp(instruction.opcode.key, "LD") == 0) {
        update_unit(iter_ld, instruction);
    }
    else if (strcmp(instruction.opcode.key, "st") == 0) {
        update_unit(st_units, instruction);
    }



   
}
    //if(instruction.dst)
    /*
    if (instruction.opcode.value == 0) {
        printf("hi");
    }
    */

    //unit_i.stage += 1;





void continue_execution(unit* unit_i) {
    //TODO: check the cases where hazards can occur.

    if (unit_i->stage == 1) { // read operands
        if ((unit_i->rj == YES) && unit_i->rk == YES) {
            read_operands(unit_i);
        }
        return;
    }
    else if (unit_i->stage == 2) { // execute
        execute(unit_i);
        return;
    }
    else if (unit_i->stage == 3) { // write result
        if (check_war()) { // no hazard
            //write_result(unit_i);
            return;
        }
        else {
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
    if (strcmp(regfile[inst.dst].unit_name,inst.opcode.key)==0) { // the instruction dst register is waiting for another unit to finish 
        return 1;
    }
    return 0;
    
}

void run_scoreboard(fifo inst_fifo, unit** add_units, unit** sub_units, unit** mul_units, unit** div_units, unit** ld_units, unit** st_units) {
    imemin_line instruction_decoded;

    while (true) {
        int is_structural = 0;
        char* instruction[20];
        opcode current_opcode = { "",0 };

        unit* iter_add, * iter_sub, * iter_mul, * iter_div, * iter_ld, * iter_st;
        iter_add = *add_units;
        iter_sub = *sub_units;
        iter_mul = *mul_units;
        iter_div = *div_units;
        iter_ld = *ld_units;
        iter_st = *st_units;

        while ((iter_add->next) != NULL) {
            if (iter_add->busy == 1) {
                continue_execution(iter_add);
               
            }
            iter_add = iter_add->next;
        }
        while (iter_sub->next != NULL) {
            if (iter_sub->busy == 1) {
                continue_execution(iter_sub);
                
            }
            iter_sub = iter_sub->next;
        }

        while (iter_mul->next != NULL) {
            if (iter_mul->busy == 1) {
                continue_execution(iter_mul);
               
            }
            iter_mul = iter_mul->next;
        }
        while (iter_div->next != NULL) {
            if (iter_div->busy == 1) {
                continue_execution(iter_div);
               
            }
            iter_div = iter_div->next;
        }
        while (iter_ld->next != NULL) {
            if (iter_ld->busy == 1) {
                continue_execution(iter_ld);
              
            }
            iter_ld = iter_ld->next;
        }
        while (iter_st->next != NULL) {
            if (iter_st->busy == 1) {
                continue_execution(iter_st);
            }
            iter_st = iter_st->next;

        }

        if (!fifo_is_empty(inst_fifo) && check_structural(current_opcode, add_units, sub_units, mul_units, div_units, ld_units, st_units) != 0) { // I'm not sure if can_issue_instruction should be in this condition
            // issue(inst_fifo.queue[0]); should be imemin_line type
            empty_fifo(inst_fifo);
            clock += 1;
            mem_index += 1;
            continue;

        }

        if (fifo_is_empty(inst_fifo)) {
            // to ask Gadi - what if getting halt and there are still commands to execute that 
            // didn't finish yet?
            instruction_decoded = decrypt_instruction(MEMORY[mem_index]);
            current_opcode = instruction_decoded.opcode;

        }

        if (current_opcode.value == 6) {
            break; // halt == 6
        }
        if ((is_structural = check_structural(current_opcode, add_units, sub_units, mul_units, div_units, ld_units, st_units) && check_waw(instruction_decoded) == 1)) { // All units full
            if (!instruction_in_fifo(instruction)) {
                add_to_fifo(instruction, inst_fifo);
            }
            clock += 1;
            continue;
        }
        issue(instruction_decoded, &add_units, &sub_units, &mul_units, &div_units, &ld_units, &st_units); // assuming that stalled ops wait for issue in fifo

        
        

        clock += 1;
        mem_index += 1;
    }

    write_memout();
    write_regout();
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
    for (int i = 0; i < scoreboard_params[0].amount; i++) { // add_units
        add_first(add_units, "add", scoreboard_params[0].delay, i);
    }
    for (int i = 0; i < scoreboard_params[1].amount; i++) { // sub_units
        add_first(sub_units, "sub", scoreboard_params[1].delay, i);
    }
    for (int i = 0; i < scoreboard_params[2].amount; i++) { // mul_units
        add_first(mul_units, "mul", scoreboard_params[2].delay, i);
    }
    for (int i = 0; i < scoreboard_params[3].amount; i++) { // div_units
        add_first(div_units, "div", scoreboard_params[3].delay, i);
    }
    for (int i = 0; i < scoreboard_params[4].amount; i++) { // ld_units
        add_first(ld_units, "ld", scoreboard_params[4].delay, i);
    }
    for (int i = 0; i < scoreboard_params[5].amount; i++) { // st_units
        add_first(st_units, "st", scoreboard_params[5].delay, i);
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
    FILE* fp_cfg, * fp_memin;

    // Read config file
    fp_cfg = fopen(argv[1], "r"); //argv[1]

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
    fp_memin = fopen(argv[2], "r"); //argv[2]
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

    for (int i = 0; i < 20; i++) {
        printf("Mem val: %s\n", MEMORY[i]);
    }



    run_scoreboard(inst_fifo, &add_units, &sub_units, &mul_units, &div_units, &ld_units, &st_units);

    free_units_list(add_units, sub_units, mul_units, div_units, ld_units, st_units);

    return 0;
}