#include "ne.h"
#include <stdio.h>

int main(int argc, char **argv) {
    struct NE_exe exe = {0};
    if (argc < 2) {
        fprintf(stderr, "ned [exe file]\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Failed open exe file");
        return 1;
    }

    if (NE_readFile(fp, &exe) < 0) {
        fprintf(
            stderr,
            "ned: Failed to read file: %s\n",
            exe.error
        );

        goto exit_prog;
    }

    NE_printInfo(exe);
exit_prog:
    NE_freeExe(&exe);

    if (fp)
        fclose(fp);

    return 0;
}
