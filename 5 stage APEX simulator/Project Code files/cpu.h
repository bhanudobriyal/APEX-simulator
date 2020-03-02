#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author : Bhanu Dobriyal bdobriy1@binghamton.edu
 * 
 *  State University of New York, Binghamton
 */

enum
{
  F,
  DRF,
  EX,
  EX2,
  MEM,
  MEM2,
  WB,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128];	// Operation Code
  int rd;		    // Destination Register Address
  int rs1;		    // Source-1 Register Address
  int rs2;        // Source-2 Register Address
  int rs3;		    // Source-3 Register Address
  int imm;		    // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;		    // Program Counter
  char opcode[128];	// Operation Code
  int rs1;		    // Source-1 Register Address
  int rs2;       // Source-2 Register Address
  int rs3;		   //Source-3 Register Address 
  int rd;		    // Destination Register Address
  int imm;		    // Literal Value
  int rs1_value;	// Source-1 Register Value
  int rs2_value;	// Source-2 Register Value
  int rs3_value; //Source-3 Register Value
  int rd_value; //Destination Register Value
  int buffer;		// Latch to hold some value
  int mem_address;	// Computed Memory Address
  int busy;		    // Flag to indicate, stage is performing some action
  int stalled;		// Flag to indicate, stage is stalled
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;

  /* Integer register file */
  int regs[32];
  int regs_valid[32];

  /* Array of 7 CPU_stage */
  CPU_Stage stage[7];

  /* Code Memory where instructions are stored */
  APEX_Instruction* code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;
  
  

} APEX_CPU;

APEX_Instruction*
create_code_memory(const char* filename, int* size);

APEX_CPU*
APEX_cpu_init(const char* filename);
int display(int variable);
int
APEX_cpu_run(APEX_CPU* cpu , int cycles , int option);
void
simulate(APEX_CPU*);



void
APEX_cpu_stop(APEX_CPU* cpu);

int
fetch(APEX_CPU* cpu,int option);

int
decode(APEX_CPU* cpu , int option);

int
execute(APEX_CPU* cpu , int option);
int
execute2(APEX_CPU* cpu , int option);
int
memory(APEX_CPU* cpu , int option);
int
memory2(APEX_CPU* cpu , int option);
int
writeback(APEX_CPU* cpu ,int option);

#endif
