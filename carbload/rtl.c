
#include "loader.h"

VOID
RtlFillMemory(
    _In_ PVOID   Destination,
    _In_ ULONG32 Fill,
    _In_ ULONG32 Length
)
{
    __stosb( Destination, Fill, Length );
}

VOID
RtlCopyMemory(
    _In_ PVOID   Destination,
    _In_ PVOID   Source,
    _In_ ULONG32 Length
)
{
    __movsb( Destination, Source, Length );
}

LONG32
RtlCompareAnsiString(
    _In_ PSTR  String1,
    _In_ PSTR  String2,
    _In_ UCHAR CaseInsensitive
)
{
    if ( CaseInsensitive ) {
        while ( *String1 && *String2 && RtlUpperChar( *String1 ) == RtlUpperChar( *String2 ) )
            String1++, String2++;
        return RtlUpperChar( *String1 ) - RtlUpperChar( *String2 );
    }
    else {
        while ( *String1 && *String2 && *String1 == *String2 )
            String1++, String2++;
        return *String1 - *String2;
    }
}

ULONG32
RtlLengthAnsiString(
    _In_ PSTR Buffer
)
{
    ULONG32 Index = 0;

    while ( Buffer[ Index ] )
        Index++;

    return Index;
}

VOID
RtlReverseAnsiString(
    _In_ PSTR Buffer
)
{
    ULONG32 Char_1, Char_2;
    CHAR    Intermediate;

    Char_1 = 0;
    Char_2 = RtlLengthAnsiString( Buffer ) - 1;

    for ( ; Char_1 < Char_2; Char_1++, Char_2-- ) {

        Intermediate = Buffer[ Char_1 ];
        Buffer[ Char_1 ] = Buffer[ Char_2 ];
        Buffer[ Char_2 ] = Intermediate;
    }
}

VOID
RtlIntToAnsiString(
    _In_ LONG32  Long,
    _In_ ULONG32 Base,
    _In_ PSTR    Buffer
)
{
    UCHAR   Temp;
    ULONG32 Index;
    ULONG32 Neg;

    Index = 0;
    Neg = 0;
    if ( Long == 0 ) {
        Buffer[ Index++ ] = '0';
        Buffer[ Index ] = 0;
        return;
    }

    if ( Base == 10 && Long < 0 ) {
        Neg = 1;
        Long = -Long;
    }

    while ( Long != 0 ) {
        Temp = Long % Base;
        Buffer[ Index++ ] = Temp > 9 ? Temp - 10 + 'a' : Temp + '0';
        Long /= Base;
    }

    if ( Neg ) {

        Buffer[ Index++ ] = '-';
    }
    Buffer[ Index ] = 0;

    RtlReverseAnsiString( Buffer );
}

VOID
BPAPI
RtlFormatAnsiStringFromList(
    _In_ PSTR    Buffer,
    _In_ PSTR    Format,
    _In_ va_list List
)
{
    ULONG32 Index;
    ULONG32 Base;
    ULONG32 Lower;
    ULONG32 Upper;
    ULONG32 Prepad;
    ULONG32 Current;
    CHAR    LowerBuffer[ 16 ];
    CHAR    UpperBuffer[ 16 ];
    PSTR    String;

    Index = 0;
    while ( *Format ) {
        if ( *Format != '%' ) {

            Buffer[ Index++ ] = *Format++;
            continue;
        }
        else {

            Format++;
        }

        switch ( *Format ) {
        case '%':
            Buffer[ Index++ ] = *Format++;
            break;
        case 'l':
            Format++;

            Base = 10;

            if ( *Format == 'l' ) {
                Format++;

                if ( *Format == 'x' ) {

                    Base = 16;
                    Format++;
                }

                Lower = __crt_va_arg( List, ULONG32 );
                Upper = __crt_va_arg( List, ULONG32 );

                RtlIntToAnsiString( Lower, Base, LowerBuffer );
                RtlIntToAnsiString( Upper, Base, UpperBuffer );
                Prepad = 8 - RtlLengthAnsiString( LowerBuffer );

                for ( Current = 0; Current < Prepad; Current++, Index++ ) {
                    Buffer[ Index ] = '0';
                }

                for ( Current = 0; UpperBuffer[ Current ]; Current++, Index++ ) {
                    Buffer[ Index ] = UpperBuffer[ Current ];
                }

                for ( Current = 0; LowerBuffer[ Current ]; Current++, Index++ ) {
                    Buffer[ Index ] = LowerBuffer[ Current ];
                }
            }
            else {

                if ( *Format == 'x' ) {

                    Base = 16;
                    Format++;
                }

                Lower = __crt_va_arg( List, ULONG32 );

                RtlIntToAnsiString( Lower, Base, LowerBuffer );

                for ( Current = 0; LowerBuffer[ Current ]; Current++, Index++ ) {
                    Buffer[ Index ] = LowerBuffer[ Current ];
                }
            }
            break;
        case 's':
            Format++;
            String = __crt_va_arg( List, PSTR );

            for ( Current = 0; String[ Current ]; Current++, Index++ ) {
                Buffer[ Index ] = String[ Current ];
            }
            break;
        default:
            break;
        }
    }

    Buffer[ Index ] = 0;
}

VOID
RtlFormatAnsiString(
    _In_ PSTR Buffer,
    _In_ PSTR Format,
    _In_ ...
)
{
    va_list List;
    __crt_va_start( List, Format );
    RtlFormatAnsiStringFromList( Buffer,
                                 Format,
                                 List );
    __crt_va_end( List );
}
