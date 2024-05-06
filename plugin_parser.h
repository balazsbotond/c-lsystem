#ifndef PLUGIN_PARSER_H
#define PLUGIN_PARSER_H

#include "lsys.h"
#include <cairo.h>

typedef enum {
    COLOR_PLUGIN_SOLID,
    COLOR_PLUGIN_LINEAR_GRADIENT,
    COLOR_PLUGIN_RADIAL_GRADIENT,
    COLOR_PLUGIN_LINEAR_PALETTE,
    COLOR_PLUGIN_STACK_DEPTH_PALETTE
} ColorPluginType;

ColorPluginType parse_color_plugin_type(const char* spec);
Plugin parse_color_plugin(const char* spec, char** plugin_str);
Plugin parse_bgcolor_plugin(const char* spec, char** plugin_str);

StackDepthLineWidthOptions parse_stack_depth_line_width_options(const char* spec);

#endif // PLUGIN_PARSER_H
