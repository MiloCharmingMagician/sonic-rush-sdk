#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;

#pragma pack(push, 1)

typedef struct {
    u32 magic;
    u32 entryCount;
    u32 fixed;
    u32 unk[5];
} Header;

typedef struct {
    u32 offset;
    u32 unk[7];
} EntryTable;

typedef struct {
    u32 unk[6];
    u32 x;
    u32 y;
} Entry;

#pragma pack(pop)

int main(int argc, char* argv[]) {
    char* inputfile;
    char* outputfile;
    FILE* fin;
    FILE* fout;
    Header header;
    EntryTable* etb;
    Entry entry;
    int i;
    int entryCount;

#ifdef DEBUG
    inputfile = "demo.ama";
    outputfile = "demo.txt";
#else
    if (argc < 3) {
        printf("Usage: %s [INPUT] [OUTPUT]\n", argv[0]);
        return 1;
    }
    inputfile = argv[1];
    outputfile = argv[2];
#endif

    fin = fopen(inputfile, "rb");
    if (!fin) {
        perror("Failed to open input AMA file");
        return 1;
    }

    if (fread(&header, sizeof(Header), 1, fin) != 1) {
        printf("Failed to read header.\n");
        fclose(fin);
        return 1;
    }

    if (header.magic != 0x414D4123) {
        printf("Invalid AMA file magic.\n");
        fclose(fin);
        return 1;
    }

    entryCount = (int)header.entryCount;
    etb = (EntryTable*)malloc(sizeof(EntryTable) * entryCount);
    if (!etb) {
        perror("Memory allocation failed");
        fclose(fin);
        return 1;
    }

    if (fread(etb, sizeof(EntryTable), entryCount, fin) != (size_t)entryCount) {
        printf("Failed to read entry table.\n");
        free(etb);
        fclose(fin);
        return 1;
    }

    fout = fopen(outputfile, "w");
    if (!fout) {
        perror("Failed to open output TXT file");
        free(etb);
        fclose(fin);
        return 1;
    }

    for (i = 0; i < entryCount; i++) {
        if (fseek(fin, etb[i].offset, SEEK_SET) != 0) {
            printf("Seek failed at entry %d\n", i);
            continue;
        }

        if (fread(&entry, sizeof(Entry), 1, fin) != 1) {
            printf("Failed to read entry %d\n", i);
            continue;
        }

        fprintf(fout, "# Object %d\n", i + 1);
        fprintf(fout, "%d, %d\n", entry.x, entry.y);
    }

    fclose(fout);
    fclose(fin);
    free(etb);

    printf("Success: Extracted %d entries to %s\n", entryCount, outputfile);

#ifdef DEBUG
    system("pause");
#endif

    return 0;
}
