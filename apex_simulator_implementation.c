// Including User Defined Header file which contains all Macros, Preprocessor Directive and Function Declarations
#include "apex_simulator_header.h"

/*
 This function will perform below activities:
 1. Takes input file path from user where instruction set is present for Pipeline Simulation
 2. Implementation of Initialization function to load Instruction Cache with the file instructions and other default initializations
 3. Implementation of Simulation function which performs operation in Pipelined Function Units with Dependencies and Forwarding
 */
int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    printf(RED"\n_______________________________________ START OF THE PROGRAM - PROJECT 3 | NITESH MISHRA ___________________________________________\n\n"RST);

    // Variable Declaration and Initialization
    int p_success = 0;

    if (argc != 2) {
        printf(YLW"WARNING ... Please pass correct number of parameters while executing the code"RST);
        printf(RED"\n\n_________________________________________END OF THE PROGRAM - PROJECT 3 | NITESH MISHRA _____________________________________________\n\n"RST);
        return 0;
    }

    strcpy(file_full_path, argv[1]);
    //strcpy(file_full_path, "/import/linux/home1/nmishra1/COA_CS520_Fall2017/Programming_Projects/Project_3/Sample_File.txt");

    p_success = initialize_simulator();

    if (p_success == 0) {

        printf("\nPlease provide Number of Cycles for Simulation --> ");
        scanf("%d", &simulate_cycle);

        if (simulate_cycle < 0) {
            printf(RED"\nIncorrect entry for cycles --- Please choose some positive integer ... !!\n"RST);
            printf("\n\nExiting from the program ... !!");
            printf(RED"\n\n_________________________________________END OF THE PROGRAM - PROJECT 3 | NITESH MISHRA _____________________________________________\n\n"RST);
            return 0;
        } else {
            printf(YLW"\n\n#--#--#--#--#--#--#--#--#--#--#--#--#--#-- SIMULATION START #--#--#--#--#--#--#--#--#--#--#--#--#--#"RST);

            // Simulation
            simulate_simulator();

            printf(YLW"\n\n#--#--#--#--#--#--#--#--#--#--#--#--#--#-- SIMULATION FINISHED #--#--#--#--#--#--#--#--#--#--#--#--#\n\n"RST);
        }

        // Display results
        display_simulator(true);
    }

    printf(RED"\n\n_________________________________________END OF THE PROGRAM - PROJECT 3 | NITESH MISHRA _____________________________________________\n\n"RST);
    return p_success;
}

/*
 This function loads Instruction Cache with File Contents and also initializes defaults values for structure elements
 */
int initialize_simulator() {

    // Variable Declaration and Initialization
    int i;

    // Initialize Data Cache
    data_list.base_address = 0;
    for (i = 0; i < DATA_MEMORY_LIMIT; i++) {
        data_list.data[i].data_address = i;
        data_list.data[i].data_value = 0;
    }

    // Initialize Architectural Register File
    for (i = 0; i < ARCHI_REGISTER_LIMIT; i++) {
        arch_reg_file.reg_R[i].reg_index = i;
        arch_reg_file.reg_R[i].reg_value = 0;
        strcpy(arch_reg_file.reg_R[i].reg_status, REG_STATUS_VALID);
    }

    // Initialize Pipeline Statistics
    stats.clk_cycle = 0;

    // Load instructions from input file - Instruction Cache
    inst_cache = load_instructions_from_file();
    if (inst_count_in_file == -1) {
        return 1;
    }

    // Initialize Stage PC values and Op Codes
    for (i = 0; i < INPUT_INST_LIMIT; i++) {
        INITIALIZE_STAGE_PC_VALUE;
        INITIALIZE_STAGE_OP_CODE;
    }
    f_stage.stall = true;
    drf_stage.stall = true;
    ex_stage.stall = true;
    mem_stage.stall = true;
    wb_stage.stall = true;
    f_stage.previous_inst_number = -1;
    drf_stage.previous_inst_number = -1;
    ex_stage.previous_inst_number = -1;
    ex_stage.previous_inst_number_MUL1 = -1;
    ex_stage.previous_inst_number_MUL2 = -1;
    ex_stage.previous_inst_number_DIV1 = -1;
    ex_stage.previous_inst_number_DIV2 = -1;
    ex_stage.previous_inst_number_DIV3 = -1;
    ex_stage.previous_inst_number_DIV4 = -1;
    mem_stage.previous_inst_number = -1;
    wb_stage.previous_inst_number = -1;

    // Initializing Zero Flag with false (i.e. computed value is not zero)
    psw_flags.zero_flag_busy_state = false;
    psw_flags.zero_flag_value = 0;

    // Initialization of Issue Queue Entries
    for (i = 0; i < IQ_SIZE; i++) {
        issue_queue[i].iq_status = 0; // Free
        issue_queue[i].clk_cycle_dispatched = 0;
        issue_queue[i].cfio_id = -10;
        strcpy(issue_queue[i].current_IQ_content, "");
        strcpy(issue_queue[i].updated_IQ_content, "");

        issue_display_queue[i].iq_status = 0; // Free
        issue_display_queue[i].clk_cycle_dispatched = 0;
        strcpy(issue_display_queue[i].current_IQ_content, "");
        strcpy(issue_display_queue[i].updated_IQ_content, "");
    }

    // Initialization of Forwarding Result Bus
    for (i = 0; i < FWD_BUS_LIMIT; i++) {
        result_bus.reg_P[i].reg_index = i;
        result_bus.reg_P[i].reg_value = 0;
        strcpy(result_bus.reg_P[i].reg_status, REG_STATUS_INVALID);
    }

    // Initialization of Physical Register File
    for (i = 0; i < PHY_REGISTER_LIMIT; i++) {
        phy_reg_file[i].phy_reg_status = 0; // Free
        phy_reg_file[i].reg_P.reg_index = i;
        phy_reg_file[i].reg_P.reg_value = 0;
        strcpy(phy_reg_file[i].reg_P.reg_status, REG_STATUS_VALID);
    }

    // Initialization of Register Alias Table (Rename Table)
    for (i = 0; i < ARCHI_REGISTER_LIMIT; i++) {
        reg_alias_table[i].reg_type = 'R';
        reg_alias_table[i].reg_index = i;
    }

    // Initialization of ROB Queue
    rob_queue.rob_head = -1;
    rob_queue.rob_tail = -1;
    for (i = 0; i < ROB_SIZE; i++) {
        rob_queue.rob_entry[i].rob_status = -1;
        strcpy(rob_queue.rob_entry[i].current_ROB_content, "");
        strcpy(rob_queue.rob_entry[i].updated_ROB_content, "");
    }

    // Initialization of LSQ Queue
    lsq_queue.lsq_head = -1;
    lsq_queue.lsq_tail = -1;
    for (i = 0; i < LSQ_SIZE; i++) {
        lsq_queue.lsq_entry[i].lsq_status = -1;
        lsq_queue.lsq_entry[i].src_status = -1;
        lsq_queue.lsq_entry[i].mem_address_status = -1;
        lsq_queue.lsq_entry[i].cfio_id = -10;
        lsq_queue.lsq_entry[i].is_load_bypassed = false;
        lsq_queue.lsq_entry[i].is_load_bypassing_possible = false;
        strcpy(lsq_queue.lsq_entry[i].current_LSQ_content, "");
        strcpy(lsq_queue.lsq_entry[i].updated_LSQ_content, "");
    }

    // Initialization of CFIO Queue
    cfio_queue.cfio_head = -1;
    cfio_queue.cfio_tail = -1;
    for (i = 0; i < CFIO_SIZE; i++) {
        cfio_queue.cfio_entry[i].current_cfid = -10;
        cfio_queue.cfio_entry[i].next_cfid = -10;
        cfio_queue.cfio_entry[i].branch_inst_detail.pc_value = -10;
    }

    // Initialize Physical and Architectural Register required for PSW Zero Flag
    psw_zero_flag_arch_reg.reg_index = -1;
    strcpy(psw_zero_flag_arch_reg.reg_status, REG_STATUS_INVALID);
    psw_zero_flag_phy_reg.reg_index = -1;
    strcpy(psw_zero_flag_phy_reg.reg_status, REG_STATUS_INVALID);

    // Initialize details of Committed Registers from ROB
    committed_registers[0].is_committed = false;
    committed_registers[0].arch_reg_address = -1;
    committed_registers[0].phy_reg_address = -1;
    committed_registers[1].is_committed = false;
    committed_registers[1].arch_reg_address = -1;
    committed_registers[1].phy_reg_address = -1;

    // Initialize Load Bypass Structure
    load_bypass.is_load_in_lsq_valid = false;
    load_bypass.is_store_in_lsq_valid = false;
    load_bypass.lsq_loads_count = 0;
    load_bypass.lsq_stores_count = 0;
    for (i = 0; i < LSQ_SIZE; i++) {
        load_bypass.bypass_entry[i].opcode = -10;
        load_bypass.bypass_entry[i].src_status = -1;
        load_bypass.bypass_entry[i].mem_address_status = -1;
    }

    return 0;
}

/*
 This function performs Pipeline Simulation using basic stages (F, DRF, EX, MEM, WB)
 */
int simulate_simulator() {

    // Fetch Stage Stall flag initialization before simulation starts
    f_stage.stall = false;

    // Looping with the condition for encountering HALT instruction or according to provided Simulation Cycles by user
    while (!exit_for_halt_instruction() && simulate_cycle != stats.clk_cycle) {

        printf(RED"\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ CLOCK CYCLE %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"RST,
               stats.clk_cycle + 1);

        // Code for debugging purpose
        if (stats.clk_cycle == 8) {
            int test = 0;
        }

        // Function Calls of Stages of Pipeline
        retirement_from_ROB();
        commitment_from_LSQ();
        execute_EX();
        issue_queue_IQ();
        decode_DRF();
        fetch_F();

        // Displaying instructions in each stage during current clock cycle
        display_simulator(false);
        printf(RED"\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"RST);

        // Conditions to maintain branching and stall operations
        if (!is_branch_inst_found) {
            overall_inst_counter++;
        }
        if (is_branch_inst_found && f_stage.stall && drf_stage.stall) {
            f_stage.stall = false;
            drf_stage.stall = false;
            is_branch_inst_found = false;
            overall_inst_counter++;
        }

        // Increase the Clock Cycle
        stats.clk_cycle++;
    }

    return 0;
}

/*
 This function displays the Instructions present in each stage and state of Architecture File + Data Memory
 */
int display_simulator(bool print_memory_state) {

    // Variable Declaration and Initialization
    int i;

    // Prints Stage Instructions not the state of Architectural Register File and Data Memory
    if (!print_memory_state) {
        printf("\n");

        // Fetch Stage Instruction - Printing on standard console
        if (f_stage.current_inst_number != -1 && strcmp(f_stage.current_inst_content, "") != 0) {
            /*if (f_stage.previous_inst_number == f_stage.current_inst_number) {
                printf(CYN"01. Instruction at FETCH______STAGE ---> \t (I%d) "RST"[Stalled] %s\n",
                       f_stage.current_inst_number, f_stage.current_inst_content);
            } else {
                printf(CYN"01. Instruction at FETCH______STAGE ---> \t (I%d) "RST"%s\n",
                       f_stage.current_inst_number, f_stage.current_inst_content);
            }*/
            printf(CYN"01. Instruction at FETCH______STAGE ---> \t (I%d) "RST"%s\n",
                   f_stage.current_inst_number, f_stage.current_inst_content);
            f_stage.previous_inst_number = f_stage.current_inst_number;
        } else {
            printf(CYN"01. Instruction at FETCH______STAGE ---> \t "RST"EMPTY\n");
        }

        // Decode/ RF Stage Instruction - Printing on standard console
        if (drf_stage.current_inst_number != -1 && strcmp(drf_stage.current_inst_content, "") != 0) {
            /*if (drf_stage.previous_inst_number == drf_stage.current_inst_number) {
                printf(CYN"02. Instruction at DECODE_RF__STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       drf_stage.current_inst_number, drf_stage.current_inst_content, drf_stage.updated_inst_content);
            } else {
                printf(CYN"02. Instruction at DECODE_RF__STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       drf_stage.current_inst_number, drf_stage.current_inst_content, drf_stage.updated_inst_content);
            }*/
            printf(CYN"02. Instruction at DECODE_RF__STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   drf_stage.current_inst_number, drf_stage.current_inst_content, drf_stage.updated_inst_content);
            drf_stage.previous_inst_number = drf_stage.current_inst_number;
        } else {
            printf(CYN"02. Instruction at DECODE_RF__STAGE ---> \t "RST"EMPTY\n");
        }

        // Register Alias Table (Rename Table)
        printf(YLW"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of RENAME TABLE (RAT) State --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        for (i = 0; i < ARCHI_REGISTER_LIMIT; i++) {
            /*if (reg_alias_table[i].reg_type == 'R') {
                printf(YLW"RAT[%02d] -->\t%c%d  \t||\t", i, reg_alias_table[i].reg_type, reg_alias_table[i].reg_index);
            } else if (reg_alias_table[i].reg_type == 'P') {
                printf(YLW"RAT[%02d] -->\t"RST"%c%d  \t||\t"RST, i, reg_alias_table[i].reg_type, reg_alias_table[i].reg_index);
            }
            if (i % 3 == 0) {
                printf("\n");
            }*/
            if (reg_alias_table[i].reg_type == 'P') {
                printf(YLW"RAT[%02d] -->\t"RST"%c%d \n"RST, i, reg_alias_table[i].reg_type, reg_alias_table[i].reg_index);
            }
        }
        printf(YLW"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);

        // LSQ Instructions
        printf(GRN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of LSQ (Load-Store Queue) State --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);
        for (i = 0; i < LSQ_SIZE; i++) {
            if (lsq_display_queue.lsq_entry[i].lsq_status != -1) {
                printf(GRN"LSQ[%02d] \t-->\t\t"RST"(I%d) %s\t[%s]\n"RST, i, lsq_display_queue.lsq_entry[i].current_LSQ_inst_num,
                       lsq_display_queue.lsq_entry[i].current_LSQ_content, lsq_display_queue.lsq_entry[i].updated_LSQ_content);
            }
        }
        printf(GRN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);

        // CFIO Instructions
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of CFIO (Control Flow Instruction Order) State --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        I_Detail display_I_detail;
        for (i = 0; i < CFIO_SIZE; i++) {
            display_I_detail = cfio_display_queue.cfio_entry[i].branch_inst_detail;
            if (cfio_display_queue.cfio_entry[i].next_cfid != -10) {
                printf("CFIO[%02d] \t-->\t\t(I%d) %s\t[%s]\n", i, display_I_detail.inst_number, display_I_detail.i_content,
                       display_I_detail.updated_i_content);
            }
        }
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

        // ROB Instructions
        printf(CYN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of ROB (Reorder Buffer) State --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);
        for (i = 0; i < ROB_SIZE; i++) {
            if (rob_display_queue.rob_entry[i].rob_status != -1) {
                printf(CYN"ROB[%02d] \t-->\t\t"RST"(I%d) %s\t[%s]\n"RST, i, rob_display_queue.rob_entry[i].current_ROB_inst_num,
                       rob_display_queue.rob_entry[i].current_ROB_content, rob_display_queue.rob_entry[i].updated_ROB_content);
            }
        }
        printf(CYN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);

        // Issue Queue Instructions
        printf(MAG"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of IQ (ISSUE QUEUE) State --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        for (int p = 0; p < IQ_SIZE; p++) {
            if (issue_display_queue[p].iq_status == 1) {
                printf(MAG"IQ[%02d]  \t-->\t\t"RST"(I%d) %s\t[%s]\n"RST, p, issue_display_queue[p].current_IQ_inst_num,
                       issue_display_queue[p].current_IQ_content, issue_display_queue[p].updated_IQ_content);
            }
        }
        printf(MAG"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);

        // Execute Stage (INT FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number != -1 && strcmp(ex_stage.current_inst_content, "") != 0) {
            /*if (ex_stage.previous_inst_number == ex_stage.current_inst_number) {
                printf(CYN"03. Instruction at EX_INT_FU__STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number, ex_stage.current_inst_content, ex_stage.updated_inst_content);
            } else {
                printf(CYN"03. Instruction at EX_INT_FU__STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number, ex_stage.current_inst_content, ex_stage.updated_inst_content);
            }*/
            printf(CYN"03. Instruction at EX_INT_FU__STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number, ex_stage.current_inst_content, ex_stage.updated_inst_content);
            ex_stage.previous_inst_number = ex_stage.current_inst_number;
        } else {
            printf(CYN"03. Instruction at EX_INT_FU__STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (MUL1 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_MUL1 != -1 && strcmp(ex_stage.current_inst_content_MUL1, "") != 0) {
            /*if (ex_stage.previous_inst_number_MUL1 == ex_stage.current_inst_number_MUL1) {
                printf(CYN"04. Instruction at EX_MUL1_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_MUL1, ex_stage.current_inst_content_MUL1, ex_stage.updated_inst_content_MUL1);
            } else {
                printf(CYN"04. Instruction at EX_MUL1_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_MUL1, ex_stage.current_inst_content_MUL1, ex_stage.updated_inst_content_MUL1);
            }*/
            printf(CYN"04. Instruction at EX_MUL1_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_MUL1, ex_stage.current_inst_content_MUL1, ex_stage.updated_inst_content_MUL1);
            ex_stage.previous_inst_number_MUL1 = ex_stage.current_inst_number_MUL1;
        } else {
            printf(CYN"04. Instruction at EX_MUL1_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (MUL2 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_MUL2 != -1 && strcmp(ex_stage.current_inst_content_MUL2, "") != 0) {
            /*if (ex_stage.previous_inst_number_MUL2 == ex_stage.current_inst_number_MUL2) {
                printf(CYN"05. Instruction at EX_MUL2_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_MUL2, ex_stage.current_inst_content_MUL2, ex_stage.updated_inst_content_MUL2);
            } else {
                printf(CYN"05. Instruction at EX_MUL2_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_MUL2, ex_stage.current_inst_content_MUL2, ex_stage.updated_inst_content_MUL2);
            }*/
            printf(CYN"05. Instruction at EX_MUL2_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_MUL2, ex_stage.current_inst_content_MUL2, ex_stage.updated_inst_content_MUL2);
            ex_stage.previous_inst_number_MUL2 = ex_stage.current_inst_number_MUL2;
        } else {
            printf(CYN"05. Instruction at EX_MUL2_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (DIV1 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_DIV1 != -1 && strcmp(ex_stage.current_inst_content_DIV1, "") != 0) {
            /*if (ex_stage.previous_inst_number_DIV1 == ex_stage.current_inst_number_DIV1) {
                printf(CYN"06. Instruction at EX_DIV1_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_DIV1, ex_stage.current_inst_content_DIV1, ex_stage.updated_inst_content_DIV1);
            } else {
                printf(CYN"06. Instruction at EX_DIV1_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_DIV1, ex_stage.current_inst_content_DIV1, ex_stage.updated_inst_content_DIV1);
            }*/
            printf(CYN"06. Instruction at EX_DIV1_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_DIV1, ex_stage.current_inst_content_DIV1, ex_stage.updated_inst_content_DIV1);
            ex_stage.previous_inst_number_DIV1 = ex_stage.current_inst_number_DIV1;
        } else {
            printf(CYN"06. Instruction at EX_DIV1_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (DIV2 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_DIV2 != -1 && strcmp(ex_stage.current_inst_content_DIV2, "") != 0) {
            /*if (ex_stage.previous_inst_number_DIV2 == ex_stage.current_inst_number_DIV2) {
                printf(CYN"07. Instruction at EX_DIV2_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_DIV2, ex_stage.current_inst_content_DIV2, ex_stage.updated_inst_content_DIV2);
            } else {
                printf(CYN"07. Instruction at EX_DIV2_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_DIV2, ex_stage.current_inst_content_DIV2, ex_stage.updated_inst_content_DIV2);
            }*/
            printf(CYN"07. Instruction at EX_DIV2_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_DIV2, ex_stage.current_inst_content_DIV2, ex_stage.updated_inst_content_DIV2);
            ex_stage.previous_inst_number_DIV2 = ex_stage.current_inst_number_DIV2;
        } else {
            printf(CYN"07. Instruction at EX_DIV2_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (DIV3 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_DIV3 != -1 && strcmp(ex_stage.current_inst_content_DIV3, "") != 0) {
            /*if (ex_stage.previous_inst_number_DIV3 == ex_stage.current_inst_number_DIV3) {
                printf(CYN"08. Instruction at EX_DIV3_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_DIV3, ex_stage.current_inst_content_DIV3, ex_stage.updated_inst_content_DIV3);
            } else {
                printf(CYN"08. Instruction at EX_DIV3_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_DIV3, ex_stage.current_inst_content_DIV3, ex_stage.updated_inst_content_DIV3);
            }*/
            printf(CYN"08. Instruction at EX_DIV3_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_DIV3, ex_stage.current_inst_content_DIV3, ex_stage.updated_inst_content_DIV3);
            ex_stage.previous_inst_number_DIV3 = ex_stage.current_inst_number_DIV3;
        } else {
            printf(CYN"08. Instruction at EX_DIV3_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // Execute Stage (DIV4 FU) Instruction - Printing on standard console
        if (ex_stage.current_inst_number_DIV4 != -1 && strcmp(ex_stage.current_inst_content_DIV4, "") != 0) {
            /*if (ex_stage.previous_inst_number_DIV4 == ex_stage.current_inst_number_DIV4) {
                printf(CYN"09. Instruction at EX_DIV4_FU_STAGE ---> \t (I%d) "RST"[Stalled] %s\t[%s]\n",
                       ex_stage.current_inst_number_DIV4, ex_stage.current_inst_content_DIV4, ex_stage.updated_inst_content_DIV4);
            } else {
                printf(CYN"09. Instruction at EX_DIV4_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                       ex_stage.current_inst_number_DIV4, ex_stage.current_inst_content_DIV4, ex_stage.updated_inst_content_DIV4);
            }*/
            printf(CYN"09. Instruction at EX_DIV4_FU_STAGE ---> \t (I%d) "RST"%s\t[%s]\n",
                   ex_stage.current_inst_number_DIV4, ex_stage.current_inst_content_DIV4, ex_stage.updated_inst_content_DIV4);
            ex_stage.previous_inst_number_DIV4 = ex_stage.current_inst_number_DIV4;
        } else {
            printf(CYN"09. Instruction at EX_DIV4_FU_STAGE ---> \t "RST"EMPTY\n");
        }

        // LSQ Committed Instruction - Printing on standard console
        printf(GRN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of LSQ Committed Instruction --::>> \n");
        printf(GRN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);
        if (strcmp(committed_lsq_content, "") != 0) {
            if (lsq_head_entry_cycle != 0) {
                printf(RST"(CYCLE: %d) --> (I%d) %s\t[%s]\n", lsq_head_entry_cycle, committed_lsq_inst_num, committed_lsq_content,
                       committed_updated_lsq_content);
            } else {
                printf(RST"(CYCLE: 3) --> (I%d) %s\t[%s]\n", committed_lsq_inst_num, committed_lsq_content, committed_updated_lsq_content);
            }
        }
        printf(GRN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);

        // ROB Retired Instructions - Printing on standard console
        printf(CYN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Details of ROB Retired Instructions --::>> \n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"RST);
        if (strcmp(retirement_rob_content_1, "") != 0) {
            printf(RST"(I%d) %s\t[%s]\n", retirement_rob_inst_num_1, retirement_rob_content_1, retirement_updated_rob_content_1);
        }
        if (strcmp(retirement_rob_content_2, "") != 0) {
            printf(RST"(I%d) %s\t[%s]\n", retirement_rob_inst_num_2, retirement_rob_content_2, retirement_updated_rob_content_2);
        }
        printf(CYN"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"RST);
    }

    // Printing state of Architecture Register File and Data Memory
    if (print_memory_state) {
        PRINT_ARCHI_REGISTER_FILE;
        PRINT_DATA_MEMORY;
    }

    return 0;
}

/*
 Implementation of Instruction Commitment in ARF in Program Order from ROB head
 */
int retirement_from_ROB() {

    // Clearing ROB printing current instruction content
    strcpy(retirement_rob_content_1, "");
    strcpy(retirement_updated_rob_content_1, "");
    strcpy(retirement_rob_content_2, "");
    strcpy(retirement_updated_rob_content_2, "");

    // Variable Declaration and Initialization
    int i, head, tail, phy_reg_index, arch_reg_index, ret_inst_cnt = 0;
    ROB_Entry rob_head_entry;

    // For display purpose
    rob_display_queue = rob_queue;
    cfio_display_queue = cfio_queue;

    if (is_halt_inst_found) {
        f_stage.stall = true;
        drf_stage.stall = true;
    }

    // Reset committed registers
    committed_registers[0].is_committed = false;
    committed_registers[0].arch_reg_address = -1;
    committed_registers[0].phy_reg_address = -1;
    committed_registers[1].is_committed = false;
    committed_registers[1].arch_reg_address = -1;
    committed_registers[1].phy_reg_address = -1;

    for (i = 0; i < 2; i++) {

        // Check the condition of empty ROB queue
        if (is_ROB_queue_empty()) {
            return 0;
        }

        // Local Variables Initialization
        head = rob_queue.rob_head;
        tail = rob_queue.rob_tail;
        rob_head_entry = rob_queue.rob_entry[head];
        phy_reg_index = rob_head_entry.dest_phy_reg_address;
        arch_reg_index = rob_head_entry.dest_arch_reg_address;
        ret_inst_cnt++;

        // Condition for retirement - Status valid and exception code 0
        if ((rob_head_entry.rob_status == 0 && rob_head_entry.ex_code == 0) || rob_head_entry.opcode == OPCODE_HALT) {

            if (rob_head_entry.opcode == OPCODE_HALT) {
                is_halt_retired_from_ROB = true;
            } else {

                // Instruction which is having Destination Registers
                if (rob_head_entry.opcode != OPCODE_STORE && rob_head_entry.opcode != OPCODE_JUMP &&
                    rob_head_entry.opcode != OPCODE_BNZ && rob_head_entry.opcode != OPCODE_BZ) {

                    // Commit the value of Physical Register to ARF
                    if (phy_reg_index >= 0) {
                        arch_reg_file.reg_R[arch_reg_index].reg_value = rob_head_entry.result;
                        strcpy(arch_reg_file.reg_R[arch_reg_index].reg_status, REG_STATUS_VALID);
                    }

                    // Update committed register details
                    if (ret_inst_cnt == 1) {
                        committed_registers[0].is_committed = true;
                        committed_registers[0].phy_reg_address = phy_reg_index;
                        committed_registers[0].arch_reg_address = arch_reg_index;
                    } else if (ret_inst_cnt == 2) {
                        committed_registers[1].is_committed = true;
                        committed_registers[1].phy_reg_address = phy_reg_index;
                        committed_registers[1].arch_reg_address = arch_reg_index;
                    }

                    // Updating most recent instance as Arch Reg in RAT
                    if (reg_alias_table[arch_reg_index].reg_type == 'P' && reg_alias_table[arch_reg_index].reg_index == phy_reg_index) {
                        reg_alias_table[arch_reg_index].reg_type = 'R';
                        reg_alias_table[arch_reg_index].reg_index = arch_reg_index;

                        // Free the physical register as it is not the most recent instance anymore
                        phy_reg_file[phy_reg_index].phy_reg_status = 0;
                    }
                }

                // Clear respective CFIO entry for Branch instructions
                int cfio_id = rob_head_entry.cfio_id;
                if (rob_head_entry.opcode == OPCODE_JUMP || rob_head_entry.opcode == OPCODE_JAL ||
                    rob_head_entry.opcode == OPCODE_BNZ || rob_head_entry.opcode == OPCODE_BZ) {
                    for (int p = 0; p < CFIO_SIZE; p++) {
                        if (cfio_queue.cfio_entry[p].current_cfid == cfio_id) {

                            cfio_queue.cfio_entry[p].current_cfid = -10;
                            cfio_queue.cfio_entry[p].next_cfid = -10;
                            strcpy(cfio_queue.cfio_entry[p].branch_inst_detail.i_content, "");
                            strcpy(cfio_queue.cfio_entry[p].branch_inst_detail.updated_i_content, "");
                            cfio_display_queue.cfio_entry[p].current_cfid = -10;
                            cfio_display_queue.cfio_entry[p].next_cfid = -10;
                            strcpy(cfio_display_queue.cfio_entry[p].branch_inst_detail.i_content, "");
                            strcpy(cfio_display_queue.cfio_entry[p].branch_inst_detail.updated_i_content, "");

                            // Modifying CFIO head pointer
                            if (cfio_queue.cfio_head == cfio_queue.cfio_tail) {
                                cfio_queue.cfio_head = -1;
                                cfio_queue.cfio_tail = -1;
                                last_control_flow_inst = -1;
                            } else {
                                cfio_queue.cfio_head += 1;
                            }
                        }
                    }
                }
            }

            // For display purpose
            if (ret_inst_cnt == 1) {
                retirement_rob_inst_num_1 = rob_head_entry.current_ROB_inst_num;
                strcpy(retirement_rob_content_1, rob_queue.rob_entry[head].current_ROB_content);
                strcpy(retirement_updated_rob_content_1, rob_queue.rob_entry[head].updated_ROB_content);
            } else if (ret_inst_cnt == 2) {
                retirement_rob_inst_num_2 = rob_head_entry.current_ROB_inst_num;
                strcpy(retirement_rob_content_2, rob_queue.rob_entry[head].current_ROB_content);
                strcpy(retirement_updated_rob_content_2, rob_queue.rob_entry[head].updated_ROB_content);
            }

            // Clear the ROB contents
            rob_queue.rob_entry[head].rob_status = -1;
            strcpy(rob_queue.rob_entry[head].current_ROB_content, "");
            strcpy(rob_queue.rob_entry[head].updated_ROB_content, "");
            rob_display_queue.rob_entry[head].rob_status = -1;
            strcpy(rob_display_queue.rob_entry[head].current_ROB_content, "");
            strcpy(rob_display_queue.rob_entry[head].updated_ROB_content, "");

            // Modifying ROB head pointer
            if (head == tail) {
                rob_queue.rob_head = -1;
                rob_queue.rob_tail = -1;
            } else {
                rob_queue.rob_head += 1;
            }
        }
    }

    return 0;
}

/*
 Implementation of Instruction Commitment in Memory in Program Order from LSQ head
 */
int commitment_from_LSQ() {

    // Clearing LSQ printing current instruction content
    strcpy(committed_lsq_content, "");
    strcpy(committed_updated_lsq_content, "");

    is_JUMP_pc_fetched = false;
    is_JAL_pc_fetched = false;
    is_BNZ_pc_fetched = false;
    is_BZ_pc_fetched = false;

    // Check for bypassing conditions
    if (lsq_queue.lsq_entry[lsq_queue.lsq_head].is_load_bypassed && lsq_queue.lsq_entry[lsq_queue.lsq_head].is_load_bypassing_possible) {

        // Clear the LSQ contents
        lsq_queue.lsq_entry[lsq_queue.lsq_head].lsq_status = -1;
        strcpy(lsq_queue.lsq_entry[lsq_queue.lsq_head].current_LSQ_content, "");
        strcpy(lsq_queue.lsq_entry[lsq_queue.lsq_head].updated_LSQ_content, "");
        lsq_display_queue.lsq_entry[lsq_queue.lsq_head].lsq_status = -1;
        strcpy(lsq_display_queue.lsq_entry[lsq_queue.lsq_head].current_LSQ_content, "");
        strcpy(lsq_display_queue.lsq_entry[lsq_queue.lsq_head].updated_LSQ_content, "");

        lsq_queue.lsq_entry[lsq_queue.lsq_head].is_load_bypassing_possible = false;
        lsq_queue.lsq_entry[lsq_queue.lsq_head].is_load_bypassed = false;
        lsq_queue.lsq_entry[lsq_queue.lsq_head].mem_address_status = -1;
        lsq_queue.lsq_entry[lsq_queue.lsq_head].src_status = -1;

        // Modifying LSQ head pointer
        if (lsq_queue.lsq_head == lsq_queue.lsq_tail) {
            lsq_queue.lsq_head = -1;
            lsq_queue.lsq_tail = -1;
        } else {
            lsq_queue.lsq_head += 1;
        }

        commitment_from_LSQ();
        return 0;
    }

    // Variable Declaration and Initialization
    int head, tail, dest_phy_reg_index, dest_arch_reg_index, src_phy_reg_index, src_arch_reg_index;
    int mem_address, rob_index, mem_data;
    LSQ_Entry lsq_head_entry;

    // For display purpose
    lsq_display_queue = lsq_queue;

    // Check the condition of empty ROB queue
    if (is_LSQ_queue_empty()) {
        return 0;
    }

    // Local Variables Initialization
    head = lsq_queue.lsq_head;
    tail = lsq_queue.lsq_tail;
    lsq_head_entry = lsq_queue.lsq_entry[head];
    dest_phy_reg_index = lsq_head_entry.dest_phy_reg_address;
    dest_arch_reg_index = lsq_head_entry.dest_arch_reg_address;
    src_phy_reg_index = lsq_head_entry.src_phy_reg_address;
    src_arch_reg_index = lsq_head_entry.src_arch_reg_address;
    rob_index = lsq_head_entry.rob_index;

    // Condition for commitment from LSQ - Valid memory address in LSQ Head Entry
    if (lsq_head_entry.mem_address_status == 0) {

        mem_address = lsq_head_entry.mem_address_result;

        // Implementation for STORE
        if (lsq_head_entry.opcode == OPCODE_STORE && lsq_head_entry.src_status == 0 &&
            (rob_queue.rob_head == lsq_head_entry.rob_index || lsq_head_entry_cycle > 0)) {
            lsq_head_entry_cycle++;

            // For display purpose
            committed_lsq_inst_num = lsq_head_entry.current_LSQ_inst_num;
            strcpy(committed_lsq_content, lsq_queue.lsq_entry[head].current_LSQ_content);
            strcpy(committed_updated_lsq_content, lsq_queue.lsq_entry[head].updated_LSQ_content);

            // Clear the respective ROB entry of STORE
            if (lsq_head_entry_cycle == 1) {
                rob_queue.rob_entry[rob_queue.rob_head].rob_status = -1;
                strcpy(rob_queue.rob_entry[rob_queue.rob_head].current_ROB_content, "");
                strcpy(rob_queue.rob_entry[rob_queue.rob_head].updated_ROB_content, "");
                rob_display_queue.rob_entry[rob_queue.rob_head].rob_status = -1;
                strcpy(rob_display_queue.rob_entry[rob_queue.rob_head].current_ROB_content, "");
                strcpy(rob_display_queue.rob_entry[rob_queue.rob_head].updated_ROB_content, "");

                // Modifying ROB head pointer
                if (rob_queue.rob_head == rob_queue.rob_tail) {
                    rob_queue.rob_head = -1;
                    rob_queue.rob_tail = -1;
                } else {
                    rob_queue.rob_head += 1;
                }
            }

            // Perform memory operation in 3 clock cycles
            if (lsq_head_entry_cycle == 3) {
                lsq_head_entry_cycle = 0;

                // Copy data to Memory/ D-Cache
                data_list.data[mem_address].data_value = lsq_head_entry.src_value;

                // Clear the LSQ contents
                lsq_queue.lsq_entry[head].lsq_status = -1;
                strcpy(lsq_queue.lsq_entry[head].current_LSQ_content, "");
                strcpy(lsq_queue.lsq_entry[head].updated_LSQ_content, "");
                lsq_display_queue.lsq_entry[head].lsq_status = -1;
                strcpy(lsq_display_queue.lsq_entry[head].current_LSQ_content, "");
                strcpy(lsq_display_queue.lsq_entry[head].updated_LSQ_content, "");

                // Modifying LSQ head pointer
                if (head == tail) {
                    lsq_queue.lsq_head = -1;
                    lsq_queue.lsq_tail = -1;
                } else {
                    lsq_queue.lsq_head += 1;
                }
            }
        }

        // Implementation for LOAD
        if (lsq_head_entry.opcode == OPCODE_LOAD) {
            lsq_head_entry_cycle++;

            // For display purpose
            committed_lsq_inst_num = lsq_head_entry.current_LSQ_inst_num;
            strcpy(committed_lsq_content, lsq_queue.lsq_entry[head].current_LSQ_content);
            strcpy(committed_updated_lsq_content, lsq_queue.lsq_entry[head].updated_LSQ_content);

            // Perform memory operation in 3 clock cycles
            if (lsq_head_entry_cycle == 3) {
                lsq_head_entry_cycle = 0;

                // Copy data from Memory to Forwarding bus
                mem_data = data_list.data[mem_address].data_value;
                result_bus.reg_P[dest_phy_reg_index].reg_value = mem_data;
                strcpy(result_bus.reg_P[dest_phy_reg_index].reg_status, REG_STATUS_VALID);

                // Committing result in PRF
                phy_reg_file[dest_phy_reg_index].reg_P.reg_value = mem_data;
                strcpy(phy_reg_file[dest_phy_reg_index].reg_P.reg_status, REG_STATUS_VALID);

                // Modify respective ROB entry
                if (rob_queue.rob_entry[rob_index].rob_status == 1 && rob_queue.rob_entry[rob_index].opcode == OPCODE_LOAD) {
                    rob_queue.rob_entry[rob_index].result = mem_data;
                    rob_queue.rob_entry[rob_index].ex_code = 0;
                    rob_queue.rob_entry[rob_index].rob_status = 0;
                }

                // Forward Data to required LSQ entries
                for (int l = 0; l < LSQ_SIZE; l++) {
                    if (lsq_queue.lsq_entry[l].opcode == OPCODE_STORE && lsq_queue.lsq_entry[l].lsq_status != -1
                            && lsq_queue.lsq_entry[l].src_status == 1 && lsq_queue.lsq_entry[l].src_phy_reg_address == dest_phy_reg_index) {
                        lsq_queue.lsq_entry[l].src_value = mem_data;
                        lsq_queue.lsq_entry[l].src_status = 0;
                    }
                }

                // Clear the LSQ contents
                lsq_queue.lsq_entry[head].lsq_status = -1;
                strcpy(lsq_queue.lsq_entry[head].current_LSQ_content, "");
                strcpy(lsq_queue.lsq_entry[head].updated_LSQ_content, "");
                lsq_display_queue.lsq_entry[head].lsq_status = -1;
                strcpy(lsq_display_queue.lsq_entry[head].current_LSQ_content, "");
                strcpy(lsq_display_queue.lsq_entry[head].updated_LSQ_content, "");

                // Modifying LSQ head pointer
                if (head == tail) {
                    lsq_queue.lsq_head = -1;
                    lsq_queue.lsq_tail = -1;
                } else {
                    lsq_queue.lsq_head += 1;
                }
            }
        }
    }

    return 0;
}

/*
 Implementation of Pipeline Execute Stage
 */
int execute_EX() {

    // Clearing EX stage printing current instruction content
    strcpy(ex_stage.current_inst_content, "");
    strcpy(ex_stage.current_inst_content_MUL1, "");
    strcpy(ex_stage.current_inst_content_MUL2, "");
    strcpy(ex_stage.current_inst_content_DIV1, "");
    strcpy(ex_stage.current_inst_content_DIV2, "");
    strcpy(ex_stage.current_inst_content_DIV3, "");
    strcpy(ex_stage.current_inst_content_DIV4, "");
    strcpy(ex_stage.updated_inst_content, "");
    strcpy(ex_stage.updated_inst_content_MUL1, "");
    strcpy(ex_stage.updated_inst_content_MUL2, "");
    strcpy(ex_stage.updated_inst_content_DIV1, "");
    strcpy(ex_stage.updated_inst_content_DIV2, "");
    strcpy(ex_stage.updated_inst_content_DIV3, "");
    strcpy(ex_stage.updated_inst_content_DIV4, "");

    if (ex_stage.stall == true) {
        return 0;
    } else {

        // Variable Declaration and Initialization
        int inst_num, loop_counter, rob_index, lsq_index;
        int temp_cnt = 0;
        int all_executed_inst_num[10] = {0};
        I_Detail all_executed_inst[10];

        // Initialization of temporary latches to keep all executed instruction in EX stage
        for (int p = 0; p < 10; p++) {
            all_executed_inst[p].op_code = -10;
            all_executed_inst[p].pc_value = -10;
        }

        // Initializing loop counter value as per the previous loop counter
        loop_counter = (overall_inst_counter > inst_to_fetch) ? overall_inst_counter : inst_to_fetch;
        if (loop_counter < ex_prev_loop_counter) {
            loop_counter = ex_prev_loop_counter;
        }
        ex_prev_loop_counter = loop_counter;

        // Initialization of respective Execute Flags
        is_INT_empty = true;
        if (is_INT_finished) {
            is_INT_finished = false;
        }
        if (is_DIV4_finished) {
            is_DIV4_finished = false;
        }
        if (is_MUL2_finished) {
            is_MUL2_finished = false;
        }

        // Looping for Input Buffer Instruction Processing
        for (inst_num = 0; inst_num <= loop_counter; inst_num++) {
            if (ex_stage.input_inst_detail[inst_num].is_DRF_done && !ex_stage.input_inst_detail[inst_num].is_EX_done) {

                // Variable Declaration and Initialization
                I_Detail input_detail, output_detail;

                // Coping Stage Input Instruction in temporary variables
                input_detail = ex_stage.input_inst_detail[inst_num];
                output_detail = input_detail;

                int src1_value = output_detail.src_reg_detail[0].reg_value;
                int src2_value = output_detail.src_reg_detail[1].reg_value;
                int dest_phy_index = output_detail.phy_reg_address;

                // Initializing EX Stage display variables with currently processing instruction
                if (output_detail.op_code == OPCODE_NOP) {
                    strcpy(ex_stage.current_inst_content, output_detail.i_content);
                    strcpy(ex_stage.updated_inst_content, output_detail.updated_i_content);
                    ex_stage.current_inst_number = output_detail.inst_number;
                } else if (output_detail.op_code != OPCODE_MUL && output_detail.op_code != OPCODE_DIV) {
                    strcpy(ex_stage.current_inst_content, output_detail.i_content);
                    strcpy(ex_stage.updated_inst_content, output_detail.updated_i_content);
                    ex_stage.current_inst_number = output_detail.inst_number;
                    //is_INT_empty = false;
                }

                // Main EX Stage logic implementation
                switch (output_detail.op_code) {

                    case OPCODE_STORE:
                        output_detail.target_memory_address = int_FU_EX(src2_value, output_detail.literal_data, OPCODE_STORE);
                        is_INT_finished = true;

                        if (output_detail.target_memory_address < 0) {
                            printf(RED"\n\nERROR::::: ARRAY_INDEX_OUT_OF_BOUND FOR DATA MEMORY !!!\n\n"RST);
                        }

                        // Forwarding computed Memory Address to respective LSQ entry
                        lsq_index = output_detail.lsq_index;
                        if (lsq_queue.lsq_entry[lsq_index].mem_address_status == 1) {
                            lsq_queue.lsq_entry[lsq_index].mem_address_result = output_detail.target_memory_address;
                            lsq_queue.lsq_entry[lsq_index].mem_address_status = 0;
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_LOAD:
                        output_detail.target_memory_address = int_FU_EX(src1_value, output_detail.literal_data, OPCODE_LOAD);
                        is_INT_finished = true;

                        if (output_detail.target_memory_address < 0) {
                            printf(RED"\n\nERROR::::: ARRAY_INDEX_OUT_OF_BOUND FOR DATA MEMORY !!!\n\n"RST);
                        }

                        // Forwarding computed Memory Address to respective LSQ entry
                        lsq_index = output_detail.lsq_index;
                        if (lsq_queue.lsq_entry[lsq_index].mem_address_status == 1) {
                            lsq_queue.lsq_entry[lsq_index].mem_address_result = output_detail.target_memory_address;
                            lsq_queue.lsq_entry[lsq_index].mem_address_status = 0;
                        }

                        // Load Bypassing Logic implementation
                        int k, l, m, n, bypass_cnt = 0;
                        LSQ_Entry current_lsq_entry;
                        Bypass_Entry new_bypass_entry;

                        // Initialize Load Bypass Structure
                        load_bypass.is_load_in_lsq_valid = false;
                        load_bypass.is_store_in_lsq_valid = false;
                        load_bypass.lsq_loads_count = 0;
                        load_bypass.lsq_stores_count = 0;
                        for (k = 0; k < LSQ_SIZE; k++) {
                            load_bypass.bypass_entry[k].opcode = -10;
                            load_bypass.bypass_entry[k].src_status = -1;
                            load_bypass.bypass_entry[k].mem_address_status = -1;
                            load_bypass.bypass_entry[k].lsq_index = -1;
                        }

                        for (k = lsq_index - 1; k >= lsq_queue.lsq_head; --k) {

                            current_lsq_entry = lsq_queue.lsq_entry[k];

                            // Making new Bypass Entry
                            new_bypass_entry.mem_address_status = current_lsq_entry.mem_address_status;
                            new_bypass_entry.mem_address_result = current_lsq_entry.mem_address_result;
                            new_bypass_entry.src_status = current_lsq_entry.src_status;
                            new_bypass_entry.src_value = current_lsq_entry.src_value;
                            new_bypass_entry.dest_phy_reg_address = current_lsq_entry.dest_phy_reg_address;
                            new_bypass_entry.src_phy_reg_address = current_lsq_entry.src_phy_reg_address;
                            new_bypass_entry.lsq_index = k;

                            if (current_lsq_entry.opcode == OPCODE_STORE) {

                                new_bypass_entry.opcode = OPCODE_STORE;

                                if (current_lsq_entry.src_status == 0 && current_lsq_entry.mem_address_status == 0) {
                                    load_bypass.is_store_in_lsq_valid = true;
                                }
                                load_bypass.lsq_stores_count += 1;
                                load_bypass.bypass_entry[bypass_cnt] = new_bypass_entry;
                                bypass_cnt++;

                            } else if (current_lsq_entry.opcode == OPCODE_LOAD) {

                                new_bypass_entry.opcode = OPCODE_LOAD;

                                if (current_lsq_entry.mem_address_status == 0) {
                                    load_bypass.is_load_in_lsq_valid = true;
                                }
                                load_bypass.lsq_loads_count += 1;
                                load_bypass.bypass_entry[bypass_cnt] = new_bypass_entry;
                                bypass_cnt++;
                            }
                        }

                        // Mark LSQ entry for LOAD as invalid for bypassing
                        if (load_bypass.lsq_stores_count == 0 && load_bypass.lsq_loads_count == 0) {
                            lsq_queue.lsq_entry[lsq_index].is_load_bypassed = false;
                            lsq_queue.lsq_entry[lsq_index].is_load_bypassing_possible = false;
                        }

                        if (load_bypass.lsq_stores_count > 0 && load_bypass.is_store_in_lsq_valid) {
                            for (l = 0; l < bypass_cnt; l++) {
                                if (load_bypass.bypass_entry[l].opcode == OPCODE_STORE) {

                                    // No bypassing for LOAD if memory address status of nearest STORE is invalid
                                    if (load_bypass.bypass_entry[l].mem_address_status == 1 || load_bypass.bypass_entry[l].src_status != 0) {
                                        lsq_queue.lsq_entry[lsq_index].is_load_bypassed = false;
                                        lsq_queue.lsq_entry[lsq_index].is_load_bypassing_possible = false;
                                        break;
                                    }

                                    // Condition to forward STORE value to LOAD satisfies - Bypassing done and forwarding completed
                                    if (load_bypass.bypass_entry[l].mem_address_result == output_detail.target_memory_address) {

                                        // Copy data from forwarded STORE to Forwarding bus
                                        int mem_data = load_bypass.bypass_entry[l].src_value;
                                        result_bus.reg_P[lsq_queue.lsq_entry[lsq_index].dest_phy_reg_address].reg_value = mem_data;
                                        strcpy(result_bus.reg_P[lsq_queue.lsq_entry[lsq_index].dest_phy_reg_address].reg_status, REG_STATUS_VALID);

                                        // Committing result in PRF
                                        phy_reg_file[lsq_queue.lsq_entry[lsq_index].dest_phy_reg_address].reg_P.reg_value = mem_data;
                                        strcpy(phy_reg_file[lsq_queue.lsq_entry[lsq_index].dest_phy_reg_address].reg_P.reg_status, REG_STATUS_VALID);

                                        // Modify respective ROB entry
                                        if (rob_queue.rob_entry[lsq_queue.lsq_entry[lsq_index].rob_index].rob_status == 1 &&
                                            rob_queue.rob_entry[lsq_queue.lsq_entry[lsq_index].rob_index].opcode == OPCODE_LOAD) {

                                            rob_queue.rob_entry[lsq_queue.lsq_entry[lsq_index].rob_index].result = mem_data;
                                            rob_queue.rob_entry[lsq_queue.lsq_entry[lsq_index].rob_index].ex_code = 0;
                                            rob_queue.rob_entry[lsq_queue.lsq_entry[lsq_index].rob_index].rob_status = 0;
                                        }

                                        lsq_queue.lsq_entry[lsq_index].is_load_bypassed = true;
                                        lsq_queue.lsq_entry[lsq_index].is_load_bypassing_possible = true;

                                        // Clear the LSQ contents
                                        lsq_queue.lsq_entry[lsq_index].lsq_status = -1;
                                        strcpy(lsq_queue.lsq_entry[lsq_index].current_LSQ_content, "");
                                        strcpy(lsq_queue.lsq_entry[lsq_index].updated_LSQ_content, "");
                                        lsq_display_queue.lsq_entry[lsq_index].lsq_status = -1;
                                        strcpy(lsq_display_queue.lsq_entry[lsq_index].current_LSQ_content, "");
                                        strcpy(lsq_display_queue.lsq_entry[lsq_index].updated_LSQ_content, "");

                                        break;

                                    }

                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_MOVC:
                        output_detail.computed_result = int_FU_EX(output_detail.literal_data, NUM_ZERO, OPCODE_MOVC);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_MUL:

                        // MUL1 FU implementation
                        if (!ex_stage.input_inst_detail[inst_num].is_MUL_done) {
                            I_Detail temp_inst;
                            bool temp_flag = true;
                            for (int p = 0; p < 10; p++) {
                                temp_inst = all_executed_inst[p];
                                if (temp_inst.op_code == OPCODE_MUL && temp_inst.is_MUL_done) {
                                    if (temp_inst.is_EX_done) {
                                        ex_stage.input_inst_detail[inst_num].is_MUL_done = mul_FU_EX1();
                                    } else if (is_MUL2_finished && !is_DIV4_finished) {
                                        ex_stage.input_inst_detail[inst_num].is_MUL_done = mul_FU_EX1();
                                    } else {
                                        temp_flag = false;
                                    }
                                }
                            }
                            if (temp_flag) {
                                ex_stage.input_inst_detail[inst_num].is_MUL_done = mul_FU_EX1();
                            }

                            // Initializing MUL1 Stage display variables with currently processing instruction
                            strcpy(ex_stage.current_inst_content_MUL1, output_detail.i_content);
                            strcpy(ex_stage.updated_inst_content_MUL1, output_detail.updated_i_content);
                            ex_stage.current_inst_number_MUL1 = output_detail.inst_number;

                            break;
                        }

                        // MUL2 FU implementation
                        output_detail.computed_result = mul_FU_EX2(src1_value, src2_value);

                        // Initializing MUL2 Stage display variables with currently processing instruction
                        strcpy(ex_stage.current_inst_content_MUL2, output_detail.i_content);
                        strcpy(ex_stage.updated_inst_content_MUL2, output_detail.updated_i_content);
                        ex_stage.current_inst_number_MUL2 = output_detail.inst_number;
                        is_MUL2_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_DIV:

                        // DIV1 FU implementation
                        if (!ex_stage.input_inst_detail[inst_num].is_DIV1_done) {
                            ex_stage.input_inst_detail[inst_num].is_DIV1_done = div_FU_EX1();

                            // Initializing DIV1 Stage display variables with currently processing instruction
                            strcpy(ex_stage.current_inst_content_DIV1, output_detail.i_content);
                            strcpy(ex_stage.updated_inst_content_DIV1, output_detail.updated_i_content);
                            ex_stage.current_inst_number_DIV1 = output_detail.inst_number;

                            break;
                        }

                        // DIV2 FU implementation
                        if (!ex_stage.input_inst_detail[inst_num].is_DIV2_done) {
                            ex_stage.input_inst_detail[inst_num].is_DIV2_done = div_FU_EX2();

                            // Initializing DIV2 Stage display variables with currently processing instruction
                            strcpy(ex_stage.current_inst_content_DIV2, output_detail.i_content);
                            strcpy(ex_stage.updated_inst_content_DIV2, output_detail.updated_i_content);
                            ex_stage.current_inst_number_DIV2 = output_detail.inst_number;

                            break;
                        }

                        // DIV3 FU implementation
                        if (!ex_stage.input_inst_detail[inst_num].is_DIV3_done) {
                            ex_stage.input_inst_detail[inst_num].is_DIV3_done = div_FU_EX3();

                            // Initializing DIV3 Stage display variables with currently processing instruction
                            strcpy(ex_stage.current_inst_content_DIV3, output_detail.i_content);
                            strcpy(ex_stage.updated_inst_content_DIV3, output_detail.updated_i_content);
                            ex_stage.current_inst_number_DIV3 = output_detail.inst_number;

                            break;
                        }

                        // DIV4 FU implementation
                        output_detail.computed_result = div_FU_EX4(src1_value, src2_value);

                        // Initializing DIV4 Stage display variables with currently processing instruction
                        strcpy(ex_stage.current_inst_content_DIV4, output_detail.i_content);
                        strcpy(ex_stage.updated_inst_content_DIV4, output_detail.updated_i_content);
                        ex_stage.current_inst_number_DIV4 = output_detail.inst_number;
                        is_DIV4_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_ADD:
                        output_detail.computed_result = int_FU_EX(src1_value, src2_value, OPCODE_ADD);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_SUB:
                        output_detail.computed_result = int_FU_EX(src1_value, src2_value, OPCODE_SUB);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_AND:
                        output_detail.computed_result = int_FU_EX(src1_value, src2_value, OPCODE_AND);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_OR:
                        output_detail.computed_result = int_FU_EX(src1_value, src2_value, OPCODE_OR);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_EXOR:
                        output_detail.computed_result = int_FU_EX(src1_value, src2_value, OPCODE_EXOR);
                        is_INT_finished = true;

                        // Forwarding result
                        result_bus.reg_P[dest_phy_index].reg_value = output_detail.computed_result;
                        strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                        // Committing result in PRF
                        phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.computed_result;
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                        // Forwarding result to respective ROB entry
                        rob_index = output_detail.rob_index;
                        if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                            rob_queue.rob_entry[rob_index].result = output_detail.computed_result;
                            rob_queue.rob_entry[rob_index].ex_code = 0;
                            rob_queue.rob_entry[rob_index].rob_status = 0;
                        }

                        // Forwarding result to required LSQ entries
                        for (int p = 0; p < LSQ_SIZE; p++) {
                            if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                    lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                    lsq_queue.lsq_entry[p].src_value = output_detail.computed_result;
                                    lsq_queue.lsq_entry[p].src_status = 0;
                                }
                            }
                        }

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_JUMP:

                        if (!is_JUMP_pc_fetched) {

                            // Logic to set PC of the next fetching instruction
                            jump_to_pc = src1_value + output_detail.literal_data;

                            I_Content i_content;
                            for (int x = 0; x < inst_count_in_file; x++) {
                                i_content = inst_cache.instructions[x];
                                if (jump_to_pc != 0 && i_content.file_line_address == jump_to_pc) {
                                    inst_to_fetch = x;
                                    break;
                                }
                            }
                            is_JUMP_pc_fetched = true;

                            // Squash ROB, LSQ, IQ, FU, D/RF and F
                            squash_instructions(output_detail, inst_num, loop_counter);

                            // Forwarding result to respective ROB entry
                            rob_index = output_detail.rob_index;
                            if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                rob_queue.rob_entry[rob_index].result = 0;
                                rob_queue.rob_entry[rob_index].ex_code = 0;
                                rob_queue.rob_entry[rob_index].rob_status = 0;
                            }

                        } else {
                            f_stage.stall = false;
                        }
                        is_branch_inst_found = true;
                        is_INT_finished = true;

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_JAL:

                        if (!is_JAL_pc_fetched) {

                            // Logic to set PC of the next fetching instruction
                            jump_to_pc = src1_value + output_detail.literal_data;

                            I_Content i_content;
                            for (int x = 0; x < inst_count_in_file; x++) {
                                i_content = inst_cache.instructions[x];
                                if (jump_to_pc != 0 && i_content.file_line_address == jump_to_pc) {
                                    inst_to_fetch = x;
                                    break;
                                }
                            }
                            is_JAL_pc_fetched = true;

                            // Squash ROB, LSQ, IQ, FU, D/RF and F
                            squash_instructions(output_detail, inst_num, loop_counter);

                            // Forwarding result
                            result_bus.reg_P[dest_phy_index].reg_value = output_detail.pc_value + BYTE_OFFSET;
                            strcpy(result_bus.reg_P[dest_phy_index].reg_status, REG_STATUS_VALID);

                            // Committing result in PRF
                            phy_reg_file[dest_phy_index].reg_P.reg_value = output_detail.pc_value + BYTE_OFFSET;
                            strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);

                            // Forwarding result to respective ROB entry
                            rob_index = output_detail.rob_index;
                            if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                rob_queue.rob_entry[rob_index].result = output_detail.pc_value + BYTE_OFFSET;
                                rob_queue.rob_entry[rob_index].ex_code = 0;
                                rob_queue.rob_entry[rob_index].rob_status = 0;
                            }

                            // Forwarding result to required LSQ entries
                            for (int p = 0; p < LSQ_SIZE; p++) {
                                if (lsq_queue.lsq_entry[p].lsq_status != -1 && lsq_queue.lsq_entry[p].opcode == OPCODE_STORE) {
                                    if (lsq_queue.lsq_entry[p].src_status == 1 &&
                                        lsq_queue.lsq_entry[p].src_phy_reg_address == dest_phy_index) {
                                        lsq_queue.lsq_entry[p].src_value = output_detail.pc_value + BYTE_OFFSET;
                                        lsq_queue.lsq_entry[p].src_status = 0;
                                    }
                                }
                            }
                        } else {
                            f_stage.stall = false;
                        }
                        is_branch_inst_found = true;
                        is_INT_finished = true;

                        // Setting EX Stage Input and Output Buffer Instruction Flags
                        SET_EX_STAGE_INPUT_INST_FLAGS;
                        SET_EX_STAGE_OUTPUT_INST_FLAGS;

                        break;
                    case OPCODE_BNZ:

                        if (output_detail.bz_bnz_src_reg_detail.reg_value != 0) {

                            if (!is_BNZ_pc_fetched) {

                                // Logic to set PC of the next fetching instruction
                                jump_to_pc = output_detail.pc_value + output_detail.literal_data;

                                I_Content i_content;
                                for (int x = 0; x < inst_count_in_file; x++) {
                                    i_content = inst_cache.instructions[x];
                                    if (jump_to_pc != 0 && i_content.file_line_address == jump_to_pc) {
                                        inst_to_fetch = x;
                                        break;
                                    }
                                }
                                is_BNZ_pc_fetched = true;

                                // Squash ROB, LSQ, IQ, FU, D/RF and F
                                squash_instructions(output_detail, inst_num, loop_counter);

                                // Forwarding result to respective ROB entry
                                rob_index = output_detail.rob_index;
                                if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                    rob_queue.rob_entry[rob_index].result = 0;
                                    rob_queue.rob_entry[rob_index].ex_code = 0;
                                    rob_queue.rob_entry[rob_index].rob_status = 0;
                                }

                            } else {
                                f_stage.stall = false;
                            }
                            is_branch_inst_found = true;

                            is_INT_finished = true;

                            // Setting EX Stage Input and Output Buffer Instruction Flags
                            SET_EX_STAGE_INPUT_INST_FLAGS;
                            SET_EX_STAGE_OUTPUT_INST_FLAGS;
                        } else {

                            // Forwarding result to respective ROB entry
                            rob_index = output_detail.rob_index;
                            if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                rob_queue.rob_entry[rob_index].result = 0;
                                rob_queue.rob_entry[rob_index].ex_code = 0;
                                rob_queue.rob_entry[rob_index].rob_status = 0;
                            }

                            is_INT_finished = true;

                            // Setting EX Stage Input and Output Buffer Instruction Flags
                            SET_EX_STAGE_INPUT_INST_FLAGS;
                            SET_EX_STAGE_OUTPUT_INST_FLAGS;
                        }

                        break;
                    case OPCODE_BZ:

                        if (output_detail.bz_bnz_src_reg_detail.reg_value == 0) {

                            if (!is_BZ_pc_fetched) {

                                // Logic to set PC of the next fetching instruction
                                jump_to_pc = output_detail.pc_value + output_detail.literal_data;

                                I_Content i_content;
                                for (int x = 0; x < inst_count_in_file; x++) {
                                    i_content = inst_cache.instructions[x];
                                    if (jump_to_pc != 0 && i_content.file_line_address == jump_to_pc) {
                                        inst_to_fetch = x;
                                        break;
                                    }
                                }
                                is_BZ_pc_fetched = true;

                                // Squash ROB, LSQ, IQ, FU, D/RF and F
                                squash_instructions(output_detail, inst_num, loop_counter);

                                // Forwarding result to respective ROB entry
                                rob_index = output_detail.rob_index;
                                if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                    rob_queue.rob_entry[rob_index].result = 0;
                                    rob_queue.rob_entry[rob_index].ex_code = 0;
                                    rob_queue.rob_entry[rob_index].rob_status = 0;
                                }

                            } else {
                                f_stage.stall = false;
                            }
                            is_branch_inst_found = true;

                            is_INT_finished = true;

                            // Setting EX Stage Input and Output Buffer Instruction Flags
                            SET_EX_STAGE_INPUT_INST_FLAGS;
                            SET_EX_STAGE_OUTPUT_INST_FLAGS;
                        } else {

                            // Forwarding result to respective ROB entry
                            rob_index = output_detail.rob_index;
                            if (rob_queue.rob_entry[rob_index].rob_status == 1) {
                                rob_queue.rob_entry[rob_index].result = 0;
                                rob_queue.rob_entry[rob_index].ex_code = 0;
                                rob_queue.rob_entry[rob_index].rob_status = 0;
                            }

                            is_INT_finished = true;

                            // Setting EX Stage Input and Output Buffer Instruction Flags
                            SET_EX_STAGE_INPUT_INST_FLAGS;
                            SET_EX_STAGE_OUTPUT_INST_FLAGS;
                        }

                        break;
                    default:
                        break;
                }

                // Coping temporary output to Stage Output Buffer
                ex_stage.output_inst_detail[inst_num] = output_detail;

                // Modifying temporary latches with executed instructions
                all_executed_inst_num[temp_cnt] = inst_num;
                all_executed_inst[temp_cnt] = ex_stage.output_inst_detail[inst_num];
                temp_cnt++;
            }
        }
    }

    return 0;
}

/*
 Implementation of Issue Queue Functionality
 */
int issue_queue_IQ() {

    // Variable Declaration and Initialization
    int i, wake_cnt = 0, iq_inst_num, ex_inst_num, selected_INT_inst = -1, selected_MUL_inst = -1, selected_DIV_inst = -1;
    int min_INT_clk = 1000, min_MUL_clk = 1000, min_DIV_clk = 1000;
    int inst_for_INT[IQ_SIZE], inst_for_MUL[IQ_SIZE], inst_for_DIV[IQ_SIZE];
    I_Detail wakeup_inst_details[IQ_SIZE], output_to_EX;
    char src_1_status[10], src_2_status[10], psw_phy_reg_status[10];

    for (i = 0; i < IQ_SIZE; i++) {
        wakeup_inst_details[i].decoded_inst_num = -1;
        inst_for_INT[i] = -1;
        inst_for_MUL[i] = -1;
        inst_for_DIV[i] = -1;
        // For display purpose
        issue_display_queue[i] = issue_queue[i];
    }

    // Implementation of Wake-Up logic
    for (i = 0; i < IQ_SIZE; i++) {
        if (issue_queue[i].iq_status == 1) {

            memset(src_1_status, '\0', 10);
            memset(src_2_status, '\0', 10);

            int src1_phy_index = issue_queue[i].src1_reg.reg_index;
            int src2_phy_index = issue_queue[i].src2_reg.reg_index;
            int psw_phy_reg_index = issue_queue[i].bz_bnz_src_reg_detail.reg_index;

            // Logic to consume value of registers from Forwarding Bus and wake up the IQ entry for selection
            if (issue_queue[i].op_code == OPCODE_MUL || issue_queue[i].op_code == OPCODE_ADD || issue_queue[i].op_code == OPCODE_DIV ||
                issue_queue[i].op_code == OPCODE_SUB || issue_queue[i].op_code == OPCODE_AND || issue_queue[i].op_code == OPCODE_OR ||
                issue_queue[i].op_code == OPCODE_EXOR) {

                strcpy(src_1_status, issue_queue[i].src1_reg.reg_status);
                strcpy(src_2_status, issue_queue[i].src2_reg.reg_status);

                // Forwarding Result Bus
                if (strcmp(src_1_status, REG_STATUS_INVALID) == 0) {
                    strcpy(src_1_status, result_bus.reg_P[src1_phy_index].reg_status);
                    if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                        issue_queue[i].src1_reg.reg_value = result_bus.reg_P[src1_phy_index].reg_value;
                        strcpy(issue_queue[i].src1_reg.reg_status, REG_STATUS_VALID);
                    }
                }
                if (strcmp(src_2_status, REG_STATUS_INVALID) == 0) {
                    strcpy(src_2_status, result_bus.reg_P[src2_phy_index].reg_status);
                    if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                        issue_queue[i].src2_reg.reg_value = result_bus.reg_P[src2_phy_index].reg_value;
                        strcpy(issue_queue[i].src2_reg.reg_status, REG_STATUS_VALID);
                    }
                }

                // Condition to wake up Valid instructions in IQ
                if (strcmp(src_1_status, REG_STATUS_VALID) == 0 && strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                    wakeup_inst_details[wake_cnt].op_code = issue_queue[i].op_code;
                    wakeup_inst_details[wake_cnt].src_reg_detail[0] = issue_queue[i].src1_reg;
                    wakeup_inst_details[wake_cnt].src_reg_detail[1] = issue_queue[i].src2_reg;
                    wakeup_inst_details[wake_cnt].literal_data = issue_queue[i].literal;
                    wakeup_inst_details[wake_cnt].dest_reg_detail[0] = issue_queue[i].dest_reg;
                    wakeup_inst_details[wake_cnt].inst_IQ_index = i;
                    wakeup_inst_details[wake_cnt].decoded_inst_num = issue_queue[i].inst_detail.decoded_inst_num;
                    wake_cnt++;
                }
            } else if (issue_queue[i].op_code == OPCODE_LOAD || issue_queue[i].op_code == OPCODE_JAL || issue_queue[i].op_code == OPCODE_JUMP) {

                strcpy(src_1_status, issue_queue[i].src1_reg.reg_status);

                // Forwarding Result Bus
                if (strcmp(src_1_status, REG_STATUS_INVALID) == 0) {
                    strcpy(src_1_status, result_bus.reg_P[src1_phy_index].reg_status);
                    if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                        issue_queue[i].src1_reg.reg_value = result_bus.reg_P[src1_phy_index].reg_value;
                        strcpy(issue_queue[i].src1_reg.reg_status, REG_STATUS_VALID);
                    }
                }

                // Condition to wake up Valid instructions in IQ
                if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                    wakeup_inst_details[wake_cnt].op_code = issue_queue[i].op_code;
                    wakeup_inst_details[wake_cnt].src_reg_detail[0] = issue_queue[i].src1_reg;
                    wakeup_inst_details[wake_cnt].src_reg_detail[1] = issue_queue[i].src2_reg;
                    wakeup_inst_details[wake_cnt].literal_data = issue_queue[i].literal;
                    wakeup_inst_details[wake_cnt].dest_reg_detail[0] = issue_queue[i].dest_reg;
                    wakeup_inst_details[wake_cnt].inst_IQ_index = i;
                    wakeup_inst_details[wake_cnt].decoded_inst_num = issue_queue[i].inst_detail.decoded_inst_num;
                    wake_cnt++;
                }
            } else if (issue_queue[i].op_code == OPCODE_STORE) {

                strcpy(src_2_status, issue_queue[i].src2_reg.reg_status);

                // Forwarding Result Bus
                if (strcmp(src_2_status, REG_STATUS_INVALID) == 0) {
                    strcpy(src_2_status, result_bus.reg_P[src2_phy_index].reg_status);
                    if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                        issue_queue[i].src2_reg.reg_value = result_bus.reg_P[src2_phy_index].reg_value;
                        strcpy(issue_queue[i].src2_reg.reg_status, REG_STATUS_VALID);
                    }
                }

                // Condition to wake up Valid instructions in IQ
                if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                    wakeup_inst_details[wake_cnt].op_code = issue_queue[i].op_code;
                    wakeup_inst_details[wake_cnt].src_reg_detail[0] = issue_queue[i].src1_reg;
                    wakeup_inst_details[wake_cnt].src_reg_detail[1] = issue_queue[i].src2_reg;
                    wakeup_inst_details[wake_cnt].literal_data = issue_queue[i].literal;
                    wakeup_inst_details[wake_cnt].dest_reg_detail[0] = issue_queue[i].dest_reg;
                    wakeup_inst_details[wake_cnt].inst_IQ_index = i;
                    wakeup_inst_details[wake_cnt].decoded_inst_num = issue_queue[i].inst_detail.decoded_inst_num;
                    wake_cnt++;
                }
            } else if (issue_queue[i].op_code == OPCODE_BZ || issue_queue[i].op_code == OPCODE_BNZ) {

                strcpy(psw_phy_reg_status, issue_queue[i].bz_bnz_src_reg_detail.reg_status);

                // Forwarding Result Bus
                if (strcmp(psw_phy_reg_status, REG_STATUS_INVALID) == 0) {
                    strcpy(psw_phy_reg_status, result_bus.reg_P[psw_phy_reg_index].reg_status);
                    if (strcmp(psw_phy_reg_status, REG_STATUS_VALID) == 0) {
                        issue_queue[i].bz_bnz_src_reg_detail.reg_value = result_bus.reg_P[psw_phy_reg_index].reg_value;
                        strcpy(issue_queue[i].bz_bnz_src_reg_detail.reg_status, REG_STATUS_VALID);
                    }
                }

                // Condition to wake up Valid instructions in IQ
                if (strcmp(psw_phy_reg_status, REG_STATUS_VALID) == 0) {
                    wakeup_inst_details[wake_cnt].op_code = issue_queue[i].op_code;
                    wakeup_inst_details[wake_cnt].src_reg_detail[0] = issue_queue[i].src1_reg;
                    wakeup_inst_details[wake_cnt].src_reg_detail[1] = issue_queue[i].src2_reg;
                    wakeup_inst_details[wake_cnt].literal_data = issue_queue[i].literal;
                    wakeup_inst_details[wake_cnt].dest_reg_detail[0] = issue_queue[i].dest_reg;
                    wakeup_inst_details[wake_cnt].bz_bnz_src_reg_detail = issue_queue[i].bz_bnz_src_reg_detail;
                    wakeup_inst_details[wake_cnt].inst_IQ_index = i;
                    wakeup_inst_details[wake_cnt].decoded_inst_num = issue_queue[i].inst_detail.decoded_inst_num;
                    wake_cnt++;
                }

            } else {

                // Condition to wake up Valid instructions in IQ
                wakeup_inst_details[wake_cnt].op_code = issue_queue[i].op_code;
                wakeup_inst_details[wake_cnt].src_reg_detail[0] = issue_queue[i].src1_reg;
                wakeup_inst_details[wake_cnt].src_reg_detail[1] = issue_queue[i].src2_reg;
                wakeup_inst_details[wake_cnt].literal_data = issue_queue[i].literal;
                wakeup_inst_details[wake_cnt].dest_reg_detail[0] = issue_queue[i].dest_reg;
                wakeup_inst_details[wake_cnt].inst_IQ_index = i;
                wakeup_inst_details[wake_cnt].decoded_inst_num = issue_queue[i].inst_detail.decoded_inst_num;
                wake_cnt++;
            }
        }
    }

    // Implementation of Selection logic
    for (i = 0; i < wake_cnt; i++) {

        iq_inst_num = wakeup_inst_details[i].inst_IQ_index;

        if (wakeup_inst_details[i].op_code == OPCODE_DIV) {
            if (!issue_queue[iq_inst_num].inst_issued && min_DIV_clk > issue_queue[iq_inst_num].clk_cycle_dispatched) {
                min_DIV_clk = issue_queue[iq_inst_num].clk_cycle_dispatched;
                selected_DIV_inst = i;
            }
        } else if (wakeup_inst_details[i].op_code == OPCODE_MUL) {
            if (!issue_queue[iq_inst_num].inst_issued && min_MUL_clk > issue_queue[iq_inst_num].clk_cycle_dispatched) {
                min_MUL_clk = issue_queue[iq_inst_num].clk_cycle_dispatched;
                selected_MUL_inst = i;
            }
        } else {
            if (!issue_queue[iq_inst_num].inst_issued && min_INT_clk > issue_queue[iq_inst_num].clk_cycle_dispatched) {
                min_INT_clk = issue_queue[iq_inst_num].clk_cycle_dispatched;
                selected_INT_inst = i;
            }
        }
    }

    // Issuing selected instructions to DIV FU for execution
    if (selected_DIV_inst != -1) {
        ex_inst_num = wakeup_inst_details[selected_DIV_inst].decoded_inst_num;
        iq_inst_num = wakeup_inst_details[selected_DIV_inst].inst_IQ_index;

        if (!issue_queue[iq_inst_num].inst_issued) {
            output_to_EX = issue_queue[iq_inst_num].inst_detail;
            output_to_EX.src_reg_detail[0] = wakeup_inst_details[selected_DIV_inst].src_reg_detail[0];
            output_to_EX.src_reg_detail[1] = wakeup_inst_details[selected_DIV_inst].src_reg_detail[1];

            issue_queue[iq_inst_num].inst_issued = true;

            // Clear IQ Entry
            issue_queue[iq_inst_num].iq_status = 0;
            issue_queue[iq_inst_num].clk_cycle_dispatched = 0;

            ex_stage.input_inst_detail[ex_inst_num] = output_to_EX;
            ex_stage.stall = false;
        }
    }
    // Issuing selected instructions to MUL FU for execution
    if (selected_MUL_inst != -1) {
        ex_inst_num = wakeup_inst_details[selected_MUL_inst].decoded_inst_num;
        iq_inst_num = wakeup_inst_details[selected_MUL_inst].inst_IQ_index;

        if (!issue_queue[iq_inst_num].inst_issued) {
            output_to_EX = issue_queue[iq_inst_num].inst_detail;
            output_to_EX.src_reg_detail[0] = wakeup_inst_details[selected_MUL_inst].src_reg_detail[0];
            output_to_EX.src_reg_detail[1] = wakeup_inst_details[selected_MUL_inst].src_reg_detail[1];

            issue_queue[iq_inst_num].inst_issued = true;

            // Clear IQ Entry
            issue_queue[iq_inst_num].iq_status = 0;
            issue_queue[iq_inst_num].clk_cycle_dispatched = 0;

            ex_stage.input_inst_detail[ex_inst_num] = output_to_EX;
            ex_stage.stall = false;
        }
    }
    // Issuing selected instructions to INT FU for execution
    if (selected_INT_inst != -1) {

        if ((is_DIV4_finished && !is_INT_empty) || (is_MUL2_finished && !is_INT_empty)) {
            // DO NOTHING
        } else {
            ex_inst_num = wakeup_inst_details[selected_INT_inst].decoded_inst_num;
            iq_inst_num = wakeup_inst_details[selected_INT_inst].inst_IQ_index;

            if (!issue_queue[iq_inst_num].inst_issued) {
                output_to_EX = issue_queue[iq_inst_num].inst_detail;
                output_to_EX.src_reg_detail[0] = wakeup_inst_details[selected_INT_inst].src_reg_detail[0];
                output_to_EX.src_reg_detail[1] = wakeup_inst_details[selected_INT_inst].src_reg_detail[1];
                output_to_EX.bz_bnz_src_reg_detail = wakeup_inst_details[selected_INT_inst].bz_bnz_src_reg_detail;

                issue_queue[iq_inst_num].inst_issued = true;

                // Clear IQ Entry
                issue_queue[iq_inst_num].iq_status = 0;
                issue_queue[iq_inst_num].clk_cycle_dispatched = 0;

                ex_stage.input_inst_detail[ex_inst_num] = output_to_EX;
                ex_stage.stall = false;
            }
        }
    }

    return 0;
}

/*
 Implementation of Pipeline Decode/ RF Stage
 */
int decode_DRF() {

    // Clearing DRF stage printing current instruction content
    strcpy(drf_stage.current_inst_content, "");
    strcpy(drf_stage.updated_inst_content, "");

    if (drf_stage.stall == true) {
        // DO NOTHING
        return 0;
    } else {

        // Variable Declaration and Initialization
        int i, split_size = 0, inst_num, src_cnt = 0, dest_cnt = 0, loop_counter, p_cnt;
        char *split_content[100];
        char src_1_status[10] = "\0";
        char src_2_status[10] = "\0";

        // Initializing loop counter value as per the previous loop counter
        loop_counter = (overall_inst_counter > inst_to_fetch) ? overall_inst_counter : inst_to_fetch;
        if (loop_counter < drf_prev_loop_counter) {
            loop_counter = drf_prev_loop_counter;
        }
        drf_prev_loop_counter = loop_counter;

        // Looping for Input Buffer Instruction Processing
        for (inst_num = 0; inst_num <= loop_counter; inst_num++) {
            if (drf_stage.input_inst_detail[inst_num].is_F_done && !drf_stage.input_inst_detail[inst_num].is_DRF_done) {

                // Variable Declaration and Initialization
                I_Detail input_detail, output_detail;
                int old_dest_reg_index = -1;
                char old_dest_reg_type = 'N';

                // Coping Stage Input Instruction in temporary variables
                input_detail = drf_stage.input_inst_detail[inst_num];
                output_detail = input_detail;

                split_size = split_string(input_detail.i_content, split_content);

                // Initializing DRF Stage display variables with currently processing instruction
                strcpy(drf_stage.current_inst_content, output_detail.i_content);
                drf_stage.current_inst_number = output_detail.inst_number;

                // Main DRF Stage logic implementation
                for (i = 0; i < split_size; i++) {
                    if (i == 0) {
                        if (strcmp(split_content[i], INST_MOVC) == 0) {
                            output_detail.op_code = OPCODE_MOVC;
                        } else if (strcmp(split_content[i], INST_STORE) == 0) {
                            output_detail.op_code = OPCODE_STORE;
                        } else if (strcmp(split_content[i], INST_LOAD) == 0) {
                            output_detail.op_code = OPCODE_LOAD;
                        } else if (strcmp(split_content[i], INST_MUL) == 0) {
                            output_detail.op_code = OPCODE_MUL;
                        } else if (strcmp(split_content[i], INST_DIV) == 0) {
                            output_detail.op_code = OPCODE_DIV;
                        } else if (strcmp(split_content[i], INST_ADD) == 0) {
                            output_detail.op_code = OPCODE_ADD;
                        } else if (strcmp(split_content[i], INST_SUB) == 0) {
                            output_detail.op_code = OPCODE_SUB;
                        } else if (strcmp(split_content[i], INST_AND) == 0) {
                            output_detail.op_code = OPCODE_AND;
                        } else if (strcmp(split_content[i], INST_OR) == 0) {
                            output_detail.op_code = OPCODE_OR;
                        } else if (strcmp(split_content[i], INST_EXOR) == 0) {
                            output_detail.op_code = OPCODE_EXOR;
                        } else if (strcmp(split_content[i], INST_JUMP) == 0) {
                            output_detail.op_code = OPCODE_JUMP;
                        } else if (strcmp(split_content[i], INST_JAL) == 0) {
                            output_detail.op_code = OPCODE_JAL;
                        } else if (strcmp(split_content[i], INST_BNZ) == 0) {
                            output_detail.op_code = OPCODE_BNZ;
                        } else if (strcmp(split_content[i], INST_BZ) == 0) {
                            output_detail.op_code = OPCODE_BZ;
                        } else if (strncmp(split_content[i], INST_HALT, 4) == 0) {
                            output_detail.op_code = OPCODE_HALT;
                            is_halt_inst_found = true;
                        }
                    }
                    if (i == 1) {
                        char sub[strlen(split_content[i]) - 1];
                        sub_string(split_content[i], sub, 1, (int) strlen(split_content[i]) - 1);
                        int i_sub = atoi(sub);
                        if (strncmp(split_content[i], "R", 1) == 0) {
                            if (output_detail.op_code == OPCODE_MOVC || output_detail.op_code == OPCODE_LOAD || output_detail.op_code == OPCODE_MUL ||
                                output_detail.op_code == OPCODE_ADD || output_detail.op_code == OPCODE_DIV || output_detail.op_code == OPCODE_SUB ||
                                output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR || output_detail.op_code == OPCODE_EXOR ||
                                output_detail.op_code == OPCODE_JAL) {

                                output_detail.dest_reg_detail[dest_cnt] = arch_reg_file.reg_R[i_sub];

                                // Check for free Physical Register
                                for (p_cnt = 0; p_cnt < PHY_REGISTER_LIMIT; p_cnt++) {
                                    if (phy_reg_file[p_cnt].phy_reg_status == 0) {
                                        // Mark as allocated
                                        phy_reg_file[p_cnt].phy_reg_status = 1;

                                        // Previous RAT entry for current architecture register
                                        old_dest_reg_index = reg_alias_table[i_sub].reg_index;
                                        old_dest_reg_type = reg_alias_table[i_sub].reg_type;

                                        // Make entry in RAT for Destination Register
                                        reg_alias_table[i_sub].reg_type = 'P';
                                        reg_alias_table[i_sub].reg_index = p_cnt;

                                        decode_rf_stalled = false;
                                        break;
                                    } else {
                                        // Stall since Physical Registers are not free
                                        decode_rf_stalled = true;
                                    }
                                }

                            } else if (output_detail.op_code == OPCODE_STORE || output_detail.op_code == OPCODE_JUMP) {
                                output_detail.src_reg_detail[src_cnt] = arch_reg_file.reg_R[i_sub];
                                if (output_detail.op_code != OPCODE_JUMP) {
                                    src_cnt++;
                                }
                            }
                        }
                        if (strncmp(split_content[i], "#", 1) == 0) {
                            if (output_detail.op_code == OPCODE_BNZ
                                || output_detail.op_code == OPCODE_BZ) {
                                output_detail.literal_data = i_sub;
                            }
                        }
                    }
                    if (i == 2) {
                        char sub[strlen(split_content[i]) - 1];
                        sub_string(split_content[i], sub, 1, (int) (strlen(split_content[i]) - 1));
                        int i_sub = atoi(sub);
                        if (strncmp(split_content[i], "R", 1) == 0) {
                            if (output_detail.op_code == OPCODE_MUL || output_detail.op_code == OPCODE_ADD || output_detail.op_code == OPCODE_DIV ||
                                output_detail.op_code == OPCODE_SUB || output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR ||
                                output_detail.op_code == OPCODE_EXOR || output_detail.op_code == OPCODE_JAL) {

                                output_detail.src_reg_detail[src_cnt] = arch_reg_file.reg_R[i_sub];
                                src_cnt++;

                            } else if (output_detail.op_code == OPCODE_STORE
                                       || output_detail.op_code == OPCODE_LOAD) {
                                output_detail.src_reg_detail[src_cnt] = arch_reg_file.reg_R[i_sub];
                            }
                        }
                        if (strncmp(split_content[i], "#", 1) == 0) {
                            if (output_detail.op_code == OPCODE_MOVC
                                || output_detail.op_code == OPCODE_JUMP) {
                                output_detail.literal_data = i_sub;
                            }
                        }
                    }
                    if (i == 3) {
                        char sub[strlen(split_content[i]) - 1];
                        sub_string(split_content[i], sub, 1, (int) (strlen(split_content[i]) - 1));
                        int i_sub = atoi(sub);
                        if (strncmp(split_content[i], "R", 1) == 0) {
                            if (output_detail.op_code == OPCODE_MUL || output_detail.op_code == OPCODE_ADD || output_detail.op_code == OPCODE_DIV ||
                                output_detail.op_code == OPCODE_SUB || output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR ||
                                output_detail.op_code == OPCODE_EXOR) {

                                output_detail.src_reg_detail[src_cnt] = arch_reg_file.reg_R[i_sub];
                            }
                        }
                        if (strncmp(split_content[i], "#", 1) == 0) {
                            if (output_detail.op_code == OPCODE_STORE || output_detail.op_code == OPCODE_LOAD ||
                                output_detail.op_code == OPCODE_JAL) {

                                output_detail.literal_data = i_sub;
                            }
                        }
                    }
                }

                // Checking STALL condition for Physical Registers availability
                if (!decode_rf_stalled) {

                    // Temporary Variable Declaration
                    int src1_arch_index = output_detail.src_reg_detail[0].reg_index;
                    int src2_arch_index = output_detail.src_reg_detail[1].reg_index;
                    int dest_arch_index;
                    int src1_phy_index = -1, src2_phy_index = -1, dest_phy_index = -1, rob_index = -1, lsq_index = -1, cfio_index = -1;
                    char src1_str[10], src2_str[10], dest_str[10], literal_str[10], opcode_str[10] = "\0";
                    bool src1_from_ARF = false;
                    bool src2_from_ARF = false;

                    if (output_detail.op_code != OPCODE_STORE && output_detail.op_code != OPCODE_JUMP && output_detail.op_code != OPCODE_HALT &&
                        output_detail.op_code != OPCODE_BNZ && output_detail.op_code != OPCODE_BZ) {
                        dest_arch_index = output_detail.dest_reg_detail[0].reg_index;
                    } else {
                        dest_arch_index = -1;
                    }

                    if (reg_alias_table[dest_arch_index].reg_type == 'P') {
                        dest_phy_index = reg_alias_table[dest_arch_index].reg_index;
                        output_detail.phy_reg_address = dest_phy_index;
                    }

                    strcpy(src_1_status, REG_STATUS_INVALID);
                    strcpy(src_2_status, REG_STATUS_INVALID);
                    strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_INVALID);
                    strcpy(output_detail.src_reg_detail[1].reg_status, REG_STATUS_INVALID);

                    // Logic to consume value of source registers from either Architecture Reg File, Physical Reg File or Forwarding Bus
                    if (output_detail.op_code == OPCODE_MUL || output_detail.op_code == OPCODE_ADD || output_detail.op_code == OPCODE_DIV ||
                        output_detail.op_code == OPCODE_SUB || output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR ||
                        output_detail.op_code == OPCODE_EXOR || output_detail.op_code == OPCODE_STORE) {

                        // Situation when both SRC1 and DEST arch registers are same
                        if (src1_arch_index == dest_arch_index && old_dest_reg_type == 'R') {
                            src1_from_ARF = true;
                        }

                        // Situation when both SRC1 and DEST arch registers are same
                        if (src2_arch_index == dest_arch_index && old_dest_reg_type == 'R') {
                            src2_from_ARF = true;
                        }

                        // Architecture Register File
                        if (reg_alias_table[src1_arch_index].reg_type == 'R') {
                            strcpy(src_1_status, arch_reg_file.reg_R[src1_arch_index].reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_arch_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = arch_reg_file.reg_R[src1_arch_index].reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        } else if (src1_from_ARF) {
                            output_detail.src_reg_detail[0] = arch_reg_file.reg_R[src1_arch_index];
                            strcpy(src_1_status, REG_STATUS_VALID);
                        }
                        if (reg_alias_table[src2_arch_index].reg_type == 'R') {
                            strcpy(src_2_status, arch_reg_file.reg_R[src2_arch_index].reg_status);
                            output_detail.src_reg_detail[1].reg_index = src2_arch_index;
                            if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[1].reg_value = arch_reg_file.reg_R[src2_arch_index].reg_value;
                                strcpy(output_detail.src_reg_detail[1].reg_status, REG_STATUS_VALID);
                            }
                        } else if (src2_from_ARF) {
                            output_detail.src_reg_detail[1] = arch_reg_file.reg_R[src2_arch_index];
                            strcpy(src_2_status, REG_STATUS_VALID);
                        }

                        // Physical Register File
                        if (strcmp(src_1_status, REG_STATUS_INVALID) == 0 && reg_alias_table[src1_arch_index].reg_type == 'P') {

                            // Situation when both SRC1 and DEST arch registers are same
                            if (src1_arch_index == dest_arch_index) {
                                src1_phy_index = old_dest_reg_index;
                            } else {
                                src1_phy_index = reg_alias_table[src1_arch_index].reg_index;
                            }
                            strcpy(src_1_status, phy_reg_file[src1_phy_index].reg_P.reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_phy_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = phy_reg_file[src1_phy_index].reg_P.reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        }
                        if (strcmp(src_2_status, REG_STATUS_INVALID) == 0 && reg_alias_table[src2_arch_index].reg_type == 'P') {

                            // Situation when both SRC2 and DEST arch registers are same
                            if (src2_arch_index == dest_arch_index) {
                                src2_phy_index = old_dest_reg_index;
                            } else {
                                src2_phy_index = reg_alias_table[src2_arch_index].reg_index;
                            }
                            strcpy(src_2_status, phy_reg_file[src2_phy_index].reg_P.reg_status);
                            output_detail.src_reg_detail[1].reg_index = src2_phy_index;
                            if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[1].reg_value = phy_reg_file[src2_phy_index].reg_P.reg_value;
                                strcpy(output_detail.src_reg_detail[1].reg_status, REG_STATUS_VALID);
                            }
                        }

                        // Forwarding Result Bus
                        if (strcmp(src_1_status, REG_STATUS_INVALID) == 0) {
                            strcpy(src_1_status, result_bus.reg_P[src1_phy_index].reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_phy_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = result_bus.reg_P[src1_phy_index].reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        }
                        if (strcmp(src_2_status, REG_STATUS_INVALID) == 0) {
                            strcpy(src_2_status, result_bus.reg_P[src2_phy_index].reg_status);
                            output_detail.src_reg_detail[1].reg_index = src2_phy_index;
                            if (strcmp(src_2_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[1].reg_value = result_bus.reg_P[src2_phy_index].reg_value;
                                strcpy(output_detail.src_reg_detail[1].reg_status, REG_STATUS_VALID);
                            }
                        }

                    } else if (output_detail.op_code == OPCODE_LOAD || output_detail.op_code == OPCODE_JAL || output_detail.op_code == OPCODE_JUMP) {

                        // Situation when both SRC1 and DEST arch registers are same
                        if (src1_arch_index == dest_arch_index && old_dest_reg_type == 'R') {
                            src1_from_ARF = true;
                        }

                        // Architecture Register File
                        if (reg_alias_table[src1_arch_index].reg_type == 'R') {
                            strcpy(src_1_status, arch_reg_file.reg_R[src1_arch_index].reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_arch_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = arch_reg_file.reg_R[src1_arch_index].reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        } else if (src1_from_ARF) {
                            output_detail.src_reg_detail[0] = arch_reg_file.reg_R[src1_arch_index];
                            strcpy(src_1_status, REG_STATUS_VALID);
                        }

                        // Physical Register File
                        if (strcmp(src_1_status, REG_STATUS_INVALID) == 0 && reg_alias_table[src1_arch_index].reg_type == 'P') {

                            // Situation when both SRC1 and DEST arch registers are same
                            if (src1_arch_index == dest_arch_index) {
                                src1_phy_index = old_dest_reg_index;
                            } else {
                                src1_phy_index = reg_alias_table[src1_arch_index].reg_index;
                            }
                            strcpy(src_1_status, phy_reg_file[src1_phy_index].reg_P.reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_phy_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = phy_reg_file[src1_phy_index].reg_P.reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        }

                        // Forwarding Result Bus
                        if (strcmp(src_1_status, REG_STATUS_INVALID) == 0) {
                            strcpy(src_1_status, result_bus.reg_P[src1_phy_index].reg_status);
                            output_detail.src_reg_detail[0].reg_index = src1_phy_index;
                            if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                output_detail.src_reg_detail[0].reg_value = result_bus.reg_P[src1_phy_index].reg_value;
                                strcpy(output_detail.src_reg_detail[0].reg_status, REG_STATUS_VALID);
                            }
                        }
                    }

                    // Check for PSW Zero Flag Physical Register in PRF and Forwarding Bus
                    char psw_phy_reg_status[10] = "\0";
                    int psw_phy_reg_index = psw_zero_flag_phy_reg.reg_index;
                    strcpy(psw_phy_reg_status, psw_zero_flag_phy_reg.reg_status);
                    if (psw_phy_reg_index != -1 && strcmp(psw_phy_reg_status, REG_STATUS_INVALID) == 0) {

                        // Check in Physical Register File
                        strcpy(psw_phy_reg_status, phy_reg_file[psw_phy_reg_index].reg_P.reg_status);
                        if (strcmp(psw_phy_reg_status, REG_STATUS_VALID) == 0) {
                            psw_zero_flag_phy_reg.reg_value = phy_reg_file[psw_phy_reg_index].reg_P.reg_value;
                            strcpy(psw_zero_flag_phy_reg.reg_status, REG_STATUS_VALID);
                        }

                        // Check in Forwarding Result Bus
                        if (strcmp(psw_phy_reg_status, REG_STATUS_INVALID) == 0) {
                            strcpy(psw_phy_reg_status, result_bus.reg_P[psw_phy_reg_index].reg_status);
                            if (strcmp(psw_phy_reg_status, REG_STATUS_VALID) == 0) {
                                psw_zero_flag_phy_reg.reg_value = result_bus.reg_P[psw_phy_reg_index].reg_value;
                                strcpy(psw_zero_flag_phy_reg.reg_status, REG_STATUS_VALID);
                            }
                        }
                    }

                    // Convert Instruction String
                    get_opcode_string(output_detail.op_code, opcode_str);
                    sprintf(literal_str, "#%d", output_detail.literal_data);
                    if (src1_phy_index != -1) {
                        sprintf(src1_str, "P%d", src1_phy_index);
                    } else {
                        sprintf(src1_str, "R%d", src1_arch_index);
                    }
                    if (src2_phy_index != -1) {
                        sprintf(src2_str, "P%d", src2_phy_index);
                    } else {
                        sprintf(src2_str, "R%d", src2_arch_index);
                    }
                    if (dest_phy_index != -1) {
                        sprintf(dest_str, "P%d", dest_phy_index);
                    } else {
                        sprintf(dest_str, "R%d", dest_arch_index);
                    }

                    // Renamed Instruction - For display purpose
                    if (output_detail.op_code == OPCODE_MUL || output_detail.op_code == OPCODE_ADD || output_detail.op_code == OPCODE_DIV ||
                        output_detail.op_code == OPCODE_SUB || output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR ||
                        output_detail.op_code == OPCODE_EXOR) {
                        sprintf(output_detail.updated_i_content, "%s,%s,%s,%s", opcode_str, dest_str, src1_str, src2_str);
                    } else if (output_detail.op_code == OPCODE_MOVC) {
                        sprintf(output_detail.updated_i_content, "%s,%s,%s", opcode_str, dest_str, literal_str);
                    } else if (output_detail.op_code == OPCODE_STORE) {
                        sprintf(output_detail.updated_i_content, "%s,%s,%s,%s", opcode_str, src1_str, src2_str, literal_str);
                    } else if (output_detail.op_code == OPCODE_LOAD || output_detail.op_code == OPCODE_JAL) {
                        sprintf(output_detail.updated_i_content, "%s,%s,%s,%s", opcode_str, dest_str, src1_str, literal_str);
                    } else if (output_detail.op_code == OPCODE_JUMP) {
                        sprintf(output_detail.updated_i_content, "%s,%s,%s", opcode_str, src1_str, literal_str);
                    } else if (output_detail.op_code == OPCODE_BNZ || output_detail.op_code == OPCODE_BZ) {
                        sprintf(output_detail.updated_i_content, "%s,%s", opcode_str, literal_str);
                    } else if (output_detail.op_code == OPCODE_HALT) {
                        sprintf(output_detail.updated_i_content, "%s", opcode_str);
                    }
                    strcpy(drf_stage.updated_inst_content, output_detail.updated_i_content);

                    // Create new CFIO entry
                    if (output_detail.op_code == OPCODE_JUMP || output_detail.op_code == OPCODE_JAL
                        || output_detail.op_code == OPCODE_BNZ || output_detail.op_code == OPCODE_BZ) {

                        if (!is_CFIO_queue_full()) {
                            CFIO_Entry new_cfio_entry;
                            new_cfio_entry.current_cfid = last_control_flow_inst;
                            new_cfio_entry.branch_inst_detail = output_detail;

                            // Insert new entry in ROB Queue
                            enqueue_CFIO_entry(new_cfio_entry);

                            cfio_index = cfio_queue.cfio_tail;
                            cfio_queue.cfio_entry[cfio_queue.cfio_tail].next_cfid = cfio_index;

                            decode_rf_stalled = false;
                        } else {
                            printf(RED"\n\nSTALL BECAUSE CFIO IS FULL\n\n"RST);
                            // Stall since CFIO Queue is full
                            decode_rf_stalled = true;
                        }
                    }

                    // Checking STALL condition for CFIO availability
                    if (!decode_rf_stalled) {

                        // Create new ROB entry in Program Order
                        if (!is_ROB_queue_full()) {
                            ROB_Entry new_rob_entry;
                            new_rob_entry.rob_status = 1; // Invalid
                            new_rob_entry.pc_value = output_detail.pc_value;
                            new_rob_entry.dest_arch_reg_address = dest_arch_index;
                            new_rob_entry.dest_phy_reg_address = dest_phy_index;
                            new_rob_entry.ex_code = -1;
                            new_rob_entry.result = -1;
                            new_rob_entry.opcode = output_detail.op_code;
                            new_rob_entry.cfio_id = last_control_flow_inst;
                            new_rob_entry.previous_dest_reg_index = old_dest_reg_index;
                            new_rob_entry.previous_dest_reg_type = old_dest_reg_type;

                            new_rob_entry.current_ROB_inst_num = inst_num;
                            strcpy(new_rob_entry.current_ROB_content, output_detail.i_content);
                            strcpy(new_rob_entry.updated_ROB_content, output_detail.updated_i_content);

                            // Insert new entry in ROB Queue
                            enqueue_ROB_entry(new_rob_entry);

                            rob_index = rob_queue.rob_tail;

                            decode_rf_stalled = false;
                        } else {
                            printf(RED"\n\nSTALL BECAUSE ROB IS FULL\n\n"RST);
                            // Stall since ROB Queue is full
                            decode_rf_stalled = true;
                        }

                        // Checking STALL condition for ROB availability or in case HALT instruction is decoded
                        if (!decode_rf_stalled && !is_halt_inst_found) {

                            // Create new LSQ entry in Program Order
                            if (output_detail.op_code == OPCODE_LOAD || output_detail.op_code == OPCODE_STORE) {
                                if (!is_LSQ_queue_full()) {
                                    LSQ_Entry new_lsq_entry;
                                    new_lsq_entry.lsq_status = 1; // Invalid
                                    new_lsq_entry.opcode = output_detail.op_code;
                                    new_lsq_entry.mem_address_status = 1; // Invalid
                                    new_lsq_entry.mem_address_result = -1;
                                    if (output_detail.op_code == OPCODE_LOAD) {
                                        new_lsq_entry.dest_arch_reg_address = dest_arch_index;
                                        new_lsq_entry.dest_phy_reg_address = dest_phy_index;
                                    } else {
                                        new_lsq_entry.dest_arch_reg_address = -1;
                                        new_lsq_entry.dest_phy_reg_address = -1;
                                    }
                                    if (output_detail.op_code == OPCODE_STORE) {
                                        if (strcmp(src_1_status, REG_STATUS_VALID) == 0) {
                                            new_lsq_entry.src_status = 0; // Valid
                                            new_lsq_entry.src_value = output_detail.src_reg_detail[0].reg_value;
                                        } else {
                                            new_lsq_entry.src_status = 1; // Invalid
                                            new_lsq_entry.src_value = -1;
                                        }
                                        new_lsq_entry.src_arch_reg_address = src1_arch_index;
                                        new_lsq_entry.src_phy_reg_address = src1_phy_index;
                                    } else {
                                        new_lsq_entry.src_status = -1;
                                        new_lsq_entry.src_arch_reg_address = -1;
                                        new_lsq_entry.src_phy_reg_address = -1;
                                        new_lsq_entry.src_value = -1;
                                    }
                                    new_lsq_entry.rob_index = rob_index;
                                    new_lsq_entry.cfio_id = last_control_flow_inst;

                                    new_lsq_entry.current_LSQ_inst_num = inst_num;
                                    strcpy(new_lsq_entry.current_LSQ_content, output_detail.i_content);
                                    strcpy(new_lsq_entry.updated_LSQ_content, output_detail.updated_i_content);

                                    new_lsq_entry.is_load_bypassing_possible = false;
                                    new_lsq_entry.is_load_bypassed = false;

                                    // Insert new entry in LSQ Queue
                                    enqueue_LSQ_entry(new_lsq_entry);

                                    lsq_index = lsq_queue.lsq_tail;

                                    decode_rf_stalled = false;
                                } else {
                                    printf(RED"\n\nSTALL BECAUSE LSQ IS FULL\n\n"RST);
                                    // Stall since LSQ Queue is full
                                    decode_rf_stalled = true;
                                }
                            }

                            // Checking STALL condition for LSQ availability
                            if (!decode_rf_stalled) {

                                // Create new IQ entry for current instruction
                                for (i = 0; i < IQ_SIZE; i++) {
                                    if (issue_queue[i].iq_status == 0) {

                                        // Mark ARCH and PHY Destination Registers as INVALID
                                        if (output_detail.op_code == OPCODE_MUL || output_detail.op_code == OPCODE_ADD ||
                                            output_detail.op_code == OPCODE_DIV || output_detail.op_code == OPCODE_SUB ||
                                            output_detail.op_code == OPCODE_AND || output_detail.op_code == OPCODE_OR ||
                                            output_detail.op_code == OPCODE_EXOR || output_detail.op_code == OPCODE_LOAD ||
                                            output_detail.op_code == OPCODE_JAL || output_detail.op_code == OPCODE_MOVC) {

                                            strcpy(arch_reg_file.reg_R[dest_arch_index].reg_status, REG_STATUS_INVALID);
                                            strcpy(output_detail.dest_reg_detail[0].reg_status, REG_STATUS_INVALID);

                                            strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_INVALID);
                                        }

                                        // Logic to update PSW Zero Flag Registers by last arithmetic instruction before branch
                                        if (is_arithmetic_operation(output_detail.op_code)) {
                                            psw_zero_flag_phy_reg.reg_index = dest_phy_index;
                                            strcpy(psw_zero_flag_phy_reg.reg_status, phy_reg_file[dest_phy_index].reg_P.reg_status);

                                            psw_zero_flag_arch_reg.reg_index = dest_arch_index;
                                            strcpy(psw_zero_flag_arch_reg.reg_status, arch_reg_file.reg_R[dest_arch_index].reg_status);
                                        }

                                        // Setting ROB, LSQ and CFIO index to output latch
                                        output_detail.rob_index = rob_index;
                                        output_detail.lsq_index = lsq_index;
                                        output_detail.cfio_id = last_control_flow_inst;

                                        // Creating IQ entry for current instruction
                                        issue_queue[i].iq_status = 1;
                                        issue_queue[i].op_code = output_detail.op_code;
                                        issue_queue[i].clk_cycle_dispatched = stats.clk_cycle + 1;
                                        issue_queue[i].dest_reg = output_detail.dest_reg_detail[0];
                                        issue_queue[i].src1_reg = output_detail.src_reg_detail[0];
                                        issue_queue[i].src2_reg = output_detail.src_reg_detail[1];
                                        if (output_detail.op_code == OPCODE_BNZ || output_detail.op_code == OPCODE_BZ) {
                                            issue_queue[i].bz_bnz_src_reg_detail = psw_zero_flag_phy_reg;
                                        }
                                        issue_queue[i].literal = output_detail.literal_data;
                                        issue_queue[i].inst_issued = false;
                                        issue_queue[i].rob_index = rob_index;
                                        issue_queue[i].lsq_index = lsq_index;
                                        issue_queue[i].cfio_id = last_control_flow_inst;

                                        issue_queue[i].current_IQ_inst_num = inst_num;
                                        strcpy(issue_queue[i].current_IQ_content, output_detail.i_content);
                                        strcpy(issue_queue[i].updated_IQ_content, output_detail.updated_i_content);

                                        // Set and Change value of global variable last_control_flow_inst
                                        if (cfio_index != -1 || output_detail.op_code == OPCODE_JUMP || output_detail.op_code == OPCODE_JAL
                                            || output_detail.op_code == OPCODE_BNZ || output_detail.op_code == OPCODE_BZ) {

                                            last_control_flow_inst = cfio_queue.cfio_tail;
                                        }

                                        // Copying temporary output to Stage Output Buffer
                                        drf_stage.output_inst_detail[inst_num] = output_detail;
                                        // Setting DRF Stage Input and Output Buffer Instruction Flags
                                        SET_DRF_STAGE_INPUT_INST_FLAGS;
                                        SET_DRF_STAGE_OUTPUT_INST_FLAGS;

                                        issue_queue[i].inst_detail = drf_stage.output_inst_detail[inst_num];
                                        issue_queue[i].inst_detail.decoded_inst_num = inst_num;
                                        issue_queue[i].inst_detail.inst_IQ_index = i;

                                        decode_rf_stalled = false;

                                        break;
                                    } else {
                                        // Stall since IQ entry is not free
                                        decode_rf_stalled = true;
                                    }
                                }
                            }
                        }
                    }
                }

                // Clearing Forwarding Bus
                for (int k = 0; k < PHY_REGISTER_LIMIT; k++) {
                    strcpy(result_bus.reg_P[k].reg_status, REG_STATUS_INVALID);
                }
            }
        }
    }

    return 0;
}

/*
 Implementation of Pipeline Fetch Stage
 */
int fetch_F() {

    // Clearing F stage printing current instruction content
    strcpy(f_stage.current_inst_content, "");
    strcpy(f_stage.updated_inst_content, "");

    if (f_stage.stall == true) {
        // DO NOTHING
        return 0;
    } else {

        // Variable Declaration and Initialization
        I_Detail input_detail, output_detail;
        int inst_num = inst_to_fetch;

        if (inst_num < inst_count_in_file) {

            // Read instruction from the Instruction Cache
            I_Content i_content = inst_cache.instructions[inst_num];
            f_stage.input_inst_detail[inst_num].inst_number = i_content.file_line_index;
            f_stage.input_inst_detail[inst_num].pc_value = i_content.file_line_address;
            strcpy(f_stage.input_inst_detail[inst_num].i_content, i_content.file_line_content);
            strcpy(f_stage.input_inst_detail[inst_num].updated_i_content, "");

            // Coping Stage Input Instruction in temporary variables
            input_detail = f_stage.input_inst_detail[inst_num];
            output_detail = input_detail;

            // Initializing F Stage display variables with currently processing instruction
            strcpy(f_stage.current_inst_content, output_detail.i_content);
            f_stage.current_inst_number = output_detail.inst_number;

            // Coping temporary output to Stage Output Buffer
            f_stage.output_inst_detail[inst_num] = output_detail;

            // Setting F Stage Input and Output Buffer Instruction Flags
            SET_F_STAGE_INPUT_INST_FLAGS;
            SET_F_STAGE_OUTPUT_INST_FLAGS;

            // Coping current stage output buffer instruction to the input buffer of next stage
            if (!decode_rf_stalled) {
                drf_stage.input_inst_detail[inst_num] = f_stage.output_inst_detail[inst_num];
                inst_to_fetch++;
            }
            drf_stage.stall = false;
        }
    }

    return 0;
}


/*
 INT Function Unit
 */
int int_FU_EX(int first_num, int second_num, int op_code) {

    // Logic to perform INT Function Unit operations
    switch (op_code) {
        case OPCODE_MOVC:
            return first_num + second_num;
        case OPCODE_STORE:
            return first_num + second_num;
        case OPCODE_LOAD:
            return first_num + second_num;
        case OPCODE_ADD:
            return first_num + second_num;
        case OPCODE_SUB:
            return first_num - second_num;
        case OPCODE_AND:
            return first_num & second_num;
        case OPCODE_OR:
            return first_num | second_num;
        case OPCODE_EXOR:
            return first_num ^ second_num;
        default:
            break;
    }

    return 0;
}

/*
 MUL1 Function Unit
 */
bool mul_FU_EX1() {

    return true;
}

/*
 MUL2 Function Unit
 */
int mul_FU_EX2(int first_num, int second_num) {

    return first_num * second_num;
}

/*
 DIV1 Function Unit
 */
bool div_FU_EX1() {

    return true;
}

/*
 DIV2 Function Unit
 */
bool div_FU_EX2() {

    return true;
}

/*
 DIV3 Function Unit
 */
bool div_FU_EX3() {

    return true;
}

/*
 DIV4 Function Unit
 */
int div_FU_EX4(int first_num, int second_num) {

    return first_num / second_num;
}

/*
 Function to flush previous instructions for JUMP
 */
int squash_instructions(I_Detail output_detail, int inst_num, int loop_counter) {

    // Variable Declaration and Initialization
    int i, p, q, dest_arch_index, dest_phy_index, cfio_head, cfio_tail;
    int branch_inst_cfio_id, rob_head, rob_tail, lsq_head, lsq_tail, next_cfio_id = -100, current_cfio_id = -100;
    bool is_reg_replaced = false;
    ROB_Entry current_rob_entry;

    // Find the next CFIO ID which we have to flush from other Data Structures
    branch_inst_cfio_id = output_detail.cfio_id;
    for (i = 0; i < CFIO_SIZE; i++) {
        if (cfio_queue.cfio_entry[i].current_cfid == branch_inst_cfio_id) {
            current_cfio_id = cfio_queue.cfio_entry[i].next_cfid;
            break;
        }
    }

    for (q = CFIO_SIZE - 1; q >= current_cfio_id; q--) {
        next_cfio_id = cfio_queue.cfio_entry[q].next_cfid;
        if (cfio_queue.cfio_entry[q].next_cfid == next_cfio_id && cfio_queue.cfio_entry[q].branch_inst_detail.pc_value >= 4000) {

            // Squash CFIO Entries
            cfio_head = cfio_queue.cfio_head;
            cfio_tail = cfio_queue.cfio_tail;
            while (cfio_tail != -1 && cfio_tail >= cfio_head) {
                if (cfio_queue.cfio_entry[cfio_queue.cfio_tail].current_cfid == next_cfio_id) {

                    // Clear CFIO current entry
                    cfio_queue.cfio_entry[cfio_queue.cfio_tail].current_cfid = -10;
                    cfio_queue.cfio_entry[cfio_queue.cfio_tail].next_cfid = -10;
                    cfio_display_queue.cfio_entry[cfio_queue.cfio_tail].current_cfid = -10;
                    cfio_display_queue.cfio_entry[cfio_queue.cfio_tail].next_cfid = -10;

                    // Modify CFIO tail to indicate this entry as free
                    if (cfio_queue.cfio_tail == 0 && cfio_queue.cfio_head > 0) {
                        cfio_queue.cfio_tail = CFIO_SIZE - 1;
                    } else if (cfio_queue.cfio_tail == 0 && cfio_queue.cfio_head == 0) {
                        cfio_queue.cfio_tail = -1;
                        cfio_queue.cfio_head = -1;
                        last_control_flow_inst = -1;
                    } else {
                        cfio_queue.cfio_tail -= 1;
                    }
                }
                cfio_tail--;
            }

            // Squash ROB Entries, Deallocate Physical Register and Modify RAT state
            rob_head = rob_queue.rob_head;
            rob_tail = rob_queue.rob_tail;
            is_reg_replaced = false;
            while (rob_tail != -1 && rob_tail >= rob_head) {
                if (rob_queue.rob_entry[rob_tail].cfio_id == next_cfio_id) {
                    current_rob_entry = rob_queue.rob_entry[rob_tail];

                    // Reset flag for HALT instruction
                    if (current_rob_entry.opcode == OPCODE_HALT) {
                        is_halt_inst_found = false;
                    } else if (current_rob_entry.opcode != OPCODE_STORE && current_rob_entry.opcode != OPCODE_JUMP &&
                               current_rob_entry.opcode != OPCODE_BNZ && current_rob_entry.opcode != OPCODE_BZ) {
                        // Modify RAT state
                        dest_arch_index = current_rob_entry.dest_arch_reg_address;
                        dest_phy_index = current_rob_entry.dest_phy_reg_address;

                        /*if (committed_registers[0].is_committed) {
                            if (committed_registers[0].arch_reg_address == dest_arch_index &&
                                (current_rob_entry.previous_dest_reg_type == 'P' &&
                                 committed_registers[0].phy_reg_address == current_rob_entry.previous_dest_reg_index)) {

                                reg_alias_table[dest_arch_index].reg_index = committed_registers[0].arch_reg_address;
                                reg_alias_table[dest_arch_index].reg_type = 'R';
                            }
                        }*/

                        // Check for Committed Register from ROB in current clock cycle
                        if (committed_registers[0].is_committed && committed_registers[0].arch_reg_address == dest_arch_index) {
                            reg_alias_table[dest_arch_index].reg_index = committed_registers[0].arch_reg_address;
                            reg_alias_table[dest_arch_index].reg_type = 'R';
                            is_reg_replaced = true;
                        }
                        if (committed_registers[1].is_committed && committed_registers[1].arch_reg_address == dest_arch_index) {
                            reg_alias_table[dest_arch_index].reg_index = committed_registers[1].arch_reg_address;
                            reg_alias_table[dest_arch_index].reg_type = 'R';
                            is_reg_replaced = true;
                        }

                        if (!is_reg_replaced) {
                            reg_alias_table[dest_arch_index].reg_index = current_rob_entry.previous_dest_reg_index;
                            reg_alias_table[dest_arch_index].reg_type = current_rob_entry.previous_dest_reg_type;
                        }

                        // Change status of ARF or PRF register according to previous register type
                        if (current_rob_entry.previous_dest_reg_type == 'R') {
                            strcpy(arch_reg_file.reg_R[current_rob_entry.previous_dest_reg_index].reg_status, REG_STATUS_VALID);

                        } else if (current_rob_entry.previous_dest_reg_type == 'P') {
                            strcpy(phy_reg_file[current_rob_entry.previous_dest_reg_index].reg_P.reg_status, REG_STATUS_VALID);
                        }

                        // Deallocate current Physical Register in PRF
                        phy_reg_file[dest_phy_index].phy_reg_status = 0; // Free
                        strcpy(phy_reg_file[dest_phy_index].reg_P.reg_status, REG_STATUS_VALID);
                    }

                    // Clear current ROB contents
                    rob_queue.rob_entry[rob_queue.rob_tail].rob_status = -1;
                    strcpy(rob_queue.rob_entry[rob_queue.rob_tail].current_ROB_content, "");
                    strcpy(rob_queue.rob_entry[rob_queue.rob_tail].updated_ROB_content, "");
                    rob_display_queue.rob_entry[rob_queue.rob_tail].rob_status = -1;
                    strcpy(rob_display_queue.rob_entry[rob_queue.rob_tail].current_ROB_content, "");
                    strcpy(rob_display_queue.rob_entry[rob_queue.rob_tail].updated_ROB_content, "");

                    // Modify ROB tail to indicate this entry as free
                    if (rob_queue.rob_tail == 0 && rob_queue.rob_head > 0) {
                        rob_queue.rob_tail = ROB_SIZE - 1;
                    } else if (rob_queue.rob_tail == 0 && rob_queue.rob_head == 0) {
                        rob_queue.rob_tail = -1;
                        rob_queue.rob_head = -1;
                    } else {
                        rob_queue.rob_tail -= 1;
                    }
                }
                rob_tail--;
            }

            // Squash LSQ Entries
            lsq_head = lsq_queue.lsq_head;
            lsq_tail = lsq_queue.lsq_tail;
            while (lsq_tail != -1 && lsq_tail >= lsq_head) {
                if (lsq_queue.lsq_entry[lsq_tail].cfio_id == next_cfio_id) {

                    // Clear current LSQ contents
                    lsq_queue.lsq_entry[lsq_queue.lsq_tail].lsq_status = -1;
                    lsq_queue.lsq_entry[lsq_queue.lsq_tail].src_status = -1;
                    lsq_queue.lsq_entry[lsq_queue.lsq_tail].mem_address_status = -1;
                    strcpy(lsq_queue.lsq_entry[lsq_queue.lsq_tail].current_LSQ_content, "");
                    strcpy(lsq_queue.lsq_entry[lsq_queue.lsq_tail].updated_LSQ_content, "");
                    lsq_display_queue.lsq_entry[lsq_queue.lsq_tail].lsq_status = -1;
                    lsq_display_queue.lsq_entry[lsq_queue.lsq_tail].src_status = -1;
                    lsq_display_queue.lsq_entry[lsq_queue.lsq_tail].mem_address_status = -1;
                    strcpy(lsq_display_queue.lsq_entry[lsq_queue.lsq_tail].current_LSQ_content, "");
                    strcpy(lsq_display_queue.lsq_entry[lsq_queue.lsq_tail].updated_LSQ_content, "");

                    // Modify LSQ tail to indicate this entry as free
                    if (lsq_queue.lsq_tail == 0 && lsq_queue.lsq_head > 0) {
                        lsq_queue.lsq_tail = LSQ_SIZE - 1;
                    } else if (lsq_queue.lsq_tail == 0 && lsq_queue.lsq_head == 0) {
                        lsq_queue.lsq_tail = -1;
                        lsq_queue.lsq_head = -1;
                    } else {
                        lsq_queue.lsq_tail -= 1;
                    }
                }
                lsq_tail--;
            }

            // Squash IQ Entries
            for (p = 0; p < IQ_SIZE; p++) {
                if (issue_queue[p].cfio_id == next_cfio_id && issue_queue[p].iq_status == 1) {
                    issue_queue[p].iq_status = 0; // Free
                    issue_queue[p].clk_cycle_dispatched = 0;
                    strcpy(issue_queue[p].current_IQ_content, "");
                    strcpy(issue_queue[p].updated_IQ_content, "");
                    issue_display_queue[p].iq_status = 0; // Free
                    issue_display_queue[p].clk_cycle_dispatched = 0;
                    strcpy(issue_display_queue[p].current_IQ_content, "");
                    strcpy(issue_display_queue[p].updated_IQ_content, "");
                }
            }

            //next_cfio_id++;
        }
    }

    // Stall Fetch and Decode stages for current clock cycle
    f_stage.stall = true;
    drf_stage.stall = true;

    // Logic to filling NOP instructions and marking respective stage buffer flags
    for (i = inst_num + 1; i <= loop_counter; i++) {
        if (f_stage.input_inst_detail[i].pc_value != -1) {
            f_stage.input_inst_detail[i].pc_value = -1;
            f_stage.output_inst_detail[i].pc_value = -1;
            f_stage.input_inst_detail[i].op_code = OPCODE_NOP;
            f_stage.output_inst_detail[i].op_code = OPCODE_NOP;
            f_stage.input_inst_detail[i].is_F_done = false;
            f_stage.output_inst_detail[i].is_F_done = false;

            f_stage.input_inst_detail[i + 1].pc_value = -1;
            f_stage.output_inst_detail[i + 1].pc_value = -1;
            f_stage.input_inst_detail[i + 1].op_code = OPCODE_NOP;
            f_stage.output_inst_detail[i + 1].op_code = OPCODE_NOP;
            f_stage.input_inst_detail[i + 1].is_F_done = false;
            f_stage.output_inst_detail[i + 1].is_F_done = false;
        }
        if (drf_stage.input_inst_detail[i].pc_value != -1) {
            drf_stage.input_inst_detail[i].pc_value = -1;
            drf_stage.output_inst_detail[i].pc_value = -1;
            drf_stage.input_inst_detail[i].op_code = OPCODE_NOP;
            drf_stage.output_inst_detail[i].op_code = OPCODE_NOP;
            drf_stage.input_inst_detail[i].is_F_done = false;
            drf_stage.output_inst_detail[i].is_F_done = false;
            drf_stage.input_inst_detail[i].is_DRF_done = false;
            drf_stage.output_inst_detail[i].is_DRF_done = false;
            drf_stage.input_inst_detail[i + 1].is_F_done = false;

            // Cleaning FU (Execute Stage)
            if (ex_stage.input_inst_detail[i].pc_value != -1 && ex_stage.input_inst_detail[i].cfio_id == next_cfio_id) {
                ex_stage.input_inst_detail[i].is_F_done = false;
                ex_stage.input_inst_detail[i].is_DRF_done = false;
            }
        }
    }

    return 0;
}

/*
 This function returns true if ROB queue is empty
 */
bool is_ROB_queue_empty() {

    if (rob_queue.rob_head == -1) {
        return true;
    }

    return false;
}

/*
 This function returns true if ROB queue is Full
 */
bool is_ROB_queue_full() {

    if ((rob_queue.rob_head == 0 && rob_queue.rob_tail == ROB_SIZE - 1) || (rob_queue.rob_head == rob_queue.rob_tail + 1)) {
        return true;
    }

    return false;
}

/*
 This function is used to push ROB Entry inside ROB Queue
 */
int enqueue_ROB_entry(ROB_Entry rob_entry) {

    if (is_ROB_queue_full()) {
        return 0;
    }

    if (rob_queue.rob_tail == -1) {
        rob_queue.rob_tail += 1;
        rob_queue.rob_head += 1;
    } else if ((rob_queue.rob_head > 0) && (rob_queue.rob_tail == ROB_SIZE - 1)) {
        rob_queue.rob_tail = 0;
    } else {
        rob_queue.rob_tail += 1;
    }

    rob_queue.rob_entry[rob_queue.rob_tail] = rob_entry;

    return 0;
}

/*
 This function returns true if LSQ queue is empty
 */
bool is_LSQ_queue_empty() {

    if (lsq_queue.lsq_head == -1) {
        return true;
    }

    return false;
}

/*
 This function returns true if LSQ queue is Full
 */
bool is_LSQ_queue_full() {

    if ((lsq_queue.lsq_head == 0 && lsq_queue.lsq_tail == LSQ_SIZE - 1) || (lsq_queue.lsq_head == lsq_queue.lsq_tail + 1)) {
        return true;
    }

    return false;
}

/*
 This function is used to push LSQ Entry inside LSQ Queue
 */
int enqueue_LSQ_entry(LSQ_Entry lsq_entry) {

    if (is_LSQ_queue_full()) {
        return 0;
    }

    if (lsq_queue.lsq_tail == -1) {
        lsq_queue.lsq_tail += 1;
        lsq_queue.lsq_head += 1;
    } else if ((lsq_queue.lsq_head > 0) && (lsq_queue.lsq_tail == LSQ_SIZE - 1)) {
        lsq_queue.lsq_tail = 0;
    } else {
        lsq_queue.lsq_tail += 1;
    }

    lsq_queue.lsq_entry[lsq_queue.lsq_tail] = lsq_entry;

    return 0;
}

/*
 This function returns true if CFIO queue is Full
 */
bool is_CFIO_queue_full() {

    if ((cfio_queue.cfio_head == 0 && cfio_queue.cfio_tail == CFIO_SIZE - 1) || (cfio_queue.cfio_head == cfio_queue.cfio_tail + 1)) {
        return true;
    }

    return false;
}

/*
 This function is used to push CFIO Entry inside CFIO Queue
 */
int enqueue_CFIO_entry(CFIO_Entry cfio_entry) {

    if (is_CFIO_queue_full()) {
        return 0;
    }

    if (cfio_queue.cfio_tail == -1) {
        cfio_queue.cfio_tail += 1;
        cfio_queue.cfio_head += 1;
    } else if ((cfio_queue.cfio_head > 0) && (cfio_queue.cfio_tail == LSQ_SIZE - 1)) {
        cfio_queue.cfio_tail = 0;
    } else {
        cfio_queue.cfio_tail += 1;
    }

    cfio_queue.cfio_entry[cfio_queue.cfio_tail] = cfio_entry;

    return 0;
}

/*
 Function to check arithmetic operation
 */
bool is_arithmetic_operation(int op_code) {

    if (op_code == OPCODE_MUL || op_code == OPCODE_ADD || op_code == OPCODE_SUB || op_code == OPCODE_DIV) {
        return true;
    }
    return false;
}

/*
 Function to read the file and load Instruction Memory
 */
I_Cache load_instructions_from_file() {

    // Variable Declaration and Initialization
    int i;
    int address = BASE_ADDRESS;
    FILE *file_ptr;
    char file_content[100];
    I_Cache inst_list = {};

    // Opening file in Read Mode
    file_ptr = fopen(file_full_path, "r");

    // Initializing file line index with -1 by default
    for (i = 0; i < 100; i++) {
        inst_list.instructions[i].file_line_index = -1;
    }

    if (file_ptr == NULL) {
        printf(RED"\nERROR ::: Could not open the file -- [ File Does Not Exist ] ... !!"RST"\n\n\n");
        inst_count_in_file = -1;
    } else {
        for (i = 0; fgets(file_content, 100, file_ptr) != NULL; i++) {
            inst_list.instructions[i].file_line_index = i;
            inst_list.instructions[i].file_line_address = address;
            strcpy(inst_list.instructions[i].file_line_content, file_content);
            if (inst_list.instructions[i].file_line_content[strlen(inst_list.instructions[i].file_line_content) - 1] == '\n') {
                inst_list.instructions[i].file_line_content[strlen(inst_list.instructions[i].file_line_content) - 1] = '\0';
            }
            address += BYTE_OFFSET;
            inst_count_in_file++;
        }
        fclose(file_ptr);
    }

    return inst_list;
}

/*
 Function to split string using delimeter
 */
int split_string(char input[], char *output[]) {

    // Variable Declaration and Initialization
    int i = 0;
    char *delimeter = ",";
    char *token_ptr = strtok(input, delimeter);

    while (token_ptr != NULL) {
        output[i] = token_ptr;
        i++;
        token_ptr = strtok(NULL, delimeter);
    }

    return i;
}

/*
 Function to sub string
 */
int sub_string(char actual_string[], char new_string[], char sub_offset, int sub_length) {

    if (sub_offset < strlen(actual_string)) {
        if (sub_offset + sub_length <= strlen(actual_string)) {
            strncpy(new_string, actual_string + sub_offset, (size_t) sub_length);
            new_string[sub_length] = 0;
            strcpy(actual_string + sub_offset, actual_string + sub_offset + sub_length);
        }
    }
    return 0;
}

/*
 Function to handle the loop for HALT operation
 */
bool exit_for_halt_instruction() {

    for (int i = 0; i < inst_count_in_file; i++) {
        if (is_halt_inst_found) {
            if (is_halt_retired_from_ROB) {
                return true;
            }
        }
    }

    return false;
}

/*
 This function provides OPCODE STRING from OPCODE NUMBER
 */
int get_opcode_string(int op_code, char opcode_str[]) {
    switch (op_code) {
        case OPCODE_MOVC:
            strcpy(opcode_str, INST_MOVC);
            break;
        case OPCODE_STORE:
            strcpy(opcode_str, INST_STORE);
            break;
        case OPCODE_LOAD:
            strcpy(opcode_str, INST_LOAD);
            break;
        case OPCODE_MUL:
            strcpy(opcode_str, INST_MUL);
            break;
        case OPCODE_ADD:
            strcpy(opcode_str, INST_ADD);
            break;
        case OPCODE_SUB:
            strcpy(opcode_str, INST_SUB);
            break;
        case OPCODE_AND:
            strcpy(opcode_str, INST_AND);
            break;
        case OPCODE_OR:
            strcpy(opcode_str, INST_OR);
            break;
        case OPCODE_EXOR:
            strcpy(opcode_str, INST_EXOR);
            break;
        case OPCODE_JUMP:
            strcpy(opcode_str, INST_JUMP);
            break;
        case OPCODE_BNZ:
            strcpy(opcode_str, INST_BNZ);
            break;
        case OPCODE_BZ:
            strcpy(opcode_str, INST_BZ);
            break;
        case OPCODE_HALT:
            strcpy(opcode_str, INST_HALT);
            break;
        case OPCODE_DIV:
            strcpy(opcode_str, INST_DIV);
            break;
        case OPCODE_JAL:
            strcpy(opcode_str, INST_JAL);
            break;
        default:
            break;
    }
    return 0;
}