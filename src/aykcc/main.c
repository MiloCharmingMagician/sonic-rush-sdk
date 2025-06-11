// WIP Sonic Rush AYK Script Compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_COMMANDS 1024

typedef unsigned char u8;
typedef unsigned long u32;

// Command structure
typedef struct {
    u8 type;
    u8 param1;
    u8 param2;
    u8 _padding;
} ScriptCommand_;

// Header structure
typedef struct {
    u32 magic;
    u32 textSize;
    u32 textOffset;
    u32 entryAddress;
    u32 rodataSize;
    u32 rodataAddress;
    u32 dataSize;
    u32 dataAddress;
    u32 bssSize;
} ScriptHeader;

// Function declarations
int GetCommandOpcode(const char *name);
void TrimLine(char *line);
int ParseLine(const char *line, ScriptCommand_ *cmd);
void WriteOutput(const char *filename, ScriptCommand_ *cmds, int count);

int main(int argc, char **argv) {
    FILE *in;
    char line[MAX_LINE_LENGTH];
    ScriptCommand_ commands[MAX_COMMANDS];
    int commandCount;
    char *inputFile;
    char *outputFile;

    commandCount = 0;

    printf("Sonic Rush AYK Compiler v1.0\n");
    printf("(C) 2025 Milo Charming Magician\n");

#ifdef DEBUG
    inputFile = "demo.cs";
    outputFile = "demo.ayk";
#else
    if (argc < 3) {
        printf("Usage: aykcc [INPUT] [OUTPUT]\n");
        return 1;
    }

    inputFile = argv[1];
    outputFile = argv[2];
#endif

    in = fopen(inputFile, "r");
    if (!in) {
        printf("Error opening input file.\n");
        return 1;
    }

    while (fgets(line, sizeof(line), in)) {
        TrimLine(line);

        if (line[0] == '\0') {
            continue;
        }

        if (commandCount >= MAX_COMMANDS) {
            printf("Too many commands.\n");
            break;
        }

        if (ParseLine(line, &commands[commandCount])) {
            commandCount++;
        }
    }

    fclose(in);

    WriteOutput(outputFile, commands, commandCount);

    printf("Compiled %d commands to %s\n", commandCount, outputFile);

#ifdef DEBUG
    system("pause");
#endif

    return 0;
}

// Command lookup
int GetCommandOpcode(const char *name) {
    if (strcmp(name, "Move") == 0) return 1;
    if (strcmp(name, "Jump") == 0) return 2;
    if (strcmp(name, "PlaySound") == 0) return 3;
    return -1;
}

// Trim whitespace and strip comments
void TrimLine(char *line) {
    char *comment;
    int len;

    comment = strstr(line, "//");
    if (comment) {
        *comment = '\0';
    }

    len = (int)strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                       line[len - 1] == ' ' || line[len - 1] == '\t')) {
        line[--len] = '\0';
    }
}

// Parse a line into a command struct
int ParseLine(const char *line, ScriptCommand_ *cmd) {
    char name[32];
    int param1, param2;
    int opcode;

    if (sscanf(line, "%31[^,],%d,%d", name, &param1, &param2) != 3) {
        return 0;
    }

    opcode = GetCommandOpcode(name);
    if (opcode < 0) {
        printf("Unknown command: %s\n", name);
        return 0;
    }

    cmd->type = (u8)opcode;
    cmd->param1 = (u8)param1;
    cmd->param2 = (u8)param2;
    cmd->_padding = 0;

    return 1;
}

// Write binary output file
void WriteOutput(const char *filename, ScriptCommand_ *cmds, int count) {
    FILE *out;
    ScriptHeader header;

    out = fopen(filename, "wb");
    if (!out) {
        printf("Error writing to output file.\n");
        return;
    }

    header.magic = 0x004B5941; // 'AYK\0'
    header.textSize = (u32)(count * sizeof(ScriptCommand_));
    header.textOffset = sizeof(ScriptHeader);
    header.entryAddress = 0;
    header.rodataSize = 0;
    header.rodataAddress = 0;
    header.dataSize = 0;
    header.dataAddress = 0;
    header.bssSize = 0;

    fwrite(&header, sizeof(ScriptHeader), 1, out);
    fwrite(cmds, sizeof(ScriptCommand_), count, out);

    fclose(out);
}