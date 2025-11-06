#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stddef.h>  // For size_t

/**
 * @brief Structure representing a 2D/3D point.
 */
typedef struct {
    float x;  /**< X-coordinate */
    float y;  /**< Y-coordinate */
    float z;  /**< Z-coordinate (ignored in 2D mode) */
} Point;

/**
 * @brief Structure representing a set of points (dynamic array).
 */
typedef struct {
    Point* points;  /**< Dynamic array of points */
    size_t count;   /**< Number of points in the set */
    int is_3d;      /**< Flag: 1 if 3D points, 0 if 2D */
} PointSet;

// IO Functions (declared in io.c)
PointSet* load_points(const char* filename);
int save_points(const PointSet* set, const char* filename);
void free_points(PointSet* set);

// Geometry Functions (declared in geometry.c)
PointSet* compute_convex_hull(const PointSet* set, int num_threads);  // Updated: added num_threads param
float compute_distance(const Point* a, const Point* b);
float compute_area(const PointSet* hull);  // Shoelace formula for 2D hull
float compute_path_length(const PointSet* hull);

// Utility Functions
int is_collinear(const Point* a, const Point* b, const Point* c);  // Helper for hull

#endif /* GEOMETRY_H */