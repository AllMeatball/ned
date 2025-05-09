#include "ne.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

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


    printf("RES TABLE OFS: 0x%04x\n", exe->header.ResTableOffset);

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
    exe->rsrc.Types = NULL;

    NE_ResType res_type = {0};

    // use 0xCAFE because we don't want it prematurely exit
    res_type.TypeID = 0xCAFE;

    while (1) {
        res_type.NameInfo = NULL;
        if (fread(&res_type.TypeID, sizeof(res_type.TypeID), 1, fp) < 1) {
            exe->error = "Failed to read type id (likely got to EOF)";
            return -1;
        }

        if (res_type.TypeID == rt_terminator) {
            // printf("terminator found. breaking...\n");
            break;
        }

        if (fread(&res_type.metadata, sizeof(res_type.metadata), 1, fp) < 1) {
            exe->error = "Failed to read metadata for type";
            return -1;
        }

        // struct NE_ResNameInfo nameinfo;
        // for (uint16_t i = 0; i < res_type.metadata.ResourceCount; i++) {
        //     size_t result = fread(&nameinfo, sizeof(nameinfo), 1, fp);
        //     // printf("%u: %zu\n", i, result);
        //     if (result < 1) {
        //         exe->error = "Failed to read nameinfo array";
        //         arrfree(res_type.NameInfo);
        //         return -1;
        //     }
        //     arrput(res_type.NameInfo, nameinfo);
        // }
        // printf("%zu\n", arrlenu(exe->rsrc.Types));
        // fseek(fp, sizeof(struct NE_ResNameInfo) * res_type.metadata.ResourceCount, SEEK_CUR);

        arrput(exe->rsrc.Types, res_type);
    }

    // ArrayUtil_create(exe->rsrc.Names);

    // for (size_t i = 0; i < exe->rsrc.Names.len; i++) {
    //     struct NE_ResNameInfo *nameinfo_ptr = res_type.metadata.NameInfo;
    //     res_type.metadata.NameInfo = malloc(sizeof(struct NE_ResNameInfo));
    //     if (!res_type.metadata.NameInfo) {
    //         exe->error = "Failed to alloc nameinfo buffer";
    //         return -1;
    //     }
    //
    //     // seek to nameinfo
    //     if (fseek(fp, (size_t)nameinfo_ptr, SEEK_SET) != 0) {
    //         exe->error = "Failed to seek to nameinfo ptr";
    //         return -1;
    //     }
    //
    //     if (fread(res_type.metadata.NameInfo, sizeof(struct NE_ResNameInfo), 1, fp) < 1) {
    //         exe->error = "Failed to read nameinfo";
    //         return -1;
    //     }
    // }

    return 0;
}

int NE_readFile(FILE *fp, struct NE_exe *exe) {
    int ret = 0;

    ret |= NE_readHeader(fp, exe);
    ret |= NE_readRsrcTable(fp, exe);
    return ret;
}

void NE_freeExe(struct NE_exe *exe) {
    NE_ResType res_type = {0};

    if (exe->rsrc.Types) {
    for (size_t i = 0; i < arrlenu(exe->rsrc.Types); i++) {
        res_type = exe->rsrc.Types[i];
        arrfree(res_type.NameInfo);
    }
    }

    arrfree(exe->rsrc.Types);
    arrfree(exe->rsrc.Names);
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

const char *NE_detectRsrcID(enum restype rt) {
    switch (rt) {
        case rt_cursor:  return "Cursor";
        case rt_bitmap:  return "Bitmap";
        case rt_icon:    return "Icon";
        case rt_menu:    return "Menu";
        case rt_dialog:  return "Dialog Box";
        case rt_string:  return "String Table";
        case rt_fontdir: return "Font Component";
        case rt_font:    return "Font Directory";

        case rt_accelerator: return "Accelerator table";
        case rt_rcdata:      return "Resource data";

        case rt_unknown:
        default: return "Unknown";
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

    printf("Resources:\n");

    NE_ResType res_type = {0};
    struct NE_ResNameInfo nameinfo = {0};
    for (int i = 0; i < arrlenu(exe.rsrc.Types); i++) {
        res_type = exe.rsrc.Types[i];
        printf("Type ID: %s [#%u]\n", NE_detectRsrcID(res_type.TypeID), res_type.TypeID);

        // nameinfo = *res_type.metadata.NameInfo;
        //
        // printf("ID: %s [#%u]", NE_detectRsrcID(nameinfo.ID), nameinfo.ID);
    }
}

