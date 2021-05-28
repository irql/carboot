
#include "loader.h"

VOID
BPAPI
BdInitGraphics(

)
{
    X86_BIOS_INTERRUPT   BiosFrame = { 0 };
    X86_VESA_INFO        VesaInfo;
    X86_VESA_MODE_INFO   ModeInfo;
    USHORT*              ModeList;

    BiosFrame.Eax = 0x4F00;
    X86_BIOS_ENCODE_FAR_PTR( BiosFrame.SegEs,
                             BiosFrame.Edi,
                             &VesaInfo );
    x86BiosCall( 0x10, &BiosFrame );

    X86_BIOS_DECODE_FAR_PTR( VesaInfo.ModeList.Segment,
                             VesaInfo.ModeList.Offset,
                             ModeList );

    while ( *ModeList != 0xFFFF ) {

        BiosFrame.Eax = 0x4F01;
        BiosFrame.Ecx = *ModeList;
        X86_BIOS_ENCODE_FAR_PTR( BiosFrame.SegEs,
                                 BiosFrame.Edi,
                                 &ModeInfo );
        x86BiosCall( 0x10, &BiosFrame );

        if ( ModeInfo.Width == 640 &&
             ModeInfo.Height == 480 &&
             ModeInfo.BitsPerPixel == 32 ) {

            BiosFrame.Eax = 0x4F02;
            BiosFrame.Ebx = *ModeList;
            x86BiosCall( 0x10, &BiosFrame );

            Loader.Graphics.Width = 640;
            Loader.Graphics.Height = 480;
            Loader.Graphics.Bpp = 32;

            Loader.Graphics.Pitch = ModeInfo.Pitch;
            Loader.Graphics.Frame = ModeInfo.Framebuffer;
            break;
        }

        ModeList++;
    }
}
