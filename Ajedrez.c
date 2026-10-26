#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NEGRO "\033[40m"
#define ROJO "\033[41m"
#define BLANCO "\033[97m" 
#define RESET "\033[0m"
#define TAM_CASILLA 4
#define MAX_VEC 8
#define MAX_CAD 100

typedef struct{
    int nestados;
    int nsimbolos;
    char simbolos[16];
    int inicial;
    int *finales, nfinales;
    int ***tabla;
    int **conteo;
} AFN;

typedef struct{
    AFN *afn;
    const char *cad;
    int camino[MAX_CAD + 1];
    int elegida[MAX_CAD + 1];
    long total;
    long aceptadas;
    FILE *fAcep, *fNoAcep;
} Recorrido;

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

void vecinasTexto(int n, int e, int color, char *buf, int tam){
    int dest[8], pos = 0;
    int cnt = vecinas(n, e, color, dest);
    if(cnt == 0){
        snprintf(buf, tam, "-");
        return;
    }
    for(int x = 0; x < cnt; x++)
        pos += snprintf(buf + pos, tam - pos, x ? ",%d" : "%d", dest[x]);
}

int Diccionario(int n, const char *nombre) {
    FILE *f = fopen(nombre, "w");
    if(f == NULL){
        printf("No se pudo crear el archivo %s \n", nombre);
        return 0;
    }

    char r[64], b[64];
    fprintf(f, "%-4s %-12s %-12s\n", "", "r", "b");
    
    for (int e = 1; e < n * n; e++) {
        vecinasTexto(n, e, 1, r, sizeof r);
        vecinasTexto(n, e, 0, b, sizeof b);
        fprintf(f, "%-5d %-20s %s \n", e, r, b);
    }

    fclose(f);
    printf("Tabla guardada en %s \n", nombre);
}

int pAlfabeto(AFN *a, char c){
    for(int i = 0; i < a->nsimbolos; i++)
        if(a->simbolos[i] == c)
            return i;
        return -1;
}

int edoFinal(AFN *a, int estado){
    for(int i = 0; i < a->nfinales; i++)
        if(a->finales[i] == estado)
            return 1;
    return 0;
}

int *leerLista(char *texto, int *cnt){
    *cnt = 0;
    if(strcmp(texto, "-") == 0)
        return NULL;
    
    int cap = 1;
    for(char *p = texto; *p; p++)
        if(*p == ',')
            cap++;
    int *lista = malloc(cap * sizeof(int));
    for(char *tok = strtok(texto, ","); tok; tok = strtok(NULL, ","))
        lista[(*cnt)++] = atoi(tok);
    return lista;
}

int cAFN(AFN *a, const char *nombre, int inicial, int final){
    memset(a, 0, sizeof *a);

    FILE *f = fopen(nombre, "r");
    if(f == NULL){
        printf("No se puede crear el archivo %s \n", nombre);
        return 0; 
    }
    char linea[512];

    if(!fgets(linea, sizeof linea, f)){
        printf("%s esta vacio \n", nombre);
        fclose(f);
        return 0;
    }
    for(char *tok = strtok(linea, " \t\r\n"); tok && a->nsimbolos < 15; tok = strtok(NULL, " \t\r\n"))
        a->simbolos[a->nsimbolos++] = tok[0];
    a->simbolos[a->nsimbolos] = '\0';

    int e;
    while(fgets(linea, sizeof linea, f))
        if(sscanf(linea, "%d", &e) == 1 && e > a->nestados)
            a->nestados = e;
    
    if(a->nsimbolos == 0 || a->nestados == 0){
        printf("Formato invalido en %s \n", nombre);
        fclose(f);
        return 0;
    }

    a->tabla = malloc((a->nestados + 1) * sizeof(int **));
    a->conteo = malloc((a->nestados + 1) * sizeof(int *));
    for(int i = 0; i <= a->nestados; i++){
        a->tabla[i] = calloc(a->nsimbolos, sizeof(int *));
        a->conteo[i] = calloc(a->nsimbolos, sizeof(int));
    }

    rewind(f);
    fgets(linea, sizeof linea, f);
    while(fgets(linea, sizeof linea, f)){
        char *col[16];
        int ncol = 0;
        for(char *tok = strtok(linea, " \t\r\n"); tok && ncol < 16; tok = strtok(NULL, " \t\r\n"))
            col[ncol++] = tok;

        if(ncol == 0)
            continue;
        if(ncol != a->nsimbolos + 1){
            printf("Fila con columnas incorrectas en %s", nombre);
            fclose(f);
            return 0;
        }
        e = atoi(col[0]);
        for(int s = 0; s < a->nsimbolos; s++)
            a->tabla[e][s] = leerLista(col[s + 1], &a->conteo[e][s]);
    }
    fclose(f);

    a->inicial = inicial;
    a->nfinales = 1;
    a->finales = malloc(sizeof(int));
    a->finales[0] = final;
    return 1;
}

void fAFN(AFN *a){
    if(a->tabla){
        for(int e = 0; e <= a->nestados; e++){
            for(int s = 0; s < a->nsimbolos; s++)
                free(a->tabla[e][s]);
            free(a->tabla[e]);
            free(a->conteo[e]);
        }
    }
    free(a->tabla);
    free(a->conteo);
    free(a->finales);
}

int manejoError(AFN *a, char *cadena){
    int error = 0, nuevo = 0;
    for(int i = 0; cadena[i]; i++){
        if(pAlfabeto(a, cadena[i]) != -1)
            cadena[nuevo++] = cadena[i];
        else {
            printf("'%c' invalido \n", cadena[i]);
            error = 1;
        }
    }
    cadena[nuevo] = '\0';
    return error;
}

void mostrarTabla(AFN *a){
    printf("%-5s", "");
    for(int s = 0; s < a->nsimbolos; s++)
        printf(" %-20c", a->simbolos[s]);
    printf("\n");

    for(int e = 1; e <= a->nestados; e++){
        printf("%-5d", e);
        for(int s = 0; s < a->nsimbolos; s++){
            char buf[64] = "";
            int pos = 0;
            for(int x = 0; x < a->conteo[e][s]; x++)
                pos += snprintf(buf + pos, sizeof buf - pos, x ? ",%d" : "%d", a->tabla[e][s][x]);
            printf(" %-20s", buf);
        }
        printf("\n");
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

void red(AFN *a, const char *cad){
    int total = a->nestados;
    int actual[total + 1], sig[total + 1];

    for(int e = 0; e <= total; e++)
        actual[e] = 0;
    actual[a->inicial] = 1;

    printf("Inicio: ");
    imprimirConjunto(total, actual);

    for(int k = 0; cad[k]; k++){
        int s = pAlfabeto(a, cad[k]);
        for(int e = 0; e <= total; e++)
            sig[e] = 0;

        printf("\n Con '%c': \n", cad[k]);
        for(int e = 1; e <= total; e++){
            if(!actual[e])
                continue;
            printf(" %d -> ", e);
            for(int x = 0; x < a->conteo[e][s]; x++){
                int d = a->tabla[e][s][x];
                printf(x ? ",%d" : "%d", d);
                sig[d] = 1;
            }
            printf("\n");
        }
        for(int e = 0; e <= total; e++)
            actual[e] = sig[e];
        printf("Estados: ");
        imprimirConjunto(total, actual);
    }

    int acepta = 0;
    for(int e = 1; e <= total; e++)
        if(actual[e] && edoFinal(a, e))
            acepta = 1;
    printf(acepta ? "\n Aceptada: se llego a un estado final \n"
                    : "\n No aceptada: no se llego a un estado final \n");
}
void escribirRuta(FILE *f, long indice, const char *cad, int camino[], int largo){
    fprintf(f, "%ld: %d", indice, camino[0]);
    for(int k = 0; k < largo; k++)
        fprintf(f, " -%c-> %d", cad[k], camino[k + 1]);
    fprintf(f, "\n");
}

void caminos(Recorrido *rc, int pos, int e){
    AFN *a = rc->afn;
    rc->camino[pos] = e;

    if(rc->cad[pos] == '\0'){
        rc->total++;
        if(edoFinal(a, e)){
            rc->aceptadas++;
            escribirRuta(rc->fAcep, rc->total, rc->cad, rc->camino, pos);

            if(rand() % rc->aceptadas == 0)
                memcpy(rc->elegida, rc->camino, (pos + 1) * sizeof(int));
        } else {
            escribirRuta(rc->fNoAcep, rc->total, rc->cad, rc->camino, pos);
        }
        return;
    }

    int s = pAlfabeto(a, rc->cad[pos]);
    for(int x = 0; x < a->conteo[e][s]; x++)
        caminos(rc, pos + 1, a->tabla[e][s][x]);
}

int main(void){
    int n;
    char cad[MAX_CAD];

    srand((unsigned) time(NULL));

    printf("Tamanio del tablero (n): ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("n invalido \n");
        return 1;
    }

    int inicial = 1;
    int final = n * n; 

    printf("Cadena de movimientos (r/b): ");
    if(scanf("%99s", cad) != 1){
        printf("Cadena invalida \n");
        return 1;
    }

    tablero(n);
    printf("\n");

    if(!Diccionario(n, "Diccionario.txt"))
        return 1;

    AFN afn;
    if(!cAFN(&afn, "Diccionario.txt", inicial, final)){
        fAFN(&afn);
        return 1;
    }

    printf("\n AFN cargado: %d estados, alfabetp [%s], incial %d, final", afn.nestados, afn.simbolos, afn.inicial);

    for(int i = 0; i< afn.nfinales; i++)
        printf(i ? ",%d" : "%d", afn.finales[i]);
    printf("\n\n");
    mostrarTabla(&afn);
    printf("\n");

    manejoError(&afn, cad);
    if(strlen(cad) > 12)
        printf("Archivos de ruta muy grandes \n");

    red(&afn, cad);

    Recorrido rc = {0};
    rc.afn = &afn;
    rc.cad = cad;
    rc.fAcep = fopen("Aceptadas.txt", "w");
    rc.fNoAcep = fopen("No_Aceptadas.txt", "w");
    if(rc.fAcep == NULL || rc.fNoAcep == NULL){
        printf("No se pudieron crear los archivos con las rutas \n");
        if(rc.fAcep) fclose(rc.fAcep);
        if(rc.fNoAcep) fclose(rc.fNoAcep);
        fAFN(&afn);
        return 1;
    }

    caminos(&rc, 0, afn.inicial);
    fclose(rc.fAcep);
    fclose(rc.fNoAcep);

    printf("\n Rutas generadas: %ld \n", rc.total);
    printf("Aceptadas: %ld (Aceptadas.txt) \n", rc.aceptadas);
    printf("No Aceptadas: %ld (No_Aceptadas.txt) \n", rc.total - rc.aceptadas);

    if(rc.aceptadas == 0){
        printf("\nNo hay caminos validos\n");
    } else {
        int largo = (int) strlen(cad);
        printf("\nCamino elegido: %d", rc.elegida[0]);
        for(int k = 0; k < largo; k++)
            printf(" -%c-> %d", cad[k], rc.elegida[k + 1]);
        printf("\n");
    }

    fAFN(&afn);
    return 0;
}
