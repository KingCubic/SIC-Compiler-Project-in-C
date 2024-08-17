#include "headers.h"
#include "string.h"
#include "ctype.h"

//Boston Woods N01388297


//Symbol table type definition
struct Symbol {
	int DefinedOnSourceLine;
	int Address;
	char Name[7];
};

struct Mrec {
	int loc;
	int halfBytes;
	char name[7];
	char flag;
};

int errFlag;

//This method handles the logic for incrementing Locctr
int CalcLocctr(char * newdir, char * newop, int line) {
    char  *preop;
    preop = malloc(32 * sizeof(char));

	//If it is a BYTE directive
        if(strstr(newdir, "BYTE")) {
                //BYTE Directive logic. Increments locctr
                //based on what is passed in through the quotes.
        		strcpy(preop, newop);
                char *op;
                op = strtok(preop, "\'");
                if(strstr(op, "C")) {
                        op = strtok(NULL, "\'");
                        return strlen(op);
                }  else if (strstr(op, "X")) {
                        op = strtok(NULL, "\'\n\r");
                        int i = 0;
    					while(op[i] != '\0') {
    						if((op[i] <= 70 && op[i] >= 65) || (op[i] <= 57 && op[i] >= 48)) {
    						} else {
    							printf("\nLine %d Error: incorrect BYTE hexidecimal input: %s\n", line, op);
    							errFlag = 1;
    							return 2000;
    						}
    						i++;
    					}
                        return (int) (strlen(op) / 2);
                }  else {
                        printf("\nLine %d Error: incorrect usage of BYTE directive.\n", line);
                        errFlag = 1;
                        return 2000;
                }//end if

	//Check if it is another type of memory directive
        } else if(strstr(newdir, "RESB")) {
        		if(newop != NULL);
        		strcpy(preop, newop);
                return (int) strtol(preop, NULL, 10);
        } else if(strstr(newdir, "RESW")) { 
        		if(newop != NULL)
        		strcpy(preop, newop);
                return (3 * (int) strtol(preop, NULL, 10));
        } else if(strstr(newdir, "WORD")){
        	int a = strtol(newop, NULL, 10);
        	if(a > 8388607) {
        		printf("Line %d Error: WORD instruction exceeds 24 bit limit (%d)\n", line, a);
        		errFlag = 1;
        		return 2000;
        	}
                return 3;
        } else {
        	return 3;
        }
	return 2000;
}

//TODO directive and operand validation
int TestValidDir(char * dir) {
	char validDir[26][6] = {"ADD", "AND", "COMP", "DIV", "J", "JEQ", "JGT", "JLT", 
				"JSUB", "LDA", "LDCH", "LDL", "LDX", "MUL", 
				"OR", "RD", "RSUB", "STA", "STCH", "STL", "STSW",
				"STX", "SUB", "TD", "TIX", "WD" 
			     };
	for(int i = 0; i < 26; i++) {
		if(strcmp(dir, validDir[i])) {
		}
	}
	return 0;
}

//This method checks if it is a duplicate symbol
int SymbolExists(struct Symbol* sym[], char* name) {
        int index = 0;
        while(sym[index] != NULL) {
                if(!strcmp(name, sym[index]->Name)) {
                        return 1;
                }
                index++;
        }
        return 0;
}

//This method adds the symbol the the symbol table
void addSymbol(struct Symbol* sym[], int addr, int ln, char* name) {
        int index;
        index = 0;
        struct Symbol* newsym;
        newsym = malloc(sizeof(struct Symbol));
        newsym->Address = addr;
        newsym->DefinedOnSourceLine = ln;
        strcpy(newsym->Name, name);
		
        while(sym[index] != NULL) {
                index++;
        }
        sym[index] = newsym;
}

void printSymbolTable(struct Symbol* sym[]) {
	int index;
	index = 0;
	while(sym[index] != NULL) {
		printf("%s\t%x\n", sym[index]->Name, sym[index]->Address);
		index++;
	}
}

int IsInstruction(FILE *OPtabFP, char * instrIn) {
	fclose(OPtabFP);
	OPtabFP = fopen("OPTAB.txt", "r");
	char line[10];
	char * instr;
	char * code;
	while(fgets(line,10, OPtabFP) != NULL) {
		instr = strtok(line, " \n");
		code = strtok(NULL, " \n");
		if(strcmp(instrIn, instr) == 0) {
			//Unsure if this returns hex or dec
			return (int) strtol(code, NULL, 16);
		}
	}
	return 0;
}

int IsSymbol(struct Symbol* sym[], char * operIn) {
	int index = 0;
	int result = 0;
	while(sym[index] != NULL) {
		if(strcmp(sym[index]->Name, operIn) == 0) {
			return sym[index]->Address;
		}
		index++;
	}
	return result;
}

void printMRec(struct Mrec* mrec[], FILE * OpCodes) {
	int index;
	index = 0;
	while(mrec[index] != NULL) {
		fprintf(OpCodes, "M%06X04+%s\n", mrec[index]->loc, mrec[index]->name);
		index++;
	}
}

void trimWhite(char * in)
{
    int index, i;
    index = -1;
    i = 0;
    while(in[i] != '\0')
    {
        if(in[i] != ' ' && in[i] != '\t' && in[i] != '\n')
        {
            index= i;
        }

        i++;
    }
    in[index + 1] = '\0';
}

void pass2(struct Symbol* sym[], char* argv[], int len) {
// locctrIn[7], symIn[7], instrIn[7], operIn[512],
	//Input variables
	char start[8], pName[7];
	char *locctrIn;
	char *symIn;
	char *instrIn;
	char *operIn;
	char line[1024];
	//Create space to hold M records to print at the end
	struct Mrec* mrec[512];
	memset(mrec, '\0', 512 * sizeof(mrec[0]));
	//File declarations and handling
	FILE *fp;
	FILE *OPtabFP;
	FILE *intrFP;
	FILE *OpCodes;
	fp = fopen( argv[1], "r");
	OPtabFP = fopen("OPTAB.txt", "r");
	intrFP = fopen("IntrFile.txt", "r");
	OpCodes = fopen("OpCodes.txt", "w");
	if(fp == NULL) {
		printf("Error no file found: %s", argv[1]);
		return;
	}
	if(OPtabFP == NULL) {
		printf("Error no file found: OPTAB.txt");
		return;
	}
	if(intrFP == NULL) {
		printf("Error no file found: IntrFile.txt");
		return;
	}
	if(OpCodes == NULL) {
		printf("Error no file found: OPCODES.txt");
		return;
	}//End file declrations and handling


	//Handle Start line of intermediate file
	fgets(line, 1024, intrFP);
	locctrIn = strtok(line, "\t\n\r");
	symIn = strtok(NULL, "\t\n\r");
	instrIn = strtok(NULL, "\t\n\r");
	operIn = strtok(NULL, "\t\n\r");

	if(strstr(instrIn, "START")) {
		strcpy(start, operIn);
		strcpy(pName, symIn);
		int a = (int) strtol(start, NULL, 16);
		fprintf(OpCodes,"H%-6s%06X%06X\n", symIn, a, len - 3);
	} else {
		printf("\nERROR first line doesn't contain START directive");
		return;
	}
	//Read next line for the first T record
	fgets(line, 1024, intrFP);
	locctrIn = strtok(line, "\t\n\r");
	symIn = strtok(NULL, "\t\n\r");
	instrIn = strtok(NULL, "\t\n\r");
	operIn = strtok(NULL, "\t\n\r");

	//Make the T records
	while(strcmp(instrIn, "END") != 0) {
		//Check For Indexed Addressing
		if(strstr(operIn, ",")) {
			int newLoc;
			strtok(operIn, ",\n");
			newLoc = IsSymbol(sym, operIn) + 32768;
			int op = IsInstruction(OPtabFP, instrIn);
			int hex = (int) strtol(locctrIn, NULL, 16);
			fprintf(OpCodes, "T%06X03%02X%04X\n", hex, op, newLoc);

			//Make M record
			struct Mrec* newMrec;
			int index = 0;
			newMrec = malloc(sizeof(struct Mrec));
			newMrec->loc = hex + 1;
			strcpy(newMrec->name, pName);
			while(mrec[index] != NULL) index++;
			mrec[index] = newMrec;

			fgets(line, 1024, intrFP);
			locctrIn = strtok(line, "\t\n\r");
			symIn = strtok(NULL, "\t\n\r");
			instrIn = strtok(NULL, "\t\n\r");
			operIn = strtok(NULL, "\t\n\r");

			continue;
		}


		//If instruction is word directive
		if(strcmp(instrIn, "WORD") == 0) {

			int val = (int) strtol(operIn, NULL, 10);
			int hex = (int) strtol(locctrIn, NULL, 16);
			fprintf(OpCodes,"T%06X03%06X\n", hex, val);

		//If instruction is RSUB
		} else if (strcmp(instrIn,"RSUB") == 0) {
			int hex = (int) strtol(locctrIn, NULL, 16);
			fprintf(OpCodes, "T%06X034C0000\n", hex);

		//If instruction is RESB or RESW
		} else if((strcmp(instrIn, "RESB") == 0) || (strcmp(instrIn, "RESW") == 0)) {

		//If instruction is BYTE Directive
		} else if (strcmp(instrIn, "BYTE") == 0) {
				int sizeOfEx = 0;
				char* a;
				char* b;
				a = strtok(operIn, "\'");
				b = strtok(NULL, "\'");
				if(strstr(a, "C")) {
					int hex = (int) strtol(locctrIn, NULL, 16);
					fprintf(OpCodes, "T%06X", hex);
					sizeOfEx = strlen(b);
					fprintf(OpCodes,"%02X", sizeOfEx);
					int i = 0;
					while(b[i] != '\0') {
						fprintf(OpCodes, "%02X", b[i]);
						i++;
					}
					fprintf(OpCodes, "\n");

				} else {
					int hex = (int) strtol(locctrIn, NULL, 16);
					fprintf(OpCodes, "T%06X", hex);
					sizeOfEx = strlen(b) / 2;
					fprintf(OpCodes,"%02X", sizeOfEx);
					fprintf(OpCodes, "%s\n", b);
				}

		//If generic instruction (Not an edge case)
		} else if((IsInstruction(OPtabFP, instrIn)) || (strcmp(instrIn, "LDA") == 0)) {

			int op = IsInstruction(OPtabFP, instrIn);
			int addr = IsSymbol(sym, operIn);
			int hex = (int) strtol(locctrIn, NULL, 16);
			fprintf(OpCodes, "T%06X03%02X%04X\n", hex, op, addr);

			//If there is a symbol in the operand, generate an M record
			if(IsSymbol(sym, operIn)) {
				struct Mrec* newMrec;
				int index = 0;
				newMrec = malloc(sizeof(struct Mrec));
				newMrec->loc = hex + 1;
				strcpy(newMrec->name, pName);
				while(mrec[index] != NULL) index++;
				mrec[index] = newMrec;
			} else if(operIn[0] != '!') {
				printf("\nError symbol not defined: %s", operIn);
				return;
			}

		}

		//Read next line in intermediate file
		fgets(line, 1024, intrFP);
		locctrIn = strtok(line, "\t\n\r");
		symIn = strtok(NULL, "\t\n\r");
		instrIn = strtok(NULL, "\t\n\r");
		operIn = strtok(NULL, "\t\n\r");
	}

	//Handle end logic here
	printMRec(mrec, OpCodes);
	int addr = IsSymbol(sym, operIn);
	fprintf(OpCodes, "E%06X\n", addr);
	fclose(OpCodes);
	fclose(OPtabFP);
	fclose(intrFP);
	fclose(fp);
}


//Main method loops through the lines in the SIC file,
//calling the methods for input validation and table
//construction.
int main( int argc, char* argv[]){

	FILE *fp;
	FILE *intrFP;
	char line[1024];
	struct Symbol* sym[1024];
	char* newsym;
	char* newop;
	char* newdir;
	int locctr;
	int tempLocctr;
	int lineNum = 0;
	int startLoc;
	memset(sym, '\0', 1024 * sizeof(sym[0]));

	/*Ensure the user invoked with 2 arguments*/
	if(argc != 2) {
		printf("ERROR: Usage: %s filename/n", argv[0] );
		return 0;
	}

	/*Ensure the assembly file exists*/
	fp = fopen( argv[1], "r");
	if(fp == NULL) {
		printf("ERROR: No file found: %s", argv[1] );
		return 0;
	}
	intrFP = fopen("IntrFile.txt", "w");
	if(intrFP == NULL) {
		printf("ERROR: No intrFile.txt found.");
		return 0;
	}

	/*Begin main loop to read each line in the file,
	simultaneously generating the symbol table*/
	newsym = malloc(1024 * sizeof(char   )   );
	memset(newsym, '\0', 1024 * sizeof(char));

	while(fgets(line, 1024, fp) != NULL) {
		lineNum++;
		if(line[0] == '\n' || line[0] == '\r') {
			printf("\nLine %d Error: Empty Line.\n", lineNum);
			return 0;
		}
		/*Ignore comment*/
		if(line[0] == 35) {
			continue;
		}

		/*If the line begins with an alphabetical character*/
		if((line[0] >= 65) && (line[0] <= 90)) {

			/*Tokenize the line*/
			newsym = strtok(line, "\t\n\r");
			newdir = strtok(NULL, "\t\n\r");
			newop = strtok(NULL, "\t\n\r");
			newop = strtok(newop, "\n\t\r");
//			int t = strlen(newop) - 1;
//			if(isspace(newop[t])) {
//				newop = strtok(NULL, " ");
//			}
			if(newop != NULL)
			trimWhite(newop);



				/*Validate symbol name*/
				if(IsAValidSymbol(newsym)) {

					/*Check if it is a duplicate*/
					if(SymbolExists(sym, newsym)) {
						printf("\nLine %d Error: Duplicate symbol (%s)\n", lineNum, newsym);
						return 0;
					}

					//Check if line contains start
					if(strstr(newdir, "START")) {
						locctr = (int) strtol(newop, NULL, 16);
						if(locctr >= 32000) {
							printf("\nError: Program does not fit in SIC memory.\n");
							return 0;
						}
						startLoc = locctr;
						addSymbol(sym, locctr, lineNum, newsym);
						fprintf(intrFP,"!\t%s\t%s\t%s\n",newsym,newdir,newop);
						tempLocctr = locctr;
						continue;
					}

					//Add symbol to symbol table and increment Locctr
					locctr = locctr + CalcLocctr(newdir, newop, lineNum);
					if(errFlag == 1) {
						return 0;
					}

					addSymbol(sym, tempLocctr, lineNum, newsym);
					if(newop != NULL) {
						fprintf(intrFP,"%X\t%s\t%s\t%s\n", tempLocctr,newsym,newdir,newop);
					} else {
						fprintf(intrFP,"%X\t%s\t%s\t!\n", tempLocctr,newsym,newdir);
					}
					tempLocctr = locctr;
					continue;
				} else {
					printf("\nLine %d Error: Not a valid symbol:  %s\n", lineNum, newsym);
					return 0;
				}//End symbol is valid symbol block

        }//End if starts with a char block

                //If it is not an empty line
		if(line[0] == '\t') {
			//Tokenize the directive and operand
			newdir = strtok(line, "\t\n");
			newop = strtok(NULL,"\t");
			newop = strtok(newop, "\n\t\r");
//			int t = strlen(newop) - 1;
//			if(isspace(newop[t])) {
//				newop = strtok(NULL, " ");
//			}
			if(newop != NULL)
			trimWhite(newop);


			//Increment Locctr
			locctr = locctr + CalcLocctr(newdir, newop, lineNum);
			if(errFlag == 1) {
				return 0;
			}
			if(newop != NULL) {
				fprintf(intrFP,"%X\t!\t%s\t%s\n",tempLocctr,newdir,newop);
			} else {
				fprintf(intrFP,"%X\t!\t%s\t!\n",tempLocctr, newdir);
			}
			tempLocctr = locctr;
			continue;
		}
		if((line[0] <= 90 && line[0] >= 65) || (line[0] <= 57 && line[0] >= 48) || (line[0] <= 122 && line[0] >= 97)) {
		} else {
			printf("\nLine %d Error: Invalid symbol in line %s\n", lineNum, line);
			return 0;
		}




    }//End while
	int finalLenInBytesDec = locctr - startLoc;
	finalLenInBytesDec = finalLenInBytesDec * 1;

    fclose(fp);
    fclose(intrFP);

    //Call pass 2
    pass2(sym, argv, finalLenInBytesDec);
}

int IsAValidSymbol(char *sym) {
        int Result = 1;
        int maxlen = 6;
        int index = 0;

        //Check if the name is the same as a directive
        if(strcmp(sym, "START") &&  strcmp(sym, "END") && strcmp(sym, "BYTE") &&
                strcmp(sym, "WORD") && strcmp(sym, "RESB") && strcmp(sym, "RESW") &&
                strcmp(sym, "RESR") && strcmp(sym, "EXPORTS"))
        {} else {
			return 0;
		}

	//Check symbol length
        if(strlen(sym) > maxlen) {
        	return 0;
        }

	//Check for illegal characters
        while (sym[index] != '\0') {
                if((sym[index] <= 90 && sym[index] >= 65) || (sym[index] <= 57 && sym[index] >= 48) || (sym[index] <= 122 && sym[index] >= 97)) {
                } else {
                	return 0;
                }
                index++;
        }
        return Result;
}
