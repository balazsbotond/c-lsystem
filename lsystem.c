#include "cache.h"
#include "color.h"
#include "coordinate_system.h"
#include "lsys.h"
#include "lsys_parser.h"
#include "rectangle.h"
#include "timer.h"
#include "turtle_stack.h"
#include "turtle.h"
#include "utils.h"
#include "vector.h"
#include "viewport.h"
#include <cairo.h>
#include <libgen.h>
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

char* get_filename_without_ext(char* filepath) {
    char* tmp = strdup(filepath);
    char* base = basename(tmp); // get the filename
    char* lastdot = strrchr(base, '.'); // find the last dot

    if (lastdot != NULL) {
        *lastdot = '\0'; // replace the dot with a null character
    }

    return base;
}

void percentage_logger_action(size_t current, size_t total) {
    printf("└ Rendering: %.2f %%\r", 100.0 * current / total);
}

typedef struct {
    Timer* timer;
    int iterations;
} IterateLoggerData;

void iterate_logger_action(int i, void* data) {
    IterateLoggerData* ild = (IterateLoggerData*) data;
    double elapsed = timer_stop(ild->timer);
    bool last = i == ild->iterations - 1;

    printf(last ? "└ " : "├ ");
    printf("i = %d: %.1f seconds\n", i + 1, elapsed);

    if (!last) timer_start(ild->timer);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <file_name> <iterations>\n", argv[0]);
        return 1;
    }

    char* file_name = argv[1];
    char* lsys_name = get_filename_without_ext(file_name);
    int iterations = atoi(argv[2]);

    LSystem* lsys = lsys_load(file_name);
    lsys->iterations = iterations;

    printf("Loaded from '%s':\n", file_name);
    lsys_print(lsys);

    create_directory_if_not_exists(OUTPUT_DIR);
    create_directory_if_not_exists(CACHE_DIR);

    Timer total_timer = timer_start();

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
        cache_stale(lsys, cached_lsys, &viewport, cached_viewport);

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
    
        printf("Iterating:\n");
        Timer timer = timer_start();
        IterateLoggerData ild = {&timer, lsys->iterations};
        instructions = lsys_iterate(lsys, iterate_logger_action, &ild);

        printf("Measuring:\n");
        timer = timer_start();
        Rectangle* bounding_rect = lsys_measure(lsys, instructions, percentage_logger_action);
        double elapsed = timer_stop(&timer);
        printf("└ Elapsed: %.1f seconds\n", elapsed);

        coord_system = cs_fit(bounding_rect, &viewport, 400);

        cache_save(lsys, &viewport, coord_system, instructions, cache_file_path);
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
    // double line_widths[] = {1, 2, 3, 4};
    // int num_line_widths = 4;
    double line_widths[] = {4};
    int num_line_widths = 1;

    for (int wi = 0; wi < num_line_widths; wi++) {
        char file_name[100];
        sprintf(
            file_name,
            "output/%s_line(%g-1)_grad(yellow-blue)_iter(%d).png",
            lsys_name,
            line_widths[wi],
            lsys->iterations
        );

        printf("Drawing: %s\n", file_name);
        Timer timer = timer_start();
        lsys_draw(
            &viewport,
            coord_system,
            lsys,
            instructions,
            colors[0],
            line_widths[wi],
            file_name,
            percentage_logger_action
        );
        double elapsed = timer_stop(&timer);
        printf("└ Elapsed: %.1f seconds\n", elapsed);
    }

    free(instructions);
    free(coord_system);
    lsys_destroy(lsys);

    double total_elapsed = timer_stop(&total_timer);
    printf("Total: %.1f seconds\n", total_elapsed);

    return 0;
}
