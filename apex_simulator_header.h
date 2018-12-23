#ifndef APEX_SIMULATOR_NITESH_H
#define APEX_SIMULATOR_NITESH_H

// Predefined Libraries
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<stdbool.h>
#include<string.h>

// Constants used for simulator
#define ARCHI_REGISTER_LIMIT 16
#define DATA_MEMORY_LIMIT 4000
#define INPUT_INST_LIMIT 100
#define DATA_MEMORY_PRINT_LIMIT 25
#define BASE_ADDRESS 4000
#define BYTE_OFFSET 4
#define REG_STATUS_VALID "VALID"
#define REG_STATUS_INVALID "INVALID"
#define NUM_ZERO 0
#define OPCODE_NOP (-1)
#define INST_MOVC "MOVC"
#define OPCODE_MOVC 1
#define INST_STORE "STORE"
#define OPCODE_STORE 2
#define INST_LOAD "LOAD"
#define OPCODE_LOAD 3
#define INST_MUL "MUL"
#define OPCODE_MUL 4
#define INST_ADD "ADD"
#define OPCODE_ADD 5
#define INST_SUB "SUB"
#define OPCODE_SUB 6
#define INST_AND "AND"
#define OPCODE_AND 7
#define INST_OR "OR"
#define OPCODE_OR 8
#define INST_EXOR "EXOR"
#define OPCODE_EXOR 9
#define INST_JUMP "JUMP"
#define OPCODE_JUMP 10
#define INST_BNZ "BNZ"
#define OPCODE_BNZ 11
#define INST_BZ "BZ"
#define OPCODE_BZ 12
#define INST_HALT "HALT"
#define OPCODE_HALT 13
#define INST_DIV "DIV"
#define OPCODE_DIV 14
#define INST_JAL "JAL"
#define OPCODE_JAL 15
#define IQ_SIZE 16
#define PHY_REGISTER_LIMIT 32
#define ROB_SIZE 32
#define FWD_BUS_LIMIT 32
#define LSQ_SIZE 32
#define CFIO_SIZE 8

// Structure for Register Details
typedef struct _Register_Detail {
    int reg_index;
    int reg_value;
    char reg_status[10];
} Register_Detail;

// Structure for Architectural Register File
typedef struct _Architectural_Register_File {
    Register_Detail reg_R[ARCHI_REGISTER_LIMIT];
} A_Register_File;

// Structure for Forwarding Result Bus
typedef struct _Forwarding_Result_Bus {
    Register_Detail reg_P[PHY_REGISTER_LIMIT];
} Forwarding_Result_Bus;

// Structure for Instruction Contents from File
typedef struct _Instruction_File_Content {
    int file_line_index;
    int file_line_address;
    char file_line_content[50];
} I_Content;

// Structure for Instruction Cache Memory
typedef struct _Instruction_Cache_Memory {
    I_Content instructions[INPUT_INST_LIMIT];
} I_Cache;

// Structure for Instruction Details
typedef struct _Instruction_Detail {
    int pc_value;
    char i_content[50];
    char updated_i_content[50];
    int op_code;
    int inst_number;
    Register_Detail src_reg_detail[5];
    Register_Detail dest_reg_detail[5];
    Register_Detail bz_bnz_src_reg_detail;
    int computed_result;
    int literal_data;
    int target_memory_address;
    int target_memory_data;
    bool is_F_done;
    bool is_DRF_done;
    bool is_EX_done;
    bool is_MEM_done;
    bool is_WB_done;
    bool is_MUL_done;
    bool is_DIV1_done;
    bool is_DIV2_done;
    bool is_DIV3_done;
    int inst_IQ_index;
    int decoded_inst_num;
    int phy_reg_address;
    int rob_index;
    int lsq_index;
    int cfio_id;
} I_Detail;

// Structure for Pipeline Stages
typedef struct _Stage {
    I_Detail input_inst_detail[INPUT_INST_LIMIT];
    I_Detail output_inst_detail[INPUT_INST_LIMIT];
    int current_inst_number;
    int current_inst_number_MUL1;
    int current_inst_number_MUL2;
    int current_inst_number_DIV1;
    int current_inst_number_DIV2;
    int current_inst_number_DIV3;
    int current_inst_number_DIV4;
    int previous_inst_number;
    int previous_inst_number_MUL1;
    int previous_inst_number_MUL2;
    int previous_inst_number_DIV1;
    int previous_inst_number_DIV2;
    int previous_inst_number_DIV3;
    int previous_inst_number_DIV4;
    char current_inst_content[50];
    char current_inst_content_MUL1[50];
    char current_inst_content_MUL2[50];
    char current_inst_content_DIV1[50];
    char current_inst_content_DIV2[50];
    char current_inst_content_DIV3[50];
    char current_inst_content_DIV4[50];
    char updated_inst_content[50];
    char updated_inst_content_MUL1[50];
    char updated_inst_content_MUL2[50];
    char updated_inst_content_DIV1[50];
    char updated_inst_content_DIV2[50];
    char updated_inst_content_DIV3[50];
    char updated_inst_content_DIV4[50];
    bool stall;
} Stage;

// Structure for Data Memory Contents
typedef struct _Data_Content {
    int data_address;
    int data_value;
} D_Content;

// Structure for Data Cache Memory
typedef struct _Data_Cache_Memory {
    int base_address;
    D_Content data[4000];
} D_Cache;

// Structure for Pipeline Statistics
typedef struct _Pipeline_Statistics {
    int clk_cycle;
} P_Stats;

// Structure for Program Status Word (PSW)
typedef struct _Program_Status_Word {
    bool zero_flag_busy_state;
    int zero_flag_value;
} PSW_Flags;

// Structure for Issue Queue (IQ)
typedef struct _Issue_Queue {
    int iq_status;
    int op_code;
    int clk_cycle_dispatched;
    Register_Detail dest_reg;
    Register_Detail src1_reg;
    Register_Detail src2_reg;
    Register_Detail bz_bnz_src_reg_detail;
    int literal;
    int rob_index;
    int lsq_index;
    int cfio_id;
    I_Detail inst_detail;
    int current_IQ_inst_num;
    char current_IQ_content[50];
    char updated_IQ_content[50];
    bool inst_issued;
} Issue_Queue;

// Structure for Physical Register File
typedef struct _Physical_Register_File {
    int phy_reg_status;
    Register_Detail reg_P;
} P_Register_File;

// Structure for RAT (Register Alias Table)/ Rename Table
typedef struct _Register_Alias_Table {
    char reg_type; // A or P
    int reg_index;
} Reg_Alias_Table;

// Structure for ROB Entry Details
typedef struct _ROB_Entry {
    int rob_status;
    int pc_value;
    int dest_arch_reg_address;
    int dest_phy_reg_address;
    char previous_dest_reg_type;
    int previous_dest_reg_index;
    int ex_code;
    int result;
    int opcode;
    int cfio_id;
    int current_ROB_inst_num;
    char current_ROB_content[50];
    char updated_ROB_content[50];
} ROB_Entry;

// Structure for ROB Circular FIFO Queue
typedef struct _ROB_Circular_Queue {
    ROB_Entry rob_entry[ROB_SIZE];
    int rob_head;
    int rob_tail;
} ROB_Queue;

// Structure for LSQ Entry Details
typedef struct _LSQ_Entry {
    int lsq_status;
    int opcode;
    int mem_address_status;
    int mem_address_result;
    int dest_arch_reg_address;
    int dest_phy_reg_address;
    int src_status;
    int src_arch_reg_address;
    int src_phy_reg_address;
    int src_value;
    int rob_index;
    int cfio_id;
    bool is_load_bypassing_possible;
    bool is_load_bypassed;
    int current_LSQ_inst_num;
    char current_LSQ_content[50];
    char updated_LSQ_content[50];
} LSQ_Entry;

// Structure for LSQ Circular FIFO Queue
typedef struct _LSQ_Circular_Queue {
    LSQ_Entry lsq_entry[LSQ_SIZE];
    int lsq_head;
    int lsq_tail;
} LSQ_Queue;

// Structure for Control Flow Instruction Order Entry Details
typedef struct _CF_Inst_Order_Entry {
    int current_cfid;
    int next_cfid;
    I_Detail branch_inst_detail;
} CFIO_Entry;

// Structure for CF Instruction Order Circular FIFO Queue
typedef struct _CF_Inst_Order_Queue {
    CFIO_Entry cfio_entry[CFIO_SIZE];
    int cfio_head;
    int cfio_tail;
} CFIO_Queue;

// Structure to handle same cycle commit and flush of instructions
typedef struct _Commit_Registers_From_ROB {
    int arch_reg_address;
    int phy_reg_address;
    bool is_committed;
} Committed_Registers;

// Structure to hold LSQ traversed entries
typedef struct _LSQ_BYPASS_ENTRY {
    int opcode;
    int mem_address_status;
    int mem_address_result;
    int dest_phy_reg_address;
    int src_status;
    int src_phy_reg_address;
    int src_value;
    int lsq_index;
} Bypass_Entry;

// Structure to manage Load-Store Bypassing
typedef struct _Load_Bypass {
    bool is_load_in_lsq_valid;
    bool is_store_in_lsq_valid;
    int lsq_loads_count;
    int lsq_stores_count;
    Bypass_Entry bypass_entry[LSQ_SIZE];
} Load_Bypass;

// Global Structure Declarations
D_Cache data_list;
A_Register_File arch_reg_file;
I_Cache inst_cache;
P_Stats stats;
Stage f_stage;
Stage drf_stage;
Stage ex_stage;
Stage mem_stage;
Stage wb_stage;
PSW_Flags psw_flags;
Forwarding_Result_Bus result_bus;
Issue_Queue issue_queue[IQ_SIZE];
Issue_Queue issue_display_queue[IQ_SIZE];
P_Register_File phy_reg_file[PHY_REGISTER_LIMIT];
Reg_Alias_Table reg_alias_table[ARCHI_REGISTER_LIMIT];
ROB_Queue rob_queue;
ROB_Queue rob_display_queue;
LSQ_Queue lsq_queue;
LSQ_Queue lsq_display_queue;
CFIO_Queue cfio_queue;
CFIO_Queue cfio_display_queue;
Committed_Registers committed_registers[2];
Load_Bypass load_bypass;

// Global Variables
char file_full_path[200] = "\0";
int inst_count_in_file = 0;
int inst_to_fetch = 0;
int overall_inst_counter = 0;
bool is_branch_inst_found = false;
bool is_halt_inst_found = false;
int jump_to_pc = 0;
bool decode_rf_stalled = false;
int simulate_cycle = 0;
bool is_INT_empty = false;
bool is_INT_finished = false;
bool is_MUL2_finished = false;
bool is_DIV4_finished = false;
int drf_prev_loop_counter = 0;
int ex_prev_loop_counter = 0;
int is_JAL_pc_fetched = false;
int is_JUMP_pc_fetched = false;
int is_BZ_pc_fetched = false;
int is_BNZ_pc_fetched = false;
int retirement_rob_inst_num_1;
char retirement_rob_content_1[50];
char retirement_updated_rob_content_1[50];
int retirement_rob_inst_num_2;
char retirement_rob_content_2[50];
char retirement_updated_rob_content_2[50];
int committed_lsq_inst_num;
char committed_lsq_content[50];
char committed_updated_lsq_content[50];
int lsq_head_entry_cycle = 0;
int last_control_flow_inst = -1;
bool is_halt_retired_from_ROB = false;
Register_Detail psw_zero_flag_phy_reg;
Register_Detail psw_zero_flag_arch_reg;

// Function Declaration
I_Cache load_instructions_from_file();

int initialize_simulator();

int simulate_simulator();

int display_simulator(bool print_memory_state);

int fetch_F();

int decode_DRF();

int issue_queue_IQ();

int execute_EX();

int split_string(char input[], char *output[]);

int sub_string(char actual_string[], char new_string[], char sub_offset, int sub_length);

int int_FU_EX(int first_num, int second_num, int op_code);

bool mul_FU_EX1();

int mul_FU_EX2(int first_num, int second_num);

bool div_FU_EX1();

bool div_FU_EX2();

bool div_FU_EX3();

int div_FU_EX4(int first_num, int second_num);

int squash_instructions(I_Detail output_detail, int inst_num, int loop_counter);

bool exit_for_halt_instruction();

bool is_arithmetic_operation(int op_code);

int get_opcode_string(int op_code, char opcode_str[]);

bool is_ROB_queue_empty();

bool is_ROB_queue_full();

int enqueue_ROB_entry(ROB_Entry rob_entry);

int retirement_from_ROB();

bool is_LSQ_queue_empty();

bool is_LSQ_queue_full();

int enqueue_LSQ_entry(LSQ_Entry lsq_entry);

int commitment_from_LSQ();

bool is_CFIO_queue_full();

int enqueue_CFIO_entry(CFIO_Entry cfio_entry);

// Macro to print Architecture Register File
#define PRINT_ARCHI_REGISTER_FILE \
        printf(MAG"\n================================== STATE OF ARCHITECTURAL REGISTER FILE ==================================\n"RST); \
        printf("-----------------------------------------------------------------------------------------------------\n"); \
        for(int i = 0; i < ARCHI_REGISTER_LIMIT; i++){ \
            if(arch_reg_file.reg_R[i].reg_value < 10 && arch_reg_file.reg_R[i].reg_value > -10){ \
                printf("\t|\tREG[%02d] \t|\t Value = %02d \t|\t Status = %s \t|\n", i, \
                        arch_reg_file.reg_R[i].reg_value, \
                        arch_reg_file.reg_R[i].reg_status); \
            } else { \
                printf("\t|\tREG[%02d] \t|\t Value = %d \t|\t Status = %s \t|\n", i, \
                        arch_reg_file.reg_R[i].reg_value, \
                        arch_reg_file.reg_R[i].reg_status); \
            } \
        } \
        printf("-----------------------------------------------------------------------------------------------------\n"); \
        printf(MAG"==========================================================================================================\n"RST); \

// Macro to print Data Memory
#define PRINT_DATA_MEMORY \
        printf(MAG"\n\n\n========================================= STATE OF DATA MEMORY ============================================\n"RST); \
        printf("--------------------------------------------------------------------------------------------\n"); \
        for(int i = 0; i < DATA_MEMORY_PRINT_LIMIT; i++){ \
            if(data_list.data[i].data_value < 10 && data_list.data[i].data_value > -10){ \
                printf("\t|\tMEM[%02d] \t|\t Data Value = %02d \t|\n", i, \
                        data_list.data[i].data_value); \
            } else { \
                printf("\t|\tMEM[%02d] \t|\t Data Value = %d \t|\n", i, \
                        data_list.data[i].data_value); \
            } \
        } \
        printf("--------------------------------------------------------------------------------------------\n"); \
        printf(MAG"\n===========================================================================================================\n"RST); \

// Macro to initialize Default PC Values
#define INITIALIZE_STAGE_PC_VALUE \
        f_stage.input_inst_detail[i].pc_value = -1; \
        f_stage.output_inst_detail[i].pc_value = -1; \
        drf_stage.input_inst_detail[i].pc_value = -1; \
        drf_stage.output_inst_detail[i].pc_value = -1; \
        ex_stage.input_inst_detail[i].pc_value = -1; \
        ex_stage.output_inst_detail[i].pc_value = -1; \
        mem_stage.input_inst_detail[i].pc_value = -1; \
        mem_stage.output_inst_detail[i].pc_value = -1; \
        wb_stage.input_inst_detail[i].pc_value = -1; \
        wb_stage.output_inst_detail[i].pc_value = -1; \

// Macro to initialize Default OP Codes
#define INITIALIZE_STAGE_OP_CODE \
        f_stage.input_inst_detail[i].op_code = 0; \
        f_stage.output_inst_detail[i].op_code = 0; \
        drf_stage.input_inst_detail[i].op_code = 0; \
        drf_stage.output_inst_detail[i].op_code = 0; \
        ex_stage.input_inst_detail[i].op_code = 0; \
        ex_stage.output_inst_detail[i].op_code = 0; \
        mem_stage.input_inst_detail[i].op_code = 0; \
        mem_stage.output_inst_detail[i].op_code = 0; \
        wb_stage.input_inst_detail[i].op_code = 0; \
        wb_stage.output_inst_detail[i].op_code = 0; \

// Macro to Set Fetch Stage Input Buffer Flags
#define SET_F_STAGE_INPUT_INST_FLAGS \
        f_stage.input_inst_detail[inst_num].is_F_done = true; \
        f_stage.input_inst_detail[inst_num].is_DRF_done = false; \
        f_stage.input_inst_detail[inst_num].is_EX_done = false; \
        f_stage.input_inst_detail[inst_num].is_MEM_done = false; \
        f_stage.input_inst_detail[inst_num].is_WB_done = false;

// Macro to Set Fetch Stage Output Buffer Flags
#define SET_F_STAGE_OUTPUT_INST_FLAGS \
        f_stage.output_inst_detail[inst_num].is_F_done = true; \
        f_stage.output_inst_detail[inst_num].is_DRF_done = false; \
        f_stage.output_inst_detail[inst_num].is_EX_done = false; \
        f_stage.output_inst_detail[inst_num].is_MEM_done = false; \
        f_stage.output_inst_detail[inst_num].is_WB_done = false;

// Macro to Set Decode/ RF Stage Input Buffer Flags
#define SET_DRF_STAGE_INPUT_INST_FLAGS \
        drf_stage.input_inst_detail[inst_num].is_F_done = true; \
        drf_stage.input_inst_detail[inst_num].is_DRF_done = true; \
        drf_stage.input_inst_detail[inst_num].is_EX_done = false; \
        drf_stage.input_inst_detail[inst_num].is_MEM_done = false; \
        drf_stage.input_inst_detail[inst_num].is_WB_done = false;

// Macro to Set Decode/ RF Stage Output Buffer Flags
#define SET_DRF_STAGE_OUTPUT_INST_FLAGS \
        drf_stage.output_inst_detail[inst_num].is_F_done = true; \
        drf_stage.output_inst_detail[inst_num].is_DRF_done = true; \
        drf_stage.output_inst_detail[inst_num].is_EX_done = false; \
        drf_stage.output_inst_detail[inst_num].is_MEM_done = false; \
        drf_stage.output_inst_detail[inst_num].is_WB_done = false;

// Macro to Set Execute Stage Input Buffer Flags
#define SET_EX_STAGE_INPUT_INST_FLAGS \
        ex_stage.input_inst_detail[inst_num].is_F_done = true; \
        ex_stage.input_inst_detail[inst_num].is_DRF_done = true; \
        ex_stage.input_inst_detail[inst_num].is_EX_done = true; \
        ex_stage.input_inst_detail[inst_num].is_MEM_done = false; \
        ex_stage.input_inst_detail[inst_num].is_WB_done = false;

// Macro to Set Execute Stage Output Buffer Flags
#define SET_EX_STAGE_OUTPUT_INST_FLAGS \
        ex_stage.output_inst_detail[inst_num].is_F_done = true; \
        ex_stage.output_inst_detail[inst_num].is_DRF_done = true; \
        ex_stage.output_inst_detail[inst_num].is_EX_done = true; \
        ex_stage.output_inst_detail[inst_num].is_MEM_done = false; \
        ex_stage.output_inst_detail[inst_num].is_WB_done = false;

// Constants used to print console outputs in color format
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YLW "\x1B[33m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define RST "\x1B[0m"

#endif