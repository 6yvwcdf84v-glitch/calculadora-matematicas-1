#include "matriz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void submatriz(Matriz in, Matriz *out, int filaExcluir, int colExcluir) {
    out->filas = in.filas - 1;
    out->cols = in.cols - 1;
    int r = 0;
    for (int i = 0; i < in.filas; i++) {
        if (i == filaExcluir) continue;
        int c = 0;
        for (int j = 0; j < in.cols; j++) {
            if (j == colExcluir) continue;
            out->datos[r][c] = in.datos[i][j];
            c++;
        }
        r++;
    }
}

double calcularDeterminante(Matriz m) {
    if (m.filas != m.cols) return NAN;
    if (m.filas == 1) return m.datos[0][0];
    if (m.filas == 2) return m.datos[0][0] * m.datos[1][1] - m.datos[0][1] * m.datos[1][0];

    double det = 0.0;
    for (int j = 0; j < m.cols; j++) {
        Matriz sub;
        submatriz(m, &sub, 0, j);
        double signo = (j % 2 == 0) ? 1.0 : -1.0;
        det += signo * m.datos[0][j] * calcularDeterminante(sub);
    }
    return det;
}

int calcularTraspuesta(Matriz in, Matriz *out) {
    out->filas = in.cols;
    out->cols = in.filas;
    for (int i = 0; i < in.filas; i++) {
        for (int j = 0; j < in.cols; j++) {
            out->datos[j][i] = in.datos[i][j];
        }
    }
    return 1;
}

void matrizToString(Matriz m, wchar_t *buf) {
    wchar_t temp[128];
    buf[0] = L'\0';
    for (int i = 0; i < m.filas; i++) {
        swprintf(temp, 128, L"│ ");
        wcscat(buf, temp);
        for (int j = 0; j < m.cols; j++) {
            swprintf(temp, 128, L"%6.2f ", m.datos[i][j]);
            wcscat(buf, temp);
        }
        swprintf(temp, 128, L"│\r\n");
        wcscat(buf, temp);
    }
}
int calcularInversa(Matriz m, Matriz *inv) {
    if (m.filas != m.cols || m.filas == 0) return 0; // Solo cuadradas
    int n = m.filas;
    double a[MAX_FILAS][MAX_COLS * 2];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = m.datos[i][j];
            a[i][j + n] = (i == j) ? 1.0 : 0.0; // Matriz identidad adjunta
        }
    }

    for (int i = 0; i < n; i++) {
        int maxRow = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(a[k][i]) > fabs(a[maxRow][i])) maxRow = k;
        }
        if (fabs(a[maxRow][i]) < 1e-12) return 0; // Determinante cero (singular)

        if (maxRow != i) {
            for (int j = 0; j < 2 * n; j++) {
                double temp = a[i][j];
                a[i][j] = a[maxRow][j];
                a[maxRow][j] = temp;
            }
        }
        double pivote = a[i][i];
        for (int j = 0; j < 2 * n; j++) a[i][j] /= pivote;

        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = a[k][i];
                for (int j = 0; j < 2 * n; j++) a[k][j] -= factor * a[i][j];
            }
        }
    }

    inv->filas = n; inv->cols = n;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) inv->datos[i][j] = a[i][j + n];
    }
    return 1;
}