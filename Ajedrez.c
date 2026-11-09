#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NEGRO "\033[40m"
#define ROJO "\033[41m"
#define BLANCO "\033[97m" 
#define RESET "\033[0m"
#define AMARILLO "\033[43m"
#define CIAN "\033[46m"
#define TEXTO_NEGRO "\033[30m"
#define PAUSA_MS 700
#define TAM_CASILLA 4
#define MAX_VEC 8
#define MAX_SIM 15
#define MAX_CAD 100

#ifdef _WIN32
#include <windows.h>
#else 
#include <unistd.h> 
#endif 

typedef struct{
    int destinos[MAX_SIM][MAX_VEC];
    int conteo[MAX_SIM];
} Transiciones;

typedef struct{
    int nestados;
    int nsimbolos;
    char simbolos[16];
    int inicial;
    int *finales, nfinales;
    Transiciones *tabla;
} AFN;

typedef struct{
    AFN *afn;
    const char *cad;
    int camino[MAX_CAD + 1];
    int elegida[MAX_CAD + 1];
    int elegidaNo[MAX_CAD + 1];
    long total;
    long aceptadas;
    long noAceptadas;
    FILE *fAcep, *fNoAcep;
} Recorrido;

typedef struct{
    int id;
    int inicial, final;
    char cad[MAX_CAD];
    AFN afn;
    int ruta[MAX_CAD + 1];
    int largo;
    int aceptada;
    int paso;
    int terminado;
    int llego;
} Jugador;

void tablero(int n, int p1, int p2){
    int ancho = 2 * TAM_CASILLA;

    for(int i = 0; i < n; i++){
        for(int alto = 0; alto < TAM_CASILLA; alto++){
            for(int j = 0; j < n; j++){
                int num = i * n + j + 1;

                if(num == p1)
                    printf("%s%s", AMARILLO, TEXTO_NEGRO);
                else if (num == p2)
                    printf("%s%s", CIAN, TEXTO_NEGRO);
                else
                    printf("%s%s", (i + j) % 2 == 0 ? NEGRO : ROJO, BLANCO);
                
                if(alto == TAM_CASILLA / 2){
                    char texto[12];
                    int len;

                    if(num == p1)
                        len = snprintf(texto, sizeof texto, p2 ? "J1" : "K");
                    else if(num == p2)
                        len = snprintf(texto, sizeof texto, "J2");
                    else 
                        len = snprintf(texto, sizeof texto, "%d", num);

                    int izq = (ancho - len) / 2;
                    int der = ancho - len - izq;
                    if(izq < 0) izq = 0;
                    if(der < 0) der = 0; 
                    printf("%*s%s%*s", izq, "", texto, der, "");
                } else{
                    printf("%*s", ancho, "");
                }
            }
            printf("%s \n", RESET);
        }
    }
}

int colorCasilla(int n, int e){
    int i = (e - 1) / n;
    int j = (e - 1) % n;
    return (i + j) % 2;
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
    fprintf(f, "%-5s %-20s %s \n", "", "r", "b");
    
    for (int e = 1; e <= n * n; e++) {
        vecinasTexto(n, e, 1, r, sizeof r);
        vecinasTexto(n, e, 0, b, sizeof b);
        fprintf(f, "%-5d %-20s %s \n", e, r, b);
    }

    fclose(f);
    printf("Tabla guardada en %s \n", nombre);
    return 1;
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

int leerLista(char *texto, int lista[], int max){
    int cnt = 0;
    if(strcmp(texto, "-") == 0)
        return 0;

    for(char *tok = strtok(texto, ","); tok; tok = strtok(NULL, ",")){
        if(cnt == max)
            return -1;
        lista[cnt++] = atoi(tok);
    }
    return cnt;
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
    for(char *tok = strtok(linea, " \t\r\n"); tok && a->nsimbolos < MAX_SIM; tok = strtok(NULL, " \t\r\n"))
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

    a->tabla = calloc(a->nestados + 1, sizeof(Transiciones));
    if(a->tabla == NULL){
        printf("Sin memoria para la tabla \n");
        fclose(f);
        return 0;
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

        if(e < 1 || e > a->nestados){
            printf("Estado %d fuera de rango en %s \n", e, nombre);
            fclose(f);
            return 0;
        }

        for(int s = 0; s < a->nsimbolos; s++){
            int cnt = leerLista(col[s + 1], a->tabla[e].destinos[s], MAX_VEC);
            if(cnt < 0){
                printf("El estado %d tiene mas de %d desstinos en %s \n", e, MAX_VEC, nombre);
                fclose(f);
                return 0;
            }
            a->tabla[e].conteo[s] = cnt;
        }
    }

    fclose(f);

    a->inicial = inicial;
    a->nfinales = 1;
    a->finales = malloc(sizeof(int));
    a->finales[0] = final;
    return 1;
}

void fAFN(AFN *a){
    free(a->tabla);
    free(a->finales);
    a->tabla = NULL;
    a->finales = NULL;
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

/*
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
*/

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
            for(int x = 0; x < a->tabla[e].conteo[s]; x++){
                int d = a->tabla[e].destinos[s][x];
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
            rc->noAceptadas++;
            escribirRuta(rc->fNoAcep, rc->total, rc->cad, rc->camino, pos);

            if(rand() % rc->noAceptadas == 0)
                memcpy(rc->elegidaNo, rc->camino, (pos + 1) * sizeof(int));
        }
        return;
    }

    int s = pAlfabeto(a, rc->cad[pos]);
    for(int x = 0; x < a->tabla[e].conteo[s]; x++)
        caminos(rc, pos + 1, a->tabla[e].destinos[s][x]);
}

void imprimirRuta(const char *titulo, int ruta[], const char *cad, int largo){
    printf("%s: %d", titulo, ruta[0]);
    for(int k = 0; k < largo; k++)
        printf(" -%c-> %d", cad[k], ruta[k + 1]);
    printf("\n");
}

int prepararJugador(Jugador *j, const char *archAcep, const char *archNoAcep){
    printf("Jugador %d (de %d a %d)", j->id, j->inicial, j->final);

    if(!cAFN(&j->afn, "Diccionario.txt", j->inicial, j->final))
        return 0;

    manejoError(&j->afn, j->cad);

    red(&j->afn, j->cad);

    Recorrido rc = {0};
    rc.afn = &j->afn;
    rc.cad = j->cad;
    rc.fAcep = fopen(archAcep, "w");
    rc.fNoAcep = fopen(archNoAcep, "w");

    caminos(&rc, 0, j->inicial);
    fclose(rc.fAcep);
    fclose(rc.fNoAcep);

    printf("\n Rutas generadas: %ld \n", rc.total);
    printf("Aceptadas: %ld (%s) \n", rc.aceptadas, archAcep);
    printf("No Aceptadas: %ld (%s) \n", rc.noAceptadas, archNoAcep);

    j->largo = (int) strlen(j->cad);
    if(rc.aceptadas > 0){
        memcpy(j->ruta, rc.elegida, (j->largo + 1) * sizeof(int));
        j->aceptada = 1;
        imprimirRuta("\n Camino Elegido", j->ruta, j->cad, j->largo);
    }else{
        memcpy(j->ruta, rc.elegidaNo, (j->largo + 1) * sizeof(int));
        j->aceptada = 0;
        printf("\n No hay caminos validos para el jugador %d \n", j->id);
        imprimirRuta("\n Ruta no aceptada elegida al azar", j->ruta, j->cad, j->largo);
    }
    return 1;
}

void espera(int ms){
    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms * 1000);
    #endif
}

void animarUnJugador(int n, Jugador *j){
    int lineas = n * TAM_CASILLA + 1;

    printf("\n Camino elegido \n");
    for(int k = 0; k <= j->largo; k++){
        if(k > 0)
            printf("\033[%dA", lineas);
        
        int e = j->ruta[k];
        tablero(n, e, 0);

        if(k == 0)
            printf("Inicio: Casilla %d (%s)", e, colorCasilla(n, e) ? "roja" : "negra");
        else
            printf("Paso %d/%d '%c' -> casila %d (%s)", k, j->largo, j->cad[k - 1], e, colorCasilla(n, e) ? "roja" : "negra");
        printf("\033[K \n");

        fflush(stdout);
        espera(PAUSA_MS);
    }
    printf("Llego a la casilla final %d \n", j->ruta[j->largo]);
}

int posicion(Jugador *j){
    return j->ruta[j->paso];
}

void cuadroJuego(int n, Jugador *j1, Jugador *j2, const char *evento, int primero){
    int lineas = n * TAM_CASILLA + 2;

    if(!primero)
        printf("\033[%dA", lineas);
    
    tablero(n, posicion(j1), posicion(j2));
    printf("%s\033[K \n", evento);
    printf("J1: paso %d/%d (casilla %d)%s J2: paso %d/%d (casilla %d)%s \033[K \n", 
            j1->paso, j1->largo, posicion(j1), j1->terminado ? " [termino]" : "",
            j2->paso, j2->largo, posicion(j2), j2->terminado ? " [termino]" : "");

    fflush(stdout);
    espera(PAUSA_MS);
}

void jugarDosJugadores(int n, Jugador *j1, Jugador *j2){
    Jugador *jug[2] = {j1, j2};
    int turno = 0;
    int numTurno = 0;
    int sinMovimiento = 0;
    int bloqueo = 0;
    int ganador = 0;
    char evento[128];

    for(int t = 0; t < 2; t++){
        jug[t]->paso = 0;
        jug[t]->terminado = (jug[t]->largo == 0);
        jug[t]->llego = jug[t]->terminado && posicion(jug[t]) == jug[t]->final;
    }

    printf("\n Juego de dos Jugadores (J1 Amarillo, J2 Cian): \n");
    snprintf(evento, sizeof evento, "Inicio: J1 en %d, J2 en %d", posicion(j1), posicion(j2));
    cuadroJuego(n, j1, j2, evento, 1);

    while(!(j1->terminado && j2->terminado) && !bloqueo){
        Jugador *yo = jug[turno];
        Jugador *otro = jug[1 - turno];

        if(yo->terminado){
            turno = 1 - turno;
            continue;
        }

        numTurno++;
        char letra = yo->cad[yo->paso];
        int destino = yo->ruta[yo->paso + 1];

        if(destino == posicion(otro)){
            sinMovimiento++;
            snprintf(evento, sizeof evento, "Turno %d - J%d: '%c' -> casilla %d OCUPADA por J%d, pierde el turno", numTurno, yo->id, letra, destino, otro->id);

            if(otro->terminado || sinMovimiento >= 2)
                bloqueo = 1;
        } else{
            yo->paso++;
            sinMovimiento = 0;
            snprintf(evento, sizeof evento, "Turno %d - J%d: '%c' -> casilla %d (%s)", numTurno, yo->id, letra, destino, colorCasilla(n, destino) ? "roja" : "negra");

            if(yo->paso == yo->largo){
                yo->terminado = 1;
                yo->llego = (posicion(yo) == yo->final);
                if(yo->llego && ganador == 0)
                    ganador = yo->id;
            }
        }
        cuadroJuego(n, j1, j2, evento, 0);
        turno = 1 - turno; 
    }

    printf("\n Resultado \n");
    if(bloqueo)
        printf("El juego termino por bloqueo");
    for(int t = 0; t < 2; t++){
        Jugador *j = jug[t];
        printf("J%d: %s en la casilla %d (su final es: %d) \n", j->id, j->llego ? "llego" : "no llego", posicion(j), j->final);
    }
    if(ganador)
        printf("Ganador: Jugador %d \n", ganador);
    else
        printf("Sin ganador \n");
}

void modoUnJugador(int n){
    Jugador j1 = {0};
    j1.id = 1;
    j1.inicial = 1;
    j1.final = n * n;

    printf("Cadena de movimientos del jugador 1 (r/b): ");
    if(scanf("%99s", j1.cad) !=1){
        printf("Cadena invalida \n");
        return;
    }
    if(prepararJugador(&j1, "Aceptadas.txt", "No_Aceptadas.txt") && j1.aceptada)
        animarUnJugador(n, &j1);
    
    fAFN(&j1.afn);
}

void modoDosJugadores(int n){
    Jugador j1 = {0}, j2 = {0};
    
    j1.id = 1;
    j1.inicial = 1;
    j1.final = n*n;

    j2.id = 2;
    j2.inicial = n;
    j2.final = n * (n - 1) + 1;

    printf("Cadena de movimientos del jugador 1 (r/b): ");
    if(scanf("%99s", j1.cad) != 1){
        printf("cadena invalida");
        return;
    }
    printf("Cadena de movimientos del jugador 2 (r/b)");
    if(scanf("%99s", j2.cad) != 1){
        printf("cadena invalida");
        return;
    }

    if(prepararJugador(&j1, "Aceptadas_J1.txt", "NoAceptadas_J1.txt") &&
        prepararJugador(&j2, "Aceptadas_J2.txt", "NoAceptadas_J2.txt"))
            jugarDosJugadores(n, &j1, &j2);
    
    fAFN(&j1.afn);
    fAFN(&j2.afn);
}

int main(void){
    int n, opcion;

    srand((unsigned) time(NULL));

    printf("Tamanio del tablero (n): ");
    if(scanf("%d", &n) !=1 || n <= 0){
        printf("n invalido");
        return 1;
    }

    tablero (n, 0, 0);
    printf("\n");

    if(!Diccionario(n, "Diccionario.txt"))
        return 1;

    printf("\n 1) Un jugador \n");
    printf("\n 2) Dos jugadores \n");
    printf("\n Opcion: \n");
    if(scanf("%d", &opcion) != 1){
        printf("Opcion invalida \n");
        return 1;
    }

    switch(opcion){
        case 1: 
            modoUnJugador(n);
            break;
        case 2:
            if(n < 3)
                printf("Para dos jugadores el tablero debe de ser al menos 3x3 \n");
            else
                modoDosJugadores(n);
            break;
        default: 
            printf("Opcion invalida \n");
            break;
    }
    return 0;
}
