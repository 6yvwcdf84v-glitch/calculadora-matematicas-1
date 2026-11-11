#ifndef MATRIZ_H
#define MATRIZ_H

#include <wchar.h>

#define MAX_FILAS 10
#define MAX_COLS  10

typedef struct {
    int filas;
    int cols;
    double datos[MAX_FILAS][MAX_COLS];
} Matriz;

double calcularDeterminante(Matriz m);
int calcularTraspuesta(Matriz in, Matriz *out);
int calcularInversa(Matriz in, Matriz *out);
void matrizToString(Matriz m, wchar_t *buf);

#endif