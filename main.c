#include <windows.h>
#include "gui.h"

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE instanciaAnterior, LPSTR argumentos, int mostrar) {
    (void)instanciaAnterior; (void)argumentos;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = VentanaPrincipal;
    wc.hInstance = instancia;
    wc.lpszClassName = L"AsistenteVirtualMatematicas1";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&wc)) return 1;

    // TÍTULO DE LA VENTANA ACTUALIZADO
    HWND ventana = CreateWindowW(
        L"AsistenteVirtualMatematicas1",
        L"Asistente Virtual Matemáticas I",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        200, 150, 515, 470,
        NULL, NULL, instancia, NULL
    );

    if (!ventana) return 1;

    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);

    MSG mensaje;
    while (GetMessageW(&mensaje, NULL, 0, 0) > 0) {
        TranslateMessage(&mensaje);
        DispatchMessageW(&mensaje);
    }

    return (int)mensaje.wParam;
}