#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;

#pragma pack(push, 1)

typedef struct {
    u32 magic;       // '#AMA' -> 0x414D4123
    u32 entryCount;
    u32 fixed;       // always 16
} Header;

typedef struct {
    u32 offset;
    u32 unk1;
	u32 unk2;
	u32 unk3;
	u32 unk4;
	u32 unk5;
} EntryTable;

typedef struct {
    u32 x;
    u32 y;
	u32 unk1;
	u32 unk2;
	u32 unk3;
	u32 unk4;
	u32 unk5;
	u32 unk6;
} Entry;

#pragma pack(pop)

#define MAX_ENTRIES 1024
#define MAX_LINE_LEN 256

int main(int argc, char* argv[]) {
    char* inputfile;
    char* outputfile;
    FILE* fin;
    FILE* fout;
    Header header;
    EntryTable entryTables[MAX_ENTRIES];
    Entry entries[MAX_ENTRIES];
    char line[MAX_LINE_LEN];
    int count = 0;
    int i, j;
    u32 baseOffset;

#ifdef DEBUG
    inputfile = "demo.txt";
    outputfile = "demo.ama";
#else
    if (argc < 3) {
        printf("Usage: %s [INPUT.TXT] [OUTPUT.AMA]\n", argv[0]);
        return 1;
    }
    inputfile = argv[1];
    outputfile = argv[2];
#endif

    fin = fopen(inputfile, "r");
    if (!fin) {
        perror("Failed to open input TXT file");
        return 1;
    }

    // Parse input lines
    while (fgets(line, sizeof(line), fin)) {
        int x, y;
        if (line[0] == '#' || strlen(line) < 3)
            continue;

        if (sscanf(line, "%d , %d", &x, &y) == 2) {
            if (count >= MAX_ENTRIES) {
                printf("Exceeded max entry count (%d)\n", MAX_ENTRIES);
                break;
            }

            memset(&entries[count], 0, sizeof(Entry));
			entries[count].unk1 = 0;
			entries[count].unk2 = 0;
            entries[count].x = (u32)x;
            entries[count].y = (u32)y;
			entries[count].unk3 = 4096;
			entries[count].unk4 = 0;
			entries[count].unk5 = 0;
			entries[count].unk6 = 0;

            count++;
        }
    }
    fclose(fin);

    // Build header
    header.magic = 0x414D4123;
    header.entryCount = (u32)count;
    header.fixed = 16;

    // Entry offset starts AFTER header + entry table block
    baseOffset = sizeof(Header) + sizeof(EntryTable) * count;

    for (i = 0; i < count; i++) {
		entryTables[i].unk1 = 0;
		entryTables[i].unk2 = 0;
		entryTables[i].unk3 = 0;
		entryTables[i].unk4 = 0;
		entryTables[i].unk5 = 0;

        entryTables[i].offset = baseOffset + sizeof(Entry) * i;
    }

    fout = fopen(outputfile, "wb");
    if (!fout) {
        perror("Failed to open output AMA file");
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
