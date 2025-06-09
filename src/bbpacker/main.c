#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 65535
#define MAX_PATH 260

typedef unsigned int u32;

typedef struct {
    u32 offset;
    u32 length;
} FileEntry;

int main(int argc, char *argv[]) {
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    char *file_paths[MAX_FILES];
    FileEntry *file_entries = NULL;
    u32 file_count = 0;
    u32 current_offset;
    FILE *out, *in;
    char search_path[MAX_PATH];
    char output_file[MAX_PATH];
    char *buffer;
    int i;

    printf("Sonic Rush BB Archive Packer v1.0\n");
    printf("(C) 2025 Milo Charming Magician\n");

    #ifdef DEBUG
    argv[1] = "demo";
    argv[2] = "demo.bb"
    #else
    if (argc != 3) {
        printf("Usage: %s [INPUT] [OUTPUT]\n", argv[0]);
        return 1;
    }
    #endif

    strncpy(search_path, argv[1], MAX_PATH - 3);
    search_path[MAX_PATH - 3] = '\0';
    strcat(search_path, "\\*");

    hFind = FindFirstFile(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error opening folder: %s\n", argv[1]);
        return 1;
    }

    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (file_count >= MAX_FILES) break;

            file_paths[file_count] = (char *)malloc(MAX_PATH);
            if (!file_paths[file_count]) {
                printf("Memory allocation failed.\n");
                FindClose(hFind);
                return 1;
            }
            snprintf(file_paths[file_count], MAX_PATH, "%s\\%s", argv[1], fd.cFileName);
            file_count++;
        }
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);

    if (file_count == 0) {
        printf("No files found.\n");
        return 1;
    }

    file_entries = (FileEntry *)malloc(file_count * sizeof(FileEntry));
    if (!file_entries) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    out = fopen(output_file, "wb");
    if (!out) {
        perror("fopen");
        return 1;
    }

    // Write file count as u32
    fwrite(&file_count, sizeof(u32), 1, out);

    // Reserve space for file entries
    current_offset = sizeof(u32) + file_count * sizeof(FileEntry);
    fseek(out, current_offset, SEEK_SET);

    for (i = 0; i < file_count; i++) {
        in = fopen(file_paths[i], "rb");
        if (!in) {
            printf("Error opening file: %s\n", file_paths[i]);
            fclose(out);
            return 1;
        }

        fseek(in, 0, SEEK_END);
        file_entries[i].length = (u32)ftell(in);
        rewind(in);

        buffer = (char *)malloc(file_entries[i].length);
        if (!buffer) {
            printf("Memory allocation failed.\n");
            fclose(in);
            fclose(out);
            return 1;
        }

        fread(buffer, 1, file_entries[i].length, in);
        fclose(in);

        file_entries[i].offset = current_offset;
        fwrite(buffer, 1, file_entries[i].length, out);
        free(buffer);

        current_offset += file_entries[i].length;
    }

    // Write file entries table (offset + length)
    fseek(out, sizeof(u32), SEEK_SET);
    fwrite(file_entries, sizeof(FileEntry), file_count, out);

    fclose(out);

    for (i = 0; i < file_count; i++) free(file_paths[i]);
    free(file_entries);

    printf("Packed %d file(s) into %s\n", file_count, output_file);
    return 0;
}
