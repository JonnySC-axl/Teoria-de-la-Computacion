#include <stdio.h>

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
            int ni = 1 + di, nj = j + dj;
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

int main(void){
    int n;
    printf("Tamanio del tablero (n): ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("n invalido \n");
        return 1;
    }
    tablero(n);
    subconjuntos(n);
    return 0;
}