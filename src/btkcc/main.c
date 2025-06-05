#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned int u32;

#define MAX_ACTIONS 1024
#define MAX_LINES 256

typedef struct {
    u32 magic;
    u32 unk;
    u32 lang;
    u32 unk2;
    u32 actionCount;
} Header;

typedef struct {
    u32 color;
    u32 unk1;
    u32 unk2;
    u32 effect;
    u32 character;
    u32 expression;
    u32 unk3;
    u32 initFlag;
    u32 endFlag;
    u32 bubble;
    u32 unk4;
    u32 bubbleFlag;
    u32 response;
    u32 responseDelay;
    u32 endDelay;
} Action;

typedef struct {
    Header header;
    Action actions[MAX_ACTIONS];
} BTKFile;

// --- Mapping functions ---

// Trim whitespace from start and end of a string (in place)
void trim(char *s) {
    char *p = s;
    int l = strlen(p);
    while (l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while (*p && isspace(*p)) ++p, --l;
    if (p != s) memmove(s, p, l + 1);
}

int stricmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower(*a);
        char cb = tolower(*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return *a - *b;
}

// Map language string to int
u32 map_lang(const char *lang) {
    if (!lang) return 1; // default English
    if (!stricmp(lang, "english")) return 1;
    if (!stricmp(lang, "french")) return 2;
    if (!stricmp(lang, "italian")) return 4;
    return 1;
}

// Map character string to int
u32 map_character(const char *c) {
    if (!stricmp(c, "none")) return 0;
    if (!stricmp(c, "sonic")) return 1;
    if (!stricmp(c, "tails")) return 2;
    if (!stricmp(c, "blaze")) return 3;
    if (!stricmp(c, "knuckles")) return 4;
    return 0;
}

// Map expression string to int
u32 map_expression(const char *e) {
    if (!stricmp(e, "none")) return 0;
    if (!stricmp(e, "happy")) return 1;
    if (!stricmp(e, "angry")) return 2;
    if (!stricmp(e, "confused")) return 3;
    if (!stricmp(e, "sad")) return 4;
    return 0;
}

// Map effect string to int
u32 map_effect(const char *e) {
    if (!stricmp(e, "natural")) return 1;       // was "normal"
    if (!stricmp(e, "explode")) return 2;
    if (!stricmp(e, "glitch")) return 3;
    if (!stricmp(e, "flicking")) return 4;
    if (!stricmp(e, "blank")) return 5;
    if (!stricmp(e, "windSfx")) return 10;      // renamed eggmanwindsfx
    if (!stricmp(e, "gemSfx")) return 11;
    return 1;
}

// Map initFlag/endFlag string to int
u32 map_flag(const char *f) {
    if (!stricmp(f, "none")) return 0;
    if (!stricmp(f, "leftSlide")) return 5;
    if (!stricmp(f, "rightSlide")) return 8;
    return 0;
}

// Map bubble string to int
u32 map_bubble(const char *b) {
    if (!stricmp(b, "none")) return 0;
    if (!stricmp(b, "neutral")) return 1;    // renamed from normal
    if (!stricmp(b, "exclamation")) return 2; // word for "!"
    return 0;
}

// Map bubbleFlag string to int (true/false)
u32 map_bubbleFlag(const char *bf) {
    if (!stricmp(bf, "true")) return 1;
    if (!stricmp(bf, "false")) return 0;
    return 0;
}

// Map response string to int
u32 map_response(const char *r) {
    if (!stricmp(r, "none")) return 0;
    if (!stricmp(r, "waitInput1")) return 1;
    if (!stricmp(r, "waitInput2")) return 2;
    if (!stricmp(r, "continue")) return 3;
    return 0;
}

// Map color string or int to int
// We support the word "blue" or a numeric value
u32 map_color(const char *c) {
    if (!stricmp(c, "blue")) return 255; // example blue color code
    // else parse as integer
    return (u32)atoi(c);
}

// --- Parsing helper to read key=value ---
int parse_key_value(char *line, char **key, char **value) {
    char *eq = strchr(line, '=');
    if (!eq) return 0;
    *eq = 0;
    *key = line;
    *value = eq + 1;
    trim(*key);
    trim(*value);
    return 1;
}

// --- Read and parse the text file ---
int read_script(const char *filename, BTKFile *btk) {
    FILE *f = fopen(filename, "rt");
    if (!f) {
        printf("Failed to open %s\n", filename);
        return 0;
    }

    char line[MAX_LINES];
    int in_action = 0;
    int action_index = 0;
    btk->header.actionCount = 0;
    btk->header.magic = 0x4B54423; // '#BTK' in little endian
    btk->header.unk = 0;
    btk->header.unk2 = 0;
    btk->header.lang = 1; // default English

    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == 0 || line[0] == ';' || line[0] == '#') continue; // skip empty or comment

        if (!stricmp(line, "[Action]")) {
            if (action_index >= MAX_ACTIONS) {
                printf("Reached max actions %d, ignoring further actions\n", MAX_ACTIONS);
                break;
            }
            in_action = 1;
            // zero clear new action
            memset(&btk->actions[action_index], 0, sizeof(Action));
            action_index++;
            continue;
        }

        if (!in_action) {
            // parse global keys (only lang)
            char *key, *value;
            if (parse_key_value(line, &key, &value)) {
                if (!stricmp(key, "lang")) {
                    btk->header.lang = map_lang(value);
                }
            }
        } else {
            // parse action keys
            char *key, *value;
            if (!parse_key_value(line, &key, &value)) {
                printf("Invalid line in action: %s\n", line);
                continue;
            }

            Action *act = &btk->actions[action_index - 1];

            if (!stricmp(key, "color")) act->color = map_color(value);
            else if (!stricmp(key, "effect")) act->effect = map_effect(value);
            else if (!stricmp(key, "character")) act->character = map_character(value);
            else if (!stricmp(key, "expression")) act->expression = map_expression(value);
            else if (!stricmp(key, "initFlag")) act->initFlag = map_flag(value);
            else if (!stricmp(key, "endFlag")) act->endFlag = map_flag(value);
            else if (!stricmp(key, "bubble")) act->bubble = map_bubble(value);
            else if (!stricmp(key, "bubbleFlag")) act->bubbleFlag = map_bubbleFlag(value);
            else if (!stricmp(key, "response")) act->response = map_response(value);
            else if (!stricmp(key, "responseDelay")) act->responseDelay = (u32)atoi(value);
            else if (!stricmp(key, "endDelay")) act->endDelay = (u32)atoi(value);
            else {
                printf("Unknown key in action: %s\n", key);
            }
        }
    }

    btk->header.actionCount = action_index;

    fclose(f);
    return 1;
}

// --- Write binary file ---
int write_btk_binary(const char *filename, BTKFile *btk) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Failed to open %s for writing\n", filename);
        return 0;
    }

    fwrite(&btk->header.magic, sizeof(u32), 1, f);
    fwrite(&btk->header.unk, sizeof(u32), 1, f);
    fwrite(&btk->header.lang, sizeof(u32), 1, f);
    fwrite(&btk->header.unk2, sizeof(u32), 1, f);
    fwrite(&btk->header.actionCount, sizeof(u32), 1, f);

    int i;
    for (i = 0; i < (int)btk->header.actionCount; i++) {
        Action *a = &btk->actions[i];
        fwrite(&a->color, sizeof(u32), 1, f);
        fwrite(&a->unk1, sizeof(u32), 1, f);
        fwrite(&a->unk2, sizeof(u32), 1, f);
        fwrite(&a->effect, sizeof(u32), 1, f);
        fwrite(&a->character, sizeof(u32), 1, f);
        fwrite(&a->expression, sizeof(u32), 1, f);
        fwrite(&a->unk3, sizeof(u32), 1, f);
        fwrite(&a->initFlag, sizeof(u32), 1, f);
        fwrite(&a->endFlag, sizeof(u32), 1, f);
        fwrite(&a->bubble, sizeof(u32), 1, f);
        fwrite(&a->unk4, sizeof(u32), 1, f);
        fwrite(&a->bubbleFlag, sizeof(u32), 1, f);
        fwrite(&a->response, sizeof(u32), 1, f);
        fwrite(&a->responseDelay, sizeof(u32), 1, f);
        fwrite(&a->endDelay, sizeof(u32), 1, f);
    }

    fclose(f);
    return 1;
}

// --- Print debug info ---
void print_debug(BTKFile *btk) {
    // Print lang
    printf("Language: ");
    switch (btk->header.lang) {
        case 1: printf("English\n"); break;
        case 2: printf("French\n"); break;
        case 4: printf("Italian\n"); break;
        default: printf("Unknown (%u)\n", btk->header.lang); break;
    }

    printf("Actions (%u):\n", btk->header.actionCount);
    int i;
    for (i = 0; i < (int)btk->header.actionCount; i++) {
        Action *a = &btk->actions[i];
        printf("Action %d:\n", i + 1);
        printf("  color = %u\n", a->color);
        printf("  effect = %u\n", a->effect);
        printf("  character = %u\n", a->character);
        printf("  expression = %u\n", a->expression);
        printf("  initFlag = %u\n", a->initFlag);
        printf("  endFlag = %u\n", a->endFlag);
        printf("  bubble = %u\n", a->bubble);
        printf("  bubbleFlag = %u\n", a->bubbleFlag);
        printf("  response = %u\n", a->response);
        printf("  responseDelay = %u\n", a->responseDelay);
        printf("  endDelay = %u\n", a->endDelay);
    }
}

int main(int argc, char *argv[]) {
    char* inputfile;
    char* outputfile;

    #ifdef DEBUG
    inputFile = "demo.txt";
    outputFile = "demo.btk";
    #else
    if (argc < 2) {
        printf("Usage: %s [INPUT] [OUTPUT] \n", argv[0]);
        return 1;
    }
    inputfile = argv[1];
    outputfile = argv[2];
    #endif

    BTKFile btk;
    memset(&btk, 0, sizeof(BTKFile));

    if (!read_script(inputfile, &btk)) {
        printf("Failed to read script.\n");
        return 1;
    }

    print_debug(&btk);

    if (!write_btk_binary(outputfile, &btk)) {
        printf("Failed to write binary.\n");
        return 1;
    }

    printf("Successfully wrote %s\n", outputfile);

    return 0;
}
