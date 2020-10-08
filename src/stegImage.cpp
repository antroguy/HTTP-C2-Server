#include <stegImage.h>

int stegImage::readPNG(std::string *fileName, std::string commandString){
    //open file for reading
    FILE* file = fopen(fileName->c_str(),"rb");
    //Check if file is null
    if(file == NULL){
    
        exit(0);
    }
    
    u_char header[8];
    fread(header,1,8,file);
    if(png_sig_cmp(header,0,8)){
        return 1;
    }
    this->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);
    if(!this->png_ptr){
        return 4;
    }
    
    this->info_ptr = png_create_info_struct(this->png_ptr);
    if(!this->info_ptr){
        png_destroy_read_struct(&this->png_ptr,NULL,NULL);
        return 4;
    }

    if(setjmp(png_jmpbuf(this->png_ptr))){
        png_destroy_read_struct(&this->png_ptr,&this->info_ptr,NULL);
        return 2;
    }
    png_init_io(this->png_ptr,file);
    png_set_sig_bytes(this->png_ptr,8);
    png_read_info(this->png_ptr,this->info_ptr);

    this->width = png_get_image_width(this->png_ptr,this->info_ptr);
    this->height = png_get_image_height(this->png_ptr,this->info_ptr);
    this->color_type = png_get_color_type(this->png_ptr,this->info_ptr);
    this->channels = png_get_channels(this->png_ptr,this->info_ptr);
    if(this->color_type != PNG_COLOR_TYPE_RGBA){
        return -1;
    }
    this->bit_depth = png_get_bit_depth(this->png_ptr,this->info_ptr);

    png_read_update_info(this->png_ptr,this->info_ptr);
    if(setjmp(png_jmpbuf(this->png_ptr))){
        return 2;
    }
    this->row_pointers = (png_bytep*)malloc(sizeof(png_bytep)*this->height);
    

    for(int i =0; i < this->height; i++){
        this->row_pointers[i] =(png_byte *) malloc(png_get_rowbytes(this->png_ptr,this->info_ptr));
    }
    png_read_image(this->png_ptr, this->row_pointers);
    int r,g,b,a;
    //std::string commands = "<COM:ALL:4-ipconfig:1-beacon-10000:0-0>";
    int comCount = 0;
    int init = 0;
    char test = (char)commandString.size();
    for(int y = 0; y < this->height; y++){
        png_byte* row = this->row_pointers[y];
        for(int x = 0; x < this->width; x++){
            png_byte* ptr = &(row[x*4]);
            if(init == 0){
                init = 1;
                ptr[0] = (char)commandString.size();
                continue;
            }
            if(comCount < commandString.size()){
                ptr[0] = commandString.at(comCount);
                comCount++;
            }
        }
    }
    fclose(file);
    return 0;

    
}

int stegImage::writePNG(std::string* filename){
    FILE *outfile = fopen(filename->c_str(), "wb");
    if(!outfile){
        return -1;
    }
    /* initialize stuff */
    this->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!this->png_ptr){
        return -1;
    }
            
    this->info_ptr = png_create_info_struct(this->png_ptr);
    if (!this->info_ptr){
        return -1;
    }

    if(setjmp(png_jmpbuf(this->png_ptr))){
        return 2;
    }

    png_init_io(this->png_ptr, outfile);


    /* write header */
    if(setjmp(png_jmpbuf(this->png_ptr))){
        return 2;
    }
    png_set_IHDR(this->png_ptr, this->info_ptr, this->width, this->height,
                    this->bit_depth, this->color_type, PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(this->png_ptr, this->info_ptr);


    /* write bytes */
    if(setjmp(png_jmpbuf(this->png_ptr))){
        return 2;
    }
    png_write_image(this->png_ptr, this->row_pointers);


    /* end write */
    if(setjmp(png_jmpbuf(this->png_ptr))){
        return 2;
    }
    png_write_end(this->png_ptr, NULL);

    /* cleanup heap allocation */
    for (int y=0; y<this->height; y++){
            free(this->row_pointers[y]);
    }
    free(this->row_pointers);

    fclose(outfile);
    return 0;

}

