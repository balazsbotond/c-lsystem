#ifndef LSYS_H
#define LSYS_H

#include "color.h"
#include "coordinate_system.h"
#include "rectangle.h"
#include "turtle.h"
#include "turtle_stack.h"
#include "vector.h"
#include "viewport.h"
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

typedef void (*TurtleAction)(Turtle*, TurtleStack* stack, Vector, void*);
typedef void (*RenderProgressAction)(size_t current, size_t total);

bool lsys_equals(LSystem* lsys1, LSystem* lsys2);

char* lsys_iterate(LSystem* lsys);

Rectangle* lsys_measure(
    LSystem* lsys,
    char* instructions,
    RenderProgressAction progress_action
);

void lsys_draw(
    Viewport* viewport,
    CoordinateSystem* cs,
    LSystem* lsys,
    char* instructions,
    Color color,
    double max_line_width,
    char* file_name,
    RenderProgressAction progress_action
);

#endif // LSYS_H