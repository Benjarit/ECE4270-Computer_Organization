#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "mu-mips.h"


/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	/*IMPLEMENT THIS*/
	
	
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	uint32_t location, value, value1;

	switch(EX_MEM.opcode){
		case 0x00000020:
			// LB
			if(EX_MEM.left){
				printf("# LB\n");
				EX_MEM.imm = (EX_MEM.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | EX_MEM.imm : EX_MEM.imm;
				value = EX_MEM.A + EX_MEM.imm;
				value1 = mem_read_32(value) & 0x000000FF;
				value1 = (value1 & 0x00000080) == 0x80 ? 0xFFFFFF00 | value1 : value1;
				EX_MEM.B = value1;
			}
			break;	
		case 0x00000021:
			if(EX_MEM.left){
				printf("# LH\n");
				EX_MEM.imm = (EX_MEM.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | EX_MEM.imm : EX_MEM.imm;
				value = EX_MEM.A + EX_MEM.imm;
				value1 = mem_read_32(value) & 0x0000FFFF;
				value1 = (value1 & 0x00008000) == 0x8000 ? 0xFFFF0000 | value1 : value1;
				EX_MEM.B = value1;	
			}
			break;
		case 0x00000023:
			if(EX_MEM.left){
				printf("# LW\n");
				EX_MEM.imm = (EX_MEM.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | EX_MEM.imm : EX_MEM.imm;
				EX_MEM.imm = EX_MEM.A + EX_MEM.imm;
				EX_MEM.B = mem_read_32(EX_MEM.imm);
			}
			break;
		case 0x0000002B:
			// SW
			printf("# SW\n");
			if(((EX_MEM.imm & 0x00008000)>>15)){
				EX_MEM.imm = EX_MEM.imm | 0xFFFF0000;
			}
			location = EX_MEM.A + EX_MEM.imm;
			value = EX_MEM.B;
			//printf("offset=%d\nbase=%d\nvalue=%x\n[rt]=%x\n", offset, base, location, value);
			mem_write_32(location, value);
			break;
		case 0x00000028:
			// SB
			printf("# SB\n");
			if(((EX_MEM.imm & 0x00008000)>>15)){
				EX_MEM.imm = EX_MEM.imm | 0xFFFF0000;
			}
			value = EX_MEM.A + EX_MEM.imm;
			//printf("offset=%d\nbase=%d\nvalue=%d\nrt=%d\n", offset, base, value, rt);
			mem_write_32(value, EX_MEM.B & 0x000000FF);
			break;

		case 0x00000029:
			// SH
			printf("# SH\n");
			EX_MEM.imm = (EX_MEM.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | EX_MEM.imm : EX_MEM.imm;
			value = EX_MEM.A + EX_MEM.imm;
			//printf("offset=%d\nbase=%d\nvalue=%d\nrt=%d\n", offset, base, value, rt);
			mem_write_32(value, EX_MEM.B & 0x0000FFFF);
			break;
	}
	
	
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	uint32_t value, temp;
	switch(EX_MEM.opcode){
		case 0x00000020: 
			// ADD
			if(EX_MEM.right){ 
				printf("# ADD\n");
				EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
				//printf("\tBefore:\n\trs=%d\t0x%x\n\trt=%d\t0x%x\n\trd=%d\n", rs, EX_MEM.A, rt, EX_MEM.B, rd);
				//printf("\tAfter:\n\t[rd]=0x%x\n", EX_MEM.ALUOutput);

			// LB	
			}else{
				printf("LB skip\n");			
			}
			break;
		case 0x00000021:
			// ADDU
			if(EX_MEM.right){ 
				printf("# ADDU\n");
				EX_MEM.ALUOutput = ID_EX.A + ID_EX.B;
				//printf("\tBefore:\n\trs=%d\t0x%x\n\trt=%d\t0x%x\n\trd=%d\n", rs, EX_MEM.A, rt, EX_MEM.B, rd);
				//printf("\tAfter:\n\t[rd]=0x%x\n", EX_MEM.ALUOutput);

			// LH
			}else{
				printf("LH Skip\n");
			}
			break;
		case 0x00000008:
			// ADDI
			if(EX_MEM.left){ 
				printf("# ADDI\n");
				//printf("ADDI %u %u %u\n", rs, rt, immediate);
				value = (ID_EX.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | ID_EX.imm : ID_EX.imm;
				EX_MEM.ALUOutput = ID_EX.A + value;
				//printf("ADDI\nrs=%d\nrt=%d\nimmediate=%d\n[rs]=%d\n[rt]=%d\nvalue=%d\n",rs,rt,immediate,NEXT_STATE.REGS[rt], EX_MEM.A, value);
			}
			break;
		case 0x00000009:
			
			if(ID_EX.left){
			
			// ADDIU
				printf("# ADDIU\n");
				value = (EX_MEM.imm & 0x00008000) == 0x8000 ? 0xFFFF0000 | EX_MEM.imm : EX_MEM.imm;
				EX_MEM.ALUOutput = EX_MEM.A + value;
			}
			break;
			
		case 0x00000022:
			//SUB
			printf("# SUB\n");
			value = EX_MEM.A - EX_MEM.B;
			EX_MEM.ALUOutput = value;
			break;
			
		case 0x00000023:
			// SUBU
			if(EX_MEM.right){
				printf("# SUBU\n");
				EX_MEM.ALUOutput = EX_MEM.A - EX_MEM.B;

			// LW
			}else{
				printf("LW skip\n");			
			}
			break;
			
		case 0x00000018:
			// MULT
			printf("# MULT\n");
			// check if previous instruction was MFHI or MFLO
			if((MEM_WB.IR == 0x00000010) | (MEM_WB.IR == 0x00000012)){
				printf("MULT: result is undefined.\n");
			}else{
				temp = EX_MEM.A * EX_MEM.B; // temp is 64 bits
				EX_MEM.HI = (temp & 0xFFFFFFFF00000000) >> 32;	// get high bits
				EX_MEM.LO = (temp & 0x00000000FFFFFFFF) & 0xFFFFFFFF;	// get low bits
			}
			break;
			
		case 0x00000019:
			// MULTU 
			printf("# MULTU\n");
			// check if previous instruction was MFHI or MFLO
			if((MEM_WB.IR == 0x00000010) | (MEM_WB.IR == 0x00000012)){
				printf("MULTU: result is undefined.\n");
			}else{
				temp = EX_MEM.A * EX_MEM.B; // temp is 64 bits
				EX_MEM.HI = (temp & 0xFFFFFFFF00000000) >> 32;	// get high bits
				EX_MEM.LO = temp & 0x00000000FFFFFFFF;		// get low bits
			}
			break;
			
		case 0x0000001A: 
			// DIV
			printf("# DIV\n");
			//check if previous instruction was MFHI or MFLO
			if((MEM_WB.IR == 0x00000010) | (MEM_WB.IR == 00000012)){
				printf("DIV: result is undefined.\n");
			}else{
				// check for division by 0
				if(EX_MEM.B == 0){
					printf("DIV: cannot divide by 0.\n");
				}else{
					EX_MEM.HI = EX_MEM.A / EX_MEM.B; // Div = A/B
					EX_MEM.LO = EX_MEM.A % EX_MEM.B; // REM = A%B
				}
			}
			break;
			
		case 0x0000001B: 
			// DIVU
			printf("# DIVU\n");
			//check if previous instruction was MFHI or MFLO
			if((MEM_WB.IR == 0x00000010) | (MEM_WB.IR == 00000012)){
				printf("DIV: result is undefined.\n");
			}else{
				// check for division by 0
				if(EX_MEM.B == 0){
					printf("DIV: cannot divide by 0.\n");
				}else{
					EX_MEM.HI = EX_MEM.A / EX_MEM.B; // Div = A/B
					EX_MEM.LO = EX_MEM.A % EX_MEM.B; // REM = A%B
				}
			}
			break;
			
		case 0x00000024:
			// AND
			printf("# AND\n");
			EX_MEM.ALUOutput = EX_MEM.A & EX_MEM.B;
			break;
			
		case 0x0000000C:
			// ANDI
			if(EX_MEM.left){
				printf("# ANDI\n");
				EX_MEM.B = EX_MEM.A & EX_MEM.imm;

			// SYSCALL
			}else{ 
				//if(CURRENT_STATE.REGS[2] = 0xA){ // check register $v0 = $2 for the value 10
					RUN_FLAG = false;
				//}
			}

			break;
		case 0x0000000F:
			// LUI
			printf("# LUI\n");
			EX_MEM.ALUOutput = (EX_MEM.imm << 16) | 0x0000;
			break;	
		case 0x00000025: 
			// OR
			printf("# OR\n");
			EX_MEM.ALUOutput = EX_MEM.A | EX_MEM.B;
			break;
			
		case 0x000000D:
			// ORI
			printf("# ORI\n");
			EX_MEM.ALUOutput = EX_MEM.A & EX_MEM.imm;
			break;
			
		case 0x00000026:
			// XOR
			printf("# XOR\n");
			EX_MEM.ALUOutput = EX_MEM.A ^ EX_MEM.B;
			break;
		
		case 0x000000E:
			// XORI
			printf("# XORI\n");
			EX_MEM.ALUOutput = EX_MEM.A ^ EX_MEM.imm;
			break;
		
		case 0x00000027:
			// NOR
			printf("# NOR\n");
			EX_MEM.ALUOutput = ~ (EX_MEM.A | EX_MEM.B);
			break;
		
		case 0x0000002A:
			// SLT
			printf("# SLT\n");
			if(EX_MEM.A < EX_MEM.B){
                		EX_MEM.ALUOutput = 0x01;
			}
            		else{
                		EX_MEM.ALUOutput = 0x00;
            		}
			break;
		
		case 0x0000000A:
			// SLTI
			printf("# SLTI\n");
			// sign extend (check if most significant bit is a 1)
			if(((EX_MEM.imm & 0x00008000)>>15)){
				EX_MEM.imm = EX_MEM.imm | 0xFFFF0000;
			}
			if(EX_MEM.A < EX_MEM.imm){
               			EX_MEM.ALUOutput = 0x01;
            		}
            		else{
            			EX_MEM.ALUOutput = 0x00;
           		}
			break;
		
		case 0x00000000:
			// SLL
			if(EX_MEM.right){
				
				printf("# SLL\n");
				EX_MEM.imm = EX_MEM.IR & 0x000007C0;
				EX_MEM.imm = EX_MEM.imm >> 6;
				EX_MEM.ALUOutput = EX_MEM.B << EX_MEM.imm;
			}
			break;
		case 0x00000002:
			// SRL
			if(EX_MEM.right){
				printf("# SRL\n");
				EX_MEM.imm = EX_MEM.IR & 0x000007C0;
				EX_MEM.imm = EX_MEM.imm >> 6;
				EX_MEM.ALUOutput = EX_MEM.B >> EX_MEM.imm;
			}
			break;
		
		case 0x00000003:
			// SRA
			if (EX_MEM.right){
				printf("# SRA\n");
				EX_MEM.imm = EX_MEM.IR & 0x000007C0;
				EX_MEM.imm = EX_MEM.imm >> 6;

				if((EX_MEM.B&0x80000000)>>31){
					EX_MEM.ALUOutput = EX_MEM.B >> EX_MEM.imm;
					EX_MEM.ALUOutput |= 0x80000000;
				}else{
					EX_MEM.ALUOutput = EX_MEM.B >> EX_MEM.imm;
				}
			}
			break;

		case 0x0000002B:
			// SW
			printf("# SW skip\n");
		
			break;

		case 0x00000028:
			// SB
			printf("# SB skip\n");
			
			break;

		case 0x00000029:
			// SH
			printf("# SH skip\n");
			
			break;

		case 0x00000010:
			// MFHI
			printf("# MFHI\n");
			EX_MEM.ALUOutput = EX_MEM.HI;
			break;

		case 0x00000012:
			// MFLO
			printf("# MFLO\n");
			EX_MEM.ALUOutput = EX_MEM.LO;
			break;

		case 0x00000011:
			// MTHI
			printf("# MTHI\n");
			EX_MEM.ALUOutput = EX_MEM.A;
			break;

		case 0x00000013:
			// MTLO
			printf("# MTLO\n");
			EX_MEM.ALUOutput = EX_MEM.A;
			break;

		default:
			// default
			printf("default\n");
			break;

	}
	MEM_WB = EX_MEM;
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	/*IMPLEMENT THIS*/
	uint32_t opc;
	uint32_t instruction = IF_ID.IR;
	uint32_t opctemp = instruction & 0xFC000000;
	opctemp = opctemp >> 26;
	
	//opcode at right
	if(opctemp == 0x0){
		ID_EX.right = true;
		ID_EX.left = false;
		opc = instruction & 0x0000003F;
	//opcode at left
	}else{
		ID_EX.left = true;
		ID_EX.right = false;
		opc = opctemp;
	}
	ID_EX.opcode = opc;
	ID_EX.A = CURRENT_STATE.REG[instruction & 0x03E00000 >> 21]; //rs
	ID_EX.B = CURRENT_STATE.REG[instruction & 0x001F0000 >> 16]; //rt
	ID_EX.C = CURRENT_STATE.REG[instruction & 0x0000FC00 >> 11]; //rd
	ID_EX.imm = instruction & 0xFFFF; //immediate
	ID_EX.HI = CURRENT_STATE.HI;
	ID_EX.LO = CURRENT_STATE.LO;

}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	/*IMPLEMENT THIS*/
	IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;

}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}	
}
char * getReg(int temp){
	if(temp == 0){
		return "$zero";
	}else if(temp == 1){
		return "$at";
	}else if(temp == 2){
		return "$v0";
	}
	else if(temp == 3){
		return "$v1";
	}
	else if(temp == 4){
		return "$a0";
	}
	else if(temp == 5){
		return "$a1";
	}
	else if(temp == 6){
		return "$a2";
	}
	else if(temp == 7){
		return "$a3";
	}
	else if(temp == 8){
		return "$t0";
	}
	else if(temp == 9){
		return "$t1";
	}
	else if(temp == 10){
		return "$t2";
	}
	else if(temp == 11){
		return "$t3";
	}
	else if(temp == 12){
		return "$t4";
	}
	else if(temp == 13){
		return "$t5";
	}
	else if(temp == 14){
		return "$t6";
	}
	else if(temp == 15){
		return "$t7";
	}
	else if(temp == 16){
		return "$s0";
	}
	else if(temp == 17){
		return "$s1";
	}
	else if(temp == 18){
		return "$s2";
	}
	else if(temp == 19){
		return "$s3";
	}
	else if(temp == 20){
		return "$s4";
	}
	else if(temp == 21){
		return "$s5";
	}
	else if(temp == 22){
		return "$s6";
	}
	else if(temp == 23){
		return "$s7";
	}
	else if(temp == 24){
		return "$t8";
	}
	else if(temp == 25){
		return "$t9";
	}
	else if(temp == 26){
		return "$k0";
	}
	else if(temp == 27){
		return "$k1";
	}
	return NULL;
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
    char buffer[50];
    uint32_t instruction = mem_read_32(addr);
	
    //-----R-Type Instruction variables-----
	uint32_t func = instruction & 0x0000003F;			//bits 0-5
	uint32_t shamt = (instruction & 0x00000780) >> 6;	//bits 6-10
	uint32_t d_reg = (instruction & 0x0000F800) >> 11;	//bits 11-15
	uint32_t t_reg = (instruction & 0x001F0000) >> 16;	//bits 16-20
	uint32_t s_reg = (instruction & 0x03E00000) >> 21;	//bits 21-25
	uint32_t special = (instruction & 0xFC000000);		//bits 26-31
	
	//------I-Type Instruction variables-----
	uint32_t immediate = instruction & 0x0000FFFF;		//bits 0-15
	
	//-----J-Type Instruction variables-----
	uint32_t branch = instruction & 0x001F0000;			//bits 16-20
	
	buffer[0] = '\0';
	
	
    switch(special) {
        case 0x00000000:                              //non-special
            switch(func) {
                case 0x00000020:                      //ADD
                   sprintf(buffer, "ADD %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000021:                      //ADDU
                    sprintf(buffer, "ADDU %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000024:                      //AND
                    sprintf(buffer, "ADD %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x0000001A:                      //DIV
                    sprintf(buffer, "DIV %s, %s\n", getReg(s_reg), getReg(t_reg));
                    break;
                case 0x0000001B:                      //DIVU
                    sprintf(buffer, "DIVU %s, %s\n", getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000008:                      //JR
                    sprintf(buffer, "JR %s\n", getReg(s_reg));
                    break;  
                case 0x00000022:                      //SUB
                    sprintf(buffer, "SUB %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000023:                      //SUBU
                    sprintf(buffer, "SUBU %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000018:                      //MULT
                    sprintf(buffer, "MULT %s, %s\n", getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000019:                      //MULTU
                    sprintf(buffer, "MULT %s, %s\n", getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000011:                      //MTHI
                    sprintf(buffer, "MTHI %s\n", getReg(s_reg));
                    break;
                case 0x00000013:                      //MTLO
                    sprintf(buffer, "MTLO %s\n", getReg(s_reg));
                    break;
                case 0x00000025:                      //OR
                    sprintf(buffer, "OR %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000026:                      //XOR
                    sprintf(buffer, "XOR %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000027:                      //NOR
                    sprintf(buffer, "NOR %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x0000002A:                      //SLT
                    sprintf(buffer, "SLT %s, %s, %s\n", getReg(d_reg), getReg(s_reg), getReg(t_reg));
                    break;
                case 0x00000000:                      //SLL
                    sprintf(buffer, "SLL %s, %s, %u\n", getReg(d_reg), getReg(t_reg), shamt);
                    break;
                case 0x00000002:                      //SRL
                    sprintf(buffer, "SRL %s, %s, %u\n", getReg(d_reg), getReg(t_reg), shamt);
                    break;
                case 0x00000003:                      //SRA
                    sprintf(buffer, "SRA %s, %s, %u\n", getReg(d_reg), getReg(t_reg), shamt);
                    break;
                case 0x00000010:                      //MFHI
                    sprintf(buffer, "MFHI %s\n", getReg(d_reg));
                    break;
                case 0x00000012:                      //MFLO
                    sprintf(buffer, "MFLO %s\n", getReg(d_reg));
                    break;
                
                case 0x00000009:                      //JALR
                    sprintf(buffer, "JALR %s, %s\n", getReg(d_reg), getReg(s_reg));
                    break;
                case 0x0000000C:                      //SYSCALL
                    sprintf(buffer, "SYSCALL\n");
                    break;
            }
            break;
        case 0x04000000:                                //REGIMM
            switch(branch) {
                case 0x00010000:                      //BGEZ
                    sprintf(buffer, "BGEZ %s, %d\n", getReg(s_reg), immediate);
                    break;
                case 0x00000000:                      //BLTZ
                    sprintf(buffer, "BLTZ %s, %d\n", getReg(s_reg), immediate);
                    break;
            }
            break;                                         //special
        case 0x20000000:                                //ADDI
            sprintf(buffer, "ADDI %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x24000000:                                //ADDIU
            sprintf(buffer, "ADDIU %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x30000000:                                //ANDI
            sprintf(buffer, "ANDI %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x10000000:                                //BEQ
            sprintf(buffer, "BEQ %s, %s, %d\n", getReg(s_reg), getReg(t_reg), immediate);
            break;
        case 0x14000000:                                //BNE
            sprintf(buffer, "BNE %s, %s, %x\n", getReg(s_reg), getReg(t_reg), immediate);
            break;
        //case 0x08000000:                                //J
        //     sprintf(buffer, "J %zu\n", (size_t)(instruction && 0x03FFFFFF) /*<< 2*/);
        //    break;
        //case 0x0C000000:                                //JAL
        //    sprintf(buffer, "JAL %zu\n", (size_t)(instruction && 0x03FFFFFF) /*<< 2*/);
        //    break;
        case 0x08000000:
        	sprintf(buffer, "J %d\n", (instruction & 0x03FFFFFF) /*<< 2*/);
        	break;
        case 0x0C000000:
        	sprintf(buffer, "JAL %d\n", (instruction & 0x03FFFFFF) /*<< 2*/);
        	break;
        case 0x3C000000:                                //LUI
            sprintf(buffer, "LUI %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0x8C000000:                                //LW
            sprintf(buffer, "LW %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0x80000000:                                //LB
            sprintf(buffer, "LB %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0x34000000:                                //ORI
            sprintf(buffer, "ORI %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x38000000:                                //XORI
            sprintf(buffer, "XORI %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x28000000:                                //SLTI
            sprintf(buffer, "SLTI %s, %s, %d\n", getReg(t_reg), getReg(s_reg), immediate);
            break;
        case 0x84000000:                                //LH
            sprintf(buffer, "LH %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0xAC000000:                                //SW
            sprintf(buffer, "SW %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0xA0000000:                                //SB
            sprintf(buffer, "SB %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0xA4000000:                                //SH
            sprintf(buffer, "SH %s, %d(%s)\n", getReg(t_reg), immediate, getReg(s_reg));
            break;
        case 0x18000000:                                //BLEZ
            sprintf(buffer, "BLEZ %s, %d\n", getReg(s_reg), immediate);
            break;
        case 0x1c000000:                                //BGTZ
           sprintf(buffer, "BGTZ %s, %d\n", getReg(s_reg), immediate);
            break;
    }
	
    printf("%s", buffer);
}
/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
	printf("Current PC: 0x%08x\n", CURRENT_STATE.PC);
	printf("IF/ID.IR: ");
	print_instruction(ID_IF.IR);
	printf("IF/ID.PC: 0x%08x\n", ID_IF.PC);
	
	printf("ID/EX.IR: ");
	print_instruction(IF_EX.IR);
	printf("ID/EX.A: 0x%08x\n", IF_EX.A);
	printf("ID/EX.B: 0x%08x\n", IF_EX.B);
	printf("ID/EX.IMM: 0x%08x\n", IF_EX.imm);

	printf("EX/MEM.IR: ");
	print_instruction(EX_MEM.IR);
	printf("EX/MEM.A: 0x%08x\n", EX_MEM.A);
	printf("EX/MEM.B: 0x%08x\n", EX_MEM.B);
	printf("EX/MEM.ALUOutput: 0x%08x\n", EX_MEM.ALUOutput);

	printf("MEM/WB.IR: ");
	print_instruction(MEM_WB.IR);
	printf("MEM/WB.ALUOutput: 0x%08x\n", MEM_WB.ALUOutput);
	printf("MEM/WB.LMD: 0x%08x\n", MEM_WB.LMD);
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
