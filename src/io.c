#include "geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  // For errno and strerror
#include <ctype.h>  // For tolower in extension check

#define INITIAL_CAPACITY 100  // Starting size for dynamic array
#define BUFFER_SIZE 256       // For reading lines

// Helper: Check if filename ends with extension (case-insensitive)
static int ends_with(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suf_len = strlen(suffix);
    if (str_len < suf_len) return 0;
    for (size_t i = 0; i < suf_len; ++i) {
        if (tolower(str[str_len - suf_len + i]) != tolower(suffix[i])) return 0;
    }
    return 1;
}

/**
 * @brief Loads points from a CSV or OBJ file (format: x,y[,z] per line for CSV; v x y z for OBJ).
 * @param filename Path to the input file.
 * @return Pointer to PointSet on success, NULL on failure.
 */
PointSet* load_points(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return NULL;
    }

    int is_obj = ends_with(filename, ".obj");

    PointSet* set = malloc(sizeof(PointSet));
    if (!set) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    set->points = malloc(INITIAL_CAPACITY * sizeof(Point));
    if (!set->points) {
        free(set);
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    set->count = 0;
    set->is_3d = 0;  // Assume 2D initially
    size_t capacity = INITIAL_CAPACITY;

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        Point p = {0.0f, 0.0f, 0.0f};
        int fields;
        if (is_obj) {
            // OBJ: skip non-"v" lines
            if (buffer[0] != 'v' || buffer[1] != ' ') continue;
            fields = sscanf(buffer + 2, "%f %f %f", &p.x, &p.y, &p.z);  // Parse after "v "
        } else {
            // CSV
            fields = sscanf(buffer, "%f,%f,%f", &p.x, &p.y, &p.z);
        }
        if (fields < 2) {
            // Invalid line: skip
            continue;
        }
        if (fields >= 3 && p.z != 0.0f) {
            set->is_3d = 1;
        }

        // Resize if needed
        if (set->count >= capacity) {
            capacity *= 2;
            Point* temp = realloc(set->points, capacity * sizeof(Point));
            if (!temp) {
                free_points(set);
                fclose(file);
                fprintf(stderr, "Memory reallocation failed\n");
                return NULL;
            }
            set->points = temp;
        }

        set->points[set->count++] = p;
    }

    fclose(file);
    // Shrink to fit
    if (set->count < capacity) {
        Point* temp = realloc(set->points, set->count * sizeof(Point));
        if (temp) set->points = temp;
    }
    return set;
}

/**
 * @brief Saves points to a CSV file (format: x,y[,z] per line).
 * @param set The PointSet to save.
 * @param filename Path to the output CSV file.
 * @return 0 on success, -1 on failure.
 */
int save_points(const PointSet* set, const char* filename) {
    if (!set || set->count == 0) {
        fprintf(stderr, "Invalid PointSet for saving\n");
        return -1;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening file '%s' for writing: %s\n", filename, strerror(errno));
        return -1;
    }

    for (size_t i = 0; i < set->count; ++i) {
        const Point* p = &set->points[i];
        if (set->is_3d) {
            fprintf(file, "%.2f,%.2f,%.2f\n", p->x, p->y, p->z);  // 2 decimal places
        } else {
            fprintf(file, "%.2f,%.2f\n", p->x, p->y);
        }
    }

    fclose(file);
    return 0;
}

/**
 * @brief Frees memory allocated for a PointSet.
 * @param set The PointSet to free.
 */
void free_points(PointSet* set) {
    if (set) {
        free(set->points);
        free(set);
    }
}