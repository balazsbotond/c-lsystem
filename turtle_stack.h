#ifndef TURTLESTACK_H
#define TURTLESTACK_H

#include "turtle.h"

extern const int TURTLE_STACK_SIZE;

typedef struct {
    Turtle* stack;
    int top;
} TurtleStack;

TurtleStack* stack_create();
void stack_destroy(TurtleStack* stack);
void stack_push(TurtleStack* stack, Turtle turtle);
Turtle stack_pop(TurtleStack* stack);

#endif // TURTLESTACK_H
