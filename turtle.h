#ifndef TURTLE_H
#define TURTLE_H

#include "vector.h"
#include <stdbool.h>

typedef struct {
    Vector pos;
    double angle;
    double stroke_length;
    bool reverse;
} Turtle;

Turtle turtle_create(double rotation);
void turtle_go_forward(Turtle* turtle);
void turtle_turn_around(Turtle* turtle);
void turtle_turn_left(Turtle* turtle, double angle);
void turtle_turn_right(Turtle* turtle, double angle);
void turtle_reverse_left_and_right(Turtle* turtle);
void turtle_change_stroke_length(Turtle* turtle, double factor);

#endif // TURTLE_H
