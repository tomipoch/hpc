#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define MAX 100
#define TOL 1e-6

// Leer matriz desde archivo
void leer_matriz(FILE *archivo, double A[MAX][MAX], int *m, int *n) {
    fscanf(archivo, "%d %d", m, n);
    for (int i = 0; i < *m; i++)
        for (int j = 0; j < *n; j++)
            fscanf(archivo, "%lf", &A[i][j]);
}

// Transponer matriz
void transpuesta(double A[MAX][MAX], double At[MAX][MAX], int m, int n) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            At[j][i] = A[i][j];
}

// Multiplicar matrices
void multiplicar(double A[MAX][MAX], double B[MAX][MAX], double R[MAX][MAX], int r, int c, int p) {
    #pragma omp parallel for
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < p; j++) {
            double suma = 0.0;
            for (int k = 0; k < c; k++)
                suma += A[i][k] * B[k][j];
            R[i][j] = suma;
        }
    }
}

// Invertir matriz con Gauss-Jordan (sin paralelismo por estabilidad)
int invertir(double A[MAX][MAX], double inv[MAX][MAX], int n) {
    double temp;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            inv[i][j] = (i == j) ? 1.0 : 0.0;

    for (int i = 0; i < n; i++) {
        if (fabs(A[i][i]) < TOL) return 0;

        temp = A[i][i];
        for (int j = 0; j < n; j++) {
            A[i][j] /= temp;
            inv[i][j] /= temp;
        }

        for (int k = 0; k < n; k++) {
            if (k == i) continue;
            temp = A[k][i];
            for (int j = 0; j < n; j++) {
                A[k][j] -= A[i][j] * temp;
                inv[k][j] -= inv[i][j] * temp;
            }
        }
    }
    return 1;
}

// Escribir resultado
void escribir_salida(FILE *archivo, char tipo, double R[MAX][MAX], int rows, int cols) {
    fprintf(archivo, "%c\n", tipo);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(archivo, "%.6lf", R[i][j]);
            if (j < cols - 1) fprintf(archivo, " ");
        }
        fprintf(archivo, "\n");
    }
}

int main(int argc, char *argv[]) {
    const char *archivo_entrada = (argc > 1) ? argv[1] : "entrada.ent";

    FILE *entrada = fopen(archivo_entrada, "r");
    FILE *salida = fopen("salida_paralelo.sal", "w");

    if (!entrada || !salida) {
        fprintf(stderr, "Error abriendo archivos.\n");
        return 1;
    }

    int m, n;
    double A[MAX][MAX], At[MAX][MAX], AA[MAX][MAX], AAt[MAX][MAX];
    double Inv[MAX][MAX], Resultado[MAX][MAX];

    leer_matriz(entrada, A, &m, &n);
    transpuesta(A, At, m, n);

    double start = omp_get_wtime();

    if (m <= n) {
        multiplicar(A, At, AAt, m, n, m);
        if (!invertir(AAt, Inv, m)) {
            fprintf(salida, "-1\n");
            fclose(entrada);
            fclose(salida);
            return 0;
        }
        multiplicar(At, Inv, Resultado, n, m, m);
        escribir_salida(salida, 'R', Resultado, n, m);
    } else {
        multiplicar(At, A, AA, n, m, n);
        if (!invertir(AA, Inv, n)) {
            fprintf(salida, "-1\n");
            fclose(entrada);
            fclose(salida);
            return 0;
        }
        multiplicar(Inv, At, Resultado, n, n, m);
        escribir_salida(salida, 'L', Resultado, n, m);
    }

    double end = omp_get_wtime();
    printf("Tiempo: %.6f segundos\n", end - start);

    fclose(entrada);
    fclose(salida);
    return 0;
}