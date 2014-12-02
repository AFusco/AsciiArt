#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "string.h"

#define NUMEROGRIGI 8

char* grigi[] =
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

//Fa la media dei valori RGB di un pixel, per trovarne la tonalità di grigio
int grey(RGBTRIPLE pixel)
{
    return ((pixel.rgbtBlue + pixel.rgbtGreen + pixel.rgbtRed)/3);
}

//Trova il range di grigio corrispondente a un pixel
int shadeofgrey(RGBTRIPLE p)
{
    return grey(p)*NUMEROGRIGI/256;
}

int main(int argc, char* argv[])
{
    
    //Variabili per i files
    char *infile, *outfile;
    FILE *inptr, *outptr;
    
    //Header e info dell'immagine
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    
    //Dimensioni e padding dell'immagine
    int padding;
    int bmpHeight, bmpWidth;
    
    //Matrici di supporto
    int** greyImage;
    char** charImage;
    
    //Dimensione dei blocchi di pixel
    int larghezzaBlocco, altezzaBlocco;
    
    //Pixel temporaneo per la lettura
    RGBTRIPLE triple;
    
    //Numero di blocchi per riga e colonna
    int blocchiOrizzontali, blocchiVerticali;
    
    
    if (argc != 4)
    {
        printf("Usage: %s [dimensione blocco] [input file] [output file]\n", argv[0]);
        return 1;
    }
    
    //Calcola le dimensioni di un blocco di pixel, tenendo conto che il rapporto altezza/larghezza della maggior parte dei font monospace è di circa 2:1
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
    
    
    //Alloca una matrice di int (uno per ogni pixel)
    greyImage = (int**)malloc(bmpHeight * sizeof(int *));
    if (greyImage == NULL)
    {
        fprintf(stderr, "Errore nell'allocazione di memoria.\n");
        exit(-10);
    }
    
    for (int i = 0; i < bmpHeight; i++)
    {
        greyImage[i] = (int*)malloc(bmpWidth * sizeof(int));
        if (greyImage[i] == NULL)
        {
                fprintf(stderr, "Errore nell'allocazione di memoria.\n");
                exit(-10);
        }
    }
    
    
    //Legge pixel per pixel e registra il range di grigio corrispondente nella matrice
    for (int i = 0; i < bmpHeight; i++)
    {
        for (int j = 0; j < bmpWidth; j++)
        {
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            greyImage[bmpHeight-1-i][j] = shadeofgrey(triple);
        }
        
        for (int j = 0; j < padding; j++)
        {
            greyImage[bmpHeight-1-i][j] = 0x00;
        }
        fseek(inptr, padding, SEEK_CUR);
    }
    
    //Calcola il numero di blocchi per riga e colonna
    blocchiOrizzontali = bmpWidth/larghezzaBlocco;
    blocchiVerticali = bmpHeight/altezzaBlocco;
    
    //Alloca una matrice di char, uno per ogni blocco
    charImage = (char**)malloc(blocchiVerticali * sizeof(char *));
    if (charImage == NULL) exit(-15);

    for (int i = 0; i < blocchiVerticali; i++)
    {
        charImage[i] = (char*)malloc(blocchiOrizzontali * sizeof(char));
        if (charImage[i] == NULL) exit(-20);
    }
    
    //Per ogni pixel di ogni blocco, calcola la media del grigio e inserisce nella matrice charImage il carattere corrispondente
    for (int i = 0; i < blocchiVerticali; i++)
    {
        for (int j = 0; j < blocchiOrizzontali; j++)
        {
            
            int somma = 0;
            
            for (int a = 0; a < altezzaBlocco; a++)
            {
                for (int b = 0; b <larghezzaBlocco; b++)
                {
                    somma += greyImage[i* altezzaBlocco + a][j*larghezzaBlocco +b];
                }
            }
            
            somma /= larghezzaBlocco*altezzaBlocco; //divide per fare la media
            
            if (somma >= 0 && somma < NUMEROGRIGI)
                charImage[i][j] =  grigi[somma][ rand()%strlen(grigi[somma])]; //tra tutti i caratteri corrispondenti, ne viene scelto uno a caso
        }
    }
    
    //Scrive nel file di testo
    for (int i = 0; i < blocchiVerticali; i++)
    {
        for (int j = 0; j<blocchiOrizzontali; j++)
        {
            fprintf(outptr, "%c", charImage[i][j]);
        }
        fprintf(outptr, "\n");
    }
    
    // Chiude i file
    fclose(outptr);
    fclose(inptr);
    
    //Libera la memoria
    for (int i = 0; i < bmpHeight; i++)
    {
        
        free(greyImage[i]);
    }
    free(greyImage);
    
    
    for (int i = 0; i < blocchiVerticali; i++)
    {
        free(charImage[i]);
    }
    free(charImage);
    
    // ZAC ZAC ZAC
    return 0;
}
