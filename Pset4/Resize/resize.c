// Resizes a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bmp.h"

void map_pixels(RGBTRIPLE row[], FILE *outptr, int out_padding, float f, int biWidth);

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize f infile outfile\n");
        return 1;
    }

    // int check = sscanf(argv[1], "%3f");
    // printf("%i", check);
    float f = atof(argv[1]);
    if (f <= 0 || f > 100) {
        fprintf(stderr, "Incorrect scaling parameter 'f'\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 1;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 1;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 1;
    }
    // saving the original dimensions in variables
    int original_width = bi.biWidth, original_height = bi.biHeight;

    // determine inputs padding for scanlines
    int in_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // scale things up
    printf("before: %i x %i\n", bi.biWidth, abs(bi.biHeight));
    bi.biWidth *= f;
    bi.biHeight *= f;
    printf("after:  %i x %i\n", bi.biWidth, abs(bi.biHeight));
    // determine outputs padding for scanlines
    int out_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // change the size of the image
    printf("size before: %i\n", bi.biSizeImage);
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth + out_padding) * abs(bi.biHeight));
    printf("size after: %i\n", bi.biSizeImage);

    // change the size of the file
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    RGBTRIPLE scanline[abs(original_height)][original_width];
    for (int i = 0, biHeight = abs(original_height); i < biHeight; i++)
    {
        // storing the whole scanline into an array
        for (int j = 0; j < original_width; j++){
            fread(&scanline[i][j], sizeof(RGBTRIPLE), 1, inptr);
        }

        // skip over padding in input, if any
        fseek(inptr, in_padding, SEEK_CUR);
        // map_pixels(scanline[(int) (i/f)], outptr, out_padding, f, bi.biWidth);
    }

    printf("green pixel: %i %i %i\n", scanline[0][0].rgbtRed, scanline[0][0].rgbtGreen, scanline[0][0].rgbtBlue);

    // mapping the scanlines into the new image
    for (int k = 0; k < bi.biHeight; k++){
        map_pixels(scanline[(int) (k/f)], outptr, out_padding, f, bi.biWidth);
    }


    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}

void map_pixels(RGBTRIPLE row[], FILE *outptr, int out_padding, float f, int biWidth){
    // mapping the original scanline into the new image
    for (int i = 0; i < biWidth; i++){
        fwrite(&row[(int) (i/f)], sizeof(RGBTRIPLE), 1, outptr);
    }

    // add padding to the output (if any)
    for (int j = 0; j < out_padding; j++)
    {
        fputc(0x00, outptr);
    }
}
