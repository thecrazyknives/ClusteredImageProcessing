#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_SIZE 262144
#define threads 8

unsigned char archivo[MAX_SIZE]; 
int size;
int histograma[256] = {0};
double histogramaNormalizado[256] = {0};
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

// paso 1: suma acumulativa

// paso 2: normalizar los valores del paso 1 dividiendo entre el numero todal de pixeles

// multiplicar los valores del paso 2 por el maximo valor de gris y redondear


void normalizarHistograma(int* histogramaOriginal,double* histogramaNormalizado){
    // suma acumulativa
    int sum = 0, histogramaAcumulativo[256] = {0};
    for(int i = 1 ; i < 256 ; i++  ){
        sum += histograma[i];
        histogramaAcumulativo[i] = sum;
    }

    //double sumaTMP = 0;
    double factorEscala = 1.0 / sum;
    #pragma omp parallel for
    for (int i = 0; i < 256; ++i) {
        histogramaNormalizado[i] = (double)(histogramaOriginal[i] * factorEscala );
        //histogramaNormalizado[i] = (double)(histogramaOriginal[i] * factorEscala * 255.0); // si se elimina el 255, la normalizacion ocurrira de 0 a 1
        //sumaTMP+=histogramaNormalizado[i];
    }
    //printf("\nSuma normalizada : %f",sumaTMP);
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





void calcularFuncionTransformacion(int* histogramaNormalizado) {
    double sumaAcumulativa = 0.0;
    for (int i = 0; i < 256; ++i) {
        sumaAcumulativa += histogramaNormalizado[i];
        funcionTransformacion[i] = sumaAcumulativa / MAX_SIZE;
    }
}

/*void ajustarContraste(){
    #pragma omp parallel shared(histograma) num_threads(threads)
    {
        #pragma omp parallel for
        for (int i = 0; i < size; ++i) {
            imagenSalida[i] = (unsigned char)(255.0 * funcionTransformacion[imagenOriginal[i]]);
        }
    }
}*/

int main() {
    printf("Obteniendo datos de la imagen..\n");
    size = obtenerDatosImagen("lena_gray.raw");
   
    printf("Procesando...\n");

    llenarHistograma(histograma);

    imprimirHistogramaInt(histograma);
    //normalizarHistograma(histograma,histogramaNormalizado);
    //imprimirHistogramaDouble(histogramaNormalizado);

    

    return 0;
}

