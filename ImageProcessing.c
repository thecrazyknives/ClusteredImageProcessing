#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_SIZE 262144
#define MAX_THREADS 8

unsigned char fileData[MAX_SIZE]; 
int DATA_SIZE;
int histogram[256] = {0};
int normalizedHistogram[256] = {0};

int getImageData(char* image, unsigned char* data){
    FILE *file = fopen(image, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int byte;
    size_t index = 0;
    while ((byte = fgetc(file)) != EOF && index < MAX_SIZE) {
        if(index == MAX_SIZE)
            printf("Warning: Maximum array size reached. Some bytes may not have been read.\n");
        data[index++] = (unsigned char)byte; 
    }
    
    fclose(file);

    return index;
}

void fillHistogram(int* histogram, unsigned char* data, int dataSize) {
    for (int i = 0; i < dataSize; i++) {
        histogram[data[i]]++;
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

    //#pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        normalizedHistogram[i] = (int)(normalizedHistogram[i] * factor);
    }
}

void processImageData(unsigned char* data, int* normalizedHistogram, int dataSize) {
    for (int i = 0; i < dataSize; ++i) {
        data[i] = (unsigned char)normalizedHistogram[data[i]];
    }
}

void saveImage(unsigned char* data, int dataSize){
    FILE* outputFile = fopen("lena_gray_processed.raw", "wb");
    if (outputFile == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fwrite(data, sizeof(unsigned char), dataSize, outputFile);
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
    int rank, size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    unsigned char* localData;
    int localSize;
    int localHistogram[256] = {0};
    
    char *image = "lena_gray.raw";
    if(rank == 0) {         // Only root process loads the image
        DATA_SIZE = getImageData(image, fileData);
    }

    // Broadcast DATA_SIZE to all processes
    MPI_Bcast(&DATA_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate local data buffer
    localSize = DATA_SIZE / size;
    localData = (unsigned char*)malloc(localSize * sizeof(unsigned char));

    // Scatter the image data
    MPI_Scatter(fileData, localSize, MPI_UNSIGNED_CHAR, localData, localSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Each process fills its local histogram
    fillHistogram(localHistogram, localData, localSize);

    // Reduce histograms to the root process
    MPI_Reduce(localHistogram, histogram, 256, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Normalize histogram in the root process
    if(rank == 0) {
        normalizeHistogram(histogram, normalizedHistogram);
    }

    // Broadcast the normalized histogram to all processes
    MPI_Bcast(normalizedHistogram, 256, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process processes its local data
    processImageData(localData, normalizedHistogram, localSize);

    // Gather the processed data
    MPI_Gather(localData, localSize, MPI_UNSIGNED_CHAR, fileData, localSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Root process saves the processed image
    if(rank == 0) {
        saveImage(fileData, DATA_SIZE);
        printf("File processed successfully!\n");
    }

    // Free allocated memory
    free(localData);

    // Finalize MPI
    MPI_Finalize();

    return 0;
}