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

typedef void (*ColorInitAction)(cairo_t* cr, void* data);
typedef void (*ColorAction)(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data);
typedef void (*ColorFinishAction)(cairo_t* cr, void* data);

typedef struct {
    ColorInitAction on_init;
    ColorAction on_draw;
    ColorFinishAction on_finish;
    void* data;
} Coloring;

Coloring coloring_pattern_create(cairo_pattern_t* pattern);
void coloring_pattern_destroy(Coloring coloring);

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
    Coloring coloring,
    double max_line_width,
    double min_line_width,
    char* file_name,
    RenderProgressAction progress_action
);

#endif // LSYS_H