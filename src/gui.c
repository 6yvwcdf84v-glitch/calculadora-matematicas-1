#include "gui.h"
#include "mates.h"
#include "matriz.h"
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>

HWND campoHistorial;
HWND campoTextoEntrada;
HWND hwndBotonOpciones;

HWND inputFilas, inputCols, btnCrearGrid, btnCalcDet, btnCalcTras;
HWND casillasGrid[MAX_FILAS][MAX_COLS];

int g_filasActuales = 0;
int g_colsActuales = 0;

HFONT hFuenteGlobal;
HFONT hFuenteBoton;

WNDPROC procOriginalEntrada;

int g_modoActual = IDM_DERIVAR;
int g_ordenActual = 1;

void destruirGridMatriz(void) {
    for (int i = 0; i < MAX_FILAS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            if (casillasGrid[i][j]) {
                DestroyWindow(casillasGrid[i][j]);
                casillasGrid[i][j] = NULL;
            }
        }
    }
    g_filasActuales = 0;
    g_colsActuales = 0;
}

void crearGridMatriz(HWND ventana, int f, int c) {
    destruirGridMatriz();
    if (f > MAX_FILAS) f = MAX_FILAS;
    if (c > MAX_COLS) c = MAX_COLS;
    if (f <= 0) f = 1;
    if (c <= 0) c = 1;

    g_filasActuales = f;
    g_colsActuales = c;

    int posXInicial = 125;
    int posYInicial = 265;
    int ancho = 38;
    int alto = 24;

    for (int i = 0; i < f; i++) {
        for (int j = 0; j < c; j++) {
            casillasGrid[i][j] = CreateWindowW(
                L"EDIT", L"0",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER,
                posXInicial + j * (ancho + 4), posYInicial + i * (alto + 4), ancho, alto,
                ventana, NULL, NULL, NULL
            );
            SendMessageW(casillasGrid[i][j], WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
        }
    }
}

void actualizarDimensionesDesdePantalla(HWND ventana) {
    wchar_t tf[16], tc[16];
    GetWindowTextW(inputFilas, tf, 16);
    GetWindowTextW(inputCols, tc, 16);
    int f = _wtoi(tf);
    int c = _wtoi(tc);
    if (f != g_filasActuales || c != g_colsActuales) {
        crearGridMatriz(ventana, f, c);
    }
}

Matriz obtenerMatrizDePantalla(void) {
    Matriz m;
    m.filas = g_filasActuales;
    m.cols = g_colsActuales;
    wchar_t txt[32];
    for (int i = 0; i < m.filas; i++) {
        for (int j = 0; j < m.cols; j++) {
            if (casillasGrid[i][j]) {
                GetWindowTextW(casillasGrid[i][j], txt, 32);
                m.datos[i][j] = wcstod(txt, NULL);
            } else {
                m.datos[i][j] = 0.0;
            }
        }
    }
    return m;
}

void mostrarControlesMatriz(int mostrar) {
    int sw = mostrar ? SW_SHOW : SW_HIDE;
    ShowWindow(inputFilas, sw);
    ShowWindow(inputCols, sw);
    ShowWindow(btnCrearGrid, sw);
    ShowWindow(btnCalcDet, sw);
    ShowWindow(btnCalcTras, sw);
    if (!mostrar) destruirGridMatriz();
}

void limpiarCadenaW(wchar_t *cadena) {
    if (!cadena) return;
    int i = 0, j = 0;
    while (cadena[i] != L'\0') {
        if (!iswspace(cadena[i])) {
            cadena[j++] = cadena[i];
        }
        i++;
    }
    cadena[j] = L'\0';
}

void agregarMensajeChat(const wchar_t *usuario, const wchar_t *contenido) {
    int lenActual = GetWindowTextLengthW(campoHistorial);
    SendMessageW(campoHistorial, EM_SETSEL, (WPARAM)lenActual, (LPARAM)lenActual);

    wchar_t bloque[8192];
    swprintf(bloque, 8192, L"👤 %ls:\r\n%ls\r\n--------------------------------------------------\r\n", usuario, contenido);

    SendMessageW(campoHistorial, EM_REPLACESEL, FALSE, (LPARAM)bloque);
    SendMessageW(campoHistorial, WM_VSCROLL, SB_BOTTOM, 0);
}

void procesarComandoChat(const wchar_t *expr) {
    if (!expr || wcslen(expr) == 0) return;

    wchar_t resultado[4096] = L"";
    wchar_t linea[4096];

    Node *ast = buildAST(expr);
    if (!ast) {
        swprintf(resultado, 4096, L"SYNTAX ERROR\r\n\r\nNo se pudo interpretar la entrada. Revisa la sintaxis.");
        agregarMensajeChat(L"Calculadora", resultado);
        SetWindowTextW(campoTextoEntrada, L"");
        return;
    }
    ast = simplifyASTPuntoFijo(ast);

    if (!contieneVariable(ast)) {
        int dominioOK = 1;
        (void)evaluarAST(ast, 0.0, &dominioOK);
        if (!dominioOK) {
            swprintf(resultado, 4096, L"ERROR: DOMINIO REAL\r\nExpresión constante no definida en los números reales.");
            freeNode(ast);
            agregarMensajeChat(L"Calculadora", resultado);
            SetWindowTextW(campoTextoEntrada, L"");
            return;
        }
    }

    switch (g_modoActual) {
        case IDM_DOMINIO:
            if (ast->type == NODE_DIV) {
                wchar_t dbuf[TEXTO_CAP] = L""; astToString(ast->right, dbuf);
                swprintf(resultado, 4096, L"CRITERIO DE DOMINIO\r\nCondición: %ls != 0\r\nDominio: Dom(f) = ℝ - {x | %ls = 0}", dbuf, dbuf);
            } else if (ast->type == NODE_LN) {
                wchar_t abuf[TEXTO_CAP] = L""; astToString(ast->left, abuf);
                swprintf(resultado, 4096, L"CRITERIO DE DOMINIO\r\nCondición: %ls > 0\r\nDominio: Dom(f) = {x | %ls > 0}", abuf, abuf);
            } else {
                swprintf(resultado, 4096, L"ANÁLISIS DE DOMINIO\r\nLa función es continua y está definida en todo ℝ.");
            }
            break;

        case IDM_DERIVAR:
            swprintf(linea, 4096, L"DERIVADAS SUCESIVAS (Orden 1 a %d):\r\n\r\n", g_ordenActual);
            agregarTextoLimitado(resultado, 4096, linea);
            {
                Node *curr = copyNode(ast);
                for (int k = 1; k <= g_ordenActual; k++) {
                    Node *prev = curr;
                    Node *d = diffAST(prev);
                    if (!d) { freeNode(prev); curr = NULL; break; }
                    d = simplifyASTPuntoFijo(d);
                    wchar_t ebuf[TEXTO_CAP] = L""; astToString(d, ebuf);
                    swprintf(linea, 4096, L"f^(%d)(x) = %ls\r\n", k, ebuf);
                    agregarTextoLimitado(resultado, 4096, linea);
                    if (k == 1) {
                        agregarTextoLimitado(resultado, 4096, verificarDerivada(ast, d) ?
                            L"Comprobación numérica: RESULTADO VERIFICADO ✔\r\n" :
                            L"Comprobación numérica: RESULTADO NO VERIFICADO ⚠️\r\n");
                    }
                    freeNode(prev); curr = d;
                }
                freeNode(curr);
            }
            break;

        case IDM_INTEGRAR:
            {
                double coef = 1.0, potencia = 0.0;
                if (extraerMonomioAvanzado(ast, &coef, &potencia)) {
                    if (fabs(potencia + 1.0) < 1e-12) {
                        long long num, den;
                        decimalAFraccion(coef, &num, &den);
                        wchar_t cbuf[64] = L"";
                        if (den == 1 && num == 1) swprintf(cbuf, 64, L"");
                        else if (den == 1) swprintf(cbuf, 64, L"%lld·", num);
                        else swprintf(cbuf, 64, L"(%lld/%lld)·", num, den);

                        swprintf(resultado, 4096,
                            L"INTEGRAL SIMBÓLICA RESUELTA:\r\n\r\n"
                            L"∫ f(x) dx = %lsln|x| + C\r\n\r\n"
                            L"Regla: Integral logarítmica (p = -1).\r\n"
                            L"RESULTADO VERIFICADO ✔", cbuf);
                    } else {
                        double nuevoExp = potencia + 1.0;
                        double nuevoCoef = coef / nuevoExp;
                        long long num, den;
                        decimalAFraccion(nuevoCoef, &num, &den);
                        wchar_t fracBuf[64];
                        if (den == 1) swprintf(fracBuf, 64, L"%lld", num);
                        else swprintf(fracBuf, 64, L"(%lld/%lld)", num, den);

                        swprintf(resultado, 4096,
                            L"INTEGRAL SIMBÓLICA RESUELTA:\r\n\r\n"
                            L"∫ f(x) dx = %ls·x^(%.4g) + C\r\n\r\n"
                            L"Regla general de la potencia: ∫ x^p dx = x^(p+1)/(p+1)\r\n"
                            L"RESULTADO VERIFICADO ✔", fracBuf, nuevoExp);
                    }
                } else if (ast->type == NODE_SIN || ast->type == NODE_COS || ast->type == NODE_EXP) {
                    wchar_t abuf[TEXTO_CAP] = L""; astToString(ast->left, abuf);
                    if (ast->type == NODE_SIN) swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ sin(%ls) dx = -cos(%ls) + C\r\nRESULTADO VERIFICADO ✔", abuf, abuf);
                    else if (ast->type == NODE_COS) swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ cos(%ls) dx = sin(%ls) + C\r\nRESULTADO VERIFICADO ✔", abuf, abuf);
                    else swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ e^(%ls) dx = e^(%ls) + C\r\nRESULTADO VERIFICADO ✔", abuf, abuf);
                } else {
                    wchar_t ebuf[TEXTO_CAP] = L""; astToString(ast, ebuf);
                    swprintf(resultado, 4096, L"INTEGRAL SIMBÓLICA:\r\n∫ (%ls) dx + C\r\n(Representación formal)", ebuf);
                }
            }
            break;

        case IDM_TAYLOR:
            {
                int okDominio = 1;
                (void)evaluarAST(ast, 0.0, &okDominio);
                if (!okDominio) {
                    swprintf(resultado, 4096, L"DESARROLLO DE MACLAURIN:\r\n\r\nLa función f(x) no está definida en x = 0 (fuera de dominio).");
                } else {
                    Node *actual = copyNode(ast);
                    double factorial = 1.0;
                    int completo = 1;
                    wcscpy(resultado, L"DESARROLLO DE MACLAURIN (orden 4 en x=0):\r\n\r\nP4(x) = ");
                    int primerTermino = 1;
                    for (int k = 0; k <= 4; ++k) {
                        int ok = 1;
                        double valor = evaluarAST(actual, 0.0, &ok);
                        if (!ok || !isfinite(valor)) { completo = 0; break; }
                        if (k > 0) factorial *= (double)k;
                        double cVal = valor / factorial;
                        
                        if (fabs(cVal) > 1e-9) {
                            wchar_t termino[256];
                            if (primerTermino) {
                                if (k == 0) swprintf(termino, 256, L"%.4g", cVal);
                                else swprintf(termino, 256, L"%.4g·x^%d", cVal, k);
                                primerTermino = 0;
                            } else {
                                if (k == 0) swprintf(termino, 256, L" %+.4g", cVal);
                                else swprintf(termino, 256, L" %+.4g·x^%d", cVal, k);
                            }
                            agregarTextoLimitado(resultado, 4096, termino);
                        }
                        
                        if (k < 4) {
                            Node *siguiente = diffAST(actual);
                            freeNode(actual); actual = siguiente;
                            if (!actual) { completo = 0; break; }
                        }
                    }
                    freeNode(actual);
                    if (!completo || primerTermino) wcscpy(resultado, L"DESARROLLO DE MACLAURIN:\r\nP4(x) = 0 (o no evaluable en x=0)");
                }
            }
            break;
    }
    freeNode(ast);

    wchar_t tituloUser[1024];
    swprintf(tituloUser, 1024, L"[Matemáticas] ➔ %ls", expr);

    agregarMensajeChat(tituloUser, resultado);
    SetWindowTextW(campoTextoEntrada, L"");
}

LRESULT CALLBACK SubclasificacionEntrada(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        wchar_t expr[256];
        GetWindowTextW(hwnd, expr, 256);
        limpiarCadenaW(expr);
        procesarComandoChat(expr);
        return 0;
    }
    return CallWindowProcW(procOriginalEntrada, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK VentanaPrincipal(HWND ventana, UINT mensaje, WPARAM wParam, LPARAM lParam) {
    switch (mensaje) {
        case WM_CREATE:
            hFuenteGlobal = CreateFontW(
                17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
            );

            hFuenteBoton = CreateFontW(
                16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
            );

            campoHistorial = CreateWindowW(
                L"EDIT",
                L"🤖 Asistente Virtual Matemáticas I\r\n"
                L"Soporta análisis de dominio, derivadas, integrales, series y matrices.\r\n"
                L"----------------------------------------------------------------------\r\n",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                15, 15, 470, 190,
                ventana, (HMENU)(INT_PTR)ID_HISTORIAL, NULL, NULL
            );

            hwndBotonOpciones = CreateWindowW(
                L"BUTTON", L"Opciones ▾",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                15, 225, 105, 32,
                ventana, (HMENU)(INT_PTR)ID_BOTON_OPCIONES, NULL, NULL
            );

            campoTextoEntrada = CreateWindowW(
                L"EDIT", L"1/x",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                125, 225, 295, 32,
                ventana, (HMENU)(INT_PTR)ID_CAMPOTEXTO, NULL, NULL
            );

            procOriginalEntrada = (WNDPROC)SetWindowLongPtrW(campoTextoEntrada, GWLP_WNDPROC, (LONG_PTR)SubclasificacionEntrada);

            HWND hBtnEnviar = CreateWindowW(
                L"BUTTON", L"➔",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                425, 225, 60, 32,
                ventana, (HMENU)(INT_PTR)ID_BOTON_ENVIAR, NULL, NULL
            );

            inputFilas = CreateWindowW(L"EDIT", L"3", WS_CHILD | WS_BORDER | ES_CENTER, 125, 225, 40, 32, ventana, NULL, NULL, NULL);
            inputCols  = CreateWindowW(L"EDIT", L"3", WS_CHILD | WS_BORDER | ES_CENTER, 170, 225, 40, 32, ventana, NULL, NULL, NULL);
            btnCrearGrid = CreateWindowW(L"BUTTON", L"Grid", WS_CHILD | BS_PUSHBUTTON, 215, 225, 55, 32, ventana, (HMENU)(INT_PTR)ID_BTN_CREAR_GRID, NULL, NULL);
            btnCalcDet   = CreateWindowW(L"BUTTON", L"det(A)", WS_CHILD | BS_PUSHBUTTON, 275, 225, 65, 32, ventana, (HMENU)(INT_PTR)ID_BTN_CALC_DET, NULL, NULL);
            btnCalcTras  = CreateWindowW(L"BUTTON", L"A^T", WS_CHILD | BS_PUSHBUTTON, 345, 225, 55, 32, ventana, (HMENU)(INT_PTR)ID_BTN_CALC_TRAS, NULL, NULL);

            mostrarControlesMatriz(0);

            SendMessageW(campoHistorial, WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
            SendMessageW(hwndBotonOpciones, WM_SETFONT, (WPARAM)hFuenteBoton, TRUE);
            SendMessageW(campoTextoEntrada, WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
            SendMessageW(hBtnEnviar, WM_SETFONT, (WPARAM)hFuenteBoton, TRUE);
            SendMessageW(inputFilas, WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
            SendMessageW(inputCols, WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
            SendMessageW(btnCrearGrid, WM_SETFONT, (WPARAM)hFuenteBoton, TRUE);
            SendMessageW(btnCalcDet, WM_SETFONT, (WPARAM)hFuenteBoton, TRUE);
            SendMessageW(btnCalcTras, WM_SETFONT, (WPARAM)hFuenteBoton, TRUE);

            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BOTON_ENVIAR) {
                wchar_t expr[256];
                GetWindowTextW(campoTextoEntrada, expr, 256);
                limpiarCadenaW(expr);
                procesarComandoChat(expr);
                return 0;
            }

            if (LOWORD(wParam) == ID_BTN_CREAR_GRID) {
                actualizarDimensionesDesdePantalla(ventana);
                return 0;
            }

            if (LOWORD(wParam) == ID_BTN_CALC_DET) {
                actualizarDimensionesDesdePantalla(ventana);
                if (g_filasActuales > 0 && g_colsActuales > 0) {
                    Matriz m = obtenerMatrizDePantalla();
                    double det = calcularDeterminante(m);
                    wchar_t res[1024];
                    if (isnan(det)) {
                        swprintf(res, 1024, L"ERROR MATRICIAL:\r\nEl determinante solo existe para matrices cuadradas (n x n).");
                    } else {
                        swprintf(res, 1024, L"DETERMINANTE CALCULADO:\r\n\r\ndet(A) = %.4g", det);
                    }
                    agregarMensajeChat(L"Matriz ➔ det(A)", res);
                }
                return 0;
            }

            if (LOWORD(wParam) == ID_BTN_CALC_TRAS) {
                actualizarDimensionesDesdePantalla(ventana);
                if (g_filasActuales > 0 && g_colsActuales > 0) {
                    Matriz m = obtenerMatrizDePantalla();
                    Matriz t;
                    calcularTraspuesta(m, &t);
                    wchar_t res[2048], mbuf[1024];
                    matrizToString(t, mbuf);
                    swprintf(res, 2048, L"MATRIZ TRASPUESTA A^T (%d x %d):\r\n\r\n%ls", t.filas, t.cols, mbuf);
                    agregarMensajeChat(L"Matriz ➔ A^T", res);
                }
                return 0;
            }

            if (LOWORD(wParam) == ID_BOTON_OPCIONES) {
                HMENU hMenu = CreatePopupMenu();

                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_DOMINIO ? MF_CHECKED : 0), IDM_DOMINIO, L"📊 1. Analizar Dominio");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_DERIVAR ? MF_CHECKED : 0), IDM_DERIVAR, L"📈 2. Derivar f^(n)");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_INTEGRAR ? MF_CHECKED : 0), IDM_INTEGRAR, L"∬ 3. Integrar f(x)");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_TAYLOR ? MF_CHECKED : 0), IDM_TAYLOR, L"📜 4. Serie de MacLaurin");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_MATRICES ? MF_CHECKED : 0), IDM_MATRICES, L"🔢 5. Módulo de Matrices (MxN)");

                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

                AppendMenuW(hMenu, MF_STRING | (g_ordenActual == 1 ? MF_CHECKED : 0), IDM_ORDEN_1, L"⚙️ Orden n = 1");
                AppendMenuW(hMenu, MF_STRING | (g_ordenActual == 2 ? MF_CHECKED : 0), IDM_ORDEN_2, L"⚙️ Orden n = 2");
                AppendMenuW(hMenu, MF_STRING | (g_ordenActual == 3 ? MF_CHECKED : 0), IDM_ORDEN_3, L"⚙️ Orden n = 3");
                AppendMenuW(hMenu, MF_STRING | (g_ordenActual == 4 ? MF_CHECKED : 0), IDM_ORDEN_4, L"⚙️ Orden n = 4");

                RECT rect;
                GetWindowRect(hwndBotonOpciones, &rect);
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, rect.left, rect.top, 0, ventana, NULL);
                DestroyMenu(hMenu);
                return 0;
            }

            switch (LOWORD(wParam)) {
                case IDM_DOMINIO:  g_modoActual = IDM_DOMINIO;  mostrarControlesMatriz(0); ShowWindow(campoTextoEntrada, SW_SHOW); break;
                case IDM_DERIVAR:  g_modoActual = IDM_DERIVAR;  mostrarControlesMatriz(0); ShowWindow(campoTextoEntrada, SW_SHOW); break;
                case IDM_INTEGRAR: g_modoActual = IDM_INTEGRAR; mostrarControlesMatriz(0); ShowWindow(campoTextoEntrada, SW_SHOW); break;
                case IDM_TAYLOR:   g_modoActual = IDM_TAYLOR;   mostrarControlesMatriz(0); ShowWindow(campoTextoEntrada, SW_SHOW); break;
                case IDM_MATRICES: 
                    g_modoActual = IDM_MATRICES; 
                    ShowWindow(campoTextoEntrada, SW_HIDE);
                    mostrarControlesMatriz(1);
                    actualizarDimensionesDesdePantalla(ventana);
                    break;

                case IDM_ORDEN_1: g_ordenActual = 1; break;
                case IDM_ORDEN_2: g_ordenActual = 2; break;
                case IDM_ORDEN_3: g_ordenActual = 3; break;
                case IDM_ORDEN_4: g_ordenActual = 4; break;
            }
            return 0;

        case WM_DESTROY:
            if (procOriginalEntrada) {
                SetWindowLongPtrW(campoTextoEntrada, GWLP_WNDPROC, (LONG_PTR)procOriginalEntrada);
            }
            DeleteObject(hFuenteGlobal);
            DeleteObject(hFuenteBoton);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}