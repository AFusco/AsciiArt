//
//  AsciiArt.h
//  AsciiArt
//
//  Created by Alessandro Fusco on 02/12/14.
//  Copyright (c) 2014 Alessandro Fusco. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include <string.h>


#define NUMEROGRIGI 8
const char* grigi[] =
{
    "#%$",
    "W8KMA",
    "mdK4ZGbNDXY5P*Q",
    "gjez2]/(YL)t[+T7Vf",
    "_ivc=!/|\\~",
    ":-",
    ".",
    " "
};


int grey(RGBTRIPLE pixel);
int shadeofgrey(RGBTRIPLE p);


int main(int argc, char* argv[])
{
    
    
    //Variabili per i file
    char *infile, *outfile;
    FILE *inptr, *outptr;
    
    
    //Per l'header e le info dell'immagine
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    
    //Dimensioni e padding dell'immagine
    int padding;
    int bmpHeight, bmpWidth;
    
    
    //Matrici di supporto
    int** greyImage;
    char** charImage;
    
    //Dimensioni dei singoli blocchi
    int larghezzaBlocco, altezzaBlocco;
    
    // Pixel temporaneo
    RGBTRIPLE triple;
    
    // Numero di blocchi in una riga e in una colonna
    int blocchiOrizzontali, blocchiVerticali;
    
    // Somma temporanea (usata per fare la media delle tonalità di grigio)
    int somma = 0;
    
    //Indici per i cicli for
    int i, j, a, b;
    
    
    if (argc != 4)
    {
        printf("Usage: %s [dimensione blocco] [input file] [output file]\n", argv[0]);
        return 1;
    }
    
    
    //Il rapporto altezza/larghezza dei font monospace è approssimabile a 2:1
    larghezzaBlocco = atoi(argv[1]);
    altezzaBlocco = larghezzaBlocco * 2;
    
    // I file di input e di output
    infile = argv[2];
    outfile = argv[3];

    // Apri il file di input
    inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }
    
    // Apri il file di output
    outptr = fopen(outfile, "w");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", outfile);
        return 2;
    }
    
    
    // Leggi l'header della bitmap
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    
    // Leggi le info della bitmap
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    
    // Assicurati che il file sia un BMP 4.0 a 24 bit
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(inptr);
        fprintf(stderr, "Formato file non supportato.\n");
        return 4;
    }
    
    
    // Determina il padding
    padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    
    // Determina l'altezza e la larghezza dell'immagine
    bmpHeight = abs(bi.biHeight);
    bmpWidth = bi.biWidth;
    
    
    
    //Allocazione della memoria per una matrice
    greyImage = (int**)malloc(bmpHeight * sizeof(int *));
    if (greyImage == NULL)
    {
        fprintf(stderr, "Errore nell'allocazione di memoria.\n");
        exit(-10);
    }
    
    for (i = 0; i < bmpHeight; i++)
    {
        greyImage[i] = (int*)malloc(bmpWidth * sizeof(int));
        if (greyImage[i] == NULL)
        {
                fprintf(stderr, "Errore nell'allocazione di memoria.\n");
                exit(-10);
        }
    }
    
    
    //Inserisce dall'ultima riga alla prima la tonalità di grigio di ogni pixel nella matrice
    for (i = 0; i < bmpHeight; i++)
    {
        for (j = 0; j < bmpWidth; j++)
        {
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr); //Legge un pixel
            greyImage[bmpHeight-1-i][j] = shadeofgrey(triple);  //Calcola a quale range di grigio corrisponde il pixel
                                                                //e lo mette nella matrice
        }
        
        
        //Aggiunge i pixel NULL del padding di un'immagine bitmap
        for (j = 0; j < padding; j++)
        {
            greyImage[bmpHeight-1-i][j] = 0x00;
        }
        fseek(inptr, padding, SEEK_CUR);
    }
    
    
    //Calcola le dimensioni dei blocchi da leggere
    blocchiOrizzontali = bmpWidth/larghezzaBlocco;
    blocchiVerticali = bmpHeight/altezzaBlocco;
    
    
    //Allocazione della memoria di una matrice di caratteri
    charImage = (char**)malloc(blocchiVerticali * sizeof(char *));
    if (charImage == NULL) exit(-15);
    
    for (i = 0; i < blocchiVerticali; i++)
    {
        charImage[i] = (char*)malloc(blocchiOrizzontali * sizeof(char));
        if (charImage[i] == NULL) exit(-20);
    }
    
    
    
    // Fa la media di ciascun blocco e mette il carattere corrispondente nella matrice charImage
    for (i = 0; i < blocchiVerticali; i++)
    {
        for (j = 0; j < blocchiOrizzontali; j++)
        {
            
            somma = 0;
            
            for (a = 0; a < altezzaBlocco; a++)
            {
                for (b = 0; b <larghezzaBlocco; b++)
                {
                    somma += greyImage[i* altezzaBlocco + a][j*larghezzaBlocco +b];
                }
            }
            
            somma /= larghezzaBlocco*altezzaBlocco;
            
            if (somma >= 0 && somma < NUMEROGRIGI)
            {
                charImage[i][j] =  grigi[somma][ random()%strlen(grigi[somma])]; // Prende un carattere a caso di una certa tonalità di grigio
            }
        }
    }
    
    
    //Scrive tutto nel file di testo
    for (int i = 0; i < blocchiVerticali; i++)
    {
        for (int j = 0; j<blocchiOrizzontali; j++)
        {
            fprintf(outptr, "%c", charImage[i][j]);
        }
        fprintf(outptr, "\n");
    }
    
    // Chiude i files
    fclose(outptr);
    fclose(inptr);
    
    //Libera la memoria
    for (i = 0; i < bmpHeight; i++)
    {
        free(greyImage[i]);
    }
    free(greyImage);
    
    
    for (i = 0; i < blocchiVerticali; i++)
    {
        free(charImage[i]);
    }
    free(charImage);
    
    // ZACZAC
    return 0;
}




int grey(RGBTRIPLE pixel)
{
    return ((pixel.rgbtBlue + pixel.rgbtGreen + pixel.rgbtRed)/3);
}

int shadeofgrey(RGBTRIPLE p)
{
    return grey(p)*NUMEROGRIGI/256;
}


