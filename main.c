#define _WIN32_WINNT 0x0600 // Le dice al compilador que usamos Windows Vista o superior
#include <windows.h>
#include "gui.h"

// Definiciones para soporte de escalado 4K moderno (DPI)
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif
typedef BOOL(WINAPI *SETPROCESSDPIAWARENESSEXPROC)(DPI_AWARENESS_CONTEXT);

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE instanciaPrevia, LPSTR lineasComando, int modoMuestra) {
    (void)instanciaPrevia;
    (void)lineasComando;

    // 1. Activar renderizado de texto de alta definición
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        // El cast a (void*) extra silencia la advertencia de GCC (-Wcast-function-type)
        SETPROCESSDPIAWARENESSEXPROC setDpiAwareness = (SETPROCESSDPIAWARENESSEXPROC)(void*)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (setDpiAwareness) setDpiAwareness(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        else SetProcessDPIAware(); 
    }

    WNDCLASSW claseVentana = {0};
    claseVentana.lpfnWndProc   = VentanaPrincipal;
    claseVentana.hInstance     = instancia;
    claseVentana.lpszClassName = L"ClaseCalculadora";
    claseVentana.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    claseVentana.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&claseVentana)) return 0;

    // WS_EX_CONTROLPARENT es clave para que Windows permita saltar entre casillas con TAB
    HWND ventana = CreateWindowExW(
        WS_EX_CONTROLPARENT,
        L"ClaseCalculadora",
        L"Asistente Virtual Matemáticas I",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 570, 360,
        NULL, NULL, instancia, NULL
    );

    if (!ventana) return 0;

    ShowWindow(ventana, modoMuestra);
    UpdateWindow(ventana);

    MSG mensaje = {0};
    while (GetMessageW(&mensaje, NULL, 0, 0)) {
        // 2. IsDialogMessage intercepta el TAB y mueve el foco automáticamente
        if (!IsDialogMessage(ventana, &mensaje)) {
            TranslateMessage(&mensaje);
            DispatchMessageW(&mensaje);
        }
    }
    return (int)mensaje.wParam;
}