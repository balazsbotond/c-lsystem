#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cache_save(
    LSystem* lsys,
    Viewport* viewport,
    CoordinateSystem* cs,
    char* instructions,
    const char* file_path
) {
    FILE* file = fopen(file_path, "wb");
    
    if (file == NULL) {
        printf("Unable to open file for writing: cache.bin\n");
        exit(1);
    }

    fwrite(lsys, sizeof(LSystem), 1, file);

    size_t axiom_length = strlen(lsys->axiom);
    fwrite(&axiom_length, sizeof(size_t), 1, file);
    fwrite(lsys->axiom, sizeof(char), axiom_length, file);

    for (int i = 0; i < lsys->rules_count; i++) {
        fwrite(&lsys->rules[i].symbol, sizeof(char), 1, file);
        size_t substitution_length = strlen(lsys->rules[i].substitution);
        fwrite(&substitution_length, sizeof(size_t), 1, file);
        fwrite(lsys->rules[i].substitution, sizeof(char), substitution_length, file);
    }

    fwrite(viewport, sizeof(Viewport), 1, file);
    fwrite(cs, sizeof(CoordinateSystem), 1, file);
    
    size_t instructions_length = strlen(instructions);
    fwrite(&instructions_length, sizeof(size_t), 1, file);
    fwrite(instructions, sizeof(char), instructions_length, file);

    fclose(file);
}

bool cache_load(
    LSystem** lsys,
    Viewport** viewport,
    CoordinateSystem** cs,
    char** instructions,
    const char* file_path
) {
    FILE* file = fopen(file_path, "rb");

    if (file == NULL) {
        return false;
    }

    *lsys = malloc(sizeof(LSystem));
    fread(*lsys, sizeof(LSystem), 1, file);

    size_t axiom_length;
    fread(&axiom_length, sizeof(size_t), 1, file);
    (*lsys)->axiom = malloc(sizeof(char) * (axiom_length + 1));
    fread((*lsys)->axiom, sizeof(char), axiom_length, file);
    (*lsys)->axiom[axiom_length] = '\0';

    (*lsys)->rules = malloc(sizeof(Rule) * (*lsys)->rules_count);
    for (int i = 0; i < (*lsys)->rules_count; i++) {
        fread(&(*lsys)->rules[i].symbol, sizeof(char), 1, file);
        size_t substitution_length;
        fread(&substitution_length, sizeof(size_t), 1, file);
        (*lsys)->rules[i].substitution = malloc(sizeof(char) * (substitution_length + 1));
        fread((*lsys)->rules[i].substitution, sizeof(char), substitution_length, file);
        (*lsys)->rules[i].substitution[substitution_length] = '\0';
    }

    *viewport = malloc(sizeof(Viewport));
    fread(*viewport, sizeof(Viewport), 1, file);

    *cs = malloc(sizeof(CoordinateSystem));
    fread(*cs, sizeof(CoordinateSystem), 1, file);

    size_t instructions_length;
    fread(&instructions_length, sizeof(size_t), 1, file);
    *instructions = malloc(sizeof(char) * (instructions_length + 1));
    fread(*instructions, sizeof(char), instructions_length, file);
    (*instructions)[instructions_length] = '\0';

    fclose(file);

    return true;
}

bool cache_stale(
    LSystem* lsys,
    LSystem* cached_lsys,
    Viewport* viewport,
    Viewport* cached_viewport
) {
    return !lsys_equals(lsys, cached_lsys) ||
        !viewport_equals(viewport, cached_viewport);
}
