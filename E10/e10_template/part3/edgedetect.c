#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "address_map_arm.h"
#include "physical.h"

typedef unsigned char byte;
int width, height;

// Declared data structure for a pixel. The ordering of pixel colors in BMP files is b, g, r
struct pixel {
    byte b;
    byte g;
    byte r;
};

// Read BMP file and extract the pixel values (store in data) and header (store in header)
// Data is data[0] = BLUE, data[1] = GREEN, data[2] = RED, data[3] = BLUE, etc...
int read_bmp(char *filename, unsigned char **header, struct pixel **data) {
    struct pixel *data_tmp;
    unsigned char *header_tmp;
    FILE *file = fopen (filename, "rb");

    if (!file) return -1;

    // read the 54-byte header
    header_tmp = malloc (54 * sizeof(unsigned char));
    fread (header_tmp, sizeof(unsigned char), 54, file); 

    // get height and width of image from the header
    width = *(int*)(header_tmp + 18);  // width is a 32-bit int at offset 18
    height = *(int*)(header_tmp + 22); // height is a 32-bit int at offset 22

    // Read in the image
    int size = width * height;
    data_tmp = malloc (size * sizeof(struct pixel)); 
    fread (data_tmp, sizeof(struct pixel), size, file); // read the data
    fclose (file);

    *header = header_tmp;
    *data = data_tmp;

    return 0;
}

void write_bmp(char *bmp, byte *header, struct pixel *data) {
    FILE *file = fopen (bmp, "wb");

    // write the 54-byte header
    fwrite (header, sizeof(byte), 54, file); 

    int size = width * height;
    fwrite (data, sizeof(struct pixel), size, file); // write the rest of the data
    fclose(file);
}

// This function flips the image vertically. It first swaps the pixel values in the top and 
// bottom rows, then the second row with the second-from-bottom, and so on, until finally 
// swapping the middle two rows.
void flip (struct pixel *data, int width, int height){
    int i, j;
    struct pixel tmp;
    // declare image as a 2-D array so that we can use the syntax image[row][column]
    struct pixel (*image)[width] = (struct pixel (*)[width]) data;

    for (i = 0; i < height / 2; ++i){
        for (j = 0; j < width; ++j) {

            /** please complete this function (you can reuse your part 2 solution)  **/
            tmp = image[i][j];
            image[i][j] = image[height - i - 1][j];
            image[height - i][j] = tmp;
        }
    }
}

// The video IP cores used for edge detection require the RGB 24 bits of each pixel to be
// word aligned (aka 1 byte of padding per pixel):
// | unused 8 bits  | red 8 bits | green 8 bits | blue 8 bits |
void memcpy_consecutive_to_padded(struct pixel *from, volatile unsigned int *to, int pixels){

    /** please implement this function (you can reuse your part 2 solution)  **/
    struct pixel (*img)[width] = (struct pixel (*)[width]) from;
    unsigned int (*padded_img)[width] = (unsigned int (*)[width]) to;

    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            padded_img[y][x] = (img[y][x].r << 16) + (img[y][x].g << 8) + img[y][x].b;
        }
    }
}

// Copies the word-aligned data (4 bytes) back into pixel data (3 bytes)
void memcpy_padded_to_consecutive(volatile unsigned int *from, struct pixel *to, int pixels){

    /** please implement this function **/
    unsigned int (*padded_img)[width] = (unsigned int (*)[width]) from;
    struct pixel (*img)[width] = (struct pixel (*)[width]) to;

    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            img[y][x].r = (byte)(  padded_img[y][x] >> 16  );
            img[y][x].g = (byte)( (padded_img[y][x] >> 8) & 0xFF );
            img[y][x].b = (byte)(  padded_img[y][x] & 0xFF );
        }
    }
}

int main(int argc, char *argv[]){
    struct pixel *data = NULL;      // used to hold the image pixels
    byte *header = NULL;            // used to hold the image header
    int fd = -1;                    // used to open/read/etc. the image file
    void *LW_virtual;
    void *SDRAM_virtual;
    time_t start, end;              // used to measure the program's run-time

    // Pointer to the DMA controller for the original image
    volatile unsigned int *mem_to_stream_dma = NULL;
    // Pointer to the DMA controller for the edge-detected image
    volatile unsigned int *stream_to_mem_dma = NULL;
    // Pointer to the DMA controller for the VGA display
    volatile unsigned int *pixel_buffer_dma = NULL;

    // Pointer to the memory location that will hold the original image. A DMA circuit reads
    // the image data from this location and streams it to the edge-detection hardware
    volatile unsigned int *mem_to_stream_dma_buffer = NULL;
    // Pointer to the memory location that will hold the edge-detected image. A DMA circuit reads
    // the streaming data from the edge-detection hardware and writes it to this location
    volatile unsigned int *stream_to_mem_dma_buffer = NULL;

    // Check inputs
    if (argc < 2){
      printf ("Usage: edgedetect <BMP filename>\n");
      return 0;
    }

    // Open input image file (24-bit bitmap image)
    if (read_bmp (argv[1], &header, &data) < 0){
      printf ("Failed to read BMP\n");
      return 0;
    }
    printf ("Image width = %d pixels, Image height = %d pixels\n", width, height);
    if (width != 640 || height != 480) {
        printf ("Error: image width must be 640, height must be 480\n");
        return -1;
    }

    // Flip the image upside down, so it will display correctly on VGA
    flip (data, width, height);

    // Create access to the FPGA light-weight bridge and
    // to SDRAM memory region (through HPS-FPGA non-lightweight bridge)
    if ((fd = open_physical (fd)) == -1)   // Open /dev/mem
      return (-1);
    LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    SDRAM_virtual = map_physical (fd, SDRAM_BASE, SDRAM_SPAN);
    if ((LW_virtual == NULL) || (SDRAM_virtual == NULL))
      return (0);

    // Set up pointers to edge-detection DMA controllers using addresses in the FPGA system
    mem_to_stream_dma = (volatile unsigned int *)(LW_virtual + 0x3100);
    stream_to_mem_dma = (volatile unsigned int *)(LW_virtual + 0x3120);

    // Set up a virtual address pointer to the memory location used to provide the original
    // image to the edge-detection hardware. The memory location is the beginning of SDRAM
    mem_to_stream_dma_buffer = (volatile unsigned int *)(SDRAM_virtual);
    // Set up a virtual address pointer to the memory location where the edge-detection 
    // hardware stores its result.
    stream_to_mem_dma_buffer = (volatile unsigned int *)(SDRAM_virtual + 0x02000000);

    // Set up pointer to DMA controller for VGA
    pixel_buffer_dma = (volatile unsigned int *)(LW_virtual + 0x3020);
    // Initialize VGA Pixel Buffer DMA to display the images. We want to store the address
    // of the original image into the front buffer pointer register, and the address of the 
    // edge-detected image into the back buffer pointer register. Since we can only write
    // to the back buffer pointer register, we have to use a swap operation as shown below
    *(pixel_buffer_dma+1) = SDRAM_BASE;      // write address of original image to back 
                                             // buffer register
    *(pixel_buffer_dma) = 1;                 // swap back/front buffer registers
    while ((*(pixel_buffer_dma+3)) & 0x1)    // wait until the swap operation is complete
        ;
    *(pixel_buffer_dma+1) = (SDRAM_BASE + 0x02000000);  // now write the address of the
                                                        // edge-detected image to the back
                                                        // buffer register
    /********************************************
    *        IMAGE PROCESSING STAGES        *
    ********************************************/

    // Start measuring time
    start = clock();

    // Turn off edge-detection hardware DMAs
    *(mem_to_stream_dma+3) = 0;
    *(stream_to_mem_dma+3) = 0;

    // Write the image to the mem-to-stream buffer
    memcpy_consecutive_to_padded (data, mem_to_stream_dma_buffer, width*height);

    // wait for user to examine the original image
    printf ("Press return to continue");
    getchar ();
    // Turn on the DMAs
    *(stream_to_mem_dma+3) = 4;
    *(mem_to_stream_dma+3) = 4;

    // Write to the buffer reg to start swap; wait for DMA operation to finish
    *(mem_to_stream_dma) = 1;
    while ((*(mem_to_stream_dma+3)) & 0x1)
        ;
    // Write to the buffer reg to start swap; wait for DMA operation to finish
    *(stream_to_mem_dma) = 1;
    while ((*(stream_to_mem_dma+3)) & 0x1)
        ;

    // Turn off DMAs
    *(mem_to_stream_dma+3) = 0;
    *(stream_to_mem_dma+3) = 0;

    // Copy the edge detected image from the stream-to-mem buffer
    memcpy_padded_to_consecutive (stream_to_mem_dma_buffer, data, width*height);
    end = clock();

    // Swap front/back buffer locations in VGA DMA so that we can see the edge-detected image
    *(pixel_buffer_dma) = 1;
    while ((*(pixel_buffer_dma+3)) & 0x1) // wait until the swap is complete
        ;

    printf("TIME ELAPSED: %.0f ms\n", ((double) (end - start)) * 1000 / CLOCKS_PER_SEC);

    // Flip the image upside down, so it will display correctly from the bitmap file
    flip (data, width, height);
    // Write out the edge detected image as a bmp
    write_bmp ("edges.bmp", header, data);

    free(header);
    free(data);

    unmap_physical (LW_virtual, LW_BRIDGE_SPAN);   // release the physical-memory mapping
    unmap_physical (SDRAM_virtual, SDRAM_SPAN);    // release the physical-memory mapping
    close_physical (fd);    // close /dev/mem

    return 0;
}

