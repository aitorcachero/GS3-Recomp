/* =============================================================================
 * NINTENDO DS RECOMPILATION - GOLDEN SUN 3: OSCURO AMANECER
 * Native PC Game Runner & Dual-Screen Presentation Engine
 * Solucion tecnica para Framebuffer Capture, Pantalla Ancha (16:9/16:10), AF y Cheats Hor+
 * ============================================================================= */

#include "nds_game_runner.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

// Sincronizar codigos Action Replay Hor+ Widescreen en formato nativo melonDS (.mch)
static void sync_melon_cheats(const NdsConfig& config, const std::string& rom_path) {
    bool enable_16_9 = (config.widescreen_3d_fov && config.aspect_ratio_mode == NDS_ASPECT_16_9);
    bool enable_16_10 = (config.widescreen_3d_fov && config.aspect_ratio_mode == NDS_ASPECT_16_10);

    // Formato nativo melonDS para codigos Action Replay
    std::string mch_content;
    mch_content += "CAT Widescreen Hor+\n";
    mch_content += "CODE " + std::string(enable_16_9 ? "1" : "0") + " Widescreen Hor+ 16:9 (Proyeccion 3D)\n";
    mch_content += "920793DC 00001555\n";
    mch_content += "120793DC 00001C72\n";
    mch_content += "D2000000 00000000\n";
    mch_content += "CODE " + std::string(enable_16_10 ? "1" : "0") + " Widescreen Hor+ 16:10 (Steam Deck)\n";
    mch_content += "920793DC 00001555\n";
    mch_content += "120793DC 0000199A\n";
    mch_content += "D2000000 00000000\n";

    // 1. Escribir junto a la ROM con el nombre correspondiente
    std::string rom_mch = rom_path.empty() ? "Golden Sun - Dark Dawn.nds" : rom_path;
    size_t dot_pos = rom_mch.find_last_of('.');
    if (dot_pos != std::string::npos) {
        rom_mch = rom_mch.substr(0, dot_pos) + ".mch";
    } else {
        rom_mch += ".mch";
    }

    FILE* f1 = fopen(rom_mch.c_str(), "w");
    if (f1) {
        fputs(mch_content.c_str(), f1);
        fclose(f1);
    }

    // 2. Escribir en la carpeta de melonDS y raiz como fallback
    FILE* f2 = fopen("Golden Sun - Dark Dawn.mch", "w");
    if (f2) {
        fputs(mch_content.c_str(), f2);
        fclose(f2);
    }

    FILE* f3 = fopen("tools\\engine\\Golden Sun - Dark Dawn.mch", "w");
    if (f3) {
        fputs(mch_content.c_str(), f3);
        fclose(f3);
    }
}

// Sincronizar ajustes del usuario con la configuracion del motor de Nintendo DS
static void sync_melon_config(const NdsConfig& config, const std::string& rom_path) {
    char toml_path[MAX_PATH];
    GetFullPathNameA("tools\\engine\\melonDS.toml", sizeof(toml_path), toml_path, NULL);

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

    // Aspect ratio en melonDS: 0 = 4:3 (native), 1 = 5:3 (3DS / Steam Deck 16:10), 2 = 16:9, 3 = 21:9
    int aspect_val = 0;
    if (config.aspect_ratio_mode == NDS_ASPECT_16_9) aspect_val = 2;
    else if (config.aspect_ratio_mode == NDS_ASPECT_16_10) aspect_val = 1;

    int aspect_top = aspect_val;
    int aspect_bot = aspect_val;
    if (config.widescreen_target_screen == 1) { // Solo pantalla 3D inferior
        aspect_top = 0;
        aspect_bot = aspect_val;
    } else if (config.widescreen_target_screen == 2) { // Solo pantalla superior
        aspect_top = aspect_val;
        aspect_bot = 0;
    }

    // Leer lineas existentes de melonDS.toml si el archivo ya existe
    std::vector<std::string> lines;
    std::ifstream infile(toml_path);
    if (infile.is_open()) {
        std::string line;
        while (std::getline(infile, line)) {
            lines.push_back(line);
        }
        infile.close();
    }

    // Diccionario de claves a sincronizar
    std::map<std::string, std::map<std::string, std::string>> updates;
    updates["3D"]["Renderer"] = "1"; // OpenGL acelerado por hardware
    updates["3D.GL"]["ScaleFactor"] = std::to_string(melon_scale); // 1x, 2x, 3x, 4x, 6x, 8x (4K)
    updates["3D.GL"]["BetterPolygons"] = config.subpixel_rasterizer ? "true" : "false";
    updates["3D.GL"]["AnisotropicFiltering"] = std::to_string(config.anisotropic_filter);
    updates["3D.GL"]["CustomTextures"] = config.custom_textures_enabled ? "true" : "false";

    updates["Screen"]["VSync"] = config.vsync ? "true" : "false";
    updates["Screen"]["VSyncInterval"] = "1";
    updates["Screen"]["UseGL"] = "true";

    updates["Instance0.Window0"]["ScreenAspectTop"] = std::to_string(aspect_top);
    updates["Instance0.Window0"]["ScreenAspectBot"] = std::to_string(aspect_bot);
    updates["Instance0.Window0"]["ScreenLayout"] = std::to_string(melon_layout);
    updates["Instance0.Window0"]["ScreenGap"] = std::to_string(config.screen_gap);
    updates["Instance0.Window0"]["ScreenSwap"] = config.swap_screens ? "true" : "false";
    updates["Instance0.Window0"]["IntegerScaling"] = config.integer_scaling ? "true" : "false";
    updates["Instance0.Window0"]["ScreenFilter"] = (config.texture_filter != 0) ? "true" : "false";

    updates["Instance0"]["EnableCheats"] = "true";
    updates["Instance0"]["SaveFilePath"] = "\"../../saves\"";
    updates["Instance0"]["CheatFilePath"] = "\"\"";

    updates["Instance0.Audio"]["Volume"] = std::to_string(melon_volume);
    updates["Instance0.Firmware"]["Language"] = std::to_string(config.selected_language);

    if (lines.empty()) {
        // Generar plantilla completa si no existia
        std::ofstream outfile(toml_path);
        outfile << "# melonDS configuration for Golden Sun 3 Recompilation\n";
        outfile << "UITheme = \"\"\n";
        outfile << "AudioSync = true\n";
        outfile << "PauseLostFocus = false\n";
        outfile << "TargetFPS = 60.0\n";
        outfile << "LimitFPS = true\n\n";

        for (auto& sec : updates) {
            outfile << "[" << sec.first << "]\n";
            for (auto& kv : sec.second) {
                outfile << kv.first << " = " << kv.second << "\n";
            }
            outfile << "\n";
        }
        outfile.close();
    } else {
        std::vector<std::string> new_lines;
        std::string current_sec = "";

        auto flush_remaining = [&](const std::string& sec) {
            auto it = updates.find(sec);
            if (it != updates.end()) {
                for (auto& kv : it->second) {
                    new_lines.push_back(kv.first + " = " + kv.second);
                }
                updates.erase(it);
            }
        };

        for (const auto& line : lines) {
            std::string trimmed = line;
            while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == ' ' || trimmed.back() == '\t')) trimmed.pop_back();
            size_t start = 0;
            while (start < trimmed.size() && (trimmed[start] == ' ' || trimmed[start] == '\t')) start++;
            trimmed = trimmed.substr(start);

            if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']') {
                flush_remaining(current_sec);
                current_sec = trimmed.substr(1, trimmed.size() - 2);
                new_lines.push_back(line);
                continue;
            }

            // Eliminar clave antigua/duplicada Scale = X en 3D.GL
            if (current_sec == "3D.GL" && (trimmed.rfind("Scale = ", 0) == 0 || trimmed.rfind("Scale=", 0) == 0)) {
                continue;
            }

            size_t eq_pos = trimmed.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = trimmed.substr(0, eq_pos);
                while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();

                auto sec_it = updates.find(current_sec);
                if (sec_it != updates.end()) {
                    auto key_it = sec_it->second.find(key);
                    if (key_it != sec_it->second.end()) {
                        new_lines.push_back(key + " = " + key_it->second);
                        sec_it->second.erase(key_it);
                        continue;
                    }
                }
            }

            new_lines.push_back(line);
        }

        flush_remaining(current_sec);

        // Secciones nuevas que no existian en el archivo original
        for (auto& sec : updates) {
            if (!sec.second.empty()) {
                new_lines.push_back("\n[" + sec.first + "]");
                for (auto& kv : sec.second) {
                    new_lines.push_back(kv.first + " = " + kv.second);
                }
            }
        }

        std::ofstream outfile(toml_path);
        for (const auto& nl : new_lines) {
            outfile << nl << "\n";
        }
        outfile.close();
    }
}

// Busqueda y renombrado transparente de la ventana del juego
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
    // 1. Sincronizar configuracion grafica (ScaleFactor, VSync, Aspect Ratio 16:9/16:10, AF)
    sync_melon_config(config, rom_info.file_path);

    // 2. Inyectar codigos Action Replay Hor+ Widescreen en formato nativo melonDS (.mch)
    sync_melon_cheats(config, rom_info.file_path);

    // 3. Resolver rutas absolutas
    char engine_exe[MAX_PATH];
    GetFullPathNameA("tools\\engine\\melonDS.exe", sizeof(engine_exe), engine_exe, NULL);

    char engine_dir[MAX_PATH];
    GetFullPathNameA("tools\\engine", sizeof(engine_dir), engine_dir, NULL);

    char full_rom[MAX_PATH];
    const char* rom_p = config.rom_path.empty() ? "Golden Sun - Dark Dawn.nds" : config.rom_path.c_str();
    GetFullPathNameA(rom_p, sizeof(full_rom), full_rom, NULL);

    // 4. Crear comando de ejecucion
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

    // 5. Iniciar hilo de personalizacion de ventana
    HANDLE hThread = CreateThread(NULL, 0, WindowBrandingThread, (LPVOID)(uintptr_t)pi.dwProcessId, 0, NULL);
    if (hThread) CloseHandle(hThread);

    // 6. Esperar a que el usuario termine de jugar
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}
