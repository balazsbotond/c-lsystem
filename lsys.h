#ifndef LSYS_H
#define LSYS_H

#include "color.h"
#include "coordinate_system.h"
#include "rectangle.h"
#include "turtle.h"
#include "turtle_stack.h"
#include "vector.h"
#include "viewport.h"
#include <cairo.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char symbol;
    char* substitution;
} Rule;

typedef struct {
    double angle;
    double rotation;
    char* axiom;
    Rule* rules;
    int rules_count;
    int iterations;
} LSystem;

typedef void (*TurtleAction)(Turtle* turtle, TurtleStack* stack, Vector prev, void* data);
typedef void (*RenderProgressAction)(size_t current, size_t total);
typedef void (*IterateProgressAction)(int i, void* data);

typedef void (*PluginInitAction)(cairo_t* cr, void* data);
typedef void (*PluginAction)(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data);
typedef void (*PluginFinishAction)(cairo_t* cr, void* data);

typedef struct {
    PluginInitAction on_init;
    PluginAction on_draw;
    PluginFinishAction on_finish;
    void* data;
} Plugin;

Plugin lsys_plugin_pattern_create(cairo_pattern_t* pattern);
void lsys_plugin_pattern_destroy(Plugin plugin);

Plugin lsys_plugin_bg_pattern_create(cairo_pattern_t* pattern);
void lsys_plugin_bg_pattern_destroy(Plugin plugin);

typedef struct {
    double max_width;
    double min_width;
    int step;
} StackDepthLineWidthOptions;

Plugin lsys_plugin_stack_depth_line_width_create(StackDepthLineWidthOptions options);
void lsys_plugin_stack_depth_line_width_destroy(Plugin plugin);

typedef enum {
    PALETTE_OVERFLOW_WRAP,
    PALETTE_OVERFLOW_CLAMP,
} PaletteOverflowBehavior;

typedef struct {
    Color* colors;
    PaletteOverflowBehavior overflow;
    int colors_count;
    int index;
} Palette;

Plugin lsys_plugin_stack_depth_palette_create(Palette palette);
void lsys_plugin_stack_depth_palette_destroy(Plugin plugin);

Plugin lsys_plugin_linear_palette_create(Palette palette);
void lsys_plugin_linear_palette_destroy(Plugin plugin);

void lsys_print(LSystem lsys);

void lsys_destroy(LSystem lsys);

bool lsys_equals(LSystem lsys1, LSystem lsys2);

char* lsys_iterate(
    LSystem lsys,
    IterateProgressAction progress_action,
    void* action_data
);

Rectangle lsys_measure(
    LSystem lsys,
    char* instructions,
    RenderProgressAction progress_action
);

void lsys_draw(
    Viewport viewport,
    CoordinateSystem cs,
    LSystem lsys,
    char* instructions,
    Plugin* plugins,
    int plugins_count,
    char* file_name,
    RenderProgressAction progress_action
);

#endif // LSYS_H