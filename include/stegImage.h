#ifndef STEGIMAGE_H
#define STEGIMAGE_H

#include "png.h"
#include <iostream>


class stegImage
{
private:
    png_byte color_type;        //Color type
    png_byte bit_depth;         //bit depth (Bits per channel)
    png_structp png_ptr;        //A read struct to keep track of the file we will be loading
    png_infop info_ptr;         //A info struct to keep track of the file we are loading/attributes
    int width, height;          //Height and Width of the image
    int channels;               //Number of channels (BGRA)
    int number_of_passes;      
    png_bytep * row_pointers;   //Point for the image
public:

    int writePNG(std::string *filename);                                //Write PNG file (Encode data buffer into PNG file)
    int readPNG(std::string *filename);       //Read PNG file (Decode it, write command to it)
    int encodeImage(std::string commandString); //Encode image with command
    int decodeImage(std::string *output);       //output to grab from client
    int cleanup();                              //Cleanup operation
};


#endif