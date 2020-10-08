#ifndef STEGIMAGE_H
#define STEGIMAGE_H

#include "png.h"
#include <iostream>


class stegImage
{
private:
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int width, height;
    int channels;
    int number_of_passes;
    png_bytep * row_pointers;
public:

    int writePNG(std::string *filename);
    int readPNG(std::string *filename,std::string commandString);   
};


#endif