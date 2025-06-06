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
    u32 language;
    u32 unk2;
    u32 actionCount;
    u32 color;
	u32 unk3;
} Header;

typedef struct {
    u32 unk;
    u32 effect;
    u32 character;
    u32 expression;
    u32 unk2;
    u32 initFlag;
    u32 endFlag;
    u32 bubble;
    u32 unk3;
    u32 bubbleFlag;
    u32 response;
    u32 responseDelay;
    u32 endDelay;
} Action;

typedef struct {
    Header header;
    Action actions[MAX_ACTIONS];
} BTKFile;

// --- String helpers ---

void trim(char *s) {
    char *p;
    int l;
    p = s;
    l = (int)strlen(p);
    while (l > 0 && isspace((unsigned char)p[l - 1])) {
        p[--l] = 0;
    }
    while (*p && isspace((unsigned char)*p)) {
        ++p;
        --l;
    }
    if (p != s) {
        memmove(s, p, l + 1);
    }
}

void strip_inline_comment(char *s) {
    char *hash = strchr(s, '#');
    if (hash) {
        *hash = 0;
    }
}

// --- Mapping functions ---

u32 map_language(const char *lang) {
    if (!lang) return 1;
    if (!_stricmp(lang, "english")) return 1;
    if (!_stricmp(lang, "french")) return 2;
    if (!_stricmp(lang, "italian")) return 4;
    return 1;
}

u32 map_character(const char *c) {
    if (!_stricmp(c, "sonic")) return 1;
    if (!_stricmp(c, "tails")) return 2;
    if (!_stricmp(c, "blaze")) return 3;
    if (!_stricmp(c, "knuckles")) return 4;
	if (!_stricmp(c, "eggman")) return 8;
    return 0;
}

u32 map_expression(const char *e) {
    if (!_stricmp(e, "happy")) return 1;
    if (!_stricmp(e, "angry")) return 2;
    if (!_stricmp(e, "confused")) return 3;
    if (!_stricmp(e, "sad")) return 4;
    return 0;
}

u32 map_effect(const char *e) {
    if (!_stricmp(e, "natural")) return 1;
    if (!_stricmp(e, "explode")) return 2;
    if (!_stricmp(e, "glitch")) return 3;
    if (!_stricmp(e, "flicking")) return 4;
    if (!_stricmp(e, "blank")) return 5;
    if (!_stricmp(e, "windSfx")) return 10;
    if (!_stricmp(e, "gemSfx")) return 11;
    return 1;
}

u32 map_flag(const char *f) {
    if (!_stricmp(f, "leftSlide")) return 5;
    if (!_stricmp(f, "rightSlide")) return 8;
    return 0;
}

u32 map_bubble(const char *b) {
    if (!_stricmp(b, "neutral")) return 1;
    if (!_stricmp(b, "exclamation")) return 2;
    return 0;
}

u32 map_bubbleFlag(const char *bf) {
    if (!_stricmp(bf, "true")) return 1;
    if (!_stricmp(bf, "false")) return 0;
    return 0;
}

u32 map_response(const char *r) {
    if (!_stricmp(r, "waitInput1")) return 1;
    if (!_stricmp(r, "waitInput2")) return 2;
    if (!_stricmp(r, "continue")) return 3;
    return 0;
}

u32 map_color(const char *c) {
    if (!_stricmp(c, "blue")) return 31349;
    return (u32)atoi(c);
}

// --- Parsing helpers ---

int parse_key_value(char *line, char **key, char **value) {
    char *eq;
    eq = strchr(line, '=');
    if (!eq) return 0;
    *eq = 0;
    *key = line;
    *value = eq + 1;
    trim(*key);
    trim(*value);
    return 1;
}

// --- Read text script ---

int read_script(const char *filename, BTKFile *btk) {
    FILE *f;
    char line[MAX_LINES];
    int in_config = 0;
    int in_action = 0;
    int action_index = 0;
    char *key, *value;
    Action *act;

    f = fopen(filename, "rt");
    if (!f) {
        printf("Failed to open %s\n", filename);
        return 0;
    }

    btk->header.magic = 1263813155; // '#BTK'
    btk->header.unk = 0;
    btk->header.unk2 = 0;
    btk->header.language = 0;
    btk->header.color = 0;
	btk->header.unk3 = 0;
    btk->header.actionCount = 0;

    while (fgets(line, sizeof(line), f)) {
        trim(line);
        strip_inline_comment(line);
        trim(line);
        if (line[0] == 0 || line[0] == ';') continue;

        if (!_stricmp(line, "[Config]")) {
            in_config = 1;
            in_action = 0;
            continue;
        }

        if (!_stricmp(line, "[Action]")) {
            if (action_index >= MAX_ACTIONS) {
                printf("Max actions reached\n");
                break;
            }
            in_action = 1;
            in_config = 0;
            memset(&btk->actions[action_index], 0, sizeof(Action));
            action_index++;
            continue;
        }

        if (in_config) {
            if (parse_key_value(line, &key, &value)) {
                if (!_stricmp(key, "language")) {
                    btk->header.language = map_language(value);
                } else if (!_stricmp(key, "color")) {
                    btk->header.color = map_color(value);
                } else {
                    printf("Unknown config key: %s\n", key);
                }
            }
        } else if (in_action) {
            if (!parse_key_value(line, &key, &value)) {
                printf("Invalid line: %s\n", line);
                continue;
            }
            act = &btk->actions[action_index - 1];

            if (!_stricmp(key, "effect")) act->effect = map_effect(value);
            else if (!_stricmp(key, "character")) act->character = map_character(value);
            else if (!_stricmp(key, "expression")) act->expression = map_expression(value);
            else if (!_stricmp(key, "initFlag")) act->initFlag = map_flag(value);
            else if (!_stricmp(key, "endFlag")) act->endFlag = map_flag(value);
            else if (!_stricmp(key, "bubble")) act->bubble = map_bubble(value);
            else if (!_stricmp(key, "bubbleFlag")) act->bubbleFlag = map_bubbleFlag(value);
            else if (!_stricmp(key, "response")) act->response = map_response(value);
            else if (!_stricmp(key, "responseDelay")) act->responseDelay = (u32)atoi(value);
            else if (!_stricmp(key, "endDelay")) act->endDelay = (u32)atoi(value);
            else printf("Unknown action key: %s\n", key);
        }
    }

    btk->header.actionCount = action_index;
    fclose(f);
    return 1;
}

// --- Write binary BTK ---

int write_btk_binary(const char *filename, BTKFile *btk) {
    FILE *f;
    int i;

    f = fopen(filename, "wb");
    if (!f) {
        printf("Cannot open %s for writing\n", filename);
        return 0;
    }

    fwrite(&btk->header, sizeof(Header), 1, f);
    for (i = 0; i < (int)btk->header.actionCount; i++) {
        fwrite(&btk->actions[i], sizeof(Action), 1, f);
    }

    fclose(f);
    return 1;
}

// --- Debug print ---

void print_debug(BTKFile *btk) {
    unsigned int i;
    Action *a;

    printf("Language: ");
    switch (btk->header.language) {
    case 1: printf("English\n"); break;
    case 2: printf("French\n"); break;
    case 4: printf("Italian\n"); break;
    default: printf("Unknown (%u)\n", btk->header.language); break;
    }

    printf("Color: ");
    switch (btk->header.color) {
    case 31349: printf("Blue\n"); break;
    default: printf("Unknown (%u)\n", btk->header.color); break;
    }

    printf("Actions (%u):\n", btk->header.actionCount);

    for (i = 0; i < btk->header.actionCount; i++) {
        a = &btk->actions[i];
        printf("Action %u:\n", i + 1);
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

// --- Main ---

int main(int argc, char *argv[]) {
    char *inputfile;
    char *outputfile;
    BTKFile btk;
    int ret;

#ifdef DEBUG
    inputfile = "demo.txt";
    outputfile = "demo.btk";
#else
    if (argc < 3) {
        printf("Usage: %s [INPUT] [OUTPUT]\n", argv[0]);
        return 1;
    }
    inputfile = argv[1];
    outputfile = argv[2];
#endif

    memset(&btk, 0, sizeof(BTKFile));

    ret = read_script(inputfile, &btk);
    if (!ret) {
        printf("Failed to read script.\n");
        return 1;
    }

    print_debug(&btk);

    ret = write_btk_binary(outputfile, &btk);
    if (!ret) {
        printf("Failed to write BTK file.\n");
        return 1;
    }

    printf("Successfully wrote %s\n", outputfile);
#ifdef DEBUG
    system("pause");
#endif
    return 0;
}
