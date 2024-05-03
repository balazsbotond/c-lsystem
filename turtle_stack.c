#include "turtle_stack.h"
#include <stdlib.h>

const int TURTLE_STACK_SIZE = 10000;

TurtleStack* stack_create() {
    TurtleStack* stack = malloc(sizeof(TurtleStack));
    stack->stack = malloc(sizeof(Turtle) * TURTLE_STACK_SIZE);
    stack->top = 0;
    return stack;
}

void stack_destroy(TurtleStack* stack) {
    free(stack->stack);
    free(stack);
}

void stack_push(TurtleStack* stack, Turtle turtle) {
    stack->stack[stack->top] = turtle;
    stack->top++;
}

Turtle stack_pop(TurtleStack* stack) {
    stack->top--;
    return stack->stack[stack->top];
}
