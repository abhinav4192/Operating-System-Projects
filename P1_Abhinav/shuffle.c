/*
 * Abhinav Garg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Init
    static const int   DEBUG       = 0;         // 1 = TRUE
    static const int   BUFFER_SIZE = 1024 * 10; // 10KB buffer
    static const char *USAGE_ERROR =
        "Usage: shuffle -i inputfile -o outputfile\n";
    static const char *FILE_OPEN_ERROR = "Error: Cannot open file ";
    char *INP                          = NULL;
    char *OUT                          = NULL;

    // Read command line arguments
    if (argc != 5) {
        fprintf(stderr, "%s", USAGE_ERROR);
        exit(1);
    }
    if (!strcmp(argv[1], "-i") && !strcmp(argv[3], "-o")) {
        INP = argv[2];
        OUT = argv[4];
    } else if (!strcmp(argv[1], "-o") && !strcmp(argv[3], "-i")) {
        INP = argv[4];
        OUT = argv[2];
    } else {
        fprintf(stderr, "%s", USAGE_ERROR);
        exit(1);
    }

    // Open Files
    FILE *ifp = fopen(INP, "r");
    if (!ifp) {
        fprintf(stderr, "%s%s\n", FILE_OPEN_ERROR, INP);
        exit(1);
    }
    FILE *ofp = fopen(OUT, "w");
    if (!ofp) {
        fprintf(stderr, "%s%s\n", FILE_OPEN_ERROR, OUT);
        exit(1);
    }

    // Count Chars in input
    char *tempChar  = malloc(BUFFER_SIZE * sizeof(char));
    int   totalChar = 0;
    while (!feof(ifp)) {
        totalChar += fread(tempChar, 1, BUFFER_SIZE, ifp);
    }
    free(tempChar);
    tempChar = NULL;
    if (DEBUG) printf("TotalChar:%d\n", totalChar);

    // Allocate Memory for storing input file;
    char *inpFile = malloc(totalChar * sizeof(char));
    if (!inpFile) {
        fprintf(stderr, "Cannot allocate enough memory !\n");
        exit(1);
    }

    // Store input file
    fseek(ifp, 0, SEEK_SET); // move pointer to start of file
    int toIgnoreWarnings = fread(inpFile, 1, totalChar, ifp);
    if (DEBUG) printf("toIgnoreWarnings:%d\n", toIgnoreWarnings);
    fclose(ifp);

    // Write output file
    int startIndex   = 0;
    int endIndex     = totalChar - 1;
    int toPrintStart = 1; // bool flag to alternate between top and bottom
    while (startIndex <= endIndex) {
        if (toPrintStart) {
            int length = 0;
            while (inpFile[startIndex + length] != '\n') ++length;
            ++length;
            fwrite(inpFile + startIndex, 1, length, ofp);
            startIndex   = startIndex + length;
            toPrintStart = 0;
            if (DEBUG) printf("length:%d startIndex:%d\n", length, startIndex);
        } else {
            int length = 1;
            while (inpFile[endIndex - length] != '\n') ++length;
            fwrite(inpFile + endIndex - length + 1, 1, length, ofp);
            endIndex     = endIndex - length;
            toPrintStart = 1;
            if (DEBUG) printf("length:%d endIndex:%d\n", length, endIndex);
        }
    }
    fclose(ofp);
    free(inpFile);
    return 0;
}
