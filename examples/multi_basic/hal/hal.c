
#pragma once

//
// Import KiDisplayString from initos.c, kernel.sys.
//

__declspec( dllimport )
void
KiDisplayString(
    char* String
);

__declspec( dllexport )
void
HalExtExample(

)
{
    //
    // This creates a cyclic dependency, build as a 
    // static library before-hand
    //

    KiDisplayString( "Hello from hal.sys!\n" );
}
