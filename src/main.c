#pragma comment( lib, "avcodec.lib" )
#pragma comment( lib, "avformat.lib" )
#pragma comment( lib, "avutil.lib" )

#include "vblender/vblender.h"

int main(int argc, char** args)
{
    return vblend(args +1, argc -1);
}