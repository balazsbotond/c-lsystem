#include <cairo.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

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

const int STACK_SIZE = 10000;

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
    char* current = malloc(sizeof(char) * (strlen(lsys->axiom) + 1));
    strcpy(current, lsys->axiom);
    char* next;
    printf("Iterating...\n");

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
        printf(i == lsys->iterations - 1 ? "└ " : "├ ");
        printf("i = %d: %f seconds\n", i + 1, elapsed);
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

void lsys_draw(Viewport* viewport, CoordinateSystem* cs, LSystem* lsys, char* instructions, Color color, double max_line_width, char* file_name) {
    clock_t start = clock();
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, viewport->width, viewport->height);
    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0, 0, 0); // black
    cairo_paint(cr);

    // cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
    // cairo_set_line_width(cr, line_width);

    // cairo_pattern_t* radpat = cairo_pattern_create_radial(1881, 3069, 100, 1881, 3069, 3000);
    cairo_pattern_t* radpat = cairo_pattern_create_radial(2464, 3223, 100, 2464, 3223, 3000);
    // cairo_pattern_t* radpat = cairo_pattern_create_radial(4960 / 2, 7016 / 2, 0, 4960 / 2, 7016 / 2, max(4960, 7016) / 2);

    // Pink
    // cairo_pattern_add_color_stop_rgb(radpat, 0.0, 234/255.0, 173/255.0, 237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.2, 1.0, 1.0, 1.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.3, 234/255.0, 173/255.0, 237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.4, 1.0, 1.0, 1.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.5, 234/255.0, 173/255.0, 237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.0, 1.0, 1.0, 1.0);

    // Horizon
    // cairo_pattern_add_color_stop_rgb(radpat, 0.00, 12/255.0, 92/255.0, 146/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.53, 255/255.0, 251/255.0,  251/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.55, 66/255.0, 31/255.0, 9/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.61, 255/255.0, 207/255.0, 140/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.94, 89/255.0, 41/255.0, 15/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0, 142/255.0, 56/255.0);

    // // Pastels
    // cairo_pattern_add_color_stop_rgb(radpat, 0, 245/255.0,224/255.0,176/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.09, 246/255.0,189/255.0, 237/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.13, 194/255.0,207/255.0,246/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.18, 184/255.0,160/255.0,168/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.25, 171/255.0,186/255.0,248/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.32, 243/255.0,248/255.0,224/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.38, 249/255.0,162/255.0,183/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.47, 100/255.0,115/255.0,124/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.58, 251/255.0,205/255.0,202/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.64, 170/255.0,128/255.0,185/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.74, 252/255.0,222/255.0,204/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.80, 206/255.0,122/255.0,218/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.86, 254/255.0,223/255.0,175/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 0.91, 254/255.0,236/255.0,244/255.0);
    // cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0,191/255.0, 221/255.0);

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

    Turtle turtle = turtle_create(lsys->rotation);
    TurtleStack* stack = stack_create();
    char* cp = instructions;
    int palette_index = 0;
    size_t i = 0;
    size_t instructions_length = strlen(instructions);

    printf("└ Rendering: %.2f %%\r", 0.0);

    while (*cp != '\0') {
        char c = *cp;

        if (c == 'F' || c == 'D') { // go forward
            Vector next = op_go_forward(turtle.pos, turtle.stroke_length, turtle.angle);

            // Color color = palette[palette_index];
            // cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
            // palette_index = (palette_index + 1) % palette_size;

            // if (stack->top <= 50) {
                double line_width = max(1, max_line_width - (stack->top / 5));
                cairo_set_line_width(cr, line_width);

                cairo_move_to(cr, cs_convert_x(cs, turtle.pos.x), cs_convert_y(cs, turtle.pos.y));
                cairo_line_to(cr, cs_convert_x(cs, next.x), cs_convert_y(cs, next.y));
                cairo_stroke(cr);
            // }

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
        i++;

        if (i % 100000 == 0) {
            printf("└ Rendering: %.2f %%\r", 100.0 * i / instructions_length);
        }
    }

    printf("└ Rendering: %.2f %%", 100.0);

    cairo_surface_write_to_png(surface, file_name);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    stack_destroy(stack);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf(" - %f seconds\n", elapsed);
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
    LSystem lsys = {30, 0, "X", NULL, 0, 29};
    lsys.rules = malloc(sizeof(Rule) * 1);
    lsys.rules[0].symbol = 'X';
    lsys.rules[0].substitution = "[+@.9FX@.5[!X]]";
    lsys.rules_count = 1;

    clock_t start = clock();

    Viewport viewport = {4960, 7016};
    char* instructions = lsys_iterate(&lsys);
    Rectangle* bounding_rect = lsys_measure(&lsys, instructions);
    CoordinateSystem* coord_system = cs_fit(bounding_rect, &viewport, 400);

    Color colors[] = {
        {234, 173, 237}, // light pink
        {143, 248, 226}, // light cyan
        {235, 190, 237}, // lighter pink
        {199, 220, 208}, // light teal
        {143, 211, 255}, // light blue
        {255, 255, 255},
        {204, 181, 74},
        {110, 240, 74},
        {84, 112, 240}
    };
    int num_colors = 9;
    double line_widths[] = {1, 2, 3, 4};
    int num_line_widths = 4;

    // Color palette[] = {
    //     {22, 99, 150}, 
    //     {43, 113, 160}, 
    //     {64, 126, 168}, 
    //     {90, 143, 180}, 
    //     {111, 156, 188}, 
    //     {131, 170, 198}, 
    //     {169, 195, 214}, 
    //     {209, 221, 231}, 
    //     {248, 246, 249}, 
    //     {228, 183, 122}, 
    //     {233, 185, 124}, 
    //     {203, 156, 101}, 
    //     {173, 125, 79}, 
    //     {144, 96, 56}, 
    //     {115, 67, 35}, 
    //     {111, 54, 21}        
    // };
    // int palette_size = 16;
    // char palette_name[] = "blue-brown";

    // Color palette[] = {
    //     {93, 146, 182}, 
    //     {107, 156, 189}, 
    //     {122, 165, 194}, 
    //     {140, 177, 203}, 
    //     {155, 186, 208}, 
    //     {169, 196, 215}, 
    //     {195, 213, 226}, 
    //     {223, 231, 238}, 
    //     {250, 249, 251}, 
    //     {236, 205, 162}, 
    //     {240, 206, 164}, 
    //     {219, 186, 148}, 
    //     {198, 164, 132}, 
    //     {178, 144, 116}, 
    //     {157, 124, 102}, 
    //     {155, 115, 92}
    // };
    // int palette_size = 16;
    // char palette_name[] = "blue-brown-light";

    // Color palette[] = {
    //     {144, 115, 27}, 
    //     {169, 144, 44}, 
    //     {199, 176, 69}, 
    //     {225, 207, 93}, 
    //     {239, 225, 106}, 
    //     {242, 233, 109}, 
    //     {226, 211, 93}, 
    //     {208, 189, 75}, 
    //     {189, 159, 54}, 
    //     {174, 146, 45}, 
    //     {191, 164, 59}, 
    //     {209, 186, 75}, 
    //     {215, 199, 83}, 
    //     {207, 186, 73}, 
    //     {191, 161, 55}, 
    //     {154, 124, 35}
    // };
    // int palette_size = 16;
    // char palette_name[] = "gold";

    // Color colors[] = {
    //     {255, 255, 255},
    //     {204, 181, 74},
    //     {110, 240, 74},
    //     {84, 112, 240}
    // };
    Color palette[] = {
        {204, 181, 74},
    };
    int palette_size = 1;
    char palette_name[] = "yellow";

    // for (int wi = 0; wi < num_line_widths; wi++) {
    //     for (int ci = 0; ci < num_colors; ci++) {
    //         char file_name[100];
    //         sprintf(
    //             file_name,
    //             "output/infinite-leaf_line(%g-1)_rgb(%d,%d,%d)_iter(%d).png",
    //             line_widths[wi],
    //             colors[ci].red,
    //             colors[ci].green,
    //             colors[ci].blue,
    //             lsys.iterations
    //         );

    //         printf("Drawing: %s\n", file_name);

    //         lsys_draw(
    //             &viewport,
    //             coord_system,
    //             &lsys,
    //             instructions,
    //             colors[ci],
    //             line_widths[wi],
    //             file_name
    //         );

    //     }
    // }

    for (int wi = 0; wi < num_line_widths; wi++) {
        char file_name[100];
        sprintf(
            file_name,
            "output/infinite-leaf_line(%g-1)_grad(yellow-blue)_iter(%d).png",
            line_widths[wi],
            lsys.iterations
        );

        printf("Drawing: %s\n", file_name);

        lsys_draw(
            &viewport,
            coord_system,
            &lsys,
            instructions,
            colors[0],
            line_widths[wi],
            file_name
        );
    }

    // for (int wi = 0; wi < 4; wi++) {
        // printf(
        //     "Drawing: output/infinite-leaf_line(3-1)_palette(%s)_iter(%d).png\n",
        //     palette_name,
        //     lsys.iterations
        // );

        // char file_name[100];
        // sprintf(
        //     file_name,
        //     "output/infinite-leaf_line(3-1)_palette(%s)_iter(%d).png",
        //     palette_name,
        //     lsys.iterations
        // );

        // lsys_draw(
        //     &viewport,
        //     coord_system,
        //     &lsys,
        //     instructions,
        //     palette,
        //     palette_size,
        //     3,
        //     file_name
        // );
    // }

    free(instructions);
    free(bounding_rect);
    free(coord_system);
    free(lsys.rules);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total: %f seconds\n", elapsed);

    return 0;
}
