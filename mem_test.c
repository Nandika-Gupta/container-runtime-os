#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Memory test started...\n");

    while (1) {
        void *p = malloc(10 * 1024 * 1024); // allocate 10MB

        if (p == NULL) {
            printf("Memory allocation failed\n");
            break;
        }

        printf("Allocated 10MB\n");
        sleep(1);
    }

    return 0;
}
