/* =============================================================================
 * NINTENDO DS RECOMPILATION - GOLDEN SUN 3: OSCURO AMANECER
 * Main Entry Point
 * ============================================================================= */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <cstdio>
#include <windows.h>
#include "nds_config.h"
#include "nds_rom_parser.h"
#include "nds_modern_launcher.h"
#include "nds_game_runner.h"

int run_app(int argc, char* argv[])
{
    SDL_SetMainReady();
    SetProcessDPIAware();

    // Cargar configuracion previa de settings.toml
    g_nds_cfg.load_from_file("settings.toml");

    // Verificar y analizar la ROM de Golden Sun 3
    NdsRomInfo rom_info;
    if (!parse_nds_rom(g_nds_cfg.rom_path, rom_info)) {
        parse_nds_rom("Golden Sun - Dark Dawn.nds", rom_info);
    }

    // Inicializar SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
        MessageBoxA(NULL, SDL_GetError(), "Error inicializando SDL2", MB_ICONERROR | MB_OK);
        return 1;
    }

    bool app_running = true;
    while (app_running) {
        // 1. Mostrar Lanzador y Suite de Mejoras Visuales
        NdsLauncherAction action = run_nds_modern_launcher(g_nds_cfg, rom_info);

        if (action == NDS_ACTION_LAUNCH) {
            // 2. Iniciar la ejecucion nativa del juego en PC
            bool return_to_launcher = run_nds_game(g_nds_cfg, rom_info);
            if (!return_to_launcher) {
                app_running = false;
            }
        } else {
            app_running = false;
        }
    }

    SDL_Quit();
    return 0;
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return run_app(__argc, __argv);
}
#endif

int main(int argc, char* argv[])
{
    return run_app(argc, argv);
}

