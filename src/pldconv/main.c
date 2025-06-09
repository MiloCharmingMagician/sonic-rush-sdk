// WIP Sonic Rush PLD Event Converter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

typedef unsigned long u32;

typedef struct {
    u32 magic; // #PLD
} Header;

int main(int argc, char *argv[]) {
    printf("Sonic Rush PLD Event Converter v1.0\n");
    printf("(C) 2025 Milo Charming Magician\n");

    #ifdef DEBUG
    argv[1] = "demo.txt";
    argv[2] = "demo.pld"
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
