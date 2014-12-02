/**
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

#define NUMEROGRIGI 4

#define DIMBLOCCO 3

char grigi[] = {' ', '.', 'O', '#'};


int grey(RGBTRIPLE pixel)
{
    return ((pixel.rgbtBlue + pixel.rgbtGreen + pixel.rgbtRed)/3);
}

int shadeofgrey(RGBTRIPLE p)
{
    return grey(p)*NUMEROGRIGI/256;
}

int main(int argc, char* argv[])
{
    
    
    // ensure proper usage
    if (argc != 3)
    {
        printf("Usage: %s infile outfile\n", argv[0]);
        return 1;
    }
    

    
    // remember filenames
    char* infile = argv[1];
    //char* infile = "/Users/afusco/Developer/AsciiArt/AsciiArt/image.bmp";
    char* output = argv[2];

    
    // open input file
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }
    
    FILE* outptr = fopen(output, "w");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", output);
        return 2;
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
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    
    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    int biHeight = abs(bi.biHeight);
    int biWidth = bi.biWidth;
    
    int** GreyImage;
    GreyImage = malloc(biHeight * sizeof(int));
    if (GreyImage == NULL)
        exit(-10);
    
    RGBTRIPLE triple;
    
    
    for (int i = 0; i < biHeight; i++)
    {
        
        GreyImage[i] = malloc(biWidth * sizeof(int));
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            GreyImage[i][j] = shadeofgrey(triple);
            
            //printf("%d ", GreyImage[i][j] );
            
            
        }
        
        for (int j = 0; j < padding; j++)
        {
            GreyImage[i][j] = 0x00;
        }
        
        //printf("\n");
        
        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
        

    }
    
    
    
    int blocchiOrizzontali = biWidth/DIMBLOCCO;
    int blocchiVerticali = biHeight/DIMBLOCCO;
    
    
    char** charImg = malloc(blocchiVerticali * sizeof(char));
    if (charImg == NULL) exit(-15);
    

    for (int i = 0; i < blocchiVerticali; i++)
    {
        charImg[i] = malloc(blocchiOrizzontali * sizeof(char));
        if (charImg[i] == NULL) exit(-20);

        for (int j = 0; j < blocchiOrizzontali; j++)
        {
            
            int somma = 0;
            
            for (int a = 0; a < DIMBLOCCO; a++)
            {
                for (int b = 0; b < DIMBLOCCO; b++)
                {
                    
                    somma += GreyImage[i*DIMBLOCCO + a][j*DIMBLOCCO +b];
                    
                    
                }
            }
            
            somma /= DIMBLOCCO*DIMBLOCCO;
            
            if (somma >= 0 && somma < NUMEROGRIGI)
                charImg[i][j] = grigi[somma];
                //fprintf(outptr, "%c", grigi[somma]);
            //printf("%c", grigi[somma]);
 
        }
        
        fprintf(outptr, "\n");
    }
    
    /*
    // iterate over infile's scanlines
 */
    
    for (int i = 0; i < blocchiVerticali; i++)
    {
        for (int j = 0; j<blocchiOrizzontali; j++)
        {
            fprintf(outptr, "%c", charImg[i][j]);
        }
    }
    
    // close infile
    fclose(outptr);
    fclose(inptr);
    
    for (int i = 0; i < biHeight; i++)
    {
        
        free(GreyImage[i]);
    }
    free(GreyImage);
    
    
    for (int i = 0; i < blocchiVerticali; i++)
    {
        free(charImg[i]);
    }
    free(charImg);

    // that's all folks
    return 0;
}
