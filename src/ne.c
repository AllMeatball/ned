#include "ne.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arrayutil.h"

#define NE_PTR_OFFSET 0x3c

int NE_readHeader(FILE *fp, struct NE_exe *exe) {
    char mz_header[2];
    exe->ready = 0;
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

    if (fread(&ne_ptr, 2, 1, fp) < 1) {
        exe->error = "Failed to read NE pointer offset";
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

    if (memcmp("NE", exe->header.sig, 2) != 0) {
        exe->error = "Not a New Executable formatted exe file";
        return -1;
    }

    exe->ready = 1;
    exe->error = "Success";
    return 0;
}

int NE_readRsrcTable(FILE *fp, struct NE_exe *exe) {
    if (!exe->ready) {
        exe->error = "Exe struct isn't setup/ready yet";
        return -1;
    }

    // seek to resource table
    if (fseek(fp, exe->header.ResTableOffset, SEEK_SET) != 0) {
        exe->error = "Failed to seek to resource table";
        return -1;
    }

    // read alignment shift value
    if (fread(
        &exe->rsrc.AlignmentShift,
        sizeof(exe->rsrc.AlignmentShift),
        1, fp) < 1
    ) {
        exe->error = "Failed to read alignment shift";
        return -1;
    }

    // read list of types
    ArrayUtil_create(exe->rsrc.Types);

    NE_ResType res_type = {0};

    // use 0xCAFE because we don't want it prematurely exit
    res_type.TypeID = 0xCAFE;

    while (res_type.TypeID != rt_terminator) {
        if (fread(&res_type.TypeID, sizeof(res_type.TypeID), 1, fp) < 1) {
            exe->error = "Failed to read type id (likely got to EOF)";
            return -1;
        }

        if (res_type.TypeID == rt_terminator) {
            break;
        }

        if (fread(&res_type.metadata, sizeof(res_type.metadata), 1, fp) < 1) {
            exe->error = "Failed to read metadata for type";
            return -1;
        }

        ArrayUtil_add(exe->rsrc.Types, res_type);
    }

    // ArrayUtil_create(exe->rsrc.Names);
    return 0;
}

int NE_readFile(FILE *fp, struct NE_exe *exe) {
    int ret = 0;

    ret |= NE_readHeader(fp, exe);
    ret |= NE_readRsrcTable(fp, exe);
    return ret;
}

void NE_freeExe(struct NE_exe *exe) {
    ArrayUtil_free(exe->rsrc.Names);
    ArrayUtil_free(exe->rsrc.Types);
}

const char *NE_detectOS(enum targetos os) {
    switch (os) {
        case os_os2:
            return "OS/2";
        case os_win:
            return "Windows (16-bit)";
        case os_dos4:
            return "MS-DOS 4.0 (Europe)";
        case os_win386:
            return "Windows (32-bit)";
        case os_BOSS:
            return "Borland Operating System Services";
        case os_unknown:
        default:
            return "Unknown";
    }
    fprintf(stderr, "somehow the program failed and went here. ask the developer about %s:%d\n", __func__, __LINE__);
    abort();
}

void NE_printInfo(struct NE_exe exe) {
    if (!exe.ready) { return; }

    printf(
        "Linker version: %u.%u\n",
        exe.header.MajLinkerVersion,
        exe.header.MinLinkerVersion
    );

    printf(
        "Target OS: %s [#%u]\n",
        NE_detectOS(exe.header.targOS),
        exe.header.targOS
    );
}

