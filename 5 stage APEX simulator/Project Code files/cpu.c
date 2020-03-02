/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  bhanu dobriyal (bdobriy1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES = 1;
int Halt = 0;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;

  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "rs3" ,"imm");

   
      for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3 ,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d", stage->opcode, stage->rd, stage->imm);
  }
  if ((strcmp(stage->opcode, "ADD") == 0)||(strcmp(stage->opcode, "SUB") == 0)) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  
  if ((strcmp(stage->opcode, "SUBL") == 0) || (strcmp(stage->opcode, "ADDL") == 0)){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->imm);
  }
   
  if ((strcmp(stage->opcode, "LOAD") == 0)){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->imm);
  }
  if ((strcmp(stage->opcode, "LDR") == 0)){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
 
  if ((strcmp(stage->opcode, "STR") == 0)) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rs1,stage->rs2, stage->rs3);
  }
  if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "EXOR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  if ((strcmp(stage->opcode, "BZ") == 0)||(strcmp(stage->opcode, "BNZ") == 0)) {
        printf("%s,#%d ", stage->opcode, stage->imm);
    }
    if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s ", stage->opcode);
  }
  if (strcmp(stage->opcode, "JUMP") == 0) {
        printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
    }
  
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */

static void
print_stage_content(char* name, CPU_Stage* stage)

{ 
  
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}



/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu , int option)
{
  int new_variable_one = 0;
  CPU_Stage* stage = &cpu->stage[F];
  if (strcmp(stage->opcode, "HALT") == 0 ){
      
        cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;
    
        return 0;

    }
  if ((stage->busy == 0) && (stage->stalled == 0)&&(new_variable_one ==0)) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
  
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    if (strcmp(stage->opcode, "HALT") == 0) {
      new_variable_one = 1;
         cpu->stage[F].busy = 1;
         cpu->stage[F].stalled = 1;
         
        return 0;
   
    }else{
    cpu->stage[DRF] = cpu->stage[F];
    }
    if(option ==0)
   {
      print_stage_content("Fetch", stage);
   }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu , int option)
{
  CPU_Stage* stage = &cpu->stage[DRF];
 
if (strcmp(stage->opcode, "HALT") == 0) {
       cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;
           
    }
  
    if(cpu->stage[F].busy==1)
    {
      cpu->stage[DRF].busy = 1;
    }
  

 
   /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    if ((strcmp(stage->opcode, "ADD") == 0 ) || (strcmp(stage->opcode, "SUB") == 0)||(strcmp(stage->opcode, "OR") == 0)||(strcmp(stage->opcode, "AND") == 0)||(strcmp(stage->opcode, "EXOR") == 0)||(strcmp(stage->opcode, "MUL") == 0) )
    { if((cpu->regs_valid[stage->rs1_value]==0)&&(cpu->regs_valid[stage->rs2_value]==0)&&(cpu->regs_valid[stage->rs1]==0)&&(cpu->regs_valid[stage->rs2]==0))
      {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
       cpu->stage[DRF].busy = 0;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].busy = 0;
     
      }
      else
      {
      
        cpu->stage[F].busy = 1;
       cpu->stage[DRF].stalled = 1;
      }
      
    }
   if (strcmp(stage->opcode, "LOAD") == 0) {
     if((cpu->regs_valid[stage->rs1_value]==0)&&(cpu->regs_valid[stage->rs1]==0))
     {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->imm = stage->imm;
      cpu->stage[DRF].busy = 0;
      cpu->stage[DRF].stalled = 0;
       cpu->stage[F].busy = 0;
     }
     else
     {
        cpu->stage[F].busy = 1;
        cpu->stage[DRF].busy = 1;
        cpu->stage[DRF].stalled = 1;
     }
     
    }
      /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      if((cpu->regs_valid[stage->rs1_value]==0)&&(cpu->regs_valid[stage->rs2_value]==0)&&(cpu->regs_valid[stage->rs1]==0)&&(cpu->regs_valid[stage->rs2])==0)
      {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      stage->imm = stage->imm;
      cpu->stage[DRF].busy = 0;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].busy = 0;
      }
      else
      {
        cpu->stage[F].busy = 1;
         cpu->stage[DRF].busy = 1;
        cpu->stage[DRF].stalled = 1;
      }
      
    }
     if (strcmp(stage->opcode, "LDR") == 0) {
       if((cpu->regs_valid[stage->rs1_value]==0)&&(cpu->regs_valid[stage->rs2_value]==0)&&(cpu->regs_valid[stage->rs1]==0)&&(cpu->regs_valid[stage->rs2]==0))
       {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->stage[DRF].busy = 0;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].busy = 0;
       }
       else
       {
         cpu->stage[F].busy = 1;
          cpu->stage[DRF].busy = 1;
        cpu->stage[DRF].stalled = 1;
       }
       
     }
    if (strcmp(stage->opcode, "STR") == 0) {
      if((cpu->regs_valid[stage->rs1_value]==0)&&(cpu->regs_valid[stage->rs2_value]==0)&&(cpu->regs_valid[stage->rs3_value]==0)&&(cpu->regs_valid[stage->rs1]==0)&&(cpu->regs_valid[stage->rs2]==0)&&(cpu->regs_valid[stage->rs3]==0))
       {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      stage->rs3_value = cpu->regs[stage->rs3];
      cpu->stage[DRF].busy = 0;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].busy = 0;
       }
       else
       {
        cpu->stage[F].busy = 1;
        cpu->stage[DRF].busy = 1;
        cpu->stage[DRF].stalled = 1;
       }
      
    }
    
    if ((strcmp(stage->opcode, "BZ") == 0)||(strcmp(stage->opcode, "BZ") == 0)) {
       stage->imm = stage->imm;
    }
    if ((strcmp(stage->opcode, "JUMP") == 0)) {
      stage->rs1_value = cpu->regs[stage->rs1];
       stage->imm = stage->imm;
    }
 

    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX] = cpu->stage[DRF];
    if(option ==0){
      print_stage_content("Decode/RF", stage);
    }
  
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu,int option)
{
  CPU_Stage* stage = &cpu->stage[EX];
  
  if(strcmp(stage->opcode, "HALT") == 0)
  {
    
    cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;
    
    
  }
  if(cpu->stage[DRF].busy==1){
    cpu->stage[EX].busy=1;
  }

  if ((stage->busy == 0) && !stage->stalled) {

     /* Store */
     if (strcmp(stage->opcode, "STORE") == 0) {
     stage->buffer = stage->rs2_value + stage->imm;
    }
    if (strcmp(stage->opcode, "LOAD") == 0) {
    stage->buffer = stage->rs1_value + stage->imm;
     }
     if (strcmp(stage->opcode, "LDR") == 0) {
     stage->buffer = stage->rs1_value + stage->rs2_value;
     }
     if (strcmp(stage->opcode, "STR") == 0) {
    stage->buffer = stage->rs2_value + stage->rs3_value;
     }
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer = stage->imm + 0;
    }

    if (strcmp(stage->opcode, "ADD") == 0) {
      stage->buffer = stage->rs1_value + stage->rs2_value;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      stage->buffer = stage->rs1_value - stage->rs2_value;
    }
    if (strcmp(stage->opcode, "MUL") == 0) {
      stage->buffer = stage->rs1_value * stage->rs2_value;
    }
    if (strcmp(stage->opcode, "AND") == 0) {
      stage->buffer = stage->rs1_value & stage->rs2_value;
    }
    if (strcmp(stage->opcode, "OR") == 0) {
      stage->buffer = stage->rs1_value | stage->rs2_value;
    }
    if (strcmp(stage->opcode, "EXOR") == 0) {
      stage->buffer = stage->rs1_value ^ stage->rs2_value;
    }

    /* Copy data from Execute latch to Execute 2 latch*/
    cpu->stage[EX2] = cpu->stage[EX];
    if(option ==0)
   {
      print_stage_content("Execute1", stage);
    }
  }
  return 0;
}
int
execute2(APEX_CPU* cpu , int option)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if(strcmp(stage->opcode, "HALT") == 0)
  {
       cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;

  }
  if(cpu->stage[EX].busy==1){
    cpu->stage[EX2].busy=1;
  }
 
  if ((stage->busy == 0) && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
     
    }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX2];
    if(option ==0)
   {
      print_stage_content("Execute2", stage);
    }
  }
  return 0;
}
/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */ 
int
memory(APEX_CPU* cpu , int option)
{
  CPU_Stage* stage = &cpu->stage[MEM];
  if(strcmp(stage->opcode, "HALT") == 0)
  {
         cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;

  }
 if(cpu->stage[EX2].busy==1){
    cpu->stage[MEM].busy=1;
  }
  if ((stage->busy == 0)&& !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
     cpu->data_memory[stage->buffer] = stage->rs1_value;
    }
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }
     if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->rd = cpu->data_memory[stage->buffer];
    }
     if (strcmp(stage->opcode, "LDR") == 0) {
       stage->rd = cpu->data_memory[stage->buffer];
     }
   
     if (strcmp(stage->opcode, "STR") == 0) {
     cpu->data_memory[stage->buffer] = stage->rs1_value;
       
    }
    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM];
    if(option ==0)
  {
      print_stage_content("Memory1", stage);
    }
  }
  return 0;
}
int
memory2(APEX_CPU* cpu , int option)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if(strcmp(stage->opcode, "HALT") == 0)
  {cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;

  }
  if(cpu->stage[MEM].busy==1){
    cpu->stage[MEM2].busy=1;
  }
  if ((stage->busy == 0) && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }
    
    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];
    if(option ==0)
  {
      print_stage_content("Memory2", stage);
    }
  }
  return 0;
}
/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu , int option)
{
  CPU_Stage* stage = &cpu->stage[WB];
   if(strcmp(stage->opcode, "HALT") == 0)
  {
  cpu->stage[F].busy = 1;
          cpu->stage[F].stalled = 1;
  }
 if(cpu->stage[MEM2].busy==1){
    cpu->stage[WB].busy=1;
  }
  
  if ((stage->busy == 0)&& !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
        cpu->regs_valid[stage->rd] = 0;
      stage->rd_value = cpu->regs[stage->rd];
      cpu->regs_valid[stage->rd_value] = 0;
    }
    if ((strcmp(stage->opcode, "ADD") == 0) || (strcmp(stage->opcode, "SUB") == 0) || (strcmp(stage->opcode, "AND") == 0)|| (strcmp(stage->opcode, "OR") == 0)|(strcmp(stage->opcode, "EXOR") == 0)||(strcmp(stage->opcode, "MUL") == 0)){
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 0;
      stage->rd_value = cpu->regs[stage->rd]; 
       cpu->regs_valid[stage->rd_value] = 0;

    }
     if (strcmp(stage->opcode, "LOAD") == 0) {
       stage->rd_value = stage->rd;
        cpu->regs_valid[stage->rd] = 0;
        cpu->regs_valid[stage->rd_value] = 0;
   }
    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->rd_value = stage->rd;
       cpu->regs_valid[stage->rd] = 0;
        cpu->regs_valid[stage->rd_value] = 0;
    }
    
    if (strcmp(stage->opcode, "HALT") == 0) {
      cpu->ins_completed = cpu ->code_memory_size;
    }
    cpu->ins_completed++;
    if(option ==0)
   {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

void simulate(APEX_CPU* cpu)
{
 
  printf("\n");
  printf("\n");
  printf("=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");

    for (int i = 0; i < 16; i++) {
      if(cpu->regs_valid[i]==1)
      {
    printf("  | REG[%03d]  |  Value = %-3d  |  Status = %-3s  ",i , cpu->regs[i] ,"INVALID");
      printf("\n ");       
      }
    else
      {
      printf("  | REG[%03d]  |  Value = %-3d  |  Status = %-3s  ",i , cpu->regs[i] ,"VALID" );
      printf("\n ");  
      }
    }
  printf("\n");
  printf("\n");
  printf("============== STATE OF DATA MEMORY =============\n");

    for (int i = 0; i < 100; i++) {
    printf("  | REG[%02d]  | Data Value= %-2d  \n",i , cpu->data_memory[i] );
             
    }

   
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu , int cycles , int option)
{
  while (1) {

     if( cpu->clock == cycles)
        {
            break;
        }


    /* All the instructions committed, so exit */
    if (cpu->ins_completed >= cpu->code_memory_size) {
    
      printf("(apex) >> Simulation Complete");
      break;
    }
   if(option ==0)
    {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }
   
    writeback(cpu ,option);
    memory2(cpu , option);
    memory(cpu, option);
    execute2(cpu , option);
    execute(cpu , option);
    decode(cpu , option);
    fetch(cpu , option);
    cpu->clock++;
  }

  return 0;
}

