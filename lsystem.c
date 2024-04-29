#include <cairo.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

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

typedef struct {
    double x;
    double y;
} Vector;

typedef struct {
    double width;
    double height;
} Viewport;

typedef struct {
    double left;
    double top;
    double right;
    double bottom;
} Rectangle;

typedef struct {
    double scale_factor;
    double origin_x;
    double origin_y;
} CoordinateSystem;

typedef struct {
    Vector pos;
    double angle;
    double stroke_length;
    bool reverse;
} Turtle;

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Color;

const int STACK_SIZE = 1000;

typedef struct {
    Turtle* stack;
    int top;
} TurtleStack;

TurtleStack* stack_create() {
    TurtleStack* stack = malloc(sizeof(TurtleStack));
    stack->stack = malloc(sizeof(Turtle) * STACK_SIZE);
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

double to_rad(double degrees) {
    return degrees * M_PI / 180;
}

Turtle turtle_create(double rotation) {
    Turtle turtle;
    turtle.pos.x = 0;
    turtle.pos.y = 0;
    turtle.angle = to_rad(rotation);
    turtle.stroke_length = 10;
    turtle.reverse = false;
    return turtle;
}

Vector op_go_forward(Vector current, double distance, double angle) {
    Vector next;
    next.x = current.x + distance * cos(angle);
    next.y = current.y + distance * sin(angle);
    return next;
}

double op_turn_around(double angle) {
    return angle + M_PI;
}

char* lookup_rule(LSystem* lsys, char symbol) {
    for (int i = 0; i < lsys->rules_count; i++) {
        if (lsys->rules[i].symbol == symbol) {
            return lsys->rules[i].substitution;
        }
    }

    return NULL;
}

char* lsys_iterate(LSystem* lsys) {
    char* current = malloc(sizeof(char) * strlen(lsys->axiom) + 1);
    strcpy(current, lsys->axiom);
    char* next;

    for (int i = 0; i < lsys->iterations; i++) {
        clock_t start = clock();
        int next_length = 0;
        char* cp = current;

        while (*cp != '\0') {
            char* substitution = lookup_rule(lsys, *cp);
            next_length += substitution != NULL ? strlen(substitution) : 1;
            cp++;
        }

        next = malloc(sizeof(char) * next_length + 1);

        cp = current;
        char* np = next;

        while (*cp != '\0') {
            char* substitution = lookup_rule(lsys, *cp);

            if (substitution != NULL) {
                strcpy(np, substitution);
                np += strlen(substitution);
            } else {
                *np = *cp;
                np++;
            }
            cp++;
        }
        *np = '\0';

        free(current);
        current = next;

        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Iteration %d: %f seconds\n", i + 1, elapsed);
    }

    return current;
}

Rectangle* lsys_measure(LSystem* lsys, char* instructions) {
    clock_t start = clock();
    Rectangle* bounding_rect = malloc(sizeof(Rectangle));
    bounding_rect->left = 0;
    bounding_rect->top = 0;
    bounding_rect->right = 0;
    bounding_rect->bottom = 0;

    Turtle turtle = turtle_create(lsys->rotation);
    TurtleStack* stack = stack_create();
    char* cp = instructions;

    while (*cp != '\0') {
        char c = *cp;

        if (c == 'F' || c == 'D' || c == 'D' || c == 'M') {
            turtle.pos = op_go_forward(turtle.pos, turtle.stroke_length, turtle.angle);

            if (turtle.pos.x < bounding_rect->left) {
                bounding_rect->left = turtle.pos.x;
            }
            if (turtle.pos.x > bounding_rect->right) {
                bounding_rect->right = turtle.pos.x;
            }
            if (turtle.pos.y < bounding_rect->bottom) {
                bounding_rect->bottom = turtle.pos.y;
            }
            if (turtle.pos.y > bounding_rect->top) {
                bounding_rect->top = turtle.pos.y;
            }
        } else if (c == '|') {
            turtle.angle = op_turn_around(turtle.angle);
        } else if (c == '+') {
            turtle.angle += (turtle.reverse ? -1 : 1) * to_rad(lsys->angle);
        } else if (c == '-') {
            turtle.angle -= (turtle.reverse ? -1 : 1) * to_rad(lsys->angle);
        } else if (c == '[') {
            stack_push(stack, turtle);
        } else if (c == ']') {
            turtle = stack_pop(stack);
        } else if (c == '!') {
            turtle.reverse = !turtle.reverse;
        } else if (c == '@') {
            cp++; // move to the char after '@'

            bool inverse = false;
            bool square_root = false;

            while (*(cp + 1) == 'I' || *(cp + 1) == 'Q') {
                c = *cp++;
                if (c == 'I') inverse = true;
                if (c == 'Q') square_root = true;
            }

            double factor = strtod(cp, &cp);
            if (inverse) factor = 1 / factor;
            if (square_root) factor = sqrt(factor);

            turtle.stroke_length *= factor;
            cp--; // needs to point to the last character of the number
        }

        cp++;
    }

    stack_destroy(stack);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Measurement: %f seconds\n", elapsed);

    return bounding_rect;
}

CoordinateSystem* cs_fit(Rectangle* bounding_rect, Viewport* viewport, int padding) {
    CoordinateSystem* cs = malloc(sizeof(CoordinateSystem));

    double viewport_width  = viewport->width  - padding * 2;
    double viewport_height = viewport->height - padding * 2;

    double bounding_rect_width  = abs(bounding_rect->right  - bounding_rect->left);
    double bounding_rect_height = abs(bounding_rect->bottom - bounding_rect->top);
    double source_aspect_ratio  = bounding_rect_width / bounding_rect_height;
    double dest_aspect_ratio    = viewport_width      / viewport_height;

    double scale_factor =
        source_aspect_ratio > dest_aspect_ratio
            ? viewport_width  / bounding_rect_width
            : viewport_height / bounding_rect_height;

    double offset_x =
        source_aspect_ratio > dest_aspect_ratio
            ? 0
            : (viewport_width - scale_factor * bounding_rect_width) / 2;

    double offset_y =
        source_aspect_ratio > dest_aspect_ratio
            ? (viewport_height - scale_factor * bounding_rect_height) / 2
            : 0;

    cs->scale_factor = scale_factor;
    cs->origin_x = -(scale_factor * bounding_rect->left) + offset_x + padding;
    cs->origin_y = scale_factor * bounding_rect->top + offset_y + padding;

    return cs;
}

double cs_convert_x(CoordinateSystem* cs, double x) {
    return cs->origin_x + cs->scale_factor * x;
}

double cs_convert_y(CoordinateSystem* cs, double y) {
    return cs->origin_y - cs->scale_factor * y;
}

Vector cs_convert(CoordinateSystem* cs, Vector v) {
    Vector result;
    result.x = cs_convert_x(cs, v.x);
    result.y = cs_convert_y(cs, v.y);
    return result;
}

void lsys_draw(Viewport* viewport, CoordinateSystem* cs, LSystem* lsys, char* instructions, Color color, double line_width, char* file_name) {
    clock_t start = clock();
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, viewport->width, viewport->height);
    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0, 0, 0); // black
    cairo_paint(cr);
    // cairo_set_source_rgb(cr, 1, 1, 1); // white
    // cairo_set_source_rgb(cr, 204 / 255.0, 181 / 255.0, 74 / 255.0); // yellow
    // cairo_set_source_rgb(cr, 110 / 255.0, 240 / 255.0, 74 / 255.0); // green
    // cairo_set_source_rgb(cr, 84 / 255.0, 112 / 255.0, 240 / 255.0); // blue
    cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
    cairo_set_line_width(cr, line_width);

    Turtle turtle = turtle_create(lsys->rotation);
    TurtleStack* stack = stack_create();
    char* cp = instructions;

    while (*cp != '\0') {
        char c = *cp;

        if (c == 'F' || c == 'D') { // go forward
            Vector next = op_go_forward(turtle.pos, turtle.stroke_length, turtle.angle);

            cairo_move_to(cr, cs_convert_x(cs, turtle.pos.x), cs_convert_y(cs, turtle.pos.y));
            cairo_line_to(cr, cs_convert_x(cs, next.x), cs_convert_y(cs, next.y));
            cairo_stroke(cr);

            turtle.pos = next;
        } else if (c == 'D' || c == 'M') { // go forward without drawing
            turtle.pos = op_go_forward(turtle.pos, turtle.stroke_length, turtle.angle);
        } else if (c == '|') {
            turtle.angle = op_turn_around(turtle.angle);
        } else if (c == '+') {
            turtle.angle += (turtle.reverse ? -1 : 1) * to_rad(lsys->angle);
        } else if (c == '-') {
            turtle.angle -= (turtle.reverse ? -1 : 1) * to_rad(lsys->angle);
        } else if (c == '[') {
            stack_push(stack, turtle);
        } else if (c == ']') {
            turtle = stack_pop(stack);
        } else if (c == '!') {
            turtle.reverse = !turtle.reverse;
        } else if (c == '@') {
            cp++; // move to the char after '@'

            bool inverse = false;
            bool square_root = false;

            while (*(cp + 1) == 'I' || *(cp + 1) == 'Q') {
                c = *cp++;
                if (c == 'I') inverse = true;
                if (c == 'Q') square_root = true;
            }

            double factor = strtod(cp, &cp);
            if (inverse) factor = 1 / factor;
            if (square_root) factor = sqrt(factor);

            turtle.stroke_length *= factor;
            cp--; // needs to point to the last character of the number
        }

        cp++;
    }

    cairo_surface_write_to_png(surface, file_name);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    stack_destroy(stack);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Drawing: %f seconds\n", elapsed);
}

int main() {
    // // Koch Curve
    // LSystem lsys = {60, 0, "F", NULL, 0, 7};
    // lsys.rules = malloc(sizeof(Rule) * 1);
    // lsys.rules[0].symbol = 'F';
    // lsys.rules[0].substitution = "F+F--F+F";
    // lsys.rules_count = 1;

    // // Hiway
    // LSystem lsys = {90, 0, "FX", NULL, 0, 13};

    // lsys.rules_count = 3;

    // lsys.rules = malloc(sizeof(Rule) * lsys.rules_count);
    // lsys.rules[0].symbol = 'X';
    // lsys.rules[0].substitution = "FX+FY";
    // lsys.rules[1].symbol = 'Y';
    // lsys.rules[1].substitution = "FX-FY";
    // lsys.rules[2].symbol = 'F';
    // lsys.rules[2].substitution = "";

    // // Y Tree
    // LSystem lsys = {45, 90, "FX", NULL, 0, 12};
    // lsys.rules = malloc(sizeof(Rule) * 1);
    // lsys.rules[0].symbol = 'X';
    // lsys.rules[0].substitution = "@0.6[-FX]+FX";
    // lsys.rules_count = 1;

    // Infinite Leaf
    LSystem lsys = {30, 0, ">X", NULL, 0, 20};
    lsys.rules = malloc(sizeof(Rule) * 1);
    lsys.rules[0].symbol = 'X';
    lsys.rules[0].substitution = "[>+@.9FX@.5[>!X]]";
    lsys.rules_count = 1;

    clock_t start = clock();

    Viewport viewport = {4960, 7016};
    char* instructions = lsys_iterate(&lsys);
    Rectangle* bounding_rect = lsys_measure(&lsys, instructions);
    CoordinateSystem* coord_system = cs_fit(bounding_rect, &viewport, 400);

    Color colors[] = {
        {255, 255, 255},
        {204, 181, 74},
        {110, 240, 74},
        {84, 112, 240}
    };
    double line_widths[] = {1, 2, 3, 4};

    char file_name[100];
    sprintf(file_name, "infinite_leaf_width(%g)_rgb(%d,%d,%d).png", line_widths[0], colors[0].red, colors[0].green, colors[0].blue);

    lsys_draw(&viewport, coord_system, &lsys, instructions, colors[0], line_widths[0], file_name);

    free(instructions);
    free(bounding_rect);
    free(coord_system);
    free(lsys.rules);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total: %f seconds\n", elapsed);

    return 0;
}
