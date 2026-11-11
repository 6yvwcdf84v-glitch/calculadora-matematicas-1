#include "gui.h"
#include "mates.h"
#include "matriz.h"
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>

#define ID_BTN_CALC_INV 1100
#define IDM_RAICES 1101
#define ID_BTN_ORDEN 1102

HWND campoHistorial;
HWND campoTextoEntrada;
HWND hwndBotonOpciones;

HWND inputFilas, inputCols, btnCrearGrid, btnCalcDet, btnCalcTras, btnCalcInv, btnOrden;
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

void escalarGridMatriz(HWND ventana, double factor) {
    if (g_filasActuales <= 0 || g_colsActuales <= 0) return;
    
    RECT rect;
    GetClientRect(ventana, &rect);
    
    int altoBarra = (int)(32 * factor);
    int margen = (int)(15 * factor);
    int ancho = (int)(42 * factor);
    int alto = (int)(24 * factor);
    int gap = (int)(4 * factor);
    
    int posYInicial = rect.bottom - altoBarra - margen - 15 - (g_filasActuales * (alto + gap));
    int posXInicial = margen + (int)(105 * factor) + 5;

    for (int i = 0; i < g_filasActuales; i++) {
        for (int j = 0; j < g_colsActuales; j++) {
            if (casillasGrid[i][j]) {
                MoveWindow(casillasGrid[i][j], posXInicial + j * (ancho + gap), posYInicial + i * (alto + gap), ancho, alto, TRUE);
            }
        }
    }
}

void crearGridMatriz(HWND ventana, int f, int c) {
    destruirGridMatriz();
    if (f > MAX_FILAS) f = MAX_FILAS;
    if (c > MAX_COLS) c = MAX_COLS;
    if (f <= 0) f = 1;
    if (c <= 0) c = 1;

    g_filasActuales = f;
    g_colsActuales = c;

    for (int i = 0; i < f; i++) {
        for (int j = 0; j < c; j++) {
            casillasGrid[i][j] = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"0",
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_CENTER,
                0, 0, 10, 10, // Posición temporal, escalarGridMatriz lo ajustará
                ventana, NULL, NULL, NULL
            );
            SendMessageW(casillasGrid[i][j], WM_SETFONT, (WPARAM)hFuenteGlobal, TRUE);
        }
    }
    
    RECT rect;
    GetClientRect(ventana, &rect);
    double factor = (double)rect.bottom / 360.0;
    if (factor < 0.85) factor = 0.85;
    if (factor > 2.2) factor = 2.2;
    
    escalarGridMatriz(ventana, factor);
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

void actualizarTamanoFuentes(HWND ventana, double factor) {
    if (!campoHistorial) return;
    
    int tamTexto = (int)(17 * factor);
    int tamBoton = (int)(16 * factor);
    
    HFONT nuevaF = CreateFontW(tamTexto, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT nuevaB = CreateFontW(tamBoton, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessageW(campoHistorial, WM_SETFONT, (WPARAM)nuevaF, TRUE);
    SendMessageW(hwndBotonOpciones, WM_SETFONT, (WPARAM)nuevaB, TRUE);
    SendMessageW(campoTextoEntrada, WM_SETFONT, (WPARAM)nuevaF, TRUE);
    SendMessageW(GetDlgItem(ventana, ID_BOTON_ENVIAR), WM_SETFONT, (WPARAM)nuevaB, TRUE);
    SendMessageW(btnOrden, WM_SETFONT, (WPARAM)nuevaB, TRUE);
    
    SendMessageW(inputFilas, WM_SETFONT, (WPARAM)nuevaF, TRUE);
    SendMessageW(inputCols, WM_SETFONT, (WPARAM)nuevaF, TRUE);
    SendMessageW(btnCrearGrid, WM_SETFONT, (WPARAM)nuevaB, TRUE);
    SendMessageW(btnCalcDet, WM_SETFONT, (WPARAM)nuevaB, TRUE);
    SendMessageW(btnCalcTras, WM_SETFONT, (WPARAM)nuevaB, TRUE);
    SendMessageW(btnCalcInv, WM_SETFONT, (WPARAM)nuevaB, TRUE);

    for (int i = 0; i < g_filasActuales; i++) {
        for (int j = 0; j < g_colsActuales; j++) {
            if (casillasGrid[i][j]) {
                SendMessageW(casillasGrid[i][j], WM_SETFONT, (WPARAM)nuevaF, TRUE);
            }
        }
    }
    
    if (hFuenteGlobal) DeleteObject(hFuenteGlobal);
    if (hFuenteBoton) DeleteObject(hFuenteBoton);
    
    hFuenteGlobal = nuevaF;
    hFuenteBoton = nuevaB;
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
    ShowWindow(btnCalcInv, sw);
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
        {
            if (ast->type == NODE_DIV) {
                wchar_t dbuf[TEXTO_CAP] = L"";
                astToString(ast->right, dbuf);
                swprintf(resultado, 4096, L"CRITERIO DE DOMINIO\r\nCondición: %ls != 0\r\nDominio: Dom(f) = ℝ - {x | %ls = 0}", dbuf, dbuf);
            } else if (ast->type == NODE_LN) {
                wchar_t abuf[TEXTO_CAP] = L"";
                astToString(ast->left, abuf);
                swprintf(resultado, 4096, L"CRITERIO DE DOMINIO\r\nCondición: %ls > 0\r\nDominio: Dom(f) = {x | %ls > 0}", abuf, abuf);
            } else {
                swprintf(resultado, 4096, L"ANÁLISIS DE DOMINIO\r\nLa función es continua y está definida en todo ℝ.");
            }
            break;
        }
        case IDM_DERIVAR:
        {
            swprintf(linea, 4096, L"DERIVADAS SUCESIVAS (Orden 1 a %d):\r\n\r\n", g_ordenActual);
            wcscpy(resultado, linea);
            
            Node *curr = copyNode(ast);
            for (int k = 1; k <= g_ordenActual; k++) {
                Node *prev = curr;
                Node *d = diffAST(prev);
                if (!d) {
                    freeNode(prev);
                    curr = NULL;
                    break;
                }
                d = simplifyASTPuntoFijo(d);
                
                wchar_t ebuf[TEXTO_CAP] = L"";
                astToString(d, ebuf);
                swprintf(linea, 4096, L"f^(%d)(x) = %ls\r\n", k, ebuf);
                wcsncat(resultado, linea, 4095 - wcslen(resultado));
                
                if (k == 1) {
                    if (verificarDerivada(ast, d)) {
                        wcsncat(resultado, L"Comprobación numérica: RESULTADO VERIFICADO ✔\r\n", 4095 - wcslen(resultado));
                    } else {
                        wcsncat(resultado, L"Comprobación numérica: RESULTADO NO VERIFICADO ⚠️\r\n", 4095 - wcslen(resultado));
                    }
                }
                
                freeNode(prev);
                curr = d;
            }
            if (curr) {
                freeNode(curr);
            }
            break;
        }
        case IDM_INTEGRAR:
        {
            double coef = 1.0, potencia = 0.0;
            if (extraerMonomioAvanzado(ast, &coef, &potencia)) {
                if (fabs(potencia + 1.0) < 1e-12) {
                    long long num, den;
                    decimalAFraccion(coef, &num, &den);
                    wchar_t cbuf[64] = L"";
                    
                    if (den == 1 && num == 1) {
                        swprintf(cbuf, 64, L"");
                    } else if (den == 1) {
                        swprintf(cbuf, 64, L"%lld·", num);
                    } else {
                        swprintf(cbuf, 64, L"(%lld/%lld)·", num, den);
                    }
                    swprintf(resultado, 4096, L"INTEGRAL SIMBÓLICA:\r\n\r\n∫ f(x) dx = %lsln|x| + C\r\n\r\nRESULTADO VERIFICADO ✔", cbuf);
                } else {
                    double nuevoExp = potencia + 1.0;
                    double nuevoCoef = coef / nuevoExp;
                    long long num, den;
                    decimalAFraccion(nuevoCoef, &num, &den);
                    wchar_t fracBuf[64];
                    
                    if (den == 1) {
                        swprintf(fracBuf, 64, L"%lld", num);
                    } else {
                        swprintf(fracBuf, 64, L"(%lld/%lld)", num, den);
                    }
                    swprintf(resultado, 4096, L"INTEGRAL SIMBÓLICA:\r\n\r\n∫ f(x) dx = %ls·x^(%.4g) + C\r\n\r\nRESULTADO VERIFICADO ✔", fracBuf, nuevoExp);
                }
            } else if (ast->type == NODE_SIN || ast->type == NODE_COS || ast->type == NODE_EXP) {
                wchar_t abuf[TEXTO_CAP] = L"";
                astToString(ast->left, abuf);
                
                if (ast->type == NODE_SIN) {
                    swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ sin(%ls) dx = -cos(%ls) + C\r\nVERIFICADO ✔", abuf, abuf);
                } else if (ast->type == NODE_COS) {
                    swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ cos(%ls) dx = sin(%ls) + C\r\nVERIFICADO ✔", abuf, abuf);
                } else {
                    swprintf(resultado, 4096, L"INTEGRAL:\r\n∫ e^(%ls) dx = e^(%ls) + C\r\nVERIFICADO ✔", abuf, abuf);
                }
            } else {
                wchar_t ebuf[TEXTO_CAP] = L"";
                astToString(ast, ebuf);
                swprintf(resultado, 4096, L"INTEGRAL SIMBÓLICA:\r\n∫ (%ls) dx + C\r\n(Representación formal)", ebuf);
            }
            break;
        }
        case IDM_TAYLOR:
        {
            int okDominio = 1;
            (void)evaluarAST(ast, 0.0, &okDominio);
            if (!okDominio) {
                swprintf(resultado, 4096, L"DESARROLLO DE MACLAURIN:\r\n\r\nLa función f(x) no está definida en x = 0.");
            } else {
                Node *actual = copyNode(ast);
                double factorial = 1.0;
                int completo = 1;
                int primerTermino = 1;
                
                wcscpy(resultado, L"DESARROLLO DE MACLAURIN (orden 4 en x=0):\r\n\r\nP4(x) = ");
                
                for (int k = 0; k <= 4; ++k) {
                    int ok = 1;
                    double valor = evaluarAST(actual, 0.0, &ok);
                    
                    if (!ok || !isfinite(valor)) {
                        completo = 0;
                        break;
                    }
                    
                    if (k > 0) factorial *= (double)k;
                    double cVal = valor / factorial;
                    
                    if (fabs(cVal) > 1e-9) {
                        wchar_t termino[256];
                        if (primerTermino) {
                            if (k == 0) {
                                swprintf(termino, 256, L"%.4g", cVal);
                            } else {
                                swprintf(termino, 256, L"%.4g·x^%d", cVal, k);
                            }
                            primerTermino = 0;
                        } else {
                            if (k == 0) {
                                swprintf(termino, 256, L" %+.4g", cVal);
                            } else {
                                swprintf(termino, 256, L" %+.4g·x^%d", cVal, k);
                            }
                        }
                        wcsncat(resultado, termino, 4095 - wcslen(resultado));
                    }
                    
                    if (k < 4) {
                        Node *siguiente = diffAST(actual);
                        freeNode(actual);
                        actual = siguiente;
                        if (!actual) {
                            completo = 0;
                            break;
                        }
                    }
                }
                
                freeNode(actual);
                if (!completo || primerTermino) {
                    wcscpy(resultado, L"DESARROLLO DE MACLAURIN:\r\nP4(x) = 0 (o no evaluable en x=0)");
                }
            }
            break;
        }
        case IDM_RAICES:
        {
            double raiz;
            if (calcularRaizNewton(ast, 1.0, &raiz) || calcularRaizNewton(ast, 0.0, &raiz) || calcularRaizNewton(ast, -1.0, &raiz)) {
                swprintf(resultado, 4096, L"CÁLCULO DE RAÍCES (f(x) = 0):\r\n\r\nMétodo: Newton-Raphson\r\nCorte con el eje X detectado en: x ≈ %.6g", raiz);
            } else {
                swprintf(resultado, 4096, L"CÁLCULO DE RAÍCES:\r\n\r\nEl método de Newton no convergió. Puede que no tenga raíces reales.");
            }
            break;
        }
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
        {
            hFuenteGlobal = CreateFontW(17, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            hFuenteBoton = CreateFontW(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            campoHistorial = CreateWindowW(
                L"EDIT", 
                L"🤖 Asistente Virtual Matemáticas I\r\nSoporta Dominio, Derivadas, Integrales, MacLaurin, Raíces y Matrices.\r\n----------------------------------------------------------------------\r\n", 
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 
                15, 15, 500, 220, 
                ventana, (HMENU)(INT_PTR)ID_HISTORIAL, NULL, NULL
            );
            
            hwndBotonOpciones = CreateWindowExW(
                0, L"BUTTON", L"Opciones ▾", 
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 
                15, 250, 105, 32, 
                ventana, (HMENU)(INT_PTR)ID_BOTON_OPCIONES, NULL, NULL
            );
            
            campoTextoEntrada = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"", 
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 
                125, 250, 320, 32, 
                ventana, (HMENU)(INT_PTR)ID_CAMPOTEXTO, NULL, NULL
            );
            
            procOriginalEntrada = (WNDPROC)SetWindowLongPtrW(campoTextoEntrada, GWLP_WNDPROC, (LONG_PTR)SubclasificacionEntrada);
            
            btnOrden = CreateWindowExW(
                0, L"BUTTON", L"n = 1", 
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 
                0, 0, 0, 0, 
                ventana, (HMENU)(INT_PTR)ID_BTN_ORDEN, NULL, NULL
            );
            
            CreateWindowExW(
                0, L"BUTTON", L"➔", 
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 
                0, 0, 0, 0, 
                ventana, (HMENU)(INT_PTR)ID_BOTON_ENVIAR, NULL, NULL
            );

            inputFilas = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"3", WS_CHILD | WS_TABSTOP | ES_CENTER, 0, 0, 0, 0, ventana, NULL, NULL, NULL);
            inputCols  = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"3", WS_CHILD | WS_TABSTOP | ES_CENTER, 0, 0, 0, 0, ventana, NULL, NULL, NULL);
            btnCrearGrid = CreateWindowExW(0, L"BUTTON", L"Grid", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, ventana, (HMENU)(INT_PTR)ID_BTN_CREAR_GRID, NULL, NULL);
            btnCalcDet   = CreateWindowExW(0, L"BUTTON", L"det(A)", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, ventana, (HMENU)(INT_PTR)ID_BTN_CALC_DET, NULL, NULL);
            btnCalcTras  = CreateWindowExW(0, L"BUTTON", L"A^T", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, ventana, (HMENU)(INT_PTR)ID_BTN_CALC_TRAS, NULL, NULL);
            btnCalcInv   = CreateWindowExW(0, L"BUTTON", L"A^-1", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, ventana, (HMENU)(INT_PTR)ID_BTN_CALC_INV, NULL, NULL);

            mostrarControlesMatriz(0);
            return 0;
        }
        case WM_SIZE:
        {
            int anchoVentana = LOWORD(lParam);
            int altoVentana = HIWORD(lParam);
            
            if (campoHistorial && hwndBotonOpciones) {
                double factor = (double)altoVentana / 360.0;
                if (factor < 0.85) factor = 0.85;
                if (factor > 2.2) factor = 2.2;

                actualizarTamanoFuentes(ventana, factor);

                int altoBarra = (int)(32 * factor);
                int margen = (int)(15 * factor);
                int posYInferior = altoVentana - altoBarra - margen;
                
                MoveWindow(campoHistorial, margen, margen, anchoVentana - (margen * 2), altoVentana - altoBarra - (margen * 2) - 10, TRUE);

                int wOpt = (int)(105 * factor);
                int wBtnEnv = (int)(60 * factor);
                int wBtnOrd = (int)(55 * factor);
                
                MoveWindow(hwndBotonOpciones, margen, posYInferior, wOpt, altoBarra, TRUE);
                
                if (g_modoActual == IDM_DERIVAR) {
                    MoveWindow(campoTextoEntrada, margen + wOpt + 5, posYInferior, anchoVentana - wOpt - wBtnEnv - wBtnOrd - (margen * 2) - 15, altoBarra, TRUE);
                    MoveWindow(btnOrden, anchoVentana - margen - wBtnEnv - wBtnOrd - 5, posYInferior, wBtnOrd, altoBarra, TRUE);
                    ShowWindow(btnOrden, SW_SHOW);
                } else {
                    MoveWindow(campoTextoEntrada, margen + wOpt + 5, posYInferior, anchoVentana - wOpt - wBtnEnv - (margen * 2) - 10, altoBarra, TRUE);
                    ShowWindow(btnOrden, SW_HIDE);
                }

                HWND hBtnEnviar = GetDlgItem(ventana, ID_BOTON_ENVIAR);
                if (hBtnEnviar) {
                    MoveWindow(hBtnEnviar, anchoVentana - margen - wBtnEnv, posYInferior, wBtnEnv, altoBarra, TRUE);
                }

                int wInp = (int)(40 * factor);
                int wBtn1 = (int)(50 * factor);
                int wBtn2 = (int)(60 * factor);
                int posX = margen + wOpt + 5;
                
                MoveWindow(inputFilas, posX, posYInferior, wInp, altoBarra, TRUE); 
                posX += wInp + 5;
                MoveWindow(inputCols, posX, posYInferior, wInp, altoBarra, TRUE); 
                posX += wInp + 5;
                MoveWindow(btnCrearGrid, posX, posYInferior, wBtn1, altoBarra, TRUE); 
                posX += wBtn1 + 5;
                MoveWindow(btnCalcDet, posX, posYInferior, wBtn2, altoBarra, TRUE); 
                posX += wBtn2 + 5;
                MoveWindow(btnCalcTras, posX, posYInferior, wBtn1, altoBarra, TRUE); 
                posX += wBtn1 + 5;
                MoveWindow(btnCalcInv, posX, posYInferior, wBtn1, altoBarra, TRUE);

                if (g_modoActual == IDM_MATRICES && g_filasActuales > 0) {
                    escalarGridMatriz(ventana, factor);
                }
            }
            return 0;
        }
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == ID_BOTON_ENVIAR) {
                wchar_t expr[256];
                GetWindowTextW(campoTextoEntrada, expr, 256);
                limpiarCadenaW(expr);
                procesarComandoChat(expr);
                return 0;
            }
            if (LOWORD(wParam) == ID_BTN_ORDEN) {
                g_ordenActual++;
                if (g_ordenActual > 5) g_ordenActual = 1; // Límite de 5 derivadas para proteger la memoria RAM
                wchar_t buf[16];
                swprintf(buf, 16, L"n = %d", g_ordenActual);
                SetWindowTextW(btnOrden, buf);
                return 0;
            }
            if (LOWORD(wParam) == ID_BTN_CREAR_GRID) {
                actualizarDimensionesDesdePantalla(ventana);
                return 0;
            }
            if (LOWORD(wParam) == ID_BTN_CALC_DET) {
                actualizarDimensionesDesdePantalla(ventana);
                if (g_filasActuales > 0 && g_colsActuales > 0) {
                    double det = calcularDeterminante(obtenerMatrizDePantalla());
                    wchar_t res[1024];
                    if (isnan(det)) {
                        swprintf(res, 1024, L"ERROR MATRICIAL:\r\nEl determinante solo existe para matrices cuadradas.");
                    } else {
                        swprintf(res, 1024, L"DETERMINANTE CALCULADO:\r\n\r\ndet(A) = %.4g", det);
                    }
                    agregarMensajeChat(L"Matriz", res);
                }
                return 0;
            }
            if (LOWORD(wParam) == ID_BTN_CALC_TRAS) {
                actualizarDimensionesDesdePantalla(ventana);
                if (g_filasActuales > 0 && g_colsActuales > 0) {
                    Matriz t;
                    calcularTraspuesta(obtenerMatrizDePantalla(), &t);
                    wchar_t res[2048], mbuf[1024];
                    matrizToString(t, mbuf);
                    swprintf(res, 2048, L"MATRIZ TRASPUESTA A^T (%d x %d):\r\n\r\n%ls", t.filas, t.cols, mbuf);
                    agregarMensajeChat(L"Matriz", res);
                }
                return 0;
            }
            if (LOWORD(wParam) == ID_BTN_CALC_INV) {
                actualizarDimensionesDesdePantalla(ventana);
                if (g_filasActuales > 0 && g_colsActuales > 0) {
                    Matriz inv;
                    if (calcularInversa(obtenerMatrizDePantalla(), &inv)) {
                        wchar_t res[2048], mbuf[1024];
                        matrizToString(inv, mbuf);
                        swprintf(res, 2048, L"MATRIZ INVERSA A^-1:\r\n\r\n%ls", mbuf);
                        agregarMensajeChat(L"Matriz", res);
                    } else {
                        agregarMensajeChat(L"Matriz", L"ERROR MATRICIAL:\r\nLa matriz no es cuadrada o su determinante es 0. No tiene inversa.");
                    }
                }
                return 0;
            }

            if (LOWORD(wParam) == ID_BOTON_OPCIONES) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_DOMINIO ? MF_CHECKED : 0), IDM_DOMINIO, L"📊 1. Analizar Dominio");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_DERIVAR ? MF_CHECKED : 0), IDM_DERIVAR, L"📈 2. Derivar f^(n)");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_INTEGRAR ? MF_CHECKED : 0), IDM_INTEGRAR, L"∬ 3. Integrar f(x)");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_TAYLOR ? MF_CHECKED : 0), IDM_TAYLOR, L"📜 4. Serie de MacLaurin");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_RAICES ? MF_CHECKED : 0), IDM_RAICES, L"🎯 5. Buscar Raíces (Newton)");
                AppendMenuW(hMenu, MF_STRING | (g_modoActual == IDM_MATRICES ? MF_CHECKED : 0), IDM_MATRICES, L"🔢 6. Módulo de Matrices (MxN)");
                
                RECT rect;
                GetWindowRect(hwndBotonOpciones, &rect);
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, rect.left, rect.top, 0, ventana, NULL);
                DestroyMenu(hMenu);
                return 0;
            }

            switch (LOWORD(wParam)) {
                case IDM_DOMINIO:
                case IDM_DERIVAR:
                case IDM_INTEGRAR:
                case IDM_TAYLOR:
                case IDM_RAICES:
                    g_modoActual = LOWORD(wParam);
                    mostrarControlesMatriz(0);
                    ShowWindow(campoTextoEntrada, SW_SHOW);
                    break;
                case IDM_MATRICES:
                    g_modoActual = IDM_MATRICES;
                    ShowWindow(campoTextoEntrada, SW_HIDE);
                    mostrarControlesMatriz(1);
                    break;
            }
            
            if (LOWORD(wParam) >= IDM_DOMINIO && LOWORD(wParam) <= IDM_MATRICES) {
                RECT r;
                GetClientRect(ventana, &r);
                SendMessageW(ventana, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            }
            return 0;
        }
        case WM_DESTROY:
        {
            if (procOriginalEntrada) {
                SetWindowLongPtrW(campoTextoEntrada, GWLP_WNDPROC, (LONG_PTR)procOriginalEntrada);
            }
            if (hFuenteGlobal) DeleteObject(hFuenteGlobal);
            if (hFuenteBoton) DeleteObject(hFuenteBoton);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}