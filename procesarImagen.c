#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 262144
#define threads 8

unsigned char archivo[MAX_SIZE]; 
int size;
int histograma[256] = {0};
int histogramaNormalizado[256] = {0};
double funcionTransformacion[256] = {0};


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
        if(index == MAX_SIZE)
            printf("Advertencia: Se alcanzó el tamaño máximo del arreglo. Algunos bytes pueden no haber sido leídos.\n");
        archivo[index++] = (unsigned char)byte; 
    }
    
    fclose(file);

    return index;
}

void llenarHistograma(int* hist){
    #pragma omp parallel shared(histograma) num_threads(threads)
    {
        #pragma omp for //schedule(static)
        for (int i = 0; i < size; i++) {
			histograma[archivo[i]] ++;
        }
    }
}

void normalizarHistograma(int* histogramaOriginal,int* histogramaNormalizado){
    // suma acumulativa
    int sum = 0; 
    //histogramaNormalizado = {0};
    for(int i = 1 ; i < 256 ; i++  ){
        sum += histograma[i];
        histogramaNormalizado[i] = sum;
    }

    int max_gray = 256; // este numero puede cambiar despues

    double factor = max_gray / sum;
    #pragma omp parallel for
    for (int i = 0; i < 256; ++i) {
        histogramaNormalizado[i] = (int)(histogramaOriginal[i] * factor );
    }
}

void procesarDatosImagen(unsigned char* datos, int* histogramaNormalizado) {
    for (int i = 0; i < size; ++i) {
        datos[i] = (unsigned char)histogramaNormalizado[datos[i]];
    }
}

void guardarImagen(unsigned char* datos){
    FILE* archivoSalida = fopen("lena_gray_processed.raw", "wb");
    if (archivoSalida == NULL) {
        perror("Error al abrir el archivo para escritura");
        return;
    }
    fwrite(datos, sizeof(unsigned char), size, archivoSalida);
    fclose(archivoSalida);
}

void imprimirDatosImagen(unsigned char* data){
    printf("Bytes leídos:\n");
    for (size_t i = 0; i < MAX_SIZE; i++) {
        printf("%u ", data[i]); 
    }
    printf("\n");    
}

void imprimirHistogramaInt(int* data) {
    for(int i=0;i<256;i++){
        printf("\n%3d : %4d",i,data[i]);
    }
}

void imprimirHistogramaDouble(double* data) { 
    for(int i=0;i<256;i++){
        printf("\n%3d : %f",i,data[i]);
    }
}

/* void calcularFuncionTransformacion(int* histogramaNormalizado) { // esto podria no ser necesario
    double sumaAcumulativa = 0.0;
    for (int i = 0; i < 256; ++i) {
        sumaAcumulativa += histogramaNormalizado[i];
        funcionTransformacion[i] = sumaAcumulativa / MAX_SIZE;
    }
} */

int main() {
    printf("Obteniendo datos de la imagen..\n");
    size = obtenerDatosImagen("lena_gray.raw");
   
    printf("Procesando...\n");

    llenarHistograma(histograma);
    normalizarHistograma(histograma,histogramaNormalizado);
    procesarDatosImagen(archivo,histogramaNormalizado);
    guardarImagen(archivo);

    printf("Archivo procesado con exito!");

    //imprimirHistogramaInt(histograma);
    //normalizarHistograma(histograma,histogramaNormalizado);
    //imprimirHistogramaDouble(histogramaNormalizado);
 
    return 0;
}