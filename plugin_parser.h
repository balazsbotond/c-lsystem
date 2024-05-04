#ifndef PLUGIN_PARSER_H
#define PLUGIN_PARSER_H

#include "lsys.h"
#include <cairo.h>

typedef enum {
    SOLID,
    LINEAR,
    RADIAL
} PatternType;

PatternType parse_pattern_type(const char* spec);
cairo_pattern_t* parse_pattern(const char* spec, char** pattern_str);

StackDepthLineWidthOptions parse_stack_depth_line_width_options(const char* spec);

#endif // PLUGIN_PARSER_H
