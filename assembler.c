#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
#define MAXLABEL 64

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);
static int endsWith(char *, char *);

struct labelTable {
    char lab[MAXLABEL];
    char section;
    int offset;
};
struct labelTable LT[MAXLINELENGTH];
int labelCount = 0;

struct symbolEntry {
    char lab[MAXLABEL];
    char section;
    int offset;
};
struct symbolEntry symbols[MAXLINELENGTH];
int symbolCount = 0;

struct relocEntry {
    int offset;
    char opcode[16];
    char lab[MAXLABEL];
};
struct relocEntry relocs[MAXLINELENGTH];
int relocCount = 0;

int textWords[MAXLINELENGTH];
int dataWords[MAXLINELENGTH];
int textCount = 0;
int dataCount = 0;

static int isGlobal(char *label) {
    return label[0] >= 'A' && label[0] <= 'Z';
}

static int findLabel(char *labelName) {
    for (int i = 0; i < labelCount; i++) {
        if (!strcmp(LT[i].lab, labelName)) {
            return i;
        }
    }
    return -1;
}

static void addUndefinedSymbol(char *labelName) {
    for (int i = 0; i < symbolCount; i++) {
        if (!strcmp(symbols[i].lab, labelName)) {
            return;
        }
    }
    strcpy(symbols[symbolCount].lab, labelName);
    symbols[symbolCount].section = 'U';
    symbols[symbolCount].offset = 0;
    symbolCount += 1;
}

static int resolveLabel(char *labelName, int allowUndefined) {
    int idx = findLabel(labelName);
    if (idx < 0) {
        if (!allowUndefined || !isGlobal(labelName)) {
            exit(1);
        }
        addUndefinedSymbol(labelName);
        return 0;
    }
    if (LT[idx].section == 'T') {
        return LT[idx].offset;
    }
    return textCount + LT[idx].offset;
}

static void addReloc(int offset, char *opcode, char *labelName) {
    relocs[relocCount].offset = offset;
    strcpy(relocs[relocCount].opcode, opcode);
    strcpy(relocs[relocCount].lab, labelName);
    relocCount += 1;
}

static int checkReg(char *arg) {
    if (!isNumber(arg)) {
        exit(1);
    }
    int reg = atoi(arg);
    if (reg < 0 || reg > 7) {
        exit(1);
    }
    return reg;
}

static int checkOffset(int offset) {
    if (offset < -32768 || offset > 32767) {
        exit(1);
    }
    return offset;
}

int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <object-code-file>\n",
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

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        int isData = !strcmp(opcode, ".fill");

        if (label[0] != '\0') {
            if (strlen(label) >= MAXLABEL || findLabel(label) >= 0) {
                exit(1);
            }
            strcpy(LT[labelCount].lab, label);
            LT[labelCount].section = isData ? 'D' : 'T';
            LT[labelCount].offset = isData ? dataCount : textCount;
            labelCount += 1;
        }

        if (isData) {
            dataCount += 1;
        } else {
            textCount += 1;
        }
    }

    rewind(inFilePtr);

    int textIdx = 0;
    int dataIdx = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        int machineCode = 0;

        if (!strcmp(opcode, "add")) {
            int opp = 0;
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            int dest = checkReg(arg2);
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | dest;
        }
        else if (!strcmp(opcode, "nor")) {
            int opp = 1;
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            int dest = checkReg(arg2);
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | dest;
        }
        else if (!strcmp(opcode, "lw")) {
            int opp = 2;
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            int offset;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            } else {
                offset = resolveLabel(arg2, 1);
                addReloc(textIdx, "lw", arg2);
            }
            checkOffset(offset);
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "sw")) {
            int opp = 3;
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            int offset;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            } else {
                offset = resolveLabel(arg2, 1);
                addReloc(textIdx, "sw", arg2);
            }
            checkOffset(offset);
            machineCode = (opp << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "beq")) {
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            int offset;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            } else {
                offset = resolveLabel(arg2, 0) - (textIdx + 1);
            }
            checkOffset(offset);
            machineCode = (4 << 22) | (rega << 19) | (regb << 16) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "jalr")) {
            int rega = checkReg(arg0);
            int regb = checkReg(arg1);
            machineCode = (5 << 22) | (rega << 19) | (regb << 16);
        }
        else if (!strcmp(opcode, "halt")) {
            machineCode = 6 << 22;
        }
        else if (!strcmp(opcode, "noop")) {
            machineCode = 7 << 22;
        }
        else if (!strcmp(opcode, "b")) {
            int offset;
            if (isNumber(arg0)) {
                offset = atoi(arg0);
            } else {
                offset = resolveLabel(arg0, 0) - (textIdx + 1);
            }
            checkOffset(offset);
            machineCode = (4 << 22) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "jump")) {
            int rega = checkReg(arg0);
            machineCode = (5 << 22) | (rega << 19);
        }
        else if (!strcmp(opcode, ".fill")) {
            if (isNumber(arg0)) {
                machineCode = atoi(arg0);
            } else {
                machineCode = resolveLabel(arg0, 1);
                addReloc(dataIdx, ".fill", arg0);
            }
            dataWords[dataIdx++] = machineCode;
            continue;
        }
        else {
            exit(1);
        }
        textWords[textIdx++] = machineCode;
    }

    for (int i = 0; i < labelCount; i++) {
        if (isGlobal(LT[i].lab)) {
            strcpy(symbols[symbolCount].lab, LT[i].lab);
            symbols[symbolCount].section = LT[i].section;
            symbols[symbolCount].offset = LT[i].offset;
            symbolCount += 1;
        }
    }

    fprintf(outFilePtr, "%d %d %d %d\n", textCount, dataCount, symbolCount, relocCount);
    for (int i = 0; i < textCount; i++) {
        printHexToFile(outFilePtr, textWords[i]);
    }
    for (int i = 0; i < dataCount; i++) {
        printHexToFile(outFilePtr, dataWords[i]);
    }
    for (int i = 0; i < symbolCount; i++) {
        fprintf(outFilePtr, "%s %c %d\n", symbols[i].lab, symbols[i].section, symbols[i].offset);
    }
    for (int i = 0; i < relocCount; i++) {
        fprintf(outFilePtr, "%d %s %s\n", relocs[i].offset, relocs[i].opcode, relocs[i].lab);
    }

    fclose(outFilePtr);
    fclose(inFilePtr);
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
