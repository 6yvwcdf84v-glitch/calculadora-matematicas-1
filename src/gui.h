#ifndef GUI_H
#define GUI_H

#include <windows.h>

#define ID_CAMPOTEXTO     101
#define ID_BOTON_OPCIONES 102
#define ID_BOTON_ENVIAR   103
#define ID_HISTORIAL      104

#define IDM_DOMINIO       202
#define IDM_DERIVAR       203
#define IDM_INTEGRAR      204
#define IDM_TAYLOR        205
#define IDM_MATRICES      206

#define IDM_ORDEN_1       301
#define IDM_ORDEN_2       302
#define IDM_ORDEN_3       303
#define IDM_ORDEN_4       304

#define ID_BTN_CREAR_GRID 401
#define ID_BTN_CALC_DET   402
#define ID_BTN_CALC_TRAS  403

extern HWND campoHistorial;
extern HWND campoTextoEntrada;
extern HWND hwndBotonOpciones;

void agregarMensajeChat(const wchar_t *usuario, const wchar_t *contenido);
void procesarComandoChat(const wchar_t *expr);
void limpiarCadenaW(wchar_t *cadena);

LRESULT CALLBACK VentanaPrincipal(HWND ventana, UINT mensaje, WPARAM wParam, LPARAM lParam);

#endif