# carboot
This project is a small x86_64 bootloader capable of reading FAT32 and mapping PE32+ images. 
It is the bootloader for carbon and is designed to do the bare minimum required for loading a kernel, 
it will simply load your kernel from inside the `/SYSTEM/` directory, and load all files under `/SYSTEM/BOOT/`,
it can deal with imports too, as long as the imported module is placed inside `/SYSTEM/BOOT/` and will relocate
your kernel and any dependencies into the upper 2MB of the 64 bit address space (`0xFFFFFFFFFFE00000`), all bootloader 
structures are placed in the 2MB of memory starting at `0xFFFFFFFFFFC00000`, just before your kernel. The project is still
very under-developed but hopefully that will change over time.

## `LOADER_BLOCK`
This structure is passed to your kernel's entry point and contains all information about the environment.
```c
typedef struct _LOADER_BLOCK {
    ULONG32               LoaderSig;
    ULONG32               RootSerial;

    ULONG32               RegionCount;
    ULONG32               FileCount;
    ULONG32               MapCount;

    LOADER_LOGICAL_REGION RegionList[ LOADER_REGION_LIST_LENGTH ];
    LOADER_BOOT_FILE      FileList[ LOADER_FILE_LIST_LENGTH ];
    LOADER_SYSTEM_MAP     MapList[ LOADER_MAP_LIST_LENGTH ];
    LOADER_BOOT_GRAPHICS  Graphics;
} LOADER_BLOCK, *PLOADER_BLOCK;
```
Your kernel entry point should look something like this:
```c
void
KiSystemStartup(
    PLOADER_BLOCK Loader
);
```
![Example](https://github.com/irql0/carboot/blob/master/ss/1.png)
All loader structures are inside [ldef.h](/carbload/inc/ldef.h).

## Memory map
```
1: 0x0000000000000000-0x0000000000200000 identity mapped, contains the bootloader module & some structures.
2: 0x0000000000200000-0x0000000000400000 2MB usable, for parts of the LOADER_BLOCK structure and system modules.
3: 0xFFFFFFFFFFC00000-0xFFFFFFFFFFE00000 maps to 1.
4: 0xFFFFFFFFFFE00000-0xFFFFFFFFFFFFFFFF maps to 2.
```

## Examples
###### Single-module example which prints Hello World can be found [here](/examples/single_basic).
###### Multi-module example which prints Hello World, and imports functions from an external boot dependency can be found [here](/examples/multi_basic).

## Usage
Extract boot.zip to boot.vhd, open an administrator command prompt in the root directory, type `build` and
the script will use diskpart to read/write to the FAT32 VHD. It will copy `CARBLOAD.SYS`, `KERNEL.SYS` and `HAL.SYS` from the multi-module example onto the VHD 
and then detach the VHD in-order to run it with QEMU.  

## Build
This project was written using Visual Studio 2017, compile as `build` `x86_64`.
