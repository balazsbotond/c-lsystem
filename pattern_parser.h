#ifndef PATTERN_PARSER_H
#define PATTERN_PARSER_H

#include <cairo.h>

typedef enum {
    SOLID,
    LINEAR,
    RADIAL
} PatternType;

PatternType parse_pattern_type(const char* spec);
cairo_pattern_t* parse_pattern(const char* spec, char** pattern_str);

#endif // PATTERN_PARSER_H
