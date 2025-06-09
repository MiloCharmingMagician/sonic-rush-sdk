// WIP Sonic Rush AYK Script Compiler

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

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
} Header;

int main(int argc, char *argv[]) {
    printf("Sonic Rush AYK Compiler v1.0\n");
    printf("(C) 2025 Milo Charming Magician\n");

    #ifdef DEBUG
    argv[1] = "demo";
    argv[2] = "demo.ayk"
    #else
    if (argc != 3) {
        printf("Usage: %s [INPUT] [OUTPUT]\n", argv[0]);
        return 1;
    }
    #endif

#ifdef DEBUG
    system("pause");
#endif

    return 0;
}
