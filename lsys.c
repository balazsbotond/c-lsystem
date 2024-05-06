#include "lsys.h"
#include "turtle_stack.h"
#include "turtle.h"
#include "utils.h"
#include <cairo.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char* lookup_rule(LSystem lsys, char symbol) {
    for (int i = 0; i < lsys.rules_count; i++) {
        if (lsys.rules[i].symbol == symbol) {
            return lsys.rules[i].substitution;
        }
    }

    return NULL;
}

void lsys_print(LSystem lsys) {
    printf("├ angle: %f°\n", lsys.angle);
    printf("├ rotation: %f°\n", lsys.rotation);
    printf("├ axiom: %s\n", lsys.axiom);
    printf("├ iterations: %d\n", lsys.iterations);
    printf("└ rules:\n");

    for (int i = 0; i < lsys.rules_count; i++) {
        bool last = i == lsys.rules_count - 1;
        printf(last ? "  └ " : "  ├ ");
        printf("%c -> %s\n", lsys.rules[i].symbol, lsys.rules[i].substitution);
    }
}

bool lsys_equals(LSystem lsys1, LSystem lsys2) {
    if (lsys1.angle != lsys2.angle) return false;
    if (lsys1.rotation != lsys2.rotation) return false;
    if (strcmp(lsys1.axiom, lsys2.axiom) != 0) return false;
    if (lsys1.rules_count != lsys2.rules_count) return false;
    if (lsys1.iterations != lsys2.iterations) return false;

    for (int i = 0; i < lsys1.rules_count; i++) {
        if (lsys1.rules[i].symbol != lsys2.rules[i].symbol) return false;
        if (strcmp(lsys1.rules[i].substitution, lsys2.rules[i].substitution) != 0) return false;
    }

    return true;
}

char* lsys_iterate(
    LSystem lsys,
    IterateProgressAction progress_action,
    void* action_data
) {
    char* current = malloc(sizeof(char) * (strlen(lsys.axiom) + 1));
    strcpy(current, lsys.axiom);
    char* next;

    for (int i = 0; i < lsys.iterations; i++) {

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
    LSystem lsys,
    char* instructions,
    void* action_data,
    TurtleAction action,
    RenderProgressAction progress_action
) {
    Turtle turtle = turtle_create(lsys.rotation);
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
            turtle_turn_left(&turtle, lsys.angle);
        } else if (c == '-') {
            turtle_turn_right(&turtle, lsys.angle);
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

Rectangle lsys_measure(
    LSystem lsys,
    char* instructions,
    RenderProgressAction progress_action
) {
    Rectangle bounding_rect;
    bounding_rect.left = 0;
    bounding_rect.top = 0;
    bounding_rect.right = 0;
    bounding_rect.bottom = 0;

printf("first instruction before measuring: %c\n", instructions[0]);
    lsys_render(lsys, instructions, &bounding_rect, measure_action, progress_action);
printf("first instruction after measuring: %c\n", instructions[0]);

    return bounding_rect;
}

/*
 * Background pattern plugin
 */

void plugin_bg_pattern_init(cairo_t* cr, void* data) {
    cairo_pattern_t* pattern = (cairo_pattern_t*)data;
    cairo_set_source(cr, pattern);
    cairo_paint(cr);
}

void plugin_bg_pattern_draw(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    // do nothing
}

void plugin_bg_pattern_finish(cairo_t* cr, void* data) {
    // do nothing
}

Plugin lsys_plugin_bg_pattern_create(cairo_pattern_t* pattern) {
    Plugin plugin;
    plugin.on_init = plugin_bg_pattern_init;
    plugin.on_draw = plugin_bg_pattern_draw;
    plugin.on_finish = plugin_bg_pattern_finish;
    plugin.data = pattern;

    return plugin;
}

void lsys_plugin_bg_pattern_destroy(Plugin plugin) {
    cairo_pattern_destroy((cairo_pattern_t*)plugin.data);
}

/*
 * Foreground pattern plugin
 */

void plugin_pattern_init(cairo_t* cr, void* data) {
    cairo_pattern_t* pattern = (cairo_pattern_t*)data;
    cairo_set_source(cr, pattern);
}

void plugin_pattern_draw(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    // do nothing
}

void plugin_pattern_finish(cairo_t* cr, void* data) {
    // do nothing
}

Plugin lsys_plugin_pattern_create(cairo_pattern_t* pattern) {
    Plugin plugin;
    plugin.on_init = plugin_pattern_init;
    plugin.on_draw = plugin_pattern_draw;
    plugin.on_finish = plugin_pattern_finish;
    plugin.data = pattern;

    return plugin;
}

void lsys_plugin_pattern_destroy(Plugin plugin) {
    cairo_pattern_destroy((cairo_pattern_t*)plugin.data);
}

/*
 * Stack depth line width plugin
 */

void plugin_stack_depth_line_width_init(cairo_t* cr, void* data) {
    // do nothing
}

void plugin_stack_depth_line_width_draw(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    StackDepthLineWidthOptions* opts = (StackDepthLineWidthOptions*)data;

    double line_width = max(
        opts->min_width,
        opts->max_width - (stack->top / (double)opts->step)
    );

    cairo_set_line_width(cr, line_width);
}

void plugin_stack_depth_line_width_finish(cairo_t* cr, void* data) {
    // do nothing
}

Plugin lsys_plugin_stack_depth_line_width_create(StackDepthLineWidthOptions options) {
    StackDepthLineWidthOptions* opts_ptr = malloc(sizeof(StackDepthLineWidthOptions));
    *opts_ptr = options;

    Plugin plugin;
    plugin.on_init = plugin_stack_depth_line_width_init;
    plugin.on_draw = plugin_stack_depth_line_width_draw;
    plugin.on_finish = plugin_stack_depth_line_width_finish;
    plugin.data = opts_ptr;

    return plugin;
}

void lsys_plugin_stack_depth_line_width_destroy(Plugin plugin) {
    // do nothing
}

/*
 * Stack depth palette plugin
 */

void plugin_stack_depth_palette_init(cairo_t* cr, void* data) {
    // do nothing
}

void plugin_stack_depth_palette_draw(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    Palette* palette = (Palette*)data;

    int color_index = palette->overflow == PALETTE_OVERFLOW_WRAP
        ? stack->top % palette->colors_count
        : min(stack->top, palette->colors_count - 1);

    Color color = palette->colors[color_index];

    cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
}

void plugin_stack_depth_palette_finish(cairo_t* cr, void* data) {
    // do nothing
}

Plugin lsys_plugin_stack_depth_palette_create(Palette palette) {
    Palette* palette_ptr = malloc(sizeof(Palette));
    *palette_ptr = palette;

    Plugin plugin;
    plugin.on_init = plugin_stack_depth_palette_init;
    plugin.on_draw = plugin_stack_depth_palette_draw;
    plugin.on_finish = plugin_stack_depth_palette_finish;
    plugin.data = palette_ptr;

    return plugin;
}

void lsys_plugin_stack_depth_palette_destroy(Plugin plugin) {
    Palette* palette = (Palette*)plugin.data;
    free(palette->colors);
    free(palette);
}

/*
 * Linear palette plugin
 */

void plugin_linear_palette_init(cairo_t* cr, void* data) {
    // do nothing
}

void plugin_linear_palette_draw(cairo_t* cr, Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    Palette* palette = (Palette*)data;

    Color color = palette->colors[palette->index];
    palette->index =
        palette->overflow == PALETTE_OVERFLOW_WRAP
            ? (palette->index + 1) % palette->colors_count
            : min(palette->index + 1, palette->colors_count - 1);

    cairo_set_source_rgb(cr, color.red / 255.0, color.green / 255.0, color.blue / 255.0);
}

void plugin_linear_palette_finish(cairo_t* cr, void* data) {
    // do nothing
}

Plugin lsys_plugin_linear_palette_create(Palette palette) {
    Palette* palette_ptr = malloc(sizeof(Palette));
    *palette_ptr = palette;

    Plugin plugin;
    plugin.on_init = plugin_linear_palette_init;
    plugin.on_draw = plugin_linear_palette_draw;
    plugin.on_finish = plugin_linear_palette_finish;
    plugin.data = palette_ptr;

    return plugin;
}

void lsys_plugin_linear_palette_destroy(Plugin plugin) {
    Palette* palette = (Palette*)plugin.data;
    free(palette->colors);
    free(palette);
}

typedef struct {
    cairo_t* cr;
    CoordinateSystem cs;
    Plugin* plugins;
    int plugins_count;
} DrawActionData;

void draw_action(Turtle* turtle, TurtleStack* stack, Vector prev, void* data) {
    DrawActionData* dad = (DrawActionData*)data;

    for (int i = 0; i < dad->plugins_count; i++) {
        dad->plugins[i].on_draw(dad->cr, turtle, stack, prev, dad->plugins[i].data);
    }

    Vector prev_cs = cs_convert(dad->cs, prev);
    Vector curr_cs = cs_convert(dad->cs, turtle->pos);

    cairo_move_to(dad->cr, prev_cs.x, prev_cs.y);
    cairo_line_to(dad->cr, curr_cs.x, curr_cs.y);
    cairo_stroke(dad->cr);
}

void lsys_draw(
    Viewport viewport,
    CoordinateSystem cs,
    LSystem lsys,
    char* instructions,
    Plugin* plugins,
    int plugins_count,
    char* file_name,
    RenderProgressAction progress_action
) {
    cairo_surface_t *surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32,
        viewport.width,
        viewport.height
    );
    cairo_t *cr = cairo_create(surface);

    for (int i = 0; i < plugins_count; i++) {
        plugins[i].on_init(cr, plugins[i].data);
    }

    DrawActionData dad = {cr, cs, plugins, plugins_count};
    lsys_render(lsys, instructions, &dad, draw_action, progress_action);

    cairo_surface_write_to_png(surface, file_name);

    for (int i = 0; i < plugins_count; i++) {
        plugins[i].on_finish(cr, plugins[i].data);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void lsys_destroy(LSystem lsys) {
    free(lsys.axiom);

    for (int i = 0; i < lsys.rules_count; i++) {
        free(lsys.rules[i].substitution);
    }

    free(lsys.rules);
}