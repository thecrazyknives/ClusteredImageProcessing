#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 262144
#define MAX_THREADS 8

unsigned char fileData[MAX_SIZE]; 
int DATA_SIZE;
int histogram[256] = {0};
int normalizedHistogram[256] = {0};

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

void fillHistogram(int* histogram,unsigned char* data){
    #pragma omp parallel shared(histogram) num_threads(MAX_THREADS)
    {
        #pragma omp for
        for (int i = 0; i < DATA_SIZE; i++) {
			histogram[data[i]] ++;
        }
    }
}

void normalizeHistogram(int* originalHistogram, int* normalizedHistogram){
    int sum = 0; 
    for(int i = 0 ; i < 256 ; i++  ){
        sum += originalHistogram[i];
        normalizedHistogram[i] = sum;
    }

    int max_gray = 255; 
    double factor = ((double)max_gray / sum);
    
    #pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        normalizedHistogram[i] = (int)(normalizedHistogram[i] * factor);
    }
}

void processImageData(unsigned char* data, int* normalizedHistogram) {
    for (int i = 0; i < DATA_SIZE; ++i) {
        data[i] = (unsigned char)normalizedHistogram[data[i]];
    }
}

void saveImage(unsigned char* data){
    FILE* outputFile = fopen("lena_gray_processed.raw", "wb");
    if (outputFile == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fwrite(data, sizeof(unsigned char), DATA_SIZE, outputFile);
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

int main(int argc, char** argv ) {

    if(argc == 0){
        printf("Demo mode: \n");
    }

    printf("Getting image data..\n");
    DATA_SIZE = getImageData("lena_gray.raw");
   
    printf("Processing...\n");

    fillHistogram(histogram,fileData);
    normalizeHistogram(histogram, normalizedHistogram);
    processImageData(fileData, normalizedHistogram);
    saveImage(fileData);

    printf("File processed successfully!");

    return 0;
}
