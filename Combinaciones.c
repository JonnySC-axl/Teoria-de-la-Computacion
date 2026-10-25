#include <stdio.h>
#include <string.h>

void ImprimirCadena(int *cadena, int n, FILE *archivo){
    for(int i = 0; i < n; i++){
        char c = '0' + cadena[i];
        putchar(c);
        fputc(c, archivo);
    }
}

void Increase(int *cadena, int n){
    for(int i = n - 1; i >= 0; i--){
        if(cadena[i] == 0){
            cadena[i] = 1;
            break;
        } else {
            cadena[i] = 0;
        }
    }
}

int Is1(int *cadena, int n){
    for(int i = 0; i < n; i++){
        if(cadena[i] == 0)
            return 0;
    }
    return 1;
}

void Sigma(int n, FILE *archivo){
    printf("{");
    fprintf(archivo, "{");

    int primero = 1;
    int cadena[1000];

    for(int nivel = 1; nivel <= n; nivel++){
        memset(cadena, 0, sizeof(int) *nivel);

        while(1){
            if(!primero){
                printf(", ");
                fprintf(archivo, ", ");
            }

            ImprimirCadena(cadena, nivel, archivo);
            primero = 0;

            if(Is1(cadena, nivel))
                break;
            
            Increase(cadena, nivel);
        }
    }

    printf("} \n");
    fprintf(archivo, "} \n");
}

int main(void){
    char res;

    do{
        int n; 
        char name_archivo[256];

        printf("Ingresa n: ");
        scanf("%d", &n);

        if(n < 0 || n > 1000){
            printf("Coloque n entre 0 y 1000");
        } else if(n == 0) {
            printf("Sigma(0) = {} \n");
        } else {
            printf("Nombre del archivo: ");
            scanf("%255s", name_archivo);
            strcat(name_archivo, ".txt");

            FILE *archivo = fopen(name_archivo, "w");
            if(archivo == NULL){
                printf("No se pudo crear el archivo '%s' \n", name_archivo);
            } else {
                Sigma(n, archivo);
                fclose(archivo);
                printf("Resultado en el archivo; '%s' \n", name_archivo);
            }
        }
        printf("Quiere ingresar otra n? (s/n) \n");
        scanf(" %c", &res);
    } while (res == 's' || res == 'S');
    printf("Se acabo \n");
    return 0;
}