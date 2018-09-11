// ECE 4270 Group 5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>


int main(int argc, char *argv[]) { 
	FILE * fp;
		int i, word, type;
		char instr[1000];
		char *op, *rs, *rt, *rd, *imm;
		/* Open program file. */
		fp = fopen("test.in", "r");
		if (fp == NULL) {
			printf("Error: Can't open program file\n");
			exit(-1);
		}

		/* Read in the program. */

		i = 0;
		while(fgets(instr, sizeof(instr), fp) != NULL){
			printf("%s",instr);
			op = strtok(instr, " ");
			printf("\t%s\n", op);
			if(strcmp(op, "add") == 0){
				rd = strtok(NULL, " ");
				rs = strtok(NULL, " ");
				rt = strtok(NULL, " ");
				type = 2;		
			}
			else if(strcmp(op, "addu") == 0){
					
			}
			else if(strcmp(op, "addu") == 0){
					
			}
			else if(strcmp(op, "and") == 0){
					
			}
			else if(strcmp(op, "div") == 0){
					
			}
			else if(strcmp(op, "divu") == 0){
					
			}
			else if(strcmp(op, "addu") == 0){
					
			}
			else if(strcmp(op, "addu") == 0){
					
			}
			else{
			}
			
			switch(rd[2]){
				case 0
			}
			
			
					


		}
	
		fclose(fp);
}
