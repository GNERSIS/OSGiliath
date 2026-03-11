#pragma once

#include <stdio.h>

extern "C"
{
#include "jerror.h"

#include <jpeglib.h>
}

#define EXIF_JPEG_MARKER JPEG_APP0 + 1

extern int
EXIF_Orientation( j_decompress_ptr cinfo );
