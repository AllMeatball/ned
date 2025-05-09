/*
 * Copyright (c) 2025 AllMeatball
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

// Code sourced mostly from https://wiki.osdev.org/NE
#pragma once
#include <stdio.h>
#include <stdint.h>

//
// In 16-bit DOS/Windows terminology, DGROUP is a segment class that referring
// to segments that are used for data.
//
// Win16 used segmentation to permit a DLL or program to have multiple
// instances along with an instance handle and manage multiple data
// segments. This allowed one NOTEPAD.EXE code segment to execute
// multiple instances of the notepad application.
//
enum FlagWord {
    NOAUTODATA,     //
    SINGLEDATA,
    MULTIPLEDATA,
    LINKERROR = 0x2000, // Linker error, module cannot lode
    LIBMODULE = 0x8000
    // The file is a DLL/Library. NE_header.InitStack
    // is therefore invalid (DLLs are libraries) so they
    // do not get their own stacks. CS:IP points to
    // the load procedure.
};

// The type is a 3-bit integer or'ed together with the other flags.
#define SEGFLAGS_HAS_RELOCS 0x0100
#define SEGFLAGS_DISCARD    0xF000
#define SEGFLAGS_TYPE_CODE  0
#define SEGFLAGS_TYPE_DATA  1

struct NE_header {
    char sig[2];                 // {'N', 'E'}
    uint8_t MajLinkerVersion;    //The major linker version
    uint8_t MinLinkerVersion;    //The minor linker version
    uint16_t EntryTableOffset;   //Offset of entry table, see below
    uint16_t EntryTableLength;   //Length of entry table in bytes
    uint32_t FileLoadCRC;        //32-bit CRC of entire contents of file
    uint8_t FlagWord;            // Uses the FlagWord enum
    uint16_t AutoDataSegIndex;   //The automatic data segment index
    uint16_t InitHeapSize;       //The initial local heap size
    uint16_t InitStackSize;      //The initial stack size
    uint32_t EntryPoint;         //CS:IP entry point, CS is index into segment table
    uint32_t InitStack;          //SS:SP initial stack pointer, SS is index into segment table
    uint16_t SegCount;           //Number of segments in segment table
    uint16_t ModRefs;            //Number of module references (DLLs)
    uint16_t NoResNamesTabSiz;   //Size of non-resident names table, in bytes (Please clarify non-resident names table)
    uint16_t SegTableOffset;     //Offset of Segment table
    uint16_t ResTableOffset;     //Offset of resources table
    uint16_t ResidNamTable;      //Offset of resident names table
    uint16_t ModRefTable;        //Offset of module reference table
    uint16_t ImportNameTable;    //Offset of imported names table (array of counted strings, terminated with string of length 00h)
    uint32_t OffStartNonResTab;  //Offset from start of file to non-resident names table
    uint16_t MovEntryCount;      //Count of moveable entry point listed in entry table
    uint16_t FileAlnSzShftCnt;   //File alignment size shift count (0=9(default 512 byte pages))
    uint16_t nResTabEntries;     //Number of resource table entries
    uint8_t targOS;              //Target OS

    //
    // The rest of these are not defined in the Windows 3.0 standard and
    // appear to be specific to OS/2.

    uint8_t OS2EXEFlags;         //Other OS/2 flags
    uint16_t retThunkOffset;     //Offset to return thunks or start of gangload area - what is gangload?
    uint16_t segrefthunksoff;    //Offset to segment reference thunks or size of gangload area
    uint16_t mincodeswap;        //Minimum code swap area size
    uint8_t expctwinver[2];      //Expected windows version (minor first)
};

#pragma pack(push,1)

// Segment table entry
typedef struct {
    uint16_t    SectorBase; // See NE_header.FileAlnSzShftCnt
    uint16_t    SegBytes;   // Segment bytes located in the file
    uint16_t    SegFlags;
    uint16_t    MinAlloc;   // Minimum number of bytes to allocate.
}NE_SegEnt;

// Resource name info
struct NE_ResNameInfo {
    uint16_t Offset;
    uint16_t Length;
    uint16_t Flags;
    uint16_t ID;
    uint16_t Handle;
    uint16_t Usage;
};

// Resource table entry
typedef struct {
    uint16_t TypeID;

    struct {
    uint16_t ResourceCount;
    uint32_t Reserved;
    } metadata;

    struct NE_ResNameInfo *NameInfo;
}NE_ResType;


// Resource table
// typedef struct {
//     size_t      len;
//     NE_ResType  *items;
// }NE_ResTypeArr;
//
// // Resource table
// typedef struct {
//     size_t len;
//     char   **items;
// }NE_ResNames;

// Resource table
struct NE_ResTable {
    uint16_t    AlignmentShift; // Alignment shift count for resource data.
    NE_ResType *Types;
    char **Names;
};

// Custom structs for NEd

struct NE_exe {
    int ready;
    const char *error;
    struct NE_header header;
    struct NE_ResTable rsrc;
};
#pragma pack(pop)

#define GLOBINIT 1<<2     //global initialization
#define PMODEONLY 1<<3    //Protected mode only
#define INSTRUC86 1<<4    //8086 instructions
#define INSTRU286 1<<5    //80286 instructions
#define INSTRU386 1<<6    //80386 instructions
#define INSTRUx87 1<<7    //80x87 (FPU) instructions

//Application flags
//Application type
enum apptype {
    none,
    fullscreeen,    //fullscreen (not aware of Windows/P.M. API)
    winpmcompat,    //compatible with Windows/P.M. API
    winpmuses       //uses Windows/P.M. API
};

// #define OS2APP 1<<3    //OS/2 family application
// //bit 4 reserved?
// #define IMAGEERROR 1<<5    //errors in image/executable
// #define NONCONFORM 1<<6    //non-conforming program?
// #define DLL        1<<7

//Target Operating System
enum targetos {
    os_unknown, //Obvious ;)
    os_os2,     //OS/2 (as if you hadn't worked that out!)
    os_win,     //Windows (Win16)
    os_dos4,    //European DOS  4.x
    os_win386,  //Windows for the 80386 (Win32s). 32 bit code.
    os_BOSS     //The boss, a.k.a Borland Operating System Services
};

//Resource Types
enum restype {
    rt_unknown = -1,
    rt_terminator,  //Marks the end of a TYPEINFO list
    rt_cursor,      //Cursor
    rt_bitmap,      //Bitmap
    rt_icon,        //Icon
    rt_menu,        //Menu
    rt_dialog,      //Dialog box
    rt_string,      //String table
    rt_fontdir,     //Font component
    rt_font,        //Font directory
    rt_accelerator, //Accelerator table
    rt_rcdata       //Resource data
};

//Other OS/2 flags
#define LFN 1        //OS/2 Long File Names (finally, no more 8.3 conversion :) )
#define PMODE 1<<1   //OS/2 2.x Protected Mode executable
#define PFONT 1<<2   //OS/2 2.x Proportional Fonts
#define GANGL 1<<3   //OS/2 Gangload area

int NE_readFile(FILE *fp, struct NE_exe *exe);
void NE_printInfo(struct NE_exe exe);
void NE_freeExe(struct NE_exe *exe);
