#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "mu-mips.h"

uint32_t prevIns;


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
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
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
			runAll(); 
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
		case 'P':https://www.google.com/search?client=ubuntu&channel=fs&q=c+convert+32+bit+hex&ie=utf-8&oe=utf-8
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
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	// read each instruction from the input program - use mem_read32 and mem_write32
	/*mem_read32()
	.
	.
	.
	parse instruction and execute instruction*/
	uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
	uint32_t opc, opctemp, of, value, value1, rs, rt, rd, immediate, offset, base;
	int64_t temp;
	//int rs, rt, rd, immediate, offset, base;
	int jmpBy = 4; // maybe useful in the jumps?
	bool left, right, middle;

	// ##### IDK IF THIS IS RIGHT ###############
	// The opcode can either be on the right, left, or in the middle
	// If at the right, the 6 most significant bits are 0x0
	// If in the middle, the 6 most significant bits are 0x01
	// otherwise, its on the left

	opctemp = instruction & 0xF0C00000;
	opctemp = opctemp >> 26;

	//opcode at right
	if(opctemp == 0x0){
		right = true;
		left = false;
		middle = false;
		opc = instruction & 0x0000003F;
	// opcode in middle
	}else if(opctemp == 0x00000001){
		right = false;
		left = false;
		middle = true;
		opc = instruction & 0x001F0000;
		opc = opc >> 16;
	//opcode at left
	}else{
		left = true;
		right = false;
		middle = false;
		opc = opctemp;
	}
	// ##########################################
	rs = instruction & 0x03E00000;
	rs = rs >> 21;
	rt = instruction & 0x001F0000;
	rt = rt >> 16;
	rd = instruction & 0x0000F800;
	rd = rd >> 11;
	offset = instruction & 0x0000FFFF;
	base = instruction & 0x03E00000;
	base = base >> 21;
	immediate = instruction & 0x0000FFFF;
	// ### is it overflow of the addition, or overflow of the instruction????
	
	switch(opc){
		case 0x00000020: 
			if(right){ // ADD
				printf("# ADD\n");
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
			}else{ // LB
				printf("# LB\n");
				offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
				value = CURRENT_STATE.REGS[base] + offset;
				value1 = mem_read_32(value) & 0x000000FF;
				value1 = (value1 & 0x00000080) == 0x80 ? 0xFFFFFF00 | value1 : value1;
				NEXT_STATE.REGS[rt] = value1;
			}
			break;
		case 0x00000021: //ADDU
			if(right){ // ADDU
				printf("# ADDU\n");
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
			}else{ // LH
				printf("# LH\n");
				offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
			
				value = CURRENT_STATE.REGS[base] + offset;
				value1 = mem_read_32(value) & 0x000000FF;
				value1 = (value1 & 0x00008000) == 0x8000 ? 0xFFFF0000 | value1 : value1;
				NEXT_STATE.REGS[rt] = value1;	
			}
			break;
			
		case 0x00000008: //ADDI (need sign extend of immediate) & check overflow
			printf("# ADDI\n");
			//printf("ADDI %u %u %u\n", rs, rt, immediate);
			value = (immediate & 0x00008000) == 0x8000 ? 0xFFFF0000 | immediate : immediate;
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + value;
			printf("ADDI\nrs=%d\nrt=%d\nimmediate=%d\n[rs]=%d\n[rt]=%d\nvalue=%d\n",rs,rt,immediate,NEXT_STATE.REGS[rt], CURRENT_STATE.REGS[rs], value);
			break;
		case 0x00000009: //ADDIU
			printf("# ADDIU\n");
			value = (immediate & 0x00008000) == 0x8000 ? 0xFFFF0000 | immediate : immediate;
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + value;
			break;
			
		case 0x00000022: //SUB
			printf("# SUB\n");
			value = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];

			// check overflow
			// of = value >> 30;
			// if((of | 0x0) == 0x0){
			 NEXT_STATE.REGS[rd] = value;
			// }else{
			// 	printf("SUB: OVERFLOW\n");
			// }
			 break;
			
		case 0x00000023:
			if(right){	//SUBU
				printf("# SUBU\n");
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
			}else{ // LW
				printf("# LW\n");
				offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
				offset = CURRENT_STATE.REGS[base] + offset;
				NEXT_STATE.REGS[rt] = mem_read_32(offset);
			}
			break;
			
		case 0x00000018: //MULT - you have to check the previous instruction..sign extend???
			printf("# MULT\n");
			// check if previous instruction was MFHI or MFLO
			if((prevIns == 0x00000010) | (prevIns == 0x00000012)){
				printf("MULT: result is undefined.\n");
			}else{
				temp = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt]; // temp is 64 bits
				NEXT_STATE.HI = (temp & 0xFFFFFFFF00000000) >> 32;	// get high bits
				NEXT_STATE.LO = (temp & 0x00000000FFFFFFFF) & 0xFFFFFFFF;	// get low bits
			}
			break;
			
		case 0x00000019: //MULTU - unsigned
			printf("# MULTU\n");
			// check if previous instruction was MFHI or MFLO
			if((prevIns == 0x00000010) | (prevIns == 0x00000012)){
				printf("MULTU: result is undefined.\n");
			}else{
				temp = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt]; // temp is 64 bits
				NEXT_STATE.HI = (temp & 0xFFFFFFFF00000000) >> 32;	// get high bits
				NEXT_STATE.LO = temp & 0x00000000FFFFFFFF;		// get low bits
			}
			break;
			
		case 0x0000001A: //DIV - sign extend or 2's complement???
			printf("# DIV\n");
			//check if previous instruction was MFHI or MFLO
			if((prevIns == 0x00000010) | (prevIns == 00000012)){
				printf("DIV: result is undefined.\n");
			}else{
				// check for division by 0
				if(CURRENT_STATE.REGS[rt] == 0){
					printf("DIV: cannot divide by 0.\n");
				}else{
					NEXT_STATE.HI = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt]; // Div = A/B
					NEXT_STATE.LO = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt]; // REM = A%B
				}
			}
			break;
			
		case 0x0000001B: //DIVU
			printf("# DIVU\n");
			//check if previous instruction was MFHI or MFLO
			if((prevIns == 0x00000010) | (prevIns == 00000012)){
				printf("DIV: result is undefined.\n");
			}else{
				// check for division by 0
				if(CURRENT_STATE.REGS[rt] == 0){
					printf("DIV: cannot divide by 0.\n");
				}else{
					NEXT_STATE.HI = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt]; // Div = A/B
					NEXT_STATE.LO = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt]; // REM = A%B
				}
			}
			break;
			
		case 0x00000024: //AND
			printf("# AND\n");
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
			break;
			
		case 0x0000000C: 
			if(left){ //ANDI
				printf("# ANDI\n");
				NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & immediate;
			}else{ //SYSCALL
				if(CURRENT_STATE.REGS[2] = 0xA){ // IF I DO == THEN IT DOESNT WORK?????
					RUN_FLAG = false;
				}
			}

			break;
			
		case 0x00000025: //OR
			printf("# OR\n");
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
			break;
			
		case 0x000000D: //ORI
			printf("# ORI\n");
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & immediate;
			break;
			
		case 0x00000026: //XOR
			printf("# XOR\n");
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
			break;
		
		case 0x000000E: //XORI
			printf("# XORI\n");
			NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ immediate;
			break;
		
		case 0x00000027: //NOR
			printf("# NOR\n");
			NEXT_STATE.REGS[rd] = ~ (CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
			break;
		
		case 0x0000002A: //SLT
			printf("# SLT\n");
			if(CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]){
                NEXT_STATE.REGS[rd] = 0x01;
			}
            else{
                NEXT_STATE.REGS[rd] = 0x00;
            }
			break;
		
		case 0x0000000A: //SLTI
			printf("# SLTI\n");
			// sign extend (check if most significant bit is a 1)
			if(((immediate & 0x00008000)>>15)){
				immediate = immediate | 0xFFFF0000;
			}
			if(CURRENT_STATE.REGS[rs] < immediate){
                NEXT_STATE.REGS[rt] = 0x01;
            }
            else{
                NEXT_STATE.REGS[rt] = 0x00;
            }
			break;
		
		case 0x00000000: //SLL
			printf("# SLL\n");
			immediate = instruction & 0x000007C0;
			immediate = immediate >> 6;
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << immediate;
			break;
		case 0x00000002: //SRL
			printf("# SRL\n");
			immediate = instruction & 0x000007C0;
			immediate = immediate >> 6;
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> immediate;
		break;
		
		case 0x00000003: //SRA 
			printf("# SRA\n");
			immediate = instruction & 0x000007C0;
			immediate = immediate >> 6;
			
			if((CURRENT_STATE.REGS[rt]&0x80000000)>>31){
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> immediate;
				NEXT_STATE.REGS[rd] |= 0x80000000;
			}else{
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> immediate;
			}
			break;

		case 0x00000004: //BEQ 
			printf("# BEQ\n");
			printf("Branch if equal\n");
			//Shift left by 2
			offset = offset << 2;
			// sign extend (check if most significant bit is a 1)
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			if(CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
				jmpBy = offset; //changed
			}
			break;

		case 0x00000005: //BNE
			printf("# BNE\n");

			offset = offset << 2;
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			if(CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
				jmpBy = offset; //changed
			}
			break;
		case 0x00000006: //BLEZ
			printf("BLEZ\n");
			offset = offset << 2;
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			if(((CURRENT_STATE.REGS[rs] & 0x80000000)>>31) || (CURRENT_STATE.REGS[rs] == 0x00)){
				jmpBy = offset; // changed
			}
			break;
		case 0x0000000F: // LUI
		 	printf("# LUI\n");
		 	NEXT_STATE.REGS[rt] = (immediate << 16) | 0x0000; // the & is just to assure that the lower 16 bits are zeros
		 	break;
		case 0x0000002B: // SW
			printf("# SW\n");
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			value = CURRENT_STATE.REGS[base] + offset;
			//printf("offset=%d\nbase=%d\nvalue=%d\nrt=%d\n", offset, base, value, rt);
			mem_write_32(value, CURRENT_STATE.REGS[rt]);
			break;
		case 0x00000028: // SB
			printf("# SB\n");
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			value = CURRENT_STATE.REGS[base] + offset;
			//printf("offset=%d\nbase=%d\nvalue=%d\nrt=%d\n", offset, base, value, rt);
			mem_write_32(value, CURRENT_STATE.REGS[rt] & 0x000000FF);
			break;
		case 0x00000029: //SH
			printf("# SH\n");
			offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
			value = CURRENT_STATE.REGS[base] + offset;
			//printf("offset=%d\nbase=%d\nvalue=%d\nrt=%d\n", offset, base, value, rt);
			mem_write_32(value, CURRENT_STATE.REGS[rt] & 0x0000FFFF);
			break;
		case 0x00000010: // MFHI
			printf("MFHI\n");
			NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
			break;
		case 0x00000012: // MFLO
			printf("MFLO\n");
			NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
			break;
		case 0x00000011: // MTHI
			printf("MTHI\n");
			NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
			break;
		case 0x00000013: // MTLO
			printf("MTLO\n");
			NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
			break;

		default:
			printf("default\n");
			break;

	}
	NEXT_STATE.PC = CURRENT_STATE.PC + jmpBy; // jmpBy is 4 by default
	prevIns = opc;	// used in MULT, MULTU, DIV, and DIVU	
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
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
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
