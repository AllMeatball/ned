#include "ne.h"
#include <stdio.h>
#include <string.h>

#define NE_PTR_OFFSET 0x3c

int NE_readFile(FILE *fp, struct NE_exe *exe) {
    char mz_header[2];
    exe->error = "Unknown";

    if (!fp) {
        exe->error = "File pointer is NULL";
        return -1;
    }

    fread(mz_header, 2, 1, fp);

    if (memcmp("MZ", mz_header, 2) != 0) {
        exe->error = "Not an EXE file. (must have MZ Header)";
        return -1;
    }

    // get pointer for NE header
    uint16_t ne_ptr = 0;
    if (fseek(fp, NE_PTR_OFFSET, SEEK_SET) != 0) {
        exe->error = "Failed to seek to NE pointer offset";
        return -1;
    }

    if (fread(&ne_ptr, 4, 1, fp) < 1) {
        exe->error = "Failed to read NE pointer offset";
        return -1;
    }

    if (fseek(fp, NE_PTR_OFFSET, SEEK_SET) != 0) {
        exe->error = "Failed to seek to NE pointer offset";
        return -1;
    }

    // now get the NE header
    if (fseek(fp, ne_ptr, SEEK_SET) != 0) {
        exe->error = "Failed to seek to NE header";
        return -1;
    }

    if (fread(&exe->header, sizeof(struct NE_header), 1, fp) < 1)  {
        exe->error = "Failed to read NE header";
        return -1;
    }

    exe->error = "Success";
    return 0;
}

void NE_printInfo(struct NE_exe exe) {
    printf(
        "Linker version: %u.%u\n",
        exe.header.MajLinkerVersion,
        exe.header.MinLinkerVersion
    );
}

