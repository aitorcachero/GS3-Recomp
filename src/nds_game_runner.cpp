/* =============================================================================
 * NINTENDO DS RECOMPILATION - GOLDEN SUN 3: OSCURO AMANECER
 * Native PC Game Runner & Dual-Screen Presentation Engine
 * Solucion tecnica para Framebuffer Capture, Pantalla Ancha (16:9/16:10) y AF
 * ============================================================================= */

#include "nds_game_runner.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

// Sincronizar ajustes del usuario con la configuración del motor de Nintendo DS
static void sync_melon_config(const NdsConfig& config, const std::string& rom_path) {
    char toml_path[MAX_PATH];
    GetFullPathNameA("tools\\engine\\melonDS.toml", sizeof(toml_path), toml_path, NULL);

    FILE* f = fopen(toml_path, "w");
    if (!f) return;

    // Mapeo de Layout de pantalla para Nintendo DS
    int melon_layout = 1;
    if (config.screen_layout == NDS_LAYOUT_VERTICAL) melon_layout = 0;
    else if (config.screen_layout == NDS_LAYOUT_HORIZONTAL) melon_layout = 1;
    else if (config.screen_layout == NDS_LAYOUT_HYBRID_TOP) melon_layout = 4;
    else if (config.screen_layout == NDS_LAYOUT_HYBRID_BOT) melon_layout = 5;
    else if (config.screen_layout == NDS_LAYOUT_SINGLE_SWAP) melon_layout = 2;

    int melon_scale = config.internal_resolution_scale;
    if (melon_scale < 1) melon_scale = 1;
    if (melon_scale > 8) melon_scale = 8;

    int melon_volume = (int)(config.volume_master * 2.56f);
    if (melon_volume < 0) melon_volume = 0;
    if (melon_volume > 256) melon_volume = 256;

    // Aspect ratio top screen (0: 4:3, 1: 16:9, 2: 16:10)
    int aspect_top = 0;
    if (config.aspect_ratio_mode == NDS_ASPECT_16_9) aspect_top = 1;
    else if (config.aspect_ratio_mode == NDS_ASPECT_16_10) aspect_top = 2;

    fprintf(f, "# Configuracion optimizada para Golden Sun 3: Oscuro Amanecer\n");
    fprintf(f, "# Mejoras activas: Framebuffer Capture HD, Aspect Ratio 16:9/16:10, AF y Texturas\n");
    fprintf(f, "UITheme = \"\"\n");
    fprintf(f, "AudioSync = true\n");
    fprintf(f, "PauseLostFocus = false\n");
    fprintf(f, "TargetFPS = 60.0\n");
    fprintf(f, "LimitFPS = true\n\n");

    fprintf(f, "[Audio]\n");
    fprintf(f, "Interpolation = %d\n", config.spu_interpolation);
    fprintf(f, "BitDepth = 0\n\n");

    fprintf(f, "[3D]\n");
    fprintf(f, "Renderer = 1\n\n"); // 1 = Motor OpenGL acelerado por hardware

    fprintf(f, "[3D.GL]\n");
    fprintf(f, "Scale = %d\n", melon_scale);
    fprintf(f, "BetterPolygons = %s\n", config.subpixel_rasterizer ? "true" : "false");
    fprintf(f, "AnisotropicFiltering = %d\n", config.anisotropic_filter);
    fprintf(f, "CustomTextures = %s\n\n", config.custom_textures_enabled ? "true" : "false");

    fprintf(f, "[Screen]\n");
    fprintf(f, "UseGL = true\n\n");

    fprintf(f, "[Instance0]\n");
    fprintf(f, "SaveFilePath = \"../../saves\"\n");
    fprintf(f, "EnableCheats = true\n\n");

    fprintf(f, "[Instance0.Window0]\n");
    fprintf(f, "ScreenFilter = %s\n", (config.texture_filter != 0) ? "true" : "false");
    fprintf(f, "ScreenAspectTop = %d\n", aspect_top);
    fprintf(f, "ScreenAspectBot = 0\n");
    fprintf(f, "IntegerScaling = %s\n", config.integer_scaling ? "true" : "false");
    fprintf(f, "ScreenSizing = 0\n");
    fprintf(f, "ScreenSwap = %s\n", config.swap_screens ? "true" : "false");
    fprintf(f, "ScreenGap = %d\n", config.screen_gap);
    fprintf(f, "ScreenRotation = 0\n");
    fprintf(f, "ScreenLayout = %d\n", melon_layout);
    fprintf(f, "ShowOSD = false\n\n");

    fprintf(f, "[Instance0.Audio]\n");
    fprintf(f, "Volume = %d\n\n", melon_volume);

    fprintf(f, "[Instance0.Firmware]\n");
    fprintf(f, "Language = %d\n", config.selected_language); // 5 = Español nativo
    fprintf(f, "Username = \"GoldenSun\"\n\n");

    fprintf(f, "[Emu]\n");
    fprintf(f, "DirectBoot = true\n");
    fprintf(f, "ExternalBIOSEnable = false\n");
    fprintf(f, "ConsoleType = 0\n\n");

    fprintf(f, "[JIT]\n");
    fprintf(f, "Enable = true\n");
    fprintf(f, "FastMemory = true\n\n");

    // Mapeo de controles ergonómico
    fprintf(f, "[Instance0.Keyboard]\n");
    fprintf(f, "Up = 16777235\n");
    fprintf(f, "Down = 16777237\n");
    fprintf(f, "Left = 16777234\n");
    fprintf(f, "Right = 16777236\n");
    fprintf(f, "A = 88\n"); // Tecla X = Botón A
    fprintf(f, "B = 90\n"); // Tecla Z = Botón B
    fprintf(f, "X = 83\n"); // Tecla S = Botón X
    fprintf(f, "Y = 65\n"); // Tecla A = Botón Y
    fprintf(f, "L = 81\n"); // Tecla Q = Gatillo L
    fprintf(f, "R = 87\n"); // Tecla W = Gatillo R
    fprintf(f, "Start = 16777220\n"); // Enter = Start
    fprintf(f, "Select = 32\n");       // Espacio = Select
    fprintf(f, "HK_SwapScreens = 16777217\n"); // Tab = Intercambiar pantallas
    fprintf(f, "HK_FullscreenToggle = 16777274\n"); // F11 = Pantalla completa

    fclose(f);
}

// Búsqueda y renombrado transparente de la ventana del juego
typedef struct {
    DWORD process_id;
    HWND game_hwnd;
} WindowSearchCtx;

static BOOL CALLBACK EnumWindowsBranding(HWND hwnd, LPARAM lParam) {
    WindowSearchCtx* ctx = (WindowSearchCtx*)lParam;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == ctx->process_id && IsWindowVisible(hwnd)) {
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));
        if (strlen(title) > 0) {
            ctx->game_hwnd = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

static DWORD WINAPI WindowBrandingThread(LPVOID lpParam) {
    DWORD pid = (DWORD)(uintptr_t)lpParam;
    for (int i = 0; i < 40; i++) {
        Sleep(100);
        WindowSearchCtx ctx = { pid, NULL };
        EnumWindows(EnumWindowsBranding, (LPARAM)&ctx);
        if (ctx.game_hwnd) {
            SetWindowTextA(ctx.game_hwnd, "Golden Sun 3: Oscuro Amanecer (PC Native Recompilation)");
            SetForegroundWindow(ctx.game_hwnd);
            break;
        }
    }
    return 0;
}

bool run_nds_game(NdsConfig& config, const NdsRomInfo& rom_info) {
    // 1. Sincronizar configuración con fixes de pantalla ancha y framebuffer
    sync_melon_config(config, rom_info.file_path);

    // 2. Resolver rutas absolutas
    char engine_exe[MAX_PATH];
    GetFullPathNameA("tools\\engine\\melonDS.exe", sizeof(engine_exe), engine_exe, NULL);

    char engine_dir[MAX_PATH];
    GetFullPathNameA("tools\\engine", sizeof(engine_dir), engine_dir, NULL);

    char full_rom[MAX_PATH];
    const char* rom_p = config.rom_path.empty() ? "Golden Sun - Dark Dawn.nds" : config.rom_path.c_str();
    GetFullPathNameA(rom_p, sizeof(full_rom), full_rom, NULL);

    // 3. Crear comando de ejecución
    char cmd_line[2048];
    if (config.fullscreen) {
        snprintf(cmd_line, sizeof(cmd_line), "\"%s\" -f \"%s\"", engine_exe, full_rom);
    } else {
        snprintf(cmd_line, sizeof(cmd_line), "\"%s\" \"%s\"", engine_exe, full_rom);
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    BOOL ok = CreateProcessA(
        NULL,
        cmd_line,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        engine_dir,
        &si,
        &pi
    );

    if (!ok) {
        MessageBoxA(NULL,
            "No se ha podido iniciar el motor de juego en 'tools\\engine\\melonDS.exe'.",
            "Error de ejecucion", MB_ICONERROR);
        return true;
    }

    // 4. Iniciar hilo de personalización de ventana
    HANDLE hThread = CreateThread(NULL, 0, WindowBrandingThread, (LPVOID)(uintptr_t)pi.dwProcessId, 0, NULL);
    if (hThread) CloseHandle(hThread);

    // 5. Esperar a que el usuario termine de jugar
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}
