#include "cache.h"
#include "color.h"
#include "coordinate_system.h"
#include "lsys.h"
#include "rectangle.h"
#include "timer.h"
#include "turtle_stack.h"
#include "turtle.h"
#include "utils.h"
#include "vector.h"
#include "viewport.h"
#include <cairo.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define OUTPUT_DIR "output"
#define CACHE_DIR "cache"
#define CACHE_FILE "cache.bin"

void create_directory_if_not_exists(const char* dir_path) {
    struct stat st = {0};

    if (stat(dir_path, &st) == -1) {
        mkdir(dir_path, 0700);
    }
}

void percentage_logger_action(size_t current, size_t total) {
    printf("└ Rendering: %.2f %%\r", 100.0 * current / total);
}

int main() {
    create_directory_if_not_exists(OUTPUT_DIR);
    create_directory_if_not_exists(CACHE_DIR);

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
    LSystem lsys = {30, 0, "X", NULL, 0, 20};
    lsys.rules = malloc(sizeof(Rule) * 1);
    lsys.rules[0].symbol = 'X';
    lsys.rules[0].substitution = "[+@.9FX@.5[!X]]";
    lsys.rules_count = 1;

    clock_t start = clock();

    Viewport viewport = {4960, 7016};

    LSystem* cached_lsys;
    Viewport* cached_viewport;
    char* instructions;
    CoordinateSystem* coord_system;

    char cache_file_path[256];
    sprintf(cache_file_path, "%s/%s", CACHE_DIR, CACHE_FILE);

    bool cache_loaded = cache_load(
        &cached_lsys,
        &cached_viewport,
        &coord_system,
        &instructions,
        cache_file_path
    );

    bool stale = !cache_loaded ||
        cache_stale(&lsys, cached_lsys, &viewport, cached_viewport);

    if (cache_loaded && !stale) {
        printf("Cache loaded.\n");
    } else {
        if (cache_loaded && stale) {
            printf("Cache stale; recalculating.\n");
            free(cached_lsys->rules);
            free(cached_lsys);
            free(cached_viewport);
            free(instructions);
            free(coord_system);
        }

        instructions = lsys_iterate(&lsys);

        printf("Measuring:\n");
        Timer timer = timer_start();
        Rectangle* bounding_rect = lsys_measure(&lsys, instructions, percentage_logger_action);
        double elapsed = timer_stop(&timer);
        printf("└ Elapsed: %.1f seconds\n", elapsed);

        coord_system = cs_fit(bounding_rect, &viewport, 400);

        cache_save(&lsys, &viewport, coord_system, instructions, cache_file_path);
        printf("Cache saved.\n");

        free(bounding_rect);
    }

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
        Timer timer = timer_start();
        lsys_draw(
            &viewport,
            coord_system,
            &lsys,
            instructions,
            colors[0],
            line_widths[wi],
            file_name,
            percentage_logger_action
        );
        double elapsed = timer_stop(&timer);
        printf("└ Elapsed: %.1f seconds\n", elapsed);
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
    free(coord_system);
    free(lsys.rules);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total: %.1f seconds\n", elapsed);

    return 0;
}
