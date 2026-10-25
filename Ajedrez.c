#include <stdio.h>
#include <string.h>

#define NEGRO "\033[40m"
#define ROJO "\033[41m"
#define BLANCO "\033[97m" 
#define RESET "\033[0m"
#define TAM_CASILLA 4

void tablero(int n){
    int ancho = 2 * TAM_CASILLA;

    for(int i = 0; i < n; i++){
        for(int alto = 0; alto < TAM_CASILLA; alto++){
            for(int j = 0; j < n; j++){
                printf("%s%s", (i + j) % 2 == 0 ? NEGRO: ROJO, BLANCO);

                if(alto == TAM_CASILLA / 2){
                    int num = i * n + j + 1;
                    char texto[12];
                    int len = snprintf(texto, sizeof texto, "%d", num);
                    int izq = (ancho - len) / 2;
                    int der = ancho - len - izq;
                    if(izq < 0) izq = 0;
                    if(der < 0) der = 0;
                    printf("%*s%s%*s", izq, "", texto, der, "");
                } else {
                    printf("%*s", ancho, "");
                }
            }
            printf("%s \n", RESET);
        }
    }
}

int vecinas(int n, int e, int color, int dest[]){
    int i = (e - 1) / n, j = (e - 1) % n, cnt = 0;
    for(int di = -1; di <= 1; di++){
        for(int dj = -1; dj <= 1; dj++){
            if(di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if(ni < 0 || ni >= n || nj < 0 || nj >= n) continue;
            if((ni + nj) % 2 == color) dest[cnt++] = ni * n + nj + 1;
        }
    }
    return cnt;
}

void imprimirVecinas(int n, int i, int j, int color){
    int dest[8];
    int cnt = vecinas(n, i * n + j + 1, color, dest);
    for(int x = 0; x < cnt; x++)
        printf(x ? ",%d" : "%d", dest[x]);
}

void subconjuntos(int n) {
    printf("%-4s %-12s %-12s\n", "", "r", "b");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%-4d ", i * n + j + 1);
            imprimirVecinas(n, i, j, 1);   
            printf("\t\t");
            imprimirVecinas(n, i, j, 0);  
            printf("\n");
        }
    }
}

void imprimirConjunto(int total, int conj[]){
    int primero = 1;
    printf("{");
    for(int e = 1; e <= total; e++){
        if(conj[e]){
            printf(primero ? "%d" : ",%d", e);
            primero = 0;
        }
    }
    printf("} \n");
}

void red(int n, const char *cad){
    int total = n * n, dest[8];
    int actual[total + 1], sig[total + 1];

    for(int e = 0; e <= total; e++) 
        actual[e] = 0;
    actual[1] = 1;

    printf("Inicio; ");
    imprimirConjunto(total, actual);

    for(int k = 0; cad[k]; k++){
        int color = cad[k] == 'r' ? 1 : 0;
        for(int e = 0; e <= total; e++)
            sig[e] = 0;

        printf("\n Con '%c': \n", cad[k]);
        for(int e = 1; e<= total; e++){
            if(!actual[e])
                continue;
            int cnt = vecinas(n, e, color, dest);
            printf(" %d -> ", e);
            for(int x = 0; x < cnt; x++){
                printf(x ? ",%d" : "%d", dest[x]);
                sig[dest[x]] = 1;
            }
            printf("\n");
        }
        for(int e = 0; e <= total; e++)
            actual[e] = sig[e];
        printf("Estados: ");

        imprimirConjunto(total, actual);
    }
    if(actual[total])
        printf("\n Aceptada:  Se llego a %d \n", total);
    else
        printf("\n No aceptada: No se llego a %d \n", total);
}

void caminos(int n, const char *cad, int pos, int e, int camino[]){
    camino[pos] = e;
    if(cad[pos] == '\0'){
        if(e == n * n){
            printf("%d", camino[0]);
            for(int k = 0; k < pos; k++)
                printf(" -%c-> %d", cad[k], camino[k + 1]);
            printf("\n");
        }
        return;
    }
    int dest[8];
    int cnt = vecinas(n, e, cad[pos] == 'r' ? 1 : 0, dest);
    for(int x = 0; x < cnt; x++)
        caminos(n, cad, pos + 1, dest[x], camino);
}

int main(void){
    int n;
    char cad[100];

    printf("Tamanio del tablero (n): ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("n invalido \n");
        return 1;
    }

    printf("Cadena de movimientos (r/b): ");
    if(scanf("%99s", cad) != 1){
        printf("Cadena invalida \n");
        return 1;
    }

    for(int k = 0; cad[k]; k++){
        if(cad[k] != 'r' && cad[k] != 'b'){
            printf("Solo r y b \n");
            return 1;
        }
    }

    tablero(n);
    printf("\n");
    subconjuntos(n);
    printf("\n");
    red(n, cad);

    int camino[strlen(cad) + 1];
    printf("\n Caminos aceptados: \n");
    caminos(n, cad, 0, 1, camino);
    return 0;
}
