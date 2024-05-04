#include "cache.h"
#include "color.h"
#include "coordinate_system.h"
#include "lsys_parser.h"
#include "lsys.h"
#include "rectangle.h"
#include "timer.h"
#include "turtle_stack.h"
#include "turtle.h"
#include "utils.h"
#include "vector.h"
#include "viewport.h"
#include <cairo.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "pattern_parser.h"

#define OUTPUT_DIR "output"
#define CACHE_DIR "cache"
#define CACHE_FILE_PATH CACHE_DIR "/cache.bin"

typedef struct {
    char* file_name;
    int iterations;
    int padding;
    double max_line_width;
    double min_line_width;
    Viewport viewport;
    PatternType fg_pattern_type;
    Coloring fg_coloring;
    char* fg_coloring_str;
} ProgramOptions;

void print_usage(char* program_name) {
    printf("Usage: %s --file filename ", program_name);
    printf("--viewport WxH ");
    printf("[--iter iterations] ");
    printf("[--max-line-width n] ");
    printf("[--min-line-width n] ");
    printf("[--padding n] ");
    printf("[--foreground spec] ");
    printf("\n");
}

ProgramOptions parse_program_options(int argc, char** argv) {
    ProgramOptions options;
    options.file_name = NULL;
    options.iterations = -1;
    options.padding = 0;
    options.max_line_width = 1.0;
    options.min_line_width = 1.0;
    options.viewport = (Viewport) {0, 0};
    options.fg_pattern_type = SOLID;
    options.fg_coloring = coloring_pattern_create(cairo_pattern_create_rgb(1, 1, 1));
    options.fg_coloring_str = "rgb(255,255,255)";

    struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"iter", required_argument, 0, 'i'},
        {"padding", required_argument, 0, 'p'},
        {"max-line-width", required_argument, 0, 'W'},
        {"min-line-width", required_argument, 0, 'w'},
        {"viewport", required_argument, 0, 'v'},
        {"foreground", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}  // End of array need to be filled with zeros
    };

    char opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "hi:f:p:W:w:v:c:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'f':
                options.file_name = optarg;
                break;
            case 'i':
                options.iterations = atoi(optarg);
                break;
            case 'p':
                options.padding = atoi(optarg);
                break;
            case 'W':
                options.max_line_width = atof(optarg);
                break;
            case 'w':
                options.min_line_width = atof(optarg);
                break;
            case 'v':
                options.viewport = viewport_parse(optarg);
                break;
            case 'c':
                coloring_pattern_destroy(options.fg_coloring);
                options.fg_pattern_type = parse_pattern_type(optarg);
                options.fg_coloring = coloring_pattern_create(
                    parse_pattern(optarg, &options.fg_coloring_str)
                );
                break; 
            case '?':
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (
        options.file_name == NULL ||
        options.iterations == -1 ||
        options.viewport.width == 0 ||
        options.viewport.height == 0 ||
        options.max_line_width < 0 ||
        options.min_line_width < 0 ||
        options.min_line_width > options.max_line_width ||
        options.padding < 0 ||
        options.padding > options.viewport.width / 2 ||
        options.padding > options.viewport.height / 2
    ) {
        print_usage(argv[0]);
        exit(1);
    }

    return options;
}

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
    ProgramOptions options = parse_program_options(argc, argv);

    char* lsys_name = get_filename_without_ext(options.file_name);

    LSystem lsys = lsys_load(options.file_name);
    lsys.iterations = options.iterations;

    printf("Loaded from '%s':\n", options.file_name);
    lsys_print(lsys);

    create_directory_if_not_exists(OUTPUT_DIR);
    create_directory_if_not_exists(CACHE_DIR);

    Timer total_timer = timer_start();

    LSystem cached_lsys;
    Viewport cached_viewport;
    int cached_padding;
    CoordinateSystem coord_system;
    char* instructions;

    bool cache_loaded = cache_load(
        &cached_lsys,
        &cached_viewport,
        &cached_padding,
        &coord_system,
        &instructions,
        CACHE_FILE_PATH
    );

    bool stale = !cache_loaded || cache_stale(
        lsys,
        cached_lsys,
        options.viewport,
        cached_viewport,
        options.padding,
        cached_padding
    );

    if (cache_loaded && !stale) {
        printf("Cache loaded.\n");
    } else {
        if (cache_loaded && stale) {
            printf("Cache stale; recalculating.\n");
            lsys_destroy(cached_lsys);
            free(instructions);
        }
    
        printf("Iterating:\n");
        Timer timer = timer_start();
        IterateLoggerData ild = {&timer, lsys.iterations};
        instructions = lsys_iterate(lsys, iterate_logger_action, &ild);

        printf("Measuring:\n");
        timer = timer_start();
        Rectangle bounding_rect = lsys_measure(lsys, instructions, percentage_logger_action);
        double elapsed = timer_stop(&timer);
        printf("└ Elapsed: %.1f seconds\n", elapsed);

        coord_system = cs_fit(bounding_rect, options.viewport, options.padding);

        cache_save(
            lsys,
            options.viewport,
            options.padding,
            coord_system,
            instructions,
            CACHE_FILE_PATH
        );

        printf("Cache saved.\n");
    }

    char file_name[100];
    sprintf(
        file_name,
        "output/%s_line(%g-%g)_%s_iter(%d).png",
        lsys_name,
        options.max_line_width,
        options.min_line_width,
        options.fg_coloring_str,
        lsys.iterations
    );

    printf("Drawing: %s\n", file_name);
    Timer timer = timer_start();
    lsys_draw(
        options.viewport,
        coord_system,
        lsys,
        instructions,
        options.fg_coloring,
        options.max_line_width,
        options.min_line_width,
        file_name,
        percentage_logger_action
    );
    double elapsed = timer_stop(&timer);
    printf("└ Elapsed: %.1f seconds\n", elapsed);

    free(instructions);
    lsys_destroy(lsys);

    double total_elapsed = timer_stop(&total_timer);
    printf("Total: %.1f seconds\n", total_elapsed);

    return 0;
}
