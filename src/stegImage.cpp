#include <stegImage.h>

//Read Encoded File
int stegImage::readPNG(std::string *fileName){              
    //fprintf(stdout,"Reading from PNG File \n");
    //open file for reading
    FILE* file = fopen(fileName->c_str(),"rb");
    //Check if file is null
    if(file == NULL){
        exit(0);
    }
    //The follow char header will be used to validate the PNG signature (Every valid png file starts with this 8 byte sig)
    u_char header[8];
    //Read the first 8 bytes of the file into the header buffer
    fread(header,1,8,file);
    //Use libPMG to check the sig, if this returns 0 everything is OK.
    if(png_sig_cmp(header,0,8)){
        return -1;
    }
    //Create a PNG read struct
    this->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
    if(!this->png_ptr){
        return -1;
    }

    //Create a PNG info struct
    this->info_ptr = png_create_info_struct(this->png_ptr);
    if(!this->info_ptr){
        this->cleanup();
        return -1;
    }
    //The function png_create_read_struct has an option to set functions that libPNG can call whenever an error occurs. This will be implemented
    //Later once the library is working. For now I will be using libPNGs default jmp fuction for error handling. If an error occurs while performing the next
    //Functions, png will jump to here.
    if(setjmp(png_jmpbuf(this->png_ptr))){
        this->cleanup();
        return -1;
    }
    //Set file pointer to png struct
    png_init_io(this->png_ptr,file);
    //Set the amount of sig bytes we read previously (8). 
    png_set_sig_bytes(this->png_ptr,8);
    //Call png_read_info with pngPtr as image handle, and infoPtr to receive file info
    png_read_info(this->png_ptr,this->info_ptr);

    //Grab the width, height, colortype, and channels.
    this->width = png_get_image_width(this->png_ptr,this->info_ptr);
    this->height = png_get_image_height(this->png_ptr,this->info_ptr);
    this->color_type = png_get_color_type(this->png_ptr,this->info_ptr);
    this->channels = png_get_channels(this->png_ptr,this->info_ptr);
    //Currently only supporting 32bit RGBA file formats (Will improve all this code later)
    //LibPNG Supports changing png file format (Will be implemented later to alter all PNG files to RGBA)
    if(this->color_type != PNG_COLOR_TYPE_RGBA){
        return -1;
    }
    //Get the bit depth (Bits per channel)
    this->bit_depth = png_get_bit_depth(this->png_ptr,this->info_ptr);
    
    //Will use png_read_update info to alter PNG format for future use
    //png_read_update_info(this->png_ptr,this->info_ptr);
   
    //Allow a row pointer for each row of pixels in the image.
    this->row_pointers = (png_bytep*)malloc(sizeof(png_bytep)*this->height);
    
    //For each row, allocate a pointer for the length of the stride (Bytes per row)
    for(int i =0; i < this->height; i++){
        this->row_pointers[i] =(png_byte *) malloc(png_get_rowbytes(this->png_ptr,this->info_ptr));
    }
    //Read the image data and write to the address pointed to by row_ptrs (DataBuffer)
    png_read_image(this->png_ptr, this->row_pointers);
    //std::string commands = "<COM:ALL:4-ipconfig:1-beacon-10000:0-0>";
    //Don't call cleanup here!!! We need the row pointers
    png_destroy_read_struct(&this->png_ptr,&this->info_ptr,NULL);
    //Close the file
    fclose(file);
    return 0;

    
}

//Write Encoded file
int stegImage::writePNG(std::string* filename){
    //fprintf((stdout,"Writing encoded image to new file\n"));
    //Open file for writing
    FILE *outfile = fopen(filename->c_str(), "wb");
    if(!outfile){
        return -1;
    }
    //Create a PNG write struct
    this->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!this->png_ptr){
        this->cleanup();
        return -1;
    }
    //Create PNG info struct
    this->info_ptr = png_create_info_struct(this->png_ptr);
    if (!this->info_ptr){
        this->cleanup();
        return -1;
    }
    //Set error handling
    if(setjmp(png_jmpbuf(this->png_ptr))){
        this->cleanup();
    }
    //Point png struct to output file
    png_init_io(this->png_ptr, outfile);
    //Set the PNG_IHDR chunck information
    png_set_IHDR(this->png_ptr, this->info_ptr, this->width, this->height,
                    this->bit_depth, this->color_type, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    //Write the png information before the header (Attributes/Sig)
    png_write_info(this->png_ptr, this->info_ptr);
    //Write image to output
    png_write_image(this->png_ptr, this->row_pointers);
    //End writing of image
    png_write_end(this->png_ptr, NULL);
    //Cleanup heap allocation
    this->cleanup();
    fclose(outfile);
    return 0;

}

//Cleanup resources from stegImage
int stegImage::cleanup(){
    //Free allocated memory for image
    if(this->row_pointers != NULL){
        for (int y=0; y<this->height; y++){
            free(this->row_pointers[y]);
        }
    }
    //Destroy png read and info structs
    if(this->info_ptr != NULL && this->png_ptr != NULL){
        png_destroy_read_struct(&this->png_ptr,&this->info_ptr,NULL);
    }else if(this->png_ptr != NULL && this->info_ptr == NULL){
        png_destroy_read_struct(&this->png_ptr,NULL,NULL);
    }

    return 0;
}

int stegImage::encodeImage(std::string commandString){
    int comCount = 0; //Count to keep track of command bytes written to image
    int init = 0;     //Init used to write the size of the command to the file
    //For each row, create a byte buffer that points to the beginning of each row
    for(int y = 0; y < this->height; y++){
        png_byte* row = this->row_pointers[y];
        //Now, create a pointer that points to each pixel (4 bytes per bixel for RGBA)
        for(int x = 0; x < this->width; x++){
            png_byte* ptr = &(row[x*4]);
            //If init, write the size of the command to the first least significant byte (Red)
            if(init == 0){
                init = 1;
                ptr[0] = (char)commandString.size();
                continue;
            }
            //Iterate over every pixel to write the next byte of the command to the least significant byte of the pixel 
            if(comCount < commandString.size()){
                ptr[0] = commandString.at(comCount);
                comCount++;
            }
        }
    }
}

int stegImage::decodeImage(std::string *output){
    int comCount = 0;
    int init = 0; //Init used to grab the size of the command to the file
    int sizeCommand =0; //size of command
    int lsb = 0;
    int msb = 0;
    //For each row, create a byte buffer that points to the beginning of each row
    for(int y = 0; y < this->height; y++){
        png_byte* row = this->row_pointers[y];
        //Now, create a pointer that points to each pixel (4 bytes per bixel for RGBA)
        for(int x = 0; x < this->width; x++){
            png_byte* ptr = &(row[x*4]);
            //If init, write the size of the command to the first least significant byte (Red)
            if(init == 0){
                init++;
                lsb = ptr[0];
                continue;
            }
            if(init == 1){
                init++;
                msb = ptr[0];
                sizeCommand = (msb << 8) | lsb;
                continue;
            }
            //Iterate over every pixel to write the next byte of the command to the least significant byte of the pixel 
            if(comCount < sizeCommand){
                *output+= ptr[0];
                comCount++;
            }
        }
    }
    return 0;
}