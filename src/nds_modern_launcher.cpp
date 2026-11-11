/* =============================================================================
 * NINTENDO DS RECOMPILATION - GOLDEN SUN 3: OSCURO AMANECER (DARK DAWN)
 * Modern PC Launcher & Visual Enhancement Suite
 * Arquitectura modular de alto rendimiento para recompilaciones nativas NDS
 * ============================================================================= */

#include "nds_modern_launcher.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <windows.h>
#include <commdlg.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

static UINT_PTR CALLBACK BrowseHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
    if (uiMsg == WM_INITDIALOG) {
        HWND hParent = GetParent(hdlg);
        HWND target = hParent ? hParent : hdlg;
        SetWindowPos(target, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        BringWindowToTop(target);
        SetForegroundWindow(target);
        SetActiveWindow(target);
    }
    return 0;
}

static bool prompt_browse_rom(SDL_Window *window, char *out_path, size_t max_len) {
    HWND hwnd = NULL;
    if (window) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(window, &wmInfo)) {
            hwnd = wmInfo.info.win.window;
        }
    }
    if (!hwnd) {
        hwnd = GetActiveWindow();
        if (!hwnd) hwnd = GetForegroundWindow();
    }

    if (hwnd) EnableWindow(hwnd, FALSE);

    char file_buf[MAX_PATH] = { 0 };
    if (out_path && out_path[0]) {
        strncpy(file_buf, out_path, sizeof(file_buf) - 1);
    }

    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrTitle = "Selecciona la ROM de Nintendo DS (.nds, .srl)";
    ofn.lpstrFilter = "Nintendo DS ROM (*.nds;*.srl)\0*.nds;*.srl\0Todos los archivos (*.*)\0*.*\0";
    ofn.lpstrFile = file_buf;
    ofn.nMaxFile = (DWORD)sizeof(file_buf);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_ENABLEHOOK | OFN_ENABLESIZING;
    ofn.lpfnHook = BrowseHookProc;

    BOOL ok = GetOpenFileNameA(&ofn);

    if (hwnd) {
        EnableWindow(hwnd, TRUE);
        SetForegroundWindow(hwnd);
        SetActiveWindow(hwnd);
    }

    if (ok && file_buf[0]) {
        strncpy(out_path, file_buf, max_len - 1);
        out_path[max_len - 1] = '\0';
        return true;
    }
    return false;
}

static GLuint load_texture_from_bmp(const char *path, int *out_w, int *out_h) {
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) return 0;

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_BGR;
    if (surface->format->BytesPerPixel == 4) {
        format = GL_BGRA;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    if (out_w) *out_w = surface->w;
    if (out_h) *out_h = surface->h;

    SDL_FreeSurface(surface);
    return tex_id;
}

static GLuint load_texture_from_rgba(const uint8_t *rgba_data, int w, int h) {
    if (!rgba_data) return 0;

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);

    return tex_id;
}

NdsLauncherAction run_nds_modern_launcher(NdsConfig& config, NdsRomInfo& rom_info) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        dm.w = 1920;
        dm.h = 1080;
    }

    int win_w = dm.w;
    int win_h = dm.h;

    SDL_Window *window = SDL_CreateWindow(
        "Nintendo DS Recomp - Golden Sun 3: Oscuro Amanecer [Modern Suite]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        window = SDL_CreateWindow(
            "Nintendo DS Recomp - Golden Sun 3: Oscuro Amanecer [Modern Suite]",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
        );
    }
    if (!window) return NDS_ACTION_QUIT;

    SDL_GetWindowSize(window, &win_w, &win_h);

    int detected_hz = (dm.refresh_rate > 0) ? dm.refresh_rate : 60;
    int disp_idx = SDL_GetWindowDisplayIndex(window);
    if (disp_idx >= 0 && SDL_GetCurrentDisplayMode(disp_idx, &dm) == 0 && dm.refresh_rate > 0) {
        detected_hz = dm.refresh_rate;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(config.vsync ? 1 : 0);

    float scale = (float)win_h / 1080.0f;
    if (scale < 0.85f) scale = 0.85f;
    if (scale > 1.80f) scale = 1.80f;

    float scale_font = scale;
    if (scale_font > 1.25f) scale_font = 1.25f;
    if (scale_font < 0.90f) scale_font = 0.90f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.IniFilename = NULL;

    ImFont *font_title   = NULL;
    ImFont *font_large   = NULL;
    ImFont *font_bold    = NULL;
    ImFont *font_regular = NULL;
    ImFont *font_small   = NULL;

    const char *bold_candidates[] = {
        "assets/fonts/LatoLatin-Bold.ttf",
        "../assets/fonts/LatoLatin-Bold.ttf",
        NULL
    };
    const char *reg_candidates[] = {
        "assets/fonts/LatoLatin-Regular.ttf",
        "../assets/fonts/LatoLatin-Regular.ttf",
        NULL
    };

    const char *found_bold = NULL;
    for (int i = 0; bold_candidates[i]; i++) {
        if (GetFileAttributesA(bold_candidates[i]) != INVALID_FILE_ATTRIBUTES) {
            found_bold = bold_candidates[i];
            break;
        }
    }
    const char *found_reg = NULL;
    for (int i = 0; reg_candidates[i]; i++) {
        if (GetFileAttributesA(reg_candidates[i]) != INVALID_FILE_ATTRIBUTES) {
            found_reg = reg_candidates[i];
            break;
        }
    }

    static const ImWchar latin_ranges[] = {
        0x0020, 0x00FF,
        0x2010, 0x2027,
        0,
    };

    if (found_bold) {
        font_title = io.Fonts->AddFontFromFileTTF(found_bold, 28.0f * scale_font, NULL, latin_ranges);
        font_large = io.Fonts->AddFontFromFileTTF(found_bold, 20.0f * scale_font, NULL, latin_ranges);
        font_bold  = io.Fonts->AddFontFromFileTTF(found_bold, 15.0f * scale_font, NULL, latin_ranges);
    }
    if (found_reg) {
        font_regular = io.Fonts->AddFontFromFileTTF(found_reg, 14.5f * scale_font, NULL, latin_ranges);
        font_small   = io.Fonts->AddFontFromFileTTF(found_reg, 12.5f * scale_font, NULL, latin_ranges);
    }
    if (!font_regular) {
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    int poster_w = 0, poster_h = 0;
    GLuint poster_tex = load_texture_from_bmp("assets/img/goldensun_poster.bmp", &poster_w, &poster_h);
    if (!poster_tex) poster_tex = load_texture_from_bmp("../assets/img/goldensun_poster.bmp", &poster_w, &poster_h);

    int top_prev_w = 0, top_prev_h = 0;
    GLuint top_preview_tex = load_texture_from_bmp("assets/img/nds_preview_top.bmp", &top_prev_w, &top_prev_h);

    int bot_prev_w = 0, bot_prev_h = 0;
    GLuint bot_preview_tex = load_texture_from_bmp("assets/img/nds_preview_bot.bmp", &bot_prev_w, &bot_prev_h);

    GLuint rom_icon_tex = 0;
    if (rom_info.banner.valid) {
        rom_icon_tex = load_texture_from_rgba(rom_info.banner.icon_rgba, 32, 32);
    } else {
        int icon_w = 0, icon_h = 0;
        rom_icon_tex = load_texture_from_bmp("assets/img/nds_icon.bmp", &icon_w, &icon_h);
    }

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 5.0f * scale;
    style.ItemSpacing       = ImVec2(10.0f * scale, 8.0f * scale);
    style.ItemInnerSpacing  = ImVec2(8.0f * scale, 6.0f * scale);
    style.ScrollbarRounding = 4.0f * scale;
    style.GrabRounding      = 4.0f * scale;
    style.FramePadding      = ImVec2(10.0f * scale, 6.0f * scale);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.040f, 0.055f, 0.090f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.065f, 0.090f, 0.145f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.160f, 0.220f, 0.320f, 1.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.090f, 0.130f, 0.200f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.130f, 0.180f, 0.270f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.170f, 0.230f, 0.340f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.040f, 0.055f, 0.090f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.040f, 0.055f, 0.090f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(1.000f, 0.720f, 0.000f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(1.000f, 0.700f, 0.000f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(1.000f, 0.820f, 0.250f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.100f, 0.145f, 0.220f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.160f, 0.220f, 0.320f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.220f, 0.300f, 0.420f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(1.000f, 0.650f, 0.000f, 0.28f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(1.000f, 0.700f, 0.000f, 0.45f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(1.000f, 0.750f, 0.000f, 0.65f);
    colors[ImGuiCol_Text]                 = ImVec4(0.950f, 0.965f, 0.985f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.550f, 0.630f, 0.720f, 1.00f);

    int active_tab = 0;
    bool running = true;
    NdsLauncherAction result_action = NDS_ACTION_NONE;
    float toast_timer = 0.0f;
    std::string toast_msg = "";

    float touch_sim_x = 128.0f;
    float touch_sim_y = 96.0f;
    bool touch_active = false;

    char rom_path_buf[MAX_PATH] = { 0 };
    strncpy(rom_path_buf, config.rom_path.c_str(), sizeof(rom_path_buf) - 1);

    Uint64 last_tick = SDL_GetPerformanceCounter();
    double perf_freq = (double)SDL_GetPerformanceFrequency();

    while (running) {
        Uint64 current_tick = SDL_GetPerformanceCounter();
        float delta_time = (float)((current_tick - last_tick) / perf_freq);
        last_tick = current_tick;

        if (toast_timer > 0.0f) {
            toast_timer -= delta_time;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
                result_action = NDS_ACTION_QUIT;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                    result_action = NDS_ACTION_QUIT;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN || event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER) {
                    running = false;
                    result_action = NDS_ACTION_LAUNCH;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)win_w, (float)win_h));
        ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("NDS_Recomp_Launcher", NULL, win_flags);

        // Header Bar
        ImGui::BeginChild("HeaderBar", ImVec2(0, 70.0f * scale), true, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::Columns(2, "HeaderCols", false);
            ImGui::SetColumnWidth(0, win_w * 0.65f);

            if (rom_icon_tex) {
                ImGui::Image((ImTextureID)(uintptr_t)rom_icon_tex, ImVec2(44.0f * scale, 44.0f * scale));
                ImGui::SameLine();
            }

            ImGui::BeginGroup();
            if (font_large) ImGui::PushFont(font_large);
            ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.15f, 1.0f), "GOLDEN SUN: OSCURO AMANECER");
            if (font_large) ImGui::PopFont();

            if (font_small) ImGui::PushFont(font_small);
            ImGui::TextColored(ImVec4(0.65f, 0.75f, 0.90f, 1.0f),
                "Nintendo DS 64-Bit Native PC Recompilation Engine | ROM: %s (%s)",
                rom_info.game_code, rom_info.get_region_string().c_str());
            if (font_small) ImGui::PopFont();
            ImGui::EndGroup();

            ImGui::NextColumn();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f * scale);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "[MOTOR RECOMP: LISTO]");
            ImGui::SameLine();
            ImGui::TextDisabled("| Monitor: %dx%d @ %dHz | VSync: %s", win_w, win_h, detected_hz, config.vsync ? "ON" : "OFF");

            ImGui::Columns(1);
        }
        ImGui::EndChild();

        // Main Body
        float content_h = win_h - (70.0f + 62.0f) * scale;
        float poster_display_w = 330.0f * scale;

        ImGui::BeginChild("PosterPanel", ImVec2(poster_display_w, content_h), true, ImGuiWindowFlags_NoScrollbar);
        {
            if (poster_tex) {
                float aspect = (float)poster_h / (float)poster_w;
                float draw_w = poster_display_w - 20.0f * scale;
                float draw_h = draw_w * aspect;
                if (draw_h > content_h - 20.0f * scale) {
                    draw_h = content_h - 20.0f * scale;
                    draw_w = draw_h / aspect;
                }
                ImGui::SetCursorPosX((poster_display_w - draw_w) * 0.5f);
                ImGui::Image((ImTextureID)(uintptr_t)poster_tex, ImVec2(draw_w, draw_h));
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Golden Sun: Dark Dawn");
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("MainTabsPanel", ImVec2(0, content_h), true);
        {
            if (font_bold) ImGui::PushFont(font_bold);

            auto TabButton = [&](const char* label, int tab_idx, ImVec4 highlight_col) {
                bool is_active = (active_tab == tab_idx);
                if (is_active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, highlight_col);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.08f, 0.12f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.90f, 0.98f, 1.0f));
                }
                float btn_w = ImGui::CalcTextSize(label).x + 36.0f * scale;
                if (btn_w < 180.0f * scale) btn_w = 180.0f * scale;
                if (ImGui::Button(label, ImVec2(btn_w, 40.0f * scale))) {
                    active_tab = tab_idx;
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
            };

            TabButton("DASHBOARD", 0, ImVec4(1.0f, 0.75f, 0.15f, 1.0f));
            TabButton("MEJORAS VISUALES", 1, ImVec4(0.2f, 0.75f, 1.0f, 1.0f));
            TabButton("PANTALLA TACTIL", 2, ImVec4(0.3f, 0.9f, 0.5f, 1.0f));
            TabButton("AUDIO & SPU", 3, ImVec4(0.85f, 0.45f, 1.0f, 1.0f));
            TabButton("PARTIDAS & SAVES", 4, ImVec4(1.0f, 0.45f, 0.35f, 1.0f));

            if (font_bold) ImGui::PopFont();
            ImGui::NewLine();
            ImGui::Separator();
            ImGui::Spacing();

            // ---------------------------------------------------------------------
            // TAB 0: DASHBOARD / JUGAR
            // ---------------------------------------------------------------------
            if (active_tab == 0) {
                ImGui::BeginChild("QuickLaunchCard", ImVec2(0, 160.0f * scale), true);
                {
                    if (font_large) ImGui::PushFont(font_large);
                    ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.2f, 1.0f), "%s", rom_info.get_display_title(config.selected_language).c_str());
                    if (font_large) ImGui::PopFont();

                    ImGui::TextDisabled("Archivo ROM: %s | Tamano: %s", rom_info.file_name.c_str(), rom_info.get_capacity_string().c_str());
                    ImGui::Spacing();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.78f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.55f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.10f, 1.0f));

                    if (font_bold) ImGui::PushFont(font_bold);
                    if (ImGui::Button("INICIAR JUEGO EN PC (ENTER)", ImVec2(380.0f * scale, 48.0f * scale))) {
                        running = false;
                        result_action = NDS_ACTION_LAUNCH;
                    }
                    if (font_bold) ImGui::PopFont();
                    ImGui::PopStyleColor(4);

                    ImGui::SameLine();
                    if (ImGui::Button("Examinar ROM...", ImVec2(190.0f * scale, 48.0f * scale))) {
                        char picked_path[MAX_PATH] = { 0 };
                        if (prompt_browse_rom(window, picked_path, sizeof(picked_path))) {
                            config.rom_path = picked_path;
                            strncpy(rom_path_buf, picked_path, sizeof(rom_path_buf) - 1);
                            parse_nds_rom(picked_path, rom_info);
                            toast_msg = "ROM cargada con exito: " + rom_info.file_name;
                            toast_timer = 3.5f;
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::Spacing();

                if (font_bold) ImGui::PushFont(font_bold);
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.15f, 1.0f), "PERFILES GRAFICOS PRECONFIGURADOS:");
                if (font_bold) ImGui::PopFont();

                auto PresetButton = [&](const char* title, const char* desc, int preset_id, ImVec4 btn_col) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.09f, 0.13f, 0.20f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Border, btn_col);
                    ImGui::BeginChild(title, ImVec2(245.0f * scale, 140.0f * scale), true);
                    {
                        if (font_bold) ImGui::PushFont(font_bold);
                        ImGui::TextColored(btn_col, "%s", title);
                        if (font_bold) ImGui::PopFont();

                        if (font_small) ImGui::PushFont(font_small);
                        ImGui::TextWrapped("%s", desc);
                        if (font_small) ImGui::PopFont();

                        ImGui::SetCursorPosY(140.0f * scale - 38.0f * scale);
                        if (ImGui::Button("Aplicar Perfil", ImVec2(-1, 32.0f * scale))) {
                            config.apply_preset(preset_id);
                            toast_msg = std::string("Perfil aplicado: ") + title;
                            toast_timer = 2.5f;
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor(2);
                    ImGui::SameLine();
                };

                PresetButton("Ultra 4K UHD", "8x Resolucion 3D, 16:9 Panoramico, MSAA 8x, AF 16x, Texturas HD.", 0, ImVec4(1.0f, 0.8f, 0.1f, 1.0f));
                PresetButton("Equilibrado 1080p", "4x Resolucion 3D, 16:9 Panoramico, MSAA 4x, 60+ FPS sin caidas.", 1, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
                PresetButton("Fiel Pixel-Art", "1x Nativa (256x192), 4:3 Clasico, Rejilla LCD DS, Bisagra original.", 2, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
                PresetButton("Steam Deck / 16:10", "4x Resolucion 3D, 16:10 Panoramico nativo, Enfoque Superior 3D.", 3, ImVec4(0.9f, 0.4f, 1.0f, 1.0f));
                ImGui::NewLine();

                ImGui::Spacing();

                ImGui::BeginChild("RomMetadataCard", ImVec2(0, 0), true);
                {
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.15f, 1.0f), "INFORMACION TECNICA DEL CARTUCHO:");
                    if (font_bold) ImGui::PopFont();

                    ImGui::Columns(2, "TechDataCols", false);
                    ImGui::Text("Codigo de Juego:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "%s", rom_info.game_code);
                    ImGui::Text("Fabricante:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s (Nintendo)", rom_info.maker_code);
                    ImGui::Text("Region / Idiomas:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), "%s", rom_info.get_region_string().c_str());
                    ImGui::Text("Checksum CRC32:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "0x%08X", rom_info.computed_crc32);

                    ImGui::NextColumn();
                    ImGui::Text("Entry ARM9 (Juego 3D):"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "0x%08X (RAM: 0x%08X)", rom_info.arm9_entry, rom_info.arm9_ram_addr);
                    ImGui::Text("Entry ARM7 (Audio/Tactil):"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "0x%08X", rom_info.arm7_entry);
                    ImGui::Text("Tamano ARM9 / ARM7:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%.1f KB / %.1f KB", rom_info.arm9_size / 1024.0, rom_info.arm7_size / 1024.0);
                    ImGui::Text("Estado de Guardado:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Bateria Flash 512KB (OK)");
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
            }

            // ---------------------------------------------------------------------
            // TAB 1: MEJORAS VISUALES (PANTALLA ANCHA 16:9/16:10, AF, TEXTURAS, FIX)
            // ---------------------------------------------------------------------
            else if (active_tab == 1) {
                ImGui::Columns(2, "VisualCols", false);
                ImGui::SetColumnWidth(0, (win_w - poster_display_w) * 0.49f);

                if (font_bold) ImGui::PushFont(font_bold);
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "RELACION DE ASPECTO & PANTALLA ANCHA (WIDESCREEN):");
                if (font_bold) ImGui::PopFont();

                const char* aspect_names[] = {
                    "4:3 Estandar DS (Aspecto original de hardware)",
                    "16:9 Panoramico Nativo (Monitores PC, TV 4K, Laptops)",
                    "16:10 Panoramico Nativo (Steam Deck 1280x800, MacBooks)"
                };
                ImGui::Combo("Relacion de Aspecto 3D", &config.aspect_ratio_mode, aspect_names, 3);
                ImGui::Checkbox("Widescreen 3D FOV Hack (Modifica matriz de proyeccion de camara)", (bool*)&config.widescreen_3d_fov);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (font_bold) ImGui::PushFont(font_bold);
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "FIXES ESPECIALES GOLDEN SUN 3 (FRAMEBUFFER & TEXTURAS):");
                if (font_bold) ImGui::PopFont();

                ImGui::Checkbox("Fix Display Capture HD (Elimina alternancia HD/SD en Invocaciones y Psinergia)", (bool*)&config.fix_hd_framebuffer);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Evita que las capturas de post-procesado (auras, resplandores y cinemáticas) reduzcan el frame a 256x192.");
                }

                ImGui::Checkbox("Cargar Texturas HD Personalizadas (textures/goldensun3/)", (bool*)&config.custom_textures_enabled);
                ImGui::SameLine();
                if (ImGui::Button("Abrir Carpeta Texturas", ImVec2(170.0f * scale, 28.0f * scale))) {
                    system("explorer textures\\goldensun3");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (font_bold) ImGui::PushFont(font_bold);
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "DISTRIBUCION DE PANTALLAS (DUAL-SCREEN):");
                if (font_bold) ImGui::PopFont();

                const char* layout_names[] = {
                    "Vertical Clasica DS (Superior arriba, Tactil abajo)",
                    "Horizontal Panoramica (Lado a lado)",
                    "Hibrido Enfoque 3D (Superior Gigante + PiP Tactil)",
                    "Hibrido Enfoque Tactil (Tactil Gigante + PiP 3D)",
                    "Pantalla Unica Conmutable (Alternar con TAB)"
                };
                ImGui::Combo("Modo de Pantallas", &config.screen_layout, layout_names, 5);

                ImGui::SliderInt("Separacion de Pantallas (Gap)", &config.screen_gap, 0, 96, "%d px");
                ImGui::Checkbox("Intercambiar Pantallas (Swap)", (bool*)&config.swap_screens);
                ImGui::SameLine();
                ImGui::Checkbox("Escalado Entero", (bool*)&config.integer_scaling);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (font_bold) ImGui::PushFont(font_bold);
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MEJORAS 3D, ANTIALIASING (MSAA) & FILTRADO (AF):");
                if (font_bold) ImGui::PopFont();

                const char* res_names[] = {
                    "1x Nativa DS (256 x 192)",
                    "2x HD (512 x 384)",
                    "3x 720p (768 x 576)",
                    "4x 1080p Full HD (1024 x 768)",
                    "6x 2K QHD (1536 x 1152)",
                    "8x 4K Ultra HD (2048 x 1536)"
                };
                int res_indices[] = { 1, 2, 3, 4, 6, 8 };
                int current_res_idx = 3;
                for (int i = 0; i < 6; i++) {
                    if (config.internal_resolution_scale == res_indices[i]) {
                        current_res_idx = i;
                        break;
                    }
                }
                if (ImGui::Combo("Resolucion Interna 3D", &current_res_idx, res_names, 6)) {
                    config.internal_resolution_scale = res_indices[current_res_idx];
                }

                const char* msaa_names[] = { "Desactivado", "2x MSAA", "4x MSAA", "8x MSAA", "16x MSAA" };
                int msaa_vals[] = { 0, 2, 4, 8, 16 };
                int current_msaa_idx = 2;
                for (int i = 0; i < 5; i++) {
                    if (config.antialiasing_msaa == msaa_vals[i]) current_msaa_idx = i;
                }
                if (ImGui::Combo("Antialiasing Geometria (MSAA)", &current_msaa_idx, msaa_names, 5)) {
                    config.antialiasing_msaa = msaa_vals[current_msaa_idx];
                }

                const char* aniso_names[] = { "Desactivado", "2x AF", "4x AF", "8x AF", "16x AF Ultra" };
                int aniso_vals[] = { 0, 2, 4, 8, 16 };
                int current_aniso_idx = 4;
                for (int i = 0; i < 5; i++) {
                    if (config.anisotropic_filter == aniso_vals[i]) current_aniso_idx = i;
                }
                if (ImGui::Combo("Filtrado Anisotropo (AF)", &current_aniso_idx, aniso_names, 5)) {
                    config.anisotropic_filter = aniso_vals[current_aniso_idx];
                }

                ImGui::Checkbox("Correccion de Perspectiva 3D", (bool*)&config.perspective_correction);
                ImGui::SameLine();
                ImGui::Checkbox("Rasterizador Subpixel (Elimina temblor)", (bool*)&config.subpixel_rasterizer);

                // =================================================================
                // LADO DERECHO: PREVISUALIZACIÓN Y DETALLES TÉCNICOS
                // =================================================================
                ImGui::NextColumn();

                ImGui::BeginChild("LiveDisplayPreviewBox", ImVec2(0, 0), true);
                {
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "PREVISUALIZACION EN VIVO (MOTOR DE RENDER DUAL):");
                    if (font_bold) ImGui::PopFont();

                    const char* aspect_labels[] = { "4:3 Estandar", "16:9 Panoramico", "16:10 Steam Deck" };
                    ImGui::TextDisabled("Resolucion 3D: %dx | Aspecto: %s | Anti-Flicker: %s",
                        config.internal_resolution_scale,
                        aspect_labels[config.aspect_ratio_mode],
                        config.fix_hd_framebuffer ? "ACTIVO" : "DESACTIVADO");

                    ImGui::Spacing();

                    float preview_box_w = ImGui::GetContentRegionAvail().x;
                    float preview_box_h = ImGui::GetContentRegionAvail().y - 120.0f * scale;

                    GLuint actual_top = config.swap_screens ? bot_preview_tex : top_preview_tex;
                    GLuint actual_bot = config.swap_screens ? top_preview_tex : bot_preview_tex;

                    // Ajustar aspect ratio visual según la opción seleccionada
                    float screen_aspect = (config.aspect_ratio_mode == NDS_ASPECT_16_9) ? (9.0f / 16.0f) :
                                          (config.aspect_ratio_mode == NDS_ASPECT_16_10) ? (10.0f / 16.0f) : (3.0f / 4.0f);

                    if (config.screen_layout == NDS_LAYOUT_HORIZONTAL) {
                        float gap = (float)config.screen_gap * 0.35f * scale;
                        float screen_w = (preview_box_w - gap - 20.0f * scale) * 0.5f;
                        float screen_h = screen_w * screen_aspect;

                        if (screen_h > preview_box_h - 20.0f * scale) {
                            screen_h = preview_box_h - 20.0f * scale;
                            screen_w = screen_h / screen_aspect;
                        }

                        float start_x = (preview_box_w - (screen_w * 2.0f + gap)) * 0.5f;
                        ImGui::SetCursorPosX(start_x);

                        ImGui::BeginGroup();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "PANTALLA SUPERIOR (3D)");
                        if (actual_top) ImGui::Image((ImTextureID)(uintptr_t)actual_top, ImVec2(screen_w, screen_h));
                        ImGui::EndGroup();

                        ImGui::SameLine(0, gap);

                        ImGui::BeginGroup();
                        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "PANTALLA TACTIL (INFERIOR)");
                        if (actual_bot) ImGui::Image((ImTextureID)(uintptr_t)actual_bot, ImVec2(screen_w, screen_h));
                        ImGui::EndGroup();
                    }
                    else {
                        float gap = (float)config.screen_gap * 0.3f * scale;
                        float screen_h = (preview_box_h - gap - 40.0f * scale) * 0.5f;
                        float screen_w = screen_h / screen_aspect;

                        if (screen_w > preview_box_w - 20.0f * scale) {
                            screen_w = preview_box_w - 20.0f * scale;
                            screen_h = screen_w * screen_aspect;
                        }

                        float start_x = (preview_box_w - screen_w) * 0.5f;
                        ImGui::SetCursorPosX(start_x);
                        if (actual_top) ImGui::Image((ImTextureID)(uintptr_t)actual_top, ImVec2(screen_w, screen_h));

                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + gap);
                        ImGui::SetCursorPosX(start_x);
                        if (actual_bot) ImGui::Image((ImTextureID)(uintptr_t)actual_bot, ImVec2(screen_w, screen_h));
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Cuadro explicativo de Golden Sun 3
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "POR QUE FALLA EN EMULADORES NORMALES:");
                    if (font_bold) ImGui::PopFont();

                    if (font_small) ImGui::PushFont(font_small);
                    ImGui::TextWrapped(
                        "Golden Sun 3 utiliza el registro de captura DISPCNT para efectos de bloom, auras de Psinergia e invocaciones. "
                        "Los emuladores tradicionales reducen el render 3D HD a 256x192 en cada captura VRAM, provocando el famoso parpadeo HD/SD. "
                        "Esta suite mantiene el buffer de captura en alta resolucion y aplica la matriz de proyeccion 16:9 / 16:10 sin distorsion."
                    );
                    if (font_small) ImGui::PopFont();
                }
                ImGui::EndChild();

                ImGui::Columns(1);
            }

            // ---------------------------------------------------------------------
            // TAB 2: PANTALLA TÁCTIL & STYLUS
            // ---------------------------------------------------------------------
            else if (active_tab == 2) {
                ImGui::BeginChild("TouchControlsCard", ImVec2(0, 0), true);
                {
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "CONFIGURACION DEL STYLUS & DIGITALIZADOR TACTIL:");
                    if (font_bold) ImGui::PopFont();

                    const char* stylus_modes[] = {
                        "Raton Directo (Clic izquierdo = Tocar pantalla inferior)",
                        "Stick Derecho de Mando como Stylus Virtual (Con aceleracion)",
                        "Pantalla Tactil Nativa Windows / Tableta Grafica"
                    };
                    ImGui::Combo("Modo de Entrada Tactil", &config.stylus_mode, stylus_modes, 3);

                    const char* cursor_styles[] = {
                        "Puntero Lapiz Stylus NDS",
                        "Mirilla Tactica Circular",
                        "Punto Minimalista",
                        "Invisible / Oculto"
                    };
                    ImGui::Combo("Estilo del Cursor del Lapiz", &config.stylus_cursor_style, cursor_styles, 4);

                    ImGui::SliderInt("Sensibilidad del Stylus", &config.stylus_sensitivity, 50, 200, "%d %%");
                    ImGui::Checkbox("Toque Rapido al Centro de Pantalla (Acceso rapido para pasar dialogos)", (bool*)&config.stylus_quick_tap);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "ASIGNACION DE MANDOS & TECLADO:");
                    if (font_bold) ImGui::PopFont();

                    ImGui::Columns(2, "PadBindsCols", false);
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "BOTONES NDS:");
                    ImGui::Text("Cruceta / D-Pad: Flechas / Stick Izquierdo");
                    ImGui::Text("Boton A (Confirmar): Tecla X / Mando Boton A");
                    ImGui::Text("Boton B (Cancelar/Correr): Tecla Z / Mando Boton B");
                    ImGui::Text("Boton X (Menu Principal): Tecla S / Mando Boton X");
                    ImGui::Text("Boton Y (Psinergia): Tecla A / Mando Boton Y");

                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "GATILLOS & ACCESOS RAPIDOS:");
                    ImGui::Text("Gatillo L: Tecla Q / Mando LB / L1");
                    ImGui::Text("Gatillo R: Tecla W / Mando RB / R1");
                    ImGui::Text("Start: Tecla Enter / Mando Start");
                    ImGui::Text("Select: Tecla Espacio / Mando Select");
                    ImGui::Text("Intercambiar Pantalla: Tecla Tab / Mando R3 (Click Stick)");
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
            }

            // ---------------------------------------------------------------------
            // TAB 3: AUDIO & SPU
            // ---------------------------------------------------------------------
            else if (active_tab == 3) {
                ImGui::BeginChild("AudioSettingsCard", ImVec2(0, 0), true);
                {
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.85f, 0.45f, 1.0f, 1.0f), "MOTOR DE AUDIO SPU & MEJORAS DE SONIDO:");
                    if (font_bold) ImGui::PopFont();

                    ImGui::SliderInt("Volumen Maestro", &config.volume_master, 0, 100, "%d %%");

                    const char* interp_names[] = {
                        "Sin Interpolacion (Audio Crudo 8-bit/16-bit)",
                        "Lineal (Estandar emulacion)",
                        "Gaussiana (Sonido calido y fiel al altavoz de Nintendo DS)",
                        "Coseno / Cubica HiFi (Maxima definicion para banda sonora de Golden Sun)"
                    };
                    ImGui::Combo("Interpolacion SPU", &config.spu_interpolation, interp_names, 4);

                    const char* rate_names[] = {
                        "32.768 kHz (Frecuencia original de Nintendo DS)",
                        "44.100 kHz (Calidad CD Audio)",
                        "48.000 kHz (Estudio Ultra HiFi Remastered)"
                    };
                    ImGui::Combo("Frecuencia de Muestreo", &config.spu_sample_rate, rate_names, 3);

                    ImGui::Checkbox("Expansion Estereo Espacial 3D (Efecto envolvente para auriculares)", (bool*)&config.stereo_expansion);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.85f, 0.45f, 1.0f, 1.0f), "IDIOMA DEL JUEGO / FIRMWARE NDS:");
                    if (font_bold) ImGui::PopFont();

                    const char* lang_names[] = { "0: Japones", "1: English", "2: Francais", "3: Deutsch", "4: Italiano", "5: Espanol (Castellano)" };
                    if (ImGui::Combo("Idioma Seleccionado", &config.selected_language, lang_names, 6)) {
                        toast_msg = "Idioma cambiado a: " + std::string(lang_names[config.selected_language]);
                        toast_timer = 2.0f;
                    }
                }
                ImGui::EndChild();
            }

            // ---------------------------------------------------------------------
            // TAB 4: PARTIDAS & ESTADOS
            // ---------------------------------------------------------------------
            else if (active_tab == 4) {
                ImGui::BeginChild("SavesManagerCard", ImVec2(0, 0), true);
                {
                    if (font_bold) ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.35f, 1.0f), "GESTOR DE PARTIDAS GUARDADAS (.SAV):");
                    if (font_bold) ImGui::PopFont();

                    ImGui::Text("Las partidas se guardan en formato estandar compatible de 512 KB Flash EEPROM.");
                    ImGui::TextDisabled("Ubicacion: saves/Golden Sun - Dark Dawn.sav");
                    ImGui::Spacing();

                    if (ImGui::Button("Abrir Carpeta de Saves", ImVec2(220.0f * scale, 36.0f * scale))) {
                        system("explorer saves");
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Crear Copia de Seguridad Inmediata", ImVec2(260.0f * scale, 36.0f * scale))) {
                        toast_msg = "Copia de seguridad guardada en saves/backup/";
                        toast_timer = 3.0f;
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "ESTADOS RAPIDOS (QUICK-SAVES):");
                    ImGui::Text("Guardar Estado: F1 a F4");
                    ImGui::Text("Cargar Estado: F5 a F8");
                    ImGui::Text("Guardado Automatico al Salir: Activado");
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        // Bottom Action Bar
        ImGui::SetCursorPosY(win_h - 58.0f * scale);
        ImGui::BeginChild("BottomBar", ImVec2(0, 54.0f * scale), true, ImGuiWindowFlags_NoScrollbar);
        {
            if (toast_timer > 0.0f) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[OK] %s", toast_msg.c_str());
            } else {
                ImGui::TextDisabled("ENTER: Jugar  |  ESC: Salir  |  Ajustes guardados en settings.toml");
            }

            ImGui::SameLine(win_w - 530.0f * scale);

            if (ImGui::Button("Guardar Ajustes", ImVec2(180.0f * scale, 38.0f * scale))) {
                config.save_to_file("settings.toml");
                toast_msg = "Ajustes guardados correctamente en settings.toml";
                toast_timer = 2.5f;
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.78f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.10f, 1.0f));
            if (font_bold) ImGui::PushFont(font_bold);
            if (ImGui::Button("JUGAR AHORA", ImVec2(180.0f * scale, 38.0f * scale))) {
                running = false;
                result_action = NDS_ACTION_LAUNCH;
            }
            if (font_bold) ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            if (ImGui::Button("Salir", ImVec2(120.0f * scale, 38.0f * scale))) {
                running = false;
                result_action = NDS_ACTION_QUIT;
            }
        }
        ImGui::EndChild();

        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, win_w, win_h);
        glClearColor(0.040f, 0.055f, 0.090f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    config.save_to_file("settings.toml");

    if (poster_tex) glDeleteTextures(1, &poster_tex);
    if (top_preview_tex) glDeleteTextures(1, &top_preview_tex);
    if (bot_preview_tex) glDeleteTextures(1, &bot_preview_tex);
    if (rom_icon_tex) glDeleteTextures(1, &rom_icon_tex);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);

    return result_action;
}
