#include "../include/geometry.h"  // Access project headers
#include <stdio.h>                // For printf in tests
#include <stdlib.h>               // For malloc/free in tests
#include <math.h>                 // For fabs in assertions
#include <string.h>               // For strcmp if needed

#define ASSERT_TRUE(cond) do { \
    tests_run++; \
    if (!(cond)) { \
        printf("FAIL: %s\n", #cond); \
        tests_failed++; \
    } \
} while (0)

#define ASSERT_FLOAT_EQ(expected, actual, epsilon) do { \
    tests_run++; \
    if (fabsf((expected) - (actual)) > (epsilon)) { \
        printf("FAIL: Expected %f, got %f\n", (float)(expected), (float)(actual)); \
        tests_failed++; \
    } \
} while (0)

static int tests_run = 0;
static int tests_failed = 0;

// Test IO: load and save (using temporary in-memory simulation)
static void test_io() {
    // Hardcoded sample data (equivalent to a small CSV)
    Point sample_points[] = {{0,0,0}, {1,1,0}, {2,0,0}};
    size_t sample_count = 3;

    // Simulate load (we'll create a set manually)
    PointSet* set = malloc(sizeof(PointSet));
    set->points = malloc(sample_count * sizeof(Point));
    memcpy(set->points, sample_points, sample_count * sizeof(Point));
    set->count = sample_count;
    set->is_3d = 0;

    // Test save (to a temp file, but for simplicity, we just call it; in real test, could check file)
    const char* temp_file = "test_output.csv";
    int save_result = save_points(set, temp_file);
    ASSERT_TRUE(save_result == 0);

    // Test load
    PointSet* loaded = load_points(temp_file);
    ASSERT_TRUE(loaded != NULL);
    ASSERT_TRUE(loaded->count == sample_count);
    ASSERT_TRUE(loaded->is_3d == 0);
    ASSERT_FLOAT_EQ(set->points[0].x, loaded->points[0].x, 0.001f);
    ASSERT_FLOAT_EQ(set->points[1].y, loaded->points[1].y, 0.001f);

    // Cleanup
    free_points(set);
    free_points(loaded);
    remove(temp_file);  // Delete temp file
}

// Test distance
static void test_distance() {
    Point a = {0, 0, 0};
    Point b = {3, 4, 0};
    ASSERT_FLOAT_EQ(5.0f, compute_distance(&a, &b), 0.001f);

    Point c = {0, 0, 0};
    Point d = {1, 2, 2};
    ASSERT_FLOAT_EQ(sqrtf(1+4+4), compute_distance(&c, &d), 0.001f);
}

// Test collinear
static void test_collinear() {
    Point a = {0, 0, 0};
    Point b = {1, 1, 0};
    Point c = {2, 2, 0};
    ASSERT_TRUE(is_collinear(&a, &b, &c) == 1);

    Point d = {0, 0, 0};
    Point e = {1, 0, 0};
    Point f = {0, 1, 0};
    ASSERT_TRUE(is_collinear(&d, &e, &f) == 0);
}

// Test convex hull (simple triangle)
static void test_convex_hull_simple() {
    Point points[] = {{0,0,0}, {1,0,0}, {0,1,0}};
    PointSet set = {points, 3, 0};

    PointSet* hull = compute_convex_hull(&set, 1);  // Fixed: added num_threads=1
    ASSERT_TRUE(hull != NULL);
    ASSERT_TRUE(hull->count == 3);  // Should remain 3 for convex set

    free_points(hull);
}

// Test convex hull with internal point
static void test_convex_hull_with_internal() {
    Point points[] = {{0,0,0}, {4,0,0}, {0,3,0}, {1,1,0}};  // (1,1) is internal
    PointSet set = {points, 4, 0};

    PointSet* hull = compute_convex_hull(&set, 1);  // Fixed: added num_threads=1
    ASSERT_TRUE(hull != NULL);
    ASSERT_TRUE(hull->count == 3);  // Should simplify to triangle

    free_points(hull);
}

// Test convex hull edge case: <3 points
static void test_convex_hull_edge() {
    Point points[] = {{0,0,0}, {1,0,0}};
    PointSet set = {points, 2, 0};

    PointSet* hull = compute_convex_hull(&set, 1);  // Fixed: added num_threads=1
    ASSERT_TRUE(hull == NULL);  // Should fail
}

// Test area
static void test_area() {
    Point points[] = {{0,0,0}, {3,0,0}, {0,4,0}};
    PointSet hull = {points, 3, 0};
    ASSERT_FLOAT_EQ(6.0f, compute_area(&hull), 0.001f);  // (3*4)/2 = 6
}

// Test path length (perimeter)
static void test_path_length() {
    Point points[] = {{0,0,0}, {3,0,0}, {0,4,0}};
    PointSet hull = {points, 3, 0};
    float expected = 3 + 5 + 4;  // Sides: 3, sqrt(9+16)=5, 4
    ASSERT_FLOAT_EQ(expected, compute_path_length(&hull), 0.001f);
}

// Run all tests
void run_all_tests() {
    test_io();
    test_distance();
    test_collinear();
    test_convex_hull_simple();
    test_convex_hull_with_internal();
    test_convex_hull_edge();
    test_area();
    test_path_length();
}

int get_tests_run() { return tests_run; }
int get_tests_failed() { return tests_failed; }