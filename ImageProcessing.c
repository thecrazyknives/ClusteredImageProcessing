#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 262144
#define MAX_THREADS 8

unsigned char fileData[MAX_SIZE]; 
int size;
int histogram[256] = {0};
int normalizedHistogram[256] = {0};
double transformationFunction[256] = {0};


int getImageData(char* image){
    char *file_name = image;
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int byte;
    size_t index = 0;
    while ((byte = fgetc(file)) != EOF && index < MAX_SIZE) {
        if(index == MAX_SIZE)
            printf("Warning: Maximum array size reached. Some bytes may not have been read.\n");
        fileData[index++] = (unsigned char)byte; 
    }
    
    fclose(file);

    return index;
}

void fillHistogram(int* histogram){
    #pragma omp parallel shared(histogram) num_threads(MAX_THREADS)
    {
        #pragma omp for
        for (int i = 0; i < size; i++) {
			histogram[fileData[i]] ++;
        }
    }
}

void normalizeHistogram(int* originalHistogram, int* normalizedHistogram){
    int sum = 0; 
    for(int i = 0 ; i < 256 ; i++  ){
        sum += histogram[i];
        normalizedHistogram[i] = sum;
        // printf("\n%3d : %4d",i,normalizedHistogram[i]);
    }

    printf("ACUM: \n");

    int max_gray = 255; // this number may change later

    double factor = ((double)max_gray / sum);   // printf("F: %f\n",factor);
    
    #pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        normalizedHistogram[i] = (int)(normalizedHistogram[i] * factor);
        //printf("\n%3d : %4d",i,normalizedHistogram[i]);
    }
}

void processImageData(unsigned char* data, int* normalizedHistogram) {
    for (int i = 0; i < size; ++i) {
        data[i] = (unsigned char)normalizedHistogram[data[i]];
    }
}

void saveImage(unsigned char* data){
    FILE* outputFile = fopen("lena_gray_processed.raw", "wb");
    if (outputFile == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fwrite(data, sizeof(unsigned char), size, outputFile);
    fclose(outputFile);
}

void printImageData(unsigned char* data){
    printf("Read bytes:\n");
    for (size_t i = 0; i < MAX_SIZE; i++) {
        printf("%u ", data[i]); 
    }
    printf("\n");    
}

void printHistogramInt(int* data) {
    for(int i=0;i<256;i++){
        printf("\n%3d : %4d",i,data[i]);
    }
}

void printHistogramDouble(double* data) { 
    for(int i=0;i<256;i++){
        printf("\n%3d : %f",i,data[i]);
    }
}

int main(int argc, char** argv ) {
    if(argc == 0){
        // TODO: Check number of arguments
    }

    printf("Getting image data..\n");
    size = getImageData("lena_gray.raw");
   
    printf("Processing...\n");

    fillHistogram(histogram);
    normalizeHistogram(histogram, normalizedHistogram);
    processImageData(fileData, normalizedHistogram);
    saveImage(fileData);
    printImageData(fileData);

    printf("File processed successfully!");

    //normalizeHistogram(histogram, normalizedHistogram);
    //printHistogramDouble(normalizedHistogram);
 
    return 0;
}
