#ifndef MATES_H
#define MATES_H

#include <windows.h>
#include <math.h>
#include <stddef.h>

#define TEXTO_CAP 8192

typedef enum {
    NODE_NUM, NODE_VAR, NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV,
    NODE_POW, NODE_SIN, NODE_COS, NODE_TAN, NODE_LN, NODE_EXP,
    NODE_SQRT, NODE_ARCSIN, NODE_ARCTAN
} NodeType;

typedef struct Node {
    NodeType type;
    double val;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    const wchar_t *src;
    int pos;
    int error;
} ParserContext;

Node* createNumNode(double val);
Node* createVarNode(void);
Node* createOpNode(NodeType type, Node *left, Node *right);
Node* createFuncNode(NodeType type, Node *child);
void freeNode(Node *n);
Node* copyNode(const Node *n);

Node* buildAST(const wchar_t *expr);
Node* simplifyAST(Node *n);
Node* simplifyASTPuntoFijo(Node *n);
Node* diffAST(const Node *n);
void astToString(const Node *n, wchar_t *buf);
double evaluarAST(const Node *n, double x, int *ok);
int contieneVariable(const Node *n);
int verificarDerivada(const Node *original, const Node *derivada);
int extraerMonomioAvanzado(const Node *n, double *coef, double *exponente);
void decimalAFraccion(double val, long long *num, long long *den);
void agregarTextoLimitado(wchar_t *destino, size_t capacidad, const wchar_t *texto);

#endif