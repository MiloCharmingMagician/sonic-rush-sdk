#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;

#pragma pack(push, 1) // disable padding

typedef struct {
    u32 magic;       // '#AMA' -> 0x414D4123
    u32 entryCount;
    u32 fixed;       // always 16
    u32 unk[5];      // zeroed
} Header;

typedef struct {
    u32 offset;
    u32 unk[7];      // zeroed
} EntryTable;

typedef struct {
    u32 unk[6];      // zeroed
    u32 x;
    u32 y;
} Entry;

#pragma pack(pop)

#define MAX_ENTRIES 1024
#define MAX_LINE_LENGTH 256

int main(int argc, char* argv[]) {
    char* inputfile;
    char* outputfile;
    FILE* fin;
    FILE* fout;
    Entry entries[MAX_ENTRIES];
    EntryTable entryTables[MAX_ENTRIES];
    Header header;
    char line[MAX_LINE_LENGTH];
    int count = 0;
    int i;
    u32 baseOffset;

#ifdef DEBUG
    inputfile = "demo.txt";
    outputfile = "demo.ama";
#else
    if (argc < 3) {
        printf("Usage: %s [INPUT] [OUTPUT]\n", argv[0]);
        return 1;
    }
    inputfile = argv[1];
    outputfile = argv[2];
#endif

    fin = fopen(inputfile, "r");
    if (!fin) {
        perror("Failed to open input file");
        return 1;
    }

    while (fgets(line, sizeof(line), fin)) {
        int x, y;
        if (line[0] == '#' || strlen(line) < 3)
            continue;

        if (sscanf(line, "%d , %d", &x, &y) == 2) {
            if (count >= MAX_ENTRIES) {
                printf("Exceeded maximum entry count (%d)\n", MAX_ENTRIES);
                break;
            }
            memset(&entries[count], 0, sizeof(Entry));
            entries[count].x = (u32)x;
            entries[count].y = (u32)y;
            count++;
        }
    }
    fclose(fin);

    header.magic = 0x414D4123; // '#AMA'
    header.entryCount = (u32)count;
    header.fixed = 16;
    memset(header.unk, 0, sizeof(header.unk));

    baseOffset = sizeof(Header) + sizeof(EntryTable) * count;
    for (i = 0; i < count; i++) {
        entryTables[i].offset = baseOffset + i * sizeof(Entry);
        memset(entryTables[i].unk, 0, sizeof(entryTables[i].unk));
    }

    fout = fopen(outputfile, "wb");
    if (!fout) {
        perror("Failed to open output file");
        return 1;
    }

    fwrite(&header, sizeof(Header), 1, fout);
    fwrite(entryTables, sizeof(EntryTable), count, fout);
    fwrite(entries, sizeof(Entry), count, fout);

    fclose(fout);
    printf("Success: Wrote %d entries to %s\n", count, outputfile);

#ifdef DEBUG
    system("pause");
#endif

    return 0;
}
