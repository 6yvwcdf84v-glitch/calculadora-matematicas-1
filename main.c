#include <windows.h>
#include "gui.h"

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE instanciaPrevia, LPSTR lineasComando, int modoMuestra) {
    (void)instanciaPrevia;
    (void)lineasComando;

    WNDCLASSW claseVentana = {0};
    claseVentana.lpfnWndProc   = VentanaPrincipal;
    claseVentana.hInstance     = instancia;
    claseVentana.lpszClassName = L"ClaseCalculadora";
    claseVentana.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    claseVentana.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&claseVentana)) {
        MessageBoxW(NULL, L"Error al registrar la clase de la ventana.", L"Error Critico", MB_ICONERROR);
        return 0;
    }

    // WS_OVERLAPPEDWINDOW habilita maximizar, minimizar y redimensionar arrastrando bordes
    HWND ventana = CreateWindowExW(
        0,
        L"ClaseCalculadora",
        L"Asistente Virtual Matemáticas I",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 360,
        NULL, NULL, instancia, NULL
    );

    if (!ventana) {
        MessageBoxW(NULL, L"Error al crear la ventana principal.", L"Error Critico", MB_ICONERROR);
        return 0;
    }

    ShowWindow(ventana, modoMuestra);
    UpdateWindow(ventana);

    MSG mensaje = {0};
    while (GetMessageW(&mensaje, NULL, 0, 0)) {
        TranslateMessage(&mensaje);
        DispatchMessageW(&mensaje);
    }

    return (int)mensaje.wParam;
}