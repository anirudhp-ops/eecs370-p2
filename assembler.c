/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);
static int endsWith(char *, char *);
int labelCount = 0;
struct labelTable {
        char lab[8];
        int address;
    };
struct labelTable LT[MAXLINELENGTH];

int findLabelAddress(char *labelName) {
    for (int i = 0; i < labelCount; i++) {
        if (!strcmp(LT[i].lab, labelName)) {
            return LT[i].address;
        }
    }
    exit(1);   // Not found
}

int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    if (!endsWith(inFileString, ".as") &&
        !endsWith(inFileString, ".s") &&
        !endsWith(inFileString, ".lc2k")
    ) {
        printf("warning: assembly code file does not end with .as, .s, or .lc2k\n");
    }

    if (!endsWith(outFileString, ".mc")) {
        printf("error: machine code file must end with .mc\n");
        exit(1);
    }

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // Pass 1: to build a label table

    int mem_count = 0; 

    

    
    
       

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        
        if (label[0] != '\0') {

            for (int i = 0; i < labelCount; i++) {
                if (!strcmp(LT[i].lab, label)) {
                    exit(1); 
                }
            }

            strcpy(LT[labelCount].lab, label);
            LT[labelCount].address = mem_count; 
            labelCount += 1; 

        }

        mem_count += 1; 
    }
    int count = 0; 
    

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        int machineCode = 0;

        
        if (!strcmp(opcode, "add")) {
            if (!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) {
                exit(1);
            }
            int opp = 0; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            int regc = atoi(arg2); 
            if (rega < 0 || rega > 7 || regb < 0 || regb > 7 || regc < 0 || regc > 7) {
                exit(1);
            }

            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (regc);  
            
        }
        else if (!strcmp(opcode, "nor")) {

            if (!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) {
                exit(1);
            }

            int opp = 1; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            int dregc = atoi(arg2); 

            if (rega < 0 || rega > 7 || regb < 0 || regb > 7 || dregc < 0 || dregc > 7) {
                exit(1);
            }

            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (dregc);  
            
            
        }
        else if (!strcmp(opcode, "lw")) {
            int opp = 2; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            int offset = 0; 

            if (!isNumber(arg0) || !isNumber(arg1)) {
                exit(1);
            }
            
            if (isNumber(arg2)) {
                offset = atoi(arg2); 
            }
            else {
                offset = findLabelAddress(arg2); 
            }
            if (offset < -32768 || offset > 32767) {
                exit(1);
            }

            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF); 
            
            
        }
        else if (!strcmp(opcode, "sw")) {

            int opp = 3; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            int offset = 0; 
            if (!isNumber(arg0) || !isNumber(arg1)) {
                exit(1);
            }
            
            if (isNumber(arg2)) {
                offset = atoi(arg2); 
            }
            else {
                offset = findLabelAddress(arg2); 
            }
            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }
            if (offset < -32768 || offset > 32767) {
                exit(1);
            }
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF); 
        
        }
        else if (!strcmp(opcode, "beq")) {
            if (!isNumber(arg0) || !isNumber(arg1)) {
                exit(1);
            }
            int opp = 4; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            int target = 0; 
            int offset = 0; 
            
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            
            }
            else {
                target = findLabelAddress(arg2); 
                offset = target - (count + 1);
            }
            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }
            if (offset < -32768 || offset > 32767) {
                exit(1);
            }
        
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "jalr")) {
            if (!isNumber(arg0) || !isNumber(arg1)) {
                exit(1);
            }
            int opp = 5; 
            int rega = atoi(arg0); 
            int regb = atoi(arg1); 
            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }
            machineCode = (opp << 22) | (rega << 19) | (regb << 16); 


        }
        else if (!strcmp(opcode, "halt")) {
            int opp = 6;
            machineCode = opp << 22;
        }
        else if (!strcmp(opcode, "noop")) {
            int opp = 7;
            machineCode = opp << 22;
        }
        else if (!strcmp(opcode, ".fill")) {
            
            if (isNumber(arg0)) {
                machineCode = atoi(arg0);
            } 
            else {
                machineCode = findLabelAddress(arg0);  
            }
            
        }
        else if (!strcmp(opcode, "b")) {
            

            int opp = 4; 
            int rega = 0; 
            int regb = 0; 
            int offset = 0; 
            
            if (isNumber(arg0)) {
                offset = atoi(arg0); 
            }
            else {
                offset = findLabelAddress(arg0) - (count + 1); 
            }
            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }
            if (offset < -32768 || offset > 32767) {
                exit(1);
            }
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF); 
        }
        
        else if (!strcmp(opcode, "jump")) {
            if (!isNumber(arg0)) {
                exit(1);
            }
            int opp = 5; 
            int rega = atoi(arg0); 
            int regb = 0; 

            if (rega < 0 || rega > 7 || regb < 0 || regb > 7) {
                exit(1);
            }

            machineCode = (opp << 22) | (rega << 19) | (regb << 16); 
        }
        else {
            exit(1); 
        }
        count += 1; 
        printHexToFile(outFilePtr, machineCode);
}
    return(0);
}



// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;   
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}


// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}

// Returns 1 if string ends with substr, 0 otherwise
static int
endsWith(char *string, char *substr) {
    size_t stringLen = strlen(string);
    size_t substrLen = strlen(substr);
    if (stringLen < substrLen) {
        return 0; // string too short
    }
    char *stringEnd = string + stringLen - substrLen;
    if (strcmp(stringEnd, substr) == 0) {
        return 1;
    }
    return 0;
}
