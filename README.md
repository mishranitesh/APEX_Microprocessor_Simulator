# APEX (A Pipeline Example) Microprocessor Simulator

This document include high level information about implementation of APEX pipeline with following features:

## Brief Information:

* Multiple latency function units (FU) for “MUL and DIV” operation types.
* Implementation of Jump-and-Link (JAL) instruction.
* Simple Interlocking Logic to handle Data Dependencies (Flow & Output).
* Dependency over Zero Flag for conditional branch instructions (BZ and BNZ).
* Data Forwarding is implemented from Execute stage (INT, MUL2 and DIV4 FU).
* PSW Flag forwarding is implemented for arithmetic instruction from EX stage.
* Conflict management to access MEM stage from EX stage as per the priorities.
* Input and Output buffers are used to move data from one stage to another stage.
* Flush mechanism is supported to add NOP and to stall stages as needed.

## Operation Types and Pipeline Diagram with Pipelined FU:

Below operations types are supported in this pipeline implementation:

* ADD
* MUL
* SUB
* DIV
* AND
* OR
* EXOR
* LOAD
* STORE
* MOVC
* JUMP
* JAL
* BZ
* BNZ
* HALT

## Stages of APEX Pipeline:

There are total of 6 stages in the pipeline, that are as follows:

* Fetch Stage
* Decode / RegisterFetchStage
* Issue Queue
* Execute Stage (INT + MUL1 → MUL2 + DIV1 → DIV2 → DIV2 → DIV4)
* LSQCommit
* ROBRetirement

## Important Functions in APEX Software Simulator:

* “initialize_simulator()” function:
	* Initialization of Architecture Register File and Data Memory with 0 as value.
	* Initialization of Pipeline Statistics and Stage information.
	* Reading instructions from file and loading these into Instruction Cache.
	* Initialization of PSW Zero flag status and value.

* “simulate_simulator()” function:
	* Calling pipeline basic functions:
		* retirement_from_ROB().
		* commitment_from_LSQ()
		* execute_EX()
		* decode_DRF()
		* fetch_F()
	* Handling stalling conditions for branch instructions.
	* Increasing clock cycle for pipeline statistics.

* “display_simulator()” function:
	* Printing instructions present in every stage.
	* Printing state of architecture registers.
	* Printing state of data memory.

## Data Structures:

* Instruction Cache/ Memory:
	* This structure is used to hold all the instructions present in the input file.
	* During initialization of program, file will be read and instructions will be loaded in the I_Cache memory using load_instructions_from_file() function.

* Data Cache/ Memory:
	* This structure is used to hold all the memory results used in the program.
	* Instructions like LOAD and STORE generally uses this memory.

* Architectural Register File:
	* This structure is used to hold the information of architectural registers in the program.

* Instruction Details:
	* This structure holds all the useful information related to an instruction.
	* Flags “is_XX_done” is used to indicate, respective instruction has completed in its stage.
	* String “i_content” holds the complete string instructions.
	* Information about source and destination registers are present in this structure.

* Stage Information:
	* This structure holds buffer input and output instruction of each stage of the pipeline.

* Forwarding Result Bus:
	* This structure holds forwarding bus registers information.

* Issue Queue
* Reorder Buffer (ROB)
* Load-Store Queue (LSQ)

#### NITESH MISHRA
##### mishra.nitesh@outlook.com
