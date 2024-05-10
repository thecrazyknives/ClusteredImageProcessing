#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 262144
#define threads 8
unsigned char archivo[MAX_SIZE]; 
int histograma[256];

int obtenerDatosImagen(char* imagen){
    char *file_name = imagen;
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    int byte;
    size_t index = 0;
    while ((byte = fgetc(file)) != EOF && index < MAX_SIZE) {
        // Guardar el byte en el arreglo
        archivo[index++] = (unsigned char)byte; // Convertido a unsigned char
    }
    /*if (index == MAX_SIZE) {
        printf("Advertencia: Se alcanzó el tamaño máximo del arreglo. Algunos bytes pueden no haber sido leídos.\n");
    }*/
    fclose(file);
    /*printf("Bytes leídos:\n");
    for (size_t i = 0; i < index; i++) {
        printf("%u ", archivo[i]); // Imprime como unsigned char
    }
    printf("\n");*/
    return index;
}

int main() {
    int size = obtenerDatosImagen("lena_gray.raw");
   
    for(int i=0;i<256;i++){
        histograma[i] = 0;
    }

    #pragma omp parallel shared(histograma) num_threads(threads)
    {
        #pragma omp for //schedule(static)
        for (int i = 0; i < size; i++) {
			histograma[archivo[i]] ++;
        }
    }

    for(int i=0;i<256;i++){
        printf("%3d : %4d \n",i,histograma[i]);
    }

    return 0;
}

