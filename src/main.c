#include "geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  // For clock() timing

/**
 * @brief Prints usage information.
 */
static void print_usage(const char* progname) {
    fprintf(stderr, "Usage: %s input.csv|input.obj output.csv [--mode hull] [--dim 2|3] [--threads N] [--benchmark]\n", progname);
    fprintf(stderr, "  Supports CSV (x,y[,z]) or OBJ (v x y z) input.\n");
    fprintf(stderr, "  --mode hull: Compute convex hull (default)\n");
    fprintf(stderr, "  --dim 2|3: Force 2D or 3D mode (default: auto-detect)\n");
    fprintf(stderr, "  --threads N: Number of threads for computation (default: 1)\n");
    fprintf(stderr, "  --benchmark: Run performance benchmarks (ignores input/output files)\n");
}

// Simple function to generate synthetic points for benchmarking
static PointSet* generate_synthetic_points(size_t count, int is_3d) {
    PointSet* set = malloc(sizeof(PointSet));
    set->points = malloc(count * sizeof(Point));
    set->count = count;
    set->is_3d = is_3d;
    for (size_t i = 0; i < count; ++i) {
        set->points[i].x = (float)rand() / RAND_MAX * 100.0f;
        set->points[i].y = (float)rand() / RAND_MAX * 100.0f;
        set->points[i].z = is_3d ? (float)rand() / RAND_MAX * 100.0f : 0.0f;
    }
    return set;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    char* mode = "hull";  // Default mode
    int forced_dim = -1;  // -1: auto, 2: force 2D, 3: force 3D
    int num_threads = 1;  // Default threads
    int benchmark = 0;    // Flag for benchmark mode

    // Simple CLI parsing
    for (int i = 3; i < argc; i += 2) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = argv[i + 1];
        } else if (strcmp(argv[i], "--dim") == 0 && i + 1 < argc) {
            forced_dim = atoi(argv[i + 1]);
            if (forced_dim != 2 && forced_dim != 3) {
                fprintf(stderr, "Invalid --dim: must be 2 or 3\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[i + 1]);
            if (num_threads < 1) {
                fprintf(stderr, "Invalid --threads: must be at least 1\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--benchmark") == 0) {
            benchmark = 1;
            i--;  // Adjust for single-arg flag
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (benchmark) {
        printf("Running benchmarks (Threads: %d, Dim: %s)...\n", num_threads, forced_dim == 3 ? "3D" : "2D");
        srand(time(NULL));  // Seed random
        size_t sizes[] = {100, 1000, 10000};  // Test sizes
        int is_3d = (forced_dim == 3 ? 1 : 0);
        for (size_t s = 0; s < sizeof(sizes)/sizeof(sizes[0]); ++s) {
            PointSet* set = generate_synthetic_points(sizes[s], is_3d);
            clock_t start = clock();
            PointSet* hull = compute_convex_hull(set, num_threads);
            double time_taken = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
            size_t hull_count = hull ? hull->count : 0;
            printf("Size %zu: Time %.2f ms, Simplified to %zu points (Reduction: %.1f%%)\n", 
                   set->count, time_taken, hull_count, set->count > 0 ? (1.0 - (double)hull_count / set->count) * 100 : 0);
            free_points(set);
            free_points(hull);
        }
        return 0;
    }

    clock_t start = clock();

    PointSet* set = load_points(input_file);
    if (!set) {
        return 1;
    }

    // Apply forced dimension if specified
    if (forced_dim != -1) {
        set->is_3d = (forced_dim == 3);
    }

    printf("Loaded %zu points (3D: %d) from %s\n", set->count, set->is_3d, input_file);  // Added file note

    PointSet* result = NULL;
    if (strcmp(mode, "hull") == 0) {
        result = compute_convex_hull(set, num_threads);
        if (!result) {
            free_points(set);
            return 1;
        }
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        free_points(set);
        return 1;
    }

    // Compute metrics
    float area = compute_area(result);
    float perimeter = compute_path_length(result);

    // Output results
    printf("Mode: %s (Threads: %d)\n", mode, num_threads);
    printf("Simplified from %zu to %zu points\n", set->count, result->count);
    printf("Area: %.2f\n", area);
    printf("Perimeter: %.2f\n", perimeter);

    if (save_points(result, output_file) != 0) {
        free_points(set);
        free_points(result);
        return 1;
    }

    double time_taken = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("Computation time: %.2f ms\n", time_taken);

    free_points(set);
    free_points(result);
    return 0;
}