#include "color.h"
#include <stdio.h>
#include <stdlib.h>

Color color_parse(const char* str) {
    Color color;
    if (sscanf(str, "%hhu,%hhu,%hhu", &color.red, &color.green, &color.blue) == 3) {
        return color;
    } else {
        printf("Invalid color string: %s\n", str);
        exit(EXIT_FAILURE);
    }
}

char* color_str(Color color) {
    char* str = malloc(12);
    sprintf(str, "%hhu,%hhu,%hhu", color.red, color.green, color.blue);
    return str;
}
