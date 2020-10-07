#ifndef SERVER_H
#include  "server.h"
#include "png.h"
//#include <jpeglib.h>
#include <iostream>

#endif

//int readJPEGFile(char *filename);
//int writeJPEG(char *filename);
int writePNG(char *filename);
int readPNG(char *filename);
int width, height;
png_byte color_type;
png_byte bit_depth;
png_structp png_ptr;
png_infop info_ptr;
int channels;
int number_of_passes;
png_bytep * row_pointers;

unsigned char* buffer;
int main(int argc, char const *argv[]){
    
    readPNG("images/testing.png");
    writePNG("images/testing.png");
    Server socketServer(100,"10.0.0.34","8081");
    socketServer.perform();
    printf("hey");
}

int readPNG(char *fileName){
    FILE* file = fopen(fileName,"rb");
    if(file == NULL){
        exit(0);
    }
    
    u_char header[8];
    fread(header,1,8,file);
    if(png_sig_cmp(header,0,8)){
        return 1;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
    if(!png_ptr){
        return 4;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr){
        png_destroy_read_struct(&png_ptr,NULL,NULL);
        return 4;
    }

    if(setjmp(png_jmpbuf(png_ptr))){
        png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
        return 2;
    }
    png_init_io(png_ptr,file);
    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr,info_ptr);

    width = png_get_image_width(png_ptr,info_ptr);
    height = png_get_image_height(png_ptr,info_ptr);
    color_type = png_get_color_type(png_ptr,info_ptr);
    channels = png_get_channels(png_ptr,info_ptr);
    if(color_type != PNG_COLOR_TYPE_RGBA){
        return -1;
    }
    bit_depth = png_get_bit_depth(png_ptr,info_ptr);

    png_read_update_info(png_ptr,info_ptr);
    if(setjmp(png_jmpbuf(png_ptr))){
        return 2;
    }
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep)*height);
    

    for(int i =0; i < height; i++){
        row_pointers[i] =(png_byte *) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }
    png_read_image(png_ptr, row_pointers);
    int r,g,b,a;
    char command[4] = {'t','e','s','t'};
    int comCount = 0;
    for(int y = 0; y < height; y++){
        png_byte* row = row_pointers[y];
        for(int x = 0; x < width; x++){
            png_byte* ptr = &(row[x*4]);
            r = ptr[0];
            g = ptr[1];
            b = ptr[2];
            a = ptr[3];
            if(comCount < 4){
                ptr[0] = command[comCount];
                comCount++;
            }
        }
    }
    fclose(file);
    return 0;

    
}
int writePNG(char* filename){
    FILE *outfile = fopen(filename, "wb");
    if(!outfile){
        return -1;
    }
    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr){
        return -1;
    }
            

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr){
        return -1;
    }

    if(setjmp(png_jmpbuf(png_ptr))){
        return 2;
    }

    png_init_io(png_ptr, outfile);


    /* write header */
    if(setjmp(png_jmpbuf(png_ptr))){
        return 2;
    }
    png_set_IHDR(png_ptr, info_ptr, width, height,
                    bit_depth, color_type, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if(setjmp(png_jmpbuf(png_ptr))){
        return 2;
    }
    png_write_image(png_ptr, row_pointers);


    /* end write */
    if(setjmp(png_jmpbuf(png_ptr))){
        return 2;
    }
    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (int y=0; y<height; y++){
            free(row_pointers[y]);
    }
    free(row_pointers);

    fclose(outfile);
    return 0;

}

/*
int readJPEGFile(char *filename){
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* file = fopen(filename,"rb");
    if(file == NULL){
        exit(0);
    }
    int jpegStatus;
    //Default error management init. Must be setup before calling jpeg_create_decompress
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);
    jpegStatus = jpeg_read_header(&cinfo, TRUE);
    jpegStatus = jpeg_start_decompress(&cinfo);
    
    width = cinfo.output_width; //Image width in pixels
    height = cinfo.output_height;
    pixelSize = cinfo.output_components; // Number of image channels (3 for an RGB Image)
    row_stride = width*pixelSize; //Number of bytes occuped in memory by a line of an image  
    //unsigned char row_pointer[0] = (unsigned char *)malloc (cinfo.output_width*cinfo.num_components);
    //std::vector<std::vector<uint8_t>> data;
    unsigned char* rowBuffer[1];
    char test[] = {"test"};
    buffer= (unsigned char*)malloc(sizeof(unsigned char)*width*height*pixelSize);
    int bufferCount = 0;
    while(cinfo.output_scanline < cinfo.image_height){
       // std::vector<uint8_t> vec(row_stride);
       rowBuffer[0] = (unsigned char*)(&buffer[3*cinfo.output_width*cinfo.output_scanline]);
       
        jpeg_read_scanlines(&cinfo,rowBuffer,1);
       /*
        for(int i = 0; i < cinfo.output_width; i ++){            
            *rowBuffer[0]++;
            *rowBuffer[0]++;
            *rowBuffer[0]++;
        }
        
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    char red;
    char green;
    char blue;
    int counter = 0;
    for(int i =0; i < cinfo.image_height;i++){
        for(int j = 0; j < cinfo.image_width; j++){
            red = buffer[(i*cinfo.image_width*3) + (j*3) + 0] ;
            blue = buffer[(i*cinfo.image_width*3) + (j*3) + 1] ;
             green  = buffer[(i*cinfo.image_width*3) + (j*3) + 2];  

        }
    }
    fclose(file);
}

int writeJPEG(char *fileName){
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JSAMPROW rowBuffer[1];
    FILE *outfile = fopen(fileName, "wb");
    if(!outfile){
        return -1;
    }
    
    cinfo.err=jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo,outfile);
    cinfo.image_height = height;
    cinfo.image_width = width;
    cinfo.input_components = pixelSize;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_quality(&cinfo,100,TRUE);   
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);
    char red;
    char green;
    char blue;
   
    for(int i =0; i < cinfo.image_height;i++){
        for(int j = 0; j < cinfo.image_width; j++){
            red = buffer[(i*cinfo.image_width*3) + (j*3) + 0] ;
            blue = buffer[(i*cinfo.image_width*3) + (j*3) + 1] ;
            green = buffer[(i*cinfo.image_width*3) + (j*3) + 2];  

        }
    }
    while(cinfo.next_scanline < cinfo.image_height){
        rowBuffer[0]=&buffer[cinfo.next_scanline * cinfo.image_width * cinfo.input_components];
        jpeg_write_scanlines(&cinfo, rowBuffer,1);
       
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
}
*/