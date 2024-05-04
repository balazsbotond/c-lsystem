#include "lsys.h"
#include "turtle_stack.h"
#include "turtle.h"
#include "utils.h"
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* lookup_rule(LSystem* lsys, char symbol) {
    for (int i = 0; i < lsys->rules_count; i++) {
        if (lsys->rules[i].symbol == symbol) {
            return lsys->rules[i].substitution;
        }
    }

    return NULL;
}

void lsys_print(LSystem* lsys) {
    printf(" - angle: %f\n", lsys->angle);
    printf(" - rotation: %f\n", lsys->rotation);
    printf(" - axiom: %s\n", lsys->axiom);
    printf(" - iterations: %d\n", lsys->iterations);

    printf(" - rules:\n");
    for (int i = 0; i < lsys->rules_count; i++) {
        printf("   - %c -> %s\n", lsys->rules[i].symbol, lsys->rules[i].substitution);
    }
}

bool lsys_equals(LSystem* lsys1, LSystem* lsys2) {
    if (lsys1->angle != lsys2->angle) return false;
    if (lsys1->rotation != lsys2->rotation) return false;
    if (strcmp(lsys1->axiom, lsys2->axiom) != 0) return false;
    if (lsys1->rules_count != lsys2->rules_count) return false;
    if (lsys1->iterations != lsys2->iterations) return false;

    for (int i = 0; i < lsys1->rules_count; i++) {
        if (lsys1->rules[i].symbol != lsys2->rules[i].symbol) return false;
        if (strcmp(lsys1->rules[i].substitution, lsys2->rules[i].substitution) != 0) return false;
    }

    return true;
}

char* lsys_iterate(
    LSystem* lsys,
    IterateProgressAction progress_action,
    void* action_data
) {
    char* current = malloc(sizeof(char) * (strlen(lsys->axiom) + 1));
    strcpy(current, lsys->axiom);
    char* next;

    for (int i = 0; i < lsys->iterations; i++) {

        clock_t start = clock();
        size_t next_length = 0;
        char* cp = current;

        while (*cp != '\0') {
            char* substitution = lookup_rule(lsys, *cp);
            next_length += substitution != NULL ? strlen(substitution) : 1;
            cp++;
        }

        next = malloc(sizeof(char) * (next_length + 1));

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
        progress_action(i, action_data);
    }

    return current;
}

void lsys_render(
    LSystem* lsys,
    char* instructions,
    void* action_data,
    TurtleAction action,
    RenderProgressAction progress_action
) {
    Turtle turtle = turtle_create(lsys->rotation);
    TurtleStack* stack = stack_create();
    char* cp = instructions;
    size_t i = 0;
    size_t instructions_length = strlen(instructions);

    progress_action(0, instructions_length);

    while (*cp != '\0') {
        char c = *cp;

        if (c == 'F' || c == 'D' || c == 'D' || c == 'M') {
            Vector prev = turtle.pos;
            turtle_go_forward(&turtle);

            action(&turtle, stack, prev, action_data);
        } else if (c == '|') {
            turtle_turn_around(&turtle);
        } else if (c == '+') {
            turtle_turn_left(&turtle, lsys->angle);
        } else if (c == '-') {
            turtle_turn_right(&turtle, lsys->angle);
        } else if (c == '[') {
            stack_push(stack, turtle);
        } else if (c == ']') {
            turtle = stack_pop(stack);
        } else if (c == '!') {
            turtle_reverse_left_and_right(&turtle);
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

            turtle_change_stroke_length(&turtle, factor);
            cp--; // needs to point to the last character of the number
        }

        cp++;
        i++;

        if (i % 100000 == 0) {
            if (progress_action != NULL) {
                progress_action(i, instructions_length);
            }
        }
    }

    progress_action(instructions_length, instructions_length);

    stack_destroy(stack);
}

void measure_action(Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    Rectangle* bounding_rect = (Rectangle*)data;

    if (turtle->pos.x < bounding_rect->left) {
        bounding_rect->left = turtle->pos.x;
    }
    if (turtle->pos.x > bounding_rect->right) {
        bounding_rect->right = turtle->pos.x;
    }
    if (turtle->pos.y < bounding_rect->bottom) {
        bounding_rect->bottom = turtle->pos.y;
    }
    if (turtle->pos.y > bounding_rect->top) {
        bounding_rect->top = turtle->pos.y;
    }
}

Rectangle* lsys_measure(
    LSystem* lsys,
    char* instructions,
    RenderProgressAction progress_action
) {
    Rectangle* bounding_rect = malloc(sizeof(Rectangle));
    bounding_rect->left = 0;
    bounding_rect->top = 0;
    bounding_rect->right = 0;
    bounding_rect->bottom = 0;

    lsys_render(
        lsys,
        instructions,
        bounding_rect,
        measure_action,
        progress_action
    );

    return bounding_rect;
}

typedef struct {
    cairo_t* cr;
    CoordinateSystem* cs;
    double max_line_width;
} DrawActionData;

void draw_action(Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    DrawActionData* dad = (DrawActionData*)data;

    double line_width = max(1, dad->max_line_width - (stack->top / 5));
    cairo_set_line_width(dad->cr, line_width);

    Vector prev_cs = cs_convert(dad->cs, prev);
    Vector curr_cs = cs_convert(dad->cs, turtle->pos);

    cairo_move_to(dad->cr, prev_cs.x, prev_cs.y);
    cairo_line_to(dad->cr, curr_cs.x, curr_cs.y);
    cairo_stroke(dad->cr);
}

void lsys_draw(
    Viewport* viewport,
    CoordinateSystem* cs,
    LSystem* lsys,
    char* instructions,
    Color color,
    double max_line_width,
    char* file_name,
    RenderProgressAction progress_action
) {
    cairo_surface_t *surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32,
        viewport->width,
        viewport->height
    );
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0, 0, 0); // black
    cairo_paint(cr);

    // cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
    // cairo_set_line_width(cr, line_width);

    cairo_pattern_t* radpat = cairo_pattern_create_radial(2464, 3223, 100, 2464, 3223, 3000);

    // // Golden
    // cairo_pattern_add_color_stop_rgb(radpat, 0.0, 235/255.0,219/255.0,102/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.41, 245/255.0, 236/255.0,112/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.51, 209/255.0,190/255.0,76/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.57, 187/255.0,156/255.0,51/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.63, 168/255.0,142/255.0, 42/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.69, 202/255.0,174/255.0,68/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.75, 218/255.0,202/255.0,86/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.81, 208/255.0,187/255.0,73/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.87, 187/255.0,156/255.0,51/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.00, 137/255.0,108/255.0,26/255.0);

    // Simple Pink
    // cairo_pattern_add_color_stop_rgb(radpat, 0.00, 234/255.0,173/255.0,237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.40, 234/255.0,173/255.0,237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0,255/255.0,255/255.0);

    // Yellow-Blue
    cairo_pattern_add_color_stop_rgb(radpat, 0.00, 251/255.0, 255/255.0, 237/134.0);
    cairo_pattern_add_color_stop_rgb(radpat, 1.00, 143/255.0, 211/255.0, 255/255.0);

    // // Strawberry
    // cairo_pattern_add_color_stop_rgb(radpat, 0.00, 166/255.0, 203/255.0, 150/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.80, 255/255.0, 162/255.0, 172/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0, 162/255.0, 172/255.0);

    cairo_set_source(cr, radpat);

    DrawActionData dad = {cr, cs, max_line_width};
    lsys_render(
        lsys,
        instructions,
        &dad,
        draw_action,
        progress_action
    );

    cairo_surface_write_to_png(surface, file_name);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void lsys_destroy(LSystem* lsys) {
    free(lsys->axiom);

    for (int i = 0; i < lsys->rules_count; i++) {
        free(lsys->rules[i].substitution);
    }

    free(lsys->rules);
    free(lsys);
}