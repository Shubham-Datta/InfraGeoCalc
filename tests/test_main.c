#include <stdio.h>  // For printf

// Forward declarations from test_geometry.c
extern void run_all_tests();
extern int get_tests_run();
extern int get_tests_failed();

int main() {
    printf("Running unit tests...\n");
    run_all_tests();

    int run = get_tests_run();
    int failed = get_tests_failed();
    printf("\nTests run: %d\n", run);
    printf("Tests failed: %d\n", failed);

    if (failed == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}