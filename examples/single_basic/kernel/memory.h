
#pragma once

//
// These define basic x86_64 specific structures
// and macros for accessing page tables when using
// recursive mappings, most are taken from Carbon. 
// Refer to the intel manual for more information.
//

typedef union _MMPTE_HARDWARE {
    struct {
        unsigned long long Present : 1;
        unsigned long long Write : 1;
        unsigned long long User : 1;
        unsigned long long WriteThrough : 1;
        unsigned long long CacheDisable : 1;
        unsigned long long Accessed : 1;
        unsigned long long Dirty : 1;
        unsigned long long Large : 1;
        unsigned long long Global : 1;
        unsigned long long Available0 : 3;
        unsigned long long PageFrame : 36;
        unsigned long long Reserved1 : 4;
        unsigned long long Available1 : 7;
        unsigned long long ProtectionKey : 4;
        unsigned long long ExecuteDisable : 1;
    } Table;

    struct {
        unsigned long long Present : 1;
        unsigned long long Write : 1;
        unsigned long long User : 1;
        unsigned long long WriteThrough : 1;
        unsigned long long CacheDisable : 1;
        unsigned long long Accessed : 1;
        unsigned long long Dirty : 1;
        unsigned long long PageAttribute : 1;
        unsigned long long Global : 1;
        unsigned long long Available0 : 3;
        unsigned long long PageFrame : 36;
        unsigned long long Reserved1 : 4;
        unsigned long long Available1 : 7;
        unsigned long long ProtectionKey : 4;
        unsigned long long ExecuteDisable : 1;
    } Entry;

    unsigned long long     Long;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

C_ASSERT( sizeof( MMPTE_HARDWARE ) == 8 );

#define MI_RECURSIVE_TABLE 0xFFFFFF0000000000ULL
#define MI_RECURSIVE_INDEX 0x1FEULL

#define MiReferenceLevel4Entry( index4 ) \
( ( PMMPTE_HARDWARE )( MI_RECURSIVE_TABLE | \
( MI_RECURSIVE_INDEX << 30ULL ) | \
( MI_RECURSIVE_INDEX << 21ULL ) | \
( ( ( unsigned long long ) ( index4 ) & 0x1FFULL ) << 12ULL) ) )

#define MiReferenceLevel3Entry( index4, index3 ) \
( ( PMMPTE_HARDWARE )( MI_RECURSIVE_TABLE | \
( MI_RECURSIVE_INDEX << 30ULL ) | \
( ( ( unsigned long long )( index4 ) & 0x1FFULL ) << 21ULL) | \
( ( ( unsigned long long )( index3 ) & 0x1FFULL ) << 12ULL) ) )

#define MiReferenceLevel2Entry( index4, index3, index2 ) \
( ( PMMPTE_HARDWARE )( MI_RECURSIVE_TABLE | \
( ( ( unsigned long long )( index4 ) & 0x1FFULL ) << 30ULL ) | \
( ( ( unsigned long long )( index3 ) & 0x1FFULL ) << 21ULL ) | \
( ( ( unsigned long long )( index2 ) & 0x1FFULL ) << 12ULL ) ) )

#define MiIndexLevel4( address )                ( ( ( unsigned long long ) ( address ) & ( 0x1FFULL << 39ULL ) ) >> 39ULL )
#define MiIndexLevel3( address )                ( ( ( unsigned long long ) ( address ) & ( 0x1FFULL << 30ULL ) ) >> 30ULL )
#define MiIndexLevel2( address )                ( ( ( unsigned long long ) ( address ) & ( 0x1FFULL << 21ULL ) ) >> 21ULL )
#define MiIndexLevel1( address )                ( ( ( unsigned long long ) ( address ) & ( 0x1FFULL << 12ULL ) ) >> 12ULL )

#define MiConstructAddress( index4, index3, index2, index1 ) \
( ( void* )( ( ( unsigned long long )( index4 ) << 39ULL ) |\
( ( unsigned long long )( index3 ) << 30ULL ) |\
( ( unsigned long long )( index2 ) << 21ULL ) |\
( ( unsigned long long )( index1 ) << 12ULL ) |\
( ( ( unsigned long long )( index4 ) / 256 ) * 0xFFFF000000000000 ) ) )
