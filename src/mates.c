#include "mates.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <math.h>

void agregarTextoLimitado(wchar_t *destino, size_t capacidad, const wchar_t *texto)
{
    size_t usados = wcslen(destino);
    if (usados >= capacidad - 1) return;
    wcsncat(destino, texto, capacidad - usados - 1);
}

void decimalAFraccion(double val, long long *num, long long *den)
{
    if (fabs(val) < 1e-9) { *num = 0; *den = 1; return; }
    double signo = (val < 0) ? -1.0 : 1.0;
    double v = fabs(val);
    long long p0 = 0, q0 = 1, p1 = 1, q1 = 0;
    double a = floor(v);
    while (q1 * a + q0 <= 10000) {
        long long p2 = p1 * (long long)a + p0;
        long long q2 = q1 * (long long)a + q0;
        p0 = p1; q0 = q1; p1 = p2; q1 = q2;
        if (fabs(v - a) < 1e-7) break;
        v = 1.0 / (v - a);
        a = floor(v);
    }
    *num = (long long)signo * p1;
    *den = q1;
}

Node* createNumNode(double val) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    n->type = NODE_NUM; n->val = val; return n;
}
Node* createVarNode(void) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    n->type = NODE_VAR; return n;
}
Node* createOpNode(NodeType type, Node *left, Node *right) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    n->type = type; n->left = left; n->right = right; return n;
}
Node* createFuncNode(NodeType type, Node *child) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    n->type = type; n->left = child; return n;
}
void freeNode(Node *n) {
    if (!n) return;
    freeNode(n->left); freeNode(n->right); free(n);
}
Node* copyNode(const Node *n) {
    if (!n) return NULL;
    Node *cp = (Node*)calloc(1, sizeof(Node));
    cp->type = n->type; cp->val = n->val;
    cp->left = copyNode(n->left); cp->right = copyNode(n->right);
    return cp;
}

static const wchar_t *g_src;
static int g_pos;
static int g_parseError;

Node* parseExpression(void);

static int empiezaFuncion(const wchar_t *nombre) {
    size_t n = wcslen(nombre);
    size_t restante = wcslen(&g_src[g_pos]);
    if (restante < n || wcsncmp(&g_src[g_pos], nombre, n) != 0) return 0;
    wchar_t siguiente = g_src[g_pos + n];
    return siguiente == L'(' || iswspace(siguiente) || siguiente == L'x' ||
           siguiente == L'X' || siguiente == L't' || siguiente == L'T';
}

static int empiezaFactor(void) {
    wchar_t c = g_src[g_pos];
    if (c == L'(' || c == L'x' || c == L'X' || c == L't' || c == L'T' ||
        iswdigit(c) || c == L'.' || c == L'e' || c == L'E') return 1;
    return empiezaFuncion(L"sin") || empiezaFuncion(L"sen") ||
           empiezaFuncion(L"cos") || empiezaFuncion(L"tan") ||
           empiezaFuncion(L"tg") || empiezaFuncion(L"ln") ||
           empiezaFuncion(L"log") || empiezaFuncion(L"sqrt") ||
           empiezaFuncion(L"raiz") || empiezaFuncion(L"arcsin") ||
           empiezaFuncion(L"arcsen") || empiezaFuncion(L"arctan") ||
           empiezaFuncion(L"arctg") || empiezaFuncion(L"exp");
}

Node* parseUnary(void);

Node* parsePrimary(void) {
    while (g_src[g_pos] != L'\0' && iswspace(g_src[g_pos])) g_pos++;
    if (g_src[g_pos] == L'(') {
        g_pos++;
        Node *n = parseExpression();
        while (g_src[g_pos] != L'\0' && iswspace(g_src[g_pos])) g_pos++;
        if (g_src[g_pos] != L')') { g_parseError = 1; freeNode(n); return NULL; }
        g_pos++;
        return n;
    }
    struct { const wchar_t *name; NodeType type; int len; } funcs[] = {
        {L"arcsin", NODE_ARCSIN, 6}, {L"arcsen", NODE_ARCSIN, 6},
        {L"arctan", NODE_ARCTAN, 6}, {L"arctg", NODE_ARCTAN, 5},
        {L"sqrt", NODE_SQRT, 4}, {L"raiz", NODE_SQRT, 4},
        {L"sin", NODE_SIN, 3}, {L"sen", NODE_SIN, 3},
        {L"cos", NODE_COS, 3}, {L"tan", NODE_TAN, 3},
        {L"tg", NODE_TAN, 2}, {L"ln", NODE_LN, 2},
        {L"log", NODE_LN, 3}, {L"exp", NODE_EXP, 3}
    };
    for (size_t i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        if (empiezaFuncion(funcs[i].name)) {
            g_pos += funcs[i].len;
            Node *arg = parseUnary();
            if (!arg) return NULL;
            return createFuncNode(funcs[i].type, arg);
        }
    }
    if (g_src[g_pos] == L'e' && g_src[g_pos+1] == L'^') {
        g_pos += 2;
        Node *arg = parseUnary();
        if (!arg) return NULL;
        return createFuncNode(NODE_EXP, arg);
    }
    if ((g_src[g_pos] == L'e' || g_src[g_pos] == L'E') &&
        !iswalpha(g_src[g_pos+1]) && g_src[g_pos+1] != L'^') {
        g_pos++; return createNumNode(exp(1.0));
    }
    if (g_src[g_pos] == L'x' || g_src[g_pos] == L'X' || g_src[g_pos] == L't' || g_src[g_pos] == L'T') {
        g_pos++; return createVarNode();
    }
    if (iswdigit(g_src[g_pos]) || g_src[g_pos] == L'.') {
        wchar_t *endPtr;
        double val = wcstod(&g_src[g_pos], &endPtr);
        if (endPtr == &g_src[g_pos] || !isfinite(val)) { g_parseError = 1; return NULL; }
        g_pos = (int)(endPtr - g_src);
        return createNumNode(val);
    }
    g_parseError = 1;
    return NULL;
}

Node* parsePower(void) {
    Node *left = parsePrimary();
    if (!left) return NULL;
    if (g_src[g_pos] == L'^') {
        g_pos++;
        Node *right = parseUnary();
        if (!right) { freeNode(left); return NULL; }
        return createOpNode(NODE_POW, left, right);
    }
    return left;
}

Node* parseUnary(void) {
    if (g_src[g_pos] == L'+') { g_pos++; return parseUnary(); }
    if (g_src[g_pos] == L'-') {
        g_pos++;
        Node *child = parseUnary();
        if (!child) return NULL;
        return createOpNode(NODE_SUB, createNumNode(0), child);
    }
    return parsePower();
}

Node* parseTerm(void) {
    Node *left = parseUnary();
    if (!left) return NULL;
    for (;;) {
        wchar_t op = g_src[g_pos];
        if (op == L'*' || op == L'/') {
            g_pos++;
            Node *right = parseUnary();
            if (!right) { freeNode(left); return NULL; }
            left = createOpNode(op == L'*' ? NODE_MUL : NODE_DIV, left, right);
        } else if (empiezaFactor()) {
            Node *right = parseUnary();
            if (!right) { freeNode(left); return NULL; }
            left = createOpNode(NODE_MUL, left, right);
        } else break;
    }
    return left;
}

Node* parseExpression(void) {
    Node *left = parseTerm();
    if (!left) return NULL;
    while (g_src[g_pos] == L'+' || g_src[g_pos] == L'-') {
        wchar_t op = g_src[g_pos++];
        Node *right = parseTerm();
        if (!right) { freeNode(left); return NULL; }
        left = createOpNode(op == L'+' ? NODE_ADD : NODE_SUB, left, right);
    }
    return left;
}

Node* buildAST(const wchar_t *expr) {
    g_src = expr; g_pos = 0; g_parseError = 0;
    Node *root = parseExpression();
    while (g_src[g_pos] != L'\0' && iswspace(g_src[g_pos])) g_pos++;
    if (g_parseError || g_src[g_pos] != L'\0') { freeNode(root); return NULL; }
    return root;
}

Node* simplifyAST(Node *n) {
    if (!n) return NULL;
    n->left = simplifyAST(n->left);
    n->right = simplifyAST(n->right);
    if ((n->type >= NODE_ADD && n->type <= NODE_POW) && (!n->left || !n->right)) return n;

    if (n->type == NODE_ADD) {
        if (n->left->type == NODE_NUM && n->left->val == 0) { Node *tmp = copyNode(n->right); freeNode(n); return tmp; }
        if (n->right->type == NODE_NUM && n->right->val == 0) { Node *tmp = copyNode(n->left); freeNode(n); return tmp; }
        if (n->left->type == NODE_NUM && n->right->type == NODE_NUM) { double v = n->left->val + n->right->val; freeNode(n); return createNumNode(v); }
    } else if (n->type == NODE_SUB) {
        if (n->left->type == NODE_NUM && n->left->val == 0) {
            if (n->right->type == NODE_SUB && n->right->left->type == NODE_NUM && n->right->left->val == 0) {
                Node *tmp = copyNode(n->right->right); freeNode(n); return tmp;
            }
        }
        if (n->right->type == NODE_NUM && n->right->val == 0) { Node *tmp = copyNode(n->left); freeNode(n); return tmp; }
        if (n->left->type == NODE_NUM && n->right->type == NODE_NUM) { double v = n->left->val - n->right->val; freeNode(n); return createNumNode(v); }
    } else if (n->type == NODE_MUL) {
        if ((n->left->type == NODE_NUM && n->left->val == 0) || (n->right->type == NODE_NUM && n->right->val == 0)) { freeNode(n); return createNumNode(0); }
        if (n->left->type == NODE_NUM && n->left->val == 1) { Node *tmp = copyNode(n->right); freeNode(n); return tmp; }
        if (n->right->type == NODE_NUM && n->right->val == 1) { Node *tmp = copyNode(n->left); freeNode(n); return tmp; }
        if (n->left->type == NODE_NUM && n->right->type == NODE_NUM) { double v = n->left->val * n->right->val; freeNode(n); return createNumNode(v); }
    } else if (n->type == NODE_DIV) {
        if (n->left->type == NODE_NUM && n->left->val == 0) { freeNode(n); return createNumNode(0); }
        if (n->right->type == NODE_NUM && n->right->val == 1) { Node *tmp = copyNode(n->left); freeNode(n); return tmp; }
    } else if (n->type == NODE_POW) {
        if (n->right->type == NODE_NUM && n->right->val == 0) { freeNode(n); return createNumNode(1); }
        if (n->right->type == NODE_NUM && n->right->val == 1) { Node *tmp = copyNode(n->left); freeNode(n); return tmp; }
    }
    return n;
}

void astToString(const Node *n, wchar_t *buf) {
    if (!n) return;
    wchar_t temp[256];
    switch (n->type) {
        case NODE_NUM:
            if (floor(n->val) == n->val) swprintf(temp, 256, L"%lld", (long long)n->val);
            else {
                long long num, den;
                decimalAFraccion(n->val, &num, &den);
                if (den != 1 && den <= 100) swprintf(temp, 256, L"(%lld/%lld)", num, den);
                else swprintf(temp, 256, L"%.4g", n->val);
            }
            agregarTextoLimitado(buf, TEXTO_CAP, temp); break;
        case NODE_VAR: agregarTextoLimitado(buf, TEXTO_CAP, L"x"); break;
        case NODE_ADD:
            astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L" + "); astToString(n->right, buf); break;
        case NODE_SUB:
            if (n->left->type == NODE_NUM && n->left->val == 0) {
                agregarTextoLimitado(buf, TEXTO_CAP, L"-");
                if (n->right->type == NODE_ADD || n->right->type == NODE_SUB) { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->right, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
                else astToString(n->right, buf);
            } else {
                astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L" - ");
                if (n->right->type == NODE_ADD || n->right->type == NODE_SUB) { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->right, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
                else astToString(n->right, buf);
            }
            break;
        case NODE_MUL:
            if (n->left->type == NODE_NUM && n->right->type == NODE_VAR) {
                if (n->left->val == 1) agregarTextoLimitado(buf, TEXTO_CAP, L"x");
                else if (n->left->val == -1) agregarTextoLimitado(buf, TEXTO_CAP, L"-x");
                else { astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L"x"); }
            } else {
                if (n->left->type == NODE_ADD || n->left->type == NODE_SUB) { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
                else astToString(n->left, buf);
                agregarTextoLimitado(buf, TEXTO_CAP, L"·");
                if (n->right->type == NODE_ADD || n->right->type == NODE_SUB) { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->right, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
                else astToString(n->right, buf);
            }
            break;
        case NODE_DIV:
            agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L") / ("); astToString(n->right, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_POW:
            if (n->left->type == NODE_VAR || n->left->type == NODE_NUM) astToString(n->left, buf);
            else { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
            agregarTextoLimitado(buf, TEXTO_CAP, L"^");
            if (n->right->type == NODE_VAR || n->right->type == NODE_NUM) astToString(n->right, buf);
            else { agregarTextoLimitado(buf, TEXTO_CAP, L"("); astToString(n->right, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); }
            break;
        case NODE_SIN: agregarTextoLimitado(buf, TEXTO_CAP, L"sin("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_COS: agregarTextoLimitado(buf, TEXTO_CAP, L"cos("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_TAN: agregarTextoLimitado(buf, TEXTO_CAP, L"tan("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_LN:  agregarTextoLimitado(buf, TEXTO_CAP, L"ln(");  astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_EXP: agregarTextoLimitado(buf, TEXTO_CAP, L"e^(");  astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_SQRT: agregarTextoLimitado(buf, TEXTO_CAP, L"sqrt("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_ARCSIN: agregarTextoLimitado(buf, TEXTO_CAP, L"arcsen("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
        case NODE_ARCTAN: agregarTextoLimitado(buf, TEXTO_CAP, L"arctg("); astToString(n->left, buf); agregarTextoLimitado(buf, TEXTO_CAP, L")"); break;
    }
}

Node* diffAST(const Node *n) {
    if (!n) return createNumNode(0);
    switch (n->type) {
        case NODE_NUM: return createNumNode(0);
        case NODE_VAR: return createNumNode(1);
        case NODE_ADD: return createOpNode(NODE_ADD, diffAST(n->left), diffAST(n->right));
        case NODE_SUB: return createOpNode(NODE_SUB, diffAST(n->left), diffAST(n->right));
        case NODE_MUL:
            return createOpNode(NODE_ADD,
                createOpNode(NODE_MUL, diffAST(n->left), copyNode(n->right)),
                createOpNode(NODE_MUL, copyNode(n->left), diffAST(n->right)));
        case NODE_DIV:
            return createOpNode(NODE_DIV,
                createOpNode(NODE_SUB,
                    createOpNode(NODE_MUL, diffAST(n->left), copyNode(n->right)),
                    createOpNode(NODE_MUL, copyNode(n->left), diffAST(n->right))
                ), createOpNode(NODE_POW, copyNode(n->right), createNumNode(2)));
        case NODE_POW:
            if (n->right->type == NODE_NUM) {
                double p = n->right->val;
                return createOpNode(NODE_MUL,
                    createOpNode(NODE_MUL, createNumNode(p), createOpNode(NODE_POW, copyNode(n->left), createNumNode(p - 1))),
                    diffAST(n->left));
            } else {
                return createOpNode(NODE_MUL, copyNode(n),
                    createOpNode(NODE_ADD,
                        createOpNode(NODE_MUL, diffAST(n->right), createFuncNode(NODE_LN, copyNode(n->left))),
                        createOpNode(NODE_MUL, copyNode(n->right), createOpNode(NODE_DIV, diffAST(n->left), copyNode(n->left)))));
            }
        case NODE_SIN: return createOpNode(NODE_MUL, createFuncNode(NODE_COS, copyNode(n->left)), diffAST(n->left));
        case NODE_COS: return createOpNode(NODE_MUL, createOpNode(NODE_SUB, createNumNode(0), createFuncNode(NODE_SIN, copyNode(n->left))), diffAST(n->left));
        case NODE_EXP: return createOpNode(NODE_MUL, createFuncNode(NODE_EXP, copyNode(n->left)), diffAST(n->left));
        case NODE_LN:  return createOpNode(NODE_DIV, diffAST(n->left), copyNode(n->left));
        case NODE_TAN:
            return createOpNode(NODE_DIV, diffAST(n->left),
                createOpNode(NODE_POW, createFuncNode(NODE_COS, copyNode(n->left)), createNumNode(2)));
        case NODE_SQRT:
            return createOpNode(NODE_DIV, diffAST(n->left),
                createOpNode(NODE_MUL, createNumNode(2), createFuncNode(NODE_SQRT, copyNode(n->left))));
        case NODE_ARCSIN:
            return createOpNode(NODE_DIV, diffAST(n->left),
                createFuncNode(NODE_SQRT, createOpNode(NODE_SUB, createNumNode(1),
                    createOpNode(NODE_POW, copyNode(n->left), createNumNode(2)))));
        case NODE_ARCTAN:
            return createOpNode(NODE_DIV, diffAST(n->left),
                createOpNode(NODE_ADD, createNumNode(1),
                    createOpNode(NODE_POW, copyNode(n->left), createNumNode(2))));
        default: break;
    }
    return NULL;
}

int contieneVariable(const Node *n) {
    if (!n) return 0;
    if (n->type == NODE_VAR) return 1;
    return contieneVariable(n->left) || contieneVariable(n->right);
}

double evaluarAST(const Node *n, double x, int *ok) {
    if (!n || !*ok) { *ok = 0; return NAN; }
    double a, b, v;
    switch (n->type) {
        case NODE_NUM: return n->val;
        case NODE_VAR: return x;
        case NODE_ADD: return evaluarAST(n->left,x,ok) + evaluarAST(n->right,x,ok);
        case NODE_SUB: return evaluarAST(n->left,x,ok) - evaluarAST(n->right,x,ok);
        case NODE_MUL: return evaluarAST(n->left,x,ok) * evaluarAST(n->right,x,ok);
        case NODE_DIV:
            a=evaluarAST(n->left,x,ok); b=evaluarAST(n->right,x,ok);
            if (!*ok || fabs(b)<1e-12) { *ok=0; return NAN; } return a/b;
        case NODE_POW:
            a=evaluarAST(n->left,x,ok); b=evaluarAST(n->right,x,ok);
            if (!*ok || (a<0 && fabs(b-round(b))>1e-10) || (a==0 && b<0)) { *ok=0; return NAN; }
            v=pow(a,b); if (!isfinite(v)) *ok=0; return v;
        default:
            a=evaluarAST(n->left,x,ok); if (!*ok) return NAN;
            switch(n->type) {
                case NODE_SIN: v=sin(a); break; case NODE_COS: v=cos(a); break;
                case NODE_TAN: if (fabs(cos(a))<1e-12) {*ok=0;return NAN;} v=tan(a); break;
                case NODE_LN: if (a<=0) {*ok=0;return NAN;} v=log(a); break;
                case NODE_EXP: v=exp(a); break;
                case NODE_SQRT: if (a<0) {*ok=0;return NAN;} v=sqrt(a); break;
                case NODE_ARCSIN: if (a<-1 || a>1) {*ok=0;return NAN;} v=asin(a); break;
                case NODE_ARCTAN: v=atan(a); break;
                default: *ok=0; return NAN;
            }
            if (!isfinite(v)) *ok=0;
            return v;
    }
}

int verificarDerivada(const Node *original, const Node *derivada) {
    const double puntos[] = {-1.37, -0.63, 0.41, 1.29};
    int validos=0;
    for (size_t i=0;i<sizeof(puntos)/sizeof(puntos[0]);i++) {
        double x=puntos[i], h=1e-5; int ok1=1,ok2=1,ok3=1;
        double fp=evaluarAST(original,x+h,&ok1), fm=evaluarAST(original,x-h,&ok2);
        double ds=evaluarAST(derivada,x,&ok3);
        if (!ok1 || !ok2 || !ok3) continue;
        double dn=(fp-fm)/(2*h);
        if (!isfinite(dn) || !isfinite(ds) || fabs(dn-ds)>2e-4*(1+fabs(dn))) return 0;
        validos++;
    }
    return validos>=2;
}

int extraerMonomioAvanzado(const Node *n, double *coef, double *exponente) {
    if (!n) return 0;
    *coef = 1.0;
    if (n->type == NODE_NUM) { *coef = n->val; *exponente = 0.0; return 1; }
    if (n->type == NODE_VAR) { *exponente = 1.0; return 1; }
    if (n->type == NODE_SQRT && n->left && n->left->type == NODE_VAR) { *exponente = 0.5; return 1; }
    if (n->type == NODE_POW && n->left && n->left->type == NODE_VAR) {
        if (n->right->type == NODE_NUM) { *exponente = n->right->val; return 1; }
        if (n->right->type == NODE_SUB && n->right->left->type == NODE_NUM && n->right->left->val == 0 && n->right->right->type == NODE_NUM) {
            *exponente = -n->right->right->val; return 1;
        }
    }
    if (n->type == NODE_DIV) {
        if (n->left->type == NODE_NUM && n->right->type == NODE_VAR) {
            *coef = n->left->val; *exponente = -1.0; return 1;
        }
        if (n->left->type == NODE_NUM && n->right->type == NODE_POW && n->right->left->type == NODE_VAR && n->right->right->type == NODE_NUM) {
            *coef = n->left->val; *exponente = -n->right->right->val; return 1;
        }
    }
    if (n->type == NODE_MUL) {
        const Node *pot = NULL;
        if (n->left && n->left->type == NODE_NUM) { *coef=n->left->val; pot=n->right; }
        else if (n->right && n->right->type == NODE_NUM) { *coef=n->right->val; pot=n->left; }
        if (!pot) return 0;
        return extraerMonomioAvanzado(pot, &(double){1.0}, exponente);
    }
    return 0;
}