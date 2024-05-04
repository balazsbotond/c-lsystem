#include "lsys.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    INIT,
    COMMENT,
    LEADING_WS,
    NAME,
    PROP_VALUE,
    RULE_VALUE
} State;

void string_to_upper(char* str) {
    for(int i = 0; str[i]; i++){
        str[i] = toupper(str[i]);
    }
}

void set_prop(LSystem* lsys, char* name, char* value, int line_number) {
    if (strcmp(name, "angle") == 0) {
        lsys->angle = atof(value);
    } else if (strcmp(name, "axiom") == 0) {
        lsys->axiom = strdup(value);
        string_to_upper(lsys->axiom);
    } else if (strcmp(name, "rotation") == 0) {
        lsys->rotation = atoi(value);
    } else if (strcmp(name, "iterations") == 0) {
        lsys->iterations = atoi(value);
    } else {
        printf("Error: Unknown property '%s' at line %d\n", name, line_number);
        exit(1);
    }
}

void add_rule(LSystem* lsys, char symbol, char* substitution) {
    lsys->rules_count++;
    lsys->rules = realloc(lsys->rules, lsys->rules_count * sizeof(Rule));
    lsys->rules[lsys->rules_count - 1].symbol = toupper(symbol);
    lsys->rules[lsys->rules_count - 1].substitution = strdup(substitution);
    string_to_upper(lsys->rules[lsys->rules_count - 1].substitution);
}

LSystem* lsys_load(char* file_name) {
    LSystem* lsys = malloc(sizeof(LSystem));
    lsys->rotation = 0;
    lsys->rules = NULL;
    lsys->rules_count = 0;
    lsys->iterations = 0;

    State state = INIT;
    FILE* file = fopen(file_name, "r");
    char name[256];
    int name_index = 0;
    char value[256];
    int value_index = 0;
    int line_number = 1;

    if (file == NULL) {
        printf("Error: Could not open file %s\n", file_name);
        exit(1);
    }

    while (true) {
        char c = fgetc(file);

        if (c == '\n') line_number++;

        switch (state) {
            case INIT:
                if (c == ':' || c == '=') {
                    printf("Error: Unexpected character '%c' at the beginning of line %d\n", c, line_number);
                    exit(1);
                } else if (c == '#') {
                    state = COMMENT;
                } else if (c == ' ' || c == '\t') {
                    state = LEADING_WS;
                } else if (c == '\n') {
                    state = INIT;
                } else if (c == EOF) {
                    goto end;
                } else {
                    name[name_index++] = c;
                    state = NAME;
                }
                break;
            case COMMENT:
                if (c == '\n') {
                    state = INIT;
                } else if (c == EOF) {
                    goto end;
                }
                break;
            case LEADING_WS:
                if (c == ':' || c == '=') {
                    printf("Error: Unexpected character '%c' at the beginning of line %d\n", c, line_number);
                    exit(1);
                } else if (c == '#') {
                    state = COMMENT;
                } else if (c == ' ' || c == '\t') {
                    state = LEADING_WS;
                } else if (c == '\n') {
                    state = INIT;
                } else if (c == EOF) {
                    goto end;
                } else {
                    name[name_index++] = c;
                    state = NAME;
                }
                break;
            case NAME:
                if (c == ' ' || c == '\t') {
                    state = NAME;
                } else if (c == ':') {
                    name[name_index] = '\0';
                    name_index = 0;
                    state = PROP_VALUE;
                } else if (c == '=') {
                    if (name_index > 1) {
                        printf("Error: Left hand side of rule must be a single character at line %d\n", line_number);
                        exit(1);
                    }
                    name[name_index] = '\0';
                    name_index = 0;
                    state = RULE_VALUE;
                } else if (c == '\n') {
                    printf("Error: Unexpected newline at line %d\n", line_number);
                    exit(1);
                } else if (c == EOF) {
                    printf("Error: Unexpected end of file\n");
                    exit(1);
                } else {
                    name[name_index++] = c;
                }
                break;
            case PROP_VALUE:
                if (c == ' ' || c == '\t') {
                    state = PROP_VALUE;
                } else if (c == '\n' || c == EOF) {
                    value[value_index] = '\0';
                    set_prop(lsys, name, value, line_number);
                    value_index = 0;
                    state = INIT;
                    if (c == EOF) {
                        goto end;
                    }
                } else {
                    value[value_index++] = c;
                    state = PROP_VALUE;
                }
                break;
            case RULE_VALUE:
                if (c == ' ' || c == '\t') {
                    state = RULE_VALUE;
                } else if (c == '\n' || c == EOF) {
                    value[value_index] = '\0';
                    add_rule(lsys, name[0], value);
                    value_index = 0;
                    if (c == EOF) {
                        goto end;
                    }
                    state = INIT;
                } else {
                    value[value_index++] = c;
                    state = RULE_VALUE;
                }
                break;
        }

        if (c == EOF) {
            goto end;
        }
    }

end:
    fclose(file);
    return lsys;
}
