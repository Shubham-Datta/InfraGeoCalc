#include "geometry.h"
#include <stdlib.h>  // For qsort, malloc
#include <math.h>    // For sqrt, fabs, atan2
#include <float.h>   // For FLT_MAX
#include <stdio.h>   // For fprintf, stderr
#include <string.h>  // For memcpy
#include <pthread.h> // For multithreading

#define EPSILON 1e-6  // Small value for floating-point comparisons

// Forward declarations for helpers
static int compare_polar(const void* a, const void* b);
static float cross_product(const Point* o, const Point* a, const Point* b);
static Point* pivot = NULL;  // Global for qsort comparator (set in compute_convex_hull)

// Thread arg struct for parallel sorting
typedef struct {
    Point* points;
    size_t start;
    size_t end;
} SortArg;

// Thread function for sorting a chunk
static void* sort_chunk(void* arg) {
    SortArg* s = (SortArg*)arg;
    qsort(s->points + s->start, s->end - s->start, sizeof(Point), compare_polar);
    return NULL;
}

/**
 * @brief Computes the Euclidean distance between two points (2D or 3D).
 * @param a First point.
 * @param b Second point.
 * @return Distance (float).
 */
float compute_distance(const Point* a, const Point* b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return sqrtf(dx*dx + dy*dy + dz*dz);  // dz=0 in 2D
}

/**
 * @brief Checks if three points are collinear.
 * @param a, b, c Points to check.
 * @return 1 if collinear, 0 otherwise.
 */
int is_collinear(const Point* a, const Point* b, const Point* c) {
    float cross = cross_product(a, b, c);
    return fabsf(cross) < EPSILON;
}

// Helper: Cross product for orientation (2D: ignores z)
static float cross_product(const Point* o, const Point* a, const Point* b) {
    return (a->x - o->x) * (b->y - o->y) - (a->y - o->y) * (b->x - o->x);
}

// Helper: Comparator for qsort by polar angle from pivot (2D)
static int compare_polar(const void* a, const void* b) {
    const Point* pa = (const Point*)a;
    const Point* pb = (const Point*)b;
    float cross = cross_product(pivot, pa, pb);
    if (fabsf(cross) < EPSILON) {
        // Collinear: sort by distance
        return (compute_distance(pivot, pa) < compute_distance(pivot, pb)) ? -1 : 1;
    }
    return (cross > 0) ? -1 : 1;  // Counterclockwise
}

/**
 * @brief Computes the convex hull of a point set using Graham's Scan (2D projection), with multithreading.
 * @param set Input PointSet.
 * @param num_threads Number of threads for parallel sorting.
 * @return New PointSet with hull points, or NULL on failure.
 */
PointSet* compute_convex_hull(const PointSet* set, int num_threads) {
    if (!set || set->count < 3) {
        fprintf(stderr, "Convex hull requires at least 3 points\n");
        return NULL;
    }
    if (num_threads < 1) num_threads = 1;  // Clamp

    // Create a copy to sort
    Point* points = malloc(set->count * sizeof(Point));
    if (!points) {
        fprintf(stderr, "Memory allocation failed for hull\n");
        return NULL;
    }
    memcpy(points, set->points, set->count * sizeof(Point));

    // Find pivot
    size_t min_idx = 0;
    for (size_t i = 1; i < set->count; ++i) {
        if (points[i].y < points[min_idx].y || 
            (points[i].y == points[min_idx].y && points[i].x < points[min_idx].x)) {
            min_idx = i;
        }
    }
    Point temp = points[0];
    points[0] = points[min_idx];
    points[min_idx] = temp;
    pivot = &points[0];

    // Parallel sort remaining points
    size_t remaining = set->count - 1;
    pthread_t threads[num_threads];
    SortArg args[num_threads];
    size_t chunk_size = remaining / num_threads;
    size_t offset = 1;  // Start after pivot
    for (int i = 0; i < num_threads; ++i) {
        args[i].points = points;
        args[i].start = offset;
        args[i].end = offset + chunk_size + ((size_t)i < remaining % (size_t)num_threads ? 1 : 0);  // Fixed: cast i to size_t for comparison
        if (args[i].start < args[i].end) {
            pthread_create(&threads[i], NULL, sort_chunk, &args[i]);
        }
        offset = args[i].end;
    }
    for (int i = 0; i < num_threads; ++i) {
        if (args[i].start < args[i].end) {
            pthread_join(threads[i], NULL);
        }
    }
    // Merge sorted chunks (simple but could use merge sort for better parallelism)
    qsort(points + 1, remaining, sizeof(Point), compare_polar);  // Final merge sort

    // Build hull (serial for simplicity)
    PointSet* hull = malloc(sizeof(PointSet));
    if (!hull) {
        free(points);
        return NULL;
    }
    hull->points = malloc(set->count * sizeof(Point));
    if (!hull->points) {
        free(hull);
        free(points);
        return NULL;
    }
    hull->count = 0;
    hull->is_3d = set->is_3d;

    hull->points[hull->count++] = points[0];
    hull->points[hull->count++] = points[1];
    if (set->count == 2) {
        free(points);
        return hull;
    }
    hull->points[hull->count++] = points[2];

    for (size_t i = 3; i < set->count; ++i) {
        while (hull->count >= 2 && cross_product(&hull->points[hull->count-2], 
                                                 &hull->points[hull->count-1], 
                                                 &points[i]) <= 0) {
            hull->count--;
        }
        hull->points[hull->count++] = points[i];
    }

    hull->points = realloc(hull->points, hull->count * sizeof(Point));
    free(points);
    return hull;
}

/**
 * @brief Computes the area of a 2D polygon (convex hull) using shoelace formula.
 * @param hull The PointSet (assumed 2D polygon).
 * @return Area (float), or -1 on invalid input.
 */
float compute_area(const PointSet* hull) {
    if (!hull || hull->count < 3) return -1.0f;

    float area = 0.0f;
    for (size_t i = 0; i < hull->count; ++i) {
        size_t j = (i + 1) % hull->count;
        area += hull->points[i].x * hull->points[j].y;
        area -= hull->points[j].x * hull->points[i].y;
    }
    return fabsf(area) / 2.0f;
}

/**
 * @brief Computes the total path length around the hull (perimeter).
 * @param hull The PointSet.
 * @return Total length (float), or -1 on invalid input.
 */
float compute_path_length(const PointSet* hull) {
    if (!hull || hull->count < 2) return -1.0f;

    float length = 0.0f;
    for (size_t i = 0; i < hull->count; ++i) {
        size_t j = (i + 1) % hull->count;
        length += compute_distance(&hull->points[i], &hull->points[j]);
    }
    return length;
}