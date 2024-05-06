#include "turtle.h"
#include "utils.h"
#include <math.h>

Turtle turtle_create(double rotation) {
    Turtle turtle;
    turtle.pos.x = 0;
    turtle.pos.y = 0;
    turtle.angle = to_rad(rotation);
    turtle.stroke_length = 10;
    turtle.reverse = false;
    return turtle;
}

void turtle_go_forward(Turtle* turtle) {
    Vector next;

    next.x = turtle->pos.x + turtle->stroke_length * cos(turtle->angle);
    next.y = turtle->pos.y + turtle->stroke_length * sin(turtle->angle);
    
    turtle->pos = next;
}

void turtle_turn_around(Turtle* turtle) {
    turtle->angle = fmod(turtle->angle + M_PI, 2 * M_PI);
}

void turtle_turn_left(Turtle *turtle, double angle) {
    double angle_rad = (turtle->reverse ? -1 : 1) * to_rad(angle);
    
    turtle->angle = fmod(turtle->angle + angle_rad, 2 * M_PI);
}

void turtle_turn_right(Turtle *turtle, double angle) {
    turtle_turn_left(turtle, -angle);
}

void turtle_reverse_left_and_right(Turtle *turtle) {
    turtle->reverse = !turtle->reverse;
}

void turtle_change_stroke_length(Turtle *turtle, double factor) {
    turtle->stroke_length *= factor;
}
