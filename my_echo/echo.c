#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("\n");
        return 0;
    }

    int equal = !strcmp(argv[1], "-n");

    for (int i = 1 + equal; i < argc; i++) {
        printf("%s", argv[i]);
        if (i != argc - 1)
            printf(" ");
    }

    if (!equal)
        printf("\n");

    return 0;
}