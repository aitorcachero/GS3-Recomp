#include "nds_config.h"
#include <cstdio>
#include <cstring>

NdsConfig g_nds_cfg;

void NdsConfig::apply_preset(int preset_id) {
    switch (preset_id) {
        case 0: // Ultra 4K Remastered (16:9)
            internal_resolution_scale = 8;
            antialiasing_msaa = 8;
            anisotropic_filter = 16;
            texture_filter = NDS_FILTER_SHARP_NEAREST;
            perspective_correction = 1;
            subpixel_rasterizer = 1;
            widescreen_3d_fov = 1;
            aspect_ratio_mode = NDS_ASPECT_16_9;
            fix_hd_framebuffer = 1;
            custom_textures_enabled = 1;
            screen_layout = NDS_LAYOUT_HORIZONTAL;
            screen_gap = 24;
            color_mode = NDS_COLOR_DSI_VIBRANT;
            spu_interpolation = NDS_SPU_COSINE_HIFI;
            spu_sample_rate = 2; // 48kHz
            stereo_expansion = 1;
            vsync = 1;
            break;

        case 1: // Equilibrado HD 1080p (16:9)
            internal_resolution_scale = 4;
            antialiasing_msaa = 4;
            anisotropic_filter = 8;
            texture_filter = NDS_FILTER_BILINEAR;
            perspective_correction = 1;
            subpixel_rasterizer = 1;
            widescreen_3d_fov = 1;
            aspect_ratio_mode = NDS_ASPECT_16_9;
            fix_hd_framebuffer = 1;
            custom_textures_enabled = 1;
            screen_layout = NDS_LAYOUT_HORIZONTAL;
            screen_gap = 16;
            color_mode = NDS_COLOR_DSI_VIBRANT;
            spu_interpolation = NDS_SPU_COSINE_HIFI;
            spu_sample_rate = 1; // 44.1kHz
            stereo_expansion = 1;
            vsync = 1;
            break;

        case 2: // Nostalgia Fiel Píxel-Art (4:3)
            internal_resolution_scale = 1;
            antialiasing_msaa = 0;
            anisotropic_filter = 0;
            texture_filter = NDS_FILTER_DS_LCD_GRID;
            perspective_correction = 0;
            subpixel_rasterizer = 0;
            widescreen_3d_fov = 0;
            aspect_ratio_mode = NDS_ASPECT_4_3;
            fix_hd_framebuffer = 1;
            custom_textures_enabled = 0;
            screen_layout = NDS_LAYOUT_VERTICAL;
            screen_gap = 64;
            color_mode = NDS_COLOR_DS_LITE;
            spu_interpolation = NDS_SPU_GAUSSIAN;
            spu_sample_rate = 0; // 32kHz
            stereo_expansion = 0;
            vsync = 1;
            break;

        case 3: // Steam Deck / Portátil (16:10)
            internal_resolution_scale = 4;
            antialiasing_msaa = 4;
            anisotropic_filter = 16;
            texture_filter = NDS_FILTER_SHARP_NEAREST;
            perspective_correction = 1;
            subpixel_rasterizer = 1;
            widescreen_3d_fov = 1;
            aspect_ratio_mode = NDS_ASPECT_16_10; // 16:10 para Steam Deck
            fix_hd_framebuffer = 1;
            custom_textures_enabled = 1;
            screen_layout = NDS_LAYOUT_HYBRID_TOP;
            screen_gap = 12;
            color_mode = NDS_COLOR_OLED_BOOST;
            spu_interpolation = NDS_SPU_COSINE_HIFI;
            spu_sample_rate = 2;
            stereo_expansion = 1;
            vsync = 1;
            break;
    }
}

void NdsConfig::reset_default_controls() {
    key_up = 16777235;        // Flecha Arriba
    key_down = 16777237;      // Flecha Abajo
    key_left = 16777234;      // Flecha Izquierda
    key_right = 16777236;     // Flecha Derecha
    key_a = 88;               // Tecla X
    key_b = 90;               // Tecla Z
    key_x = 83;               // Tecla S
    key_y = 65;               // Tecla A
    key_l = 81;               // Tecla Q
    key_r = 87;               // Tecla W
    key_start = 16777220;     // Enter
    key_select = 32;          // Espacio
    hk_turbo = 16777217;        // Tabulador (Turbo Fast Forward)
    hk_swap_screens = 16777219; // Retroceso / Backspace (Cambiar pantalla)
    hk_fullscreen = 16777274;   // F11
    hk_pause = -1;              // Pausa
}

void NdsConfig::load_from_file(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int v = 0;
        char str[256] = { 0 };

        if (sscanf(line, "fullscreen = %d", &v) == 1) fullscreen = v;
        else if (sscanf(line, "window_scale = %d", &v) == 1) window_scale = v;
        else if (sscanf(line, "vsync = %d", &v) == 1) vsync = v;
        else if (sscanf(line, "refresh_rate = %d", &v) == 1) refresh_rate = v;
        else if (sscanf(line, "integer_scaling = %d", &v) == 1) integer_scaling = v;

        else if (sscanf(line, "screen_layout = %d", &v) == 1) screen_layout = v;
        else if (sscanf(line, "screen_gap = %d", &v) == 1) screen_gap = v;
        else if (sscanf(line, "swap_screens = %d", &v) == 1) swap_screens = v;
        else if (sscanf(line, "active_single_screen = %d", &v) == 1) active_single_screen = v;
        else if (sscanf(line, "aspect_ratio_mode = %d", &v) == 1) aspect_ratio_mode = v;
        else if (sscanf(line, "widescreen_target_screen = %d", &v) == 1) widescreen_target_screen = v;

        else if (sscanf(line, "fix_hd_framebuffer = %d", &v) == 1) fix_hd_framebuffer = v;
        else if (sscanf(line, "custom_textures_enabled = %d", &v) == 1) custom_textures_enabled = v;

        else if (sscanf(line, "internal_resolution_scale = %d", &v) == 1) internal_resolution_scale = v;
        else if (sscanf(line, "texture_filter = %d", &v) == 1) texture_filter = v;
        else if (sscanf(line, "antialiasing_msaa = %d", &v) == 1) antialiasing_msaa = v;
        else if (sscanf(line, "anisotropic_filter = %d", &v) == 1) anisotropic_filter = v;
        else if (sscanf(line, "perspective_correction = %d", &v) == 1) perspective_correction = v;
        else if (sscanf(line, "subpixel_rasterizer = %d", &v) == 1) subpixel_rasterizer = v;
        else if (sscanf(line, "widescreen_3d_fov = %d", &v) == 1) widescreen_3d_fov = v;
        else if (sscanf(line, "color_mode = %d", &v) == 1) color_mode = v;

        else if (sscanf(line, "stylus_mode = %d", &v) == 1) stylus_mode = v;
        else if (sscanf(line, "stylus_cursor_style = %d", &v) == 1) stylus_cursor_style = v;
        else if (sscanf(line, "stylus_quick_tap = %d", &v) == 1) stylus_quick_tap = v;
        else if (sscanf(line, "stylus_sensitivity = %d", &v) == 1) stylus_sensitivity = v;

        else if (sscanf(line, "volume_master = %d", &v) == 1) volume_master = v;
        else if (sscanf(line, "spu_interpolation = %d", &v) == 1) spu_interpolation = v;
        else if (sscanf(line, "spu_sample_rate = %d", &v) == 1) spu_sample_rate = v;
        else if (sscanf(line, "stereo_expansion = %d", &v) == 1) stereo_expansion = v;

        else if (sscanf(line, "selected_language = %d", &v) == 1) selected_language = v;
        else if (sscanf(line, "rom_path = \"%[^\"]\"", str) == 1 || sscanf(line, "rom_path = %s", str) == 1) {
            if (str[0] != '\0') rom_path = str;
        }

        else if (sscanf(line, "key_up = %d", &v) == 1) key_up = v;
        else if (sscanf(line, "key_down = %d", &v) == 1) key_down = v;
        else if (sscanf(line, "key_left = %d", &v) == 1) key_left = v;
        else if (sscanf(line, "key_right = %d", &v) == 1) key_right = v;
        else if (sscanf(line, "key_a = %d", &v) == 1) key_a = v;
        else if (sscanf(line, "key_b = %d", &v) == 1) key_b = v;
        else if (sscanf(line, "key_x = %d", &v) == 1) key_x = v;
        else if (sscanf(line, "key_y = %d", &v) == 1) key_y = v;
        else if (sscanf(line, "key_l = %d", &v) == 1) key_l = v;
        else if (sscanf(line, "key_r = %d", &v) == 1) key_r = v;
        else if (sscanf(line, "key_start = %d", &v) == 1) key_start = v;
        else if (sscanf(line, "key_select = %d", &v) == 1) key_select = v;

        else if (sscanf(line, "hk_turbo = %d", &v) == 1) hk_turbo = v;
        else if (sscanf(line, "hk_swap_screens = %d", &v) == 1) hk_swap_screens = v;
        else if (sscanf(line, "hk_fullscreen = %d", &v) == 1) hk_fullscreen = v;
        else if (sscanf(line, "hk_pause = %d", &v) == 1) hk_pause = v;
    }
    fclose(f);
}

void NdsConfig::save_to_file(const char* filepath) const {
    FILE* f = fopen(filepath, "w");
    if (!f) return;

    fprintf(f, "# =============================================================================\n");
    fprintf(f, "# Nintendo DS Recompilation Settings - Golden Sun: Dark Dawn\n");
    fprintf(f, "# =============================================================================\n\n");

    fprintf(f, "[display]\n");
    fprintf(f, "fullscreen = %d\n", fullscreen);
    fprintf(f, "window_scale = %d\n", window_scale);
    fprintf(f, "vsync = %d\n", vsync);
    fprintf(f, "refresh_rate = %d\n", refresh_rate);
    fprintf(f, "integer_scaling = %d\n\n", integer_scaling);

    fprintf(f, "[dual_screen]\n");
    fprintf(f, "screen_layout = %d\n", screen_layout);
    fprintf(f, "screen_gap = %d\n", screen_gap);
    fprintf(f, "swap_screens = %d\n", swap_screens);
    fprintf(f, "active_single_screen = %d\n", active_single_screen);
    fprintf(f, "aspect_ratio_mode = %d\n", aspect_ratio_mode);
    fprintf(f, "widescreen_target_screen = %d\n\n", widescreen_target_screen);

    fprintf(f, "[goldensun_enhancements]\n");
    fprintf(f, "fix_hd_framebuffer = %d\n", fix_hd_framebuffer);
    fprintf(f, "custom_textures_enabled = %d\n\n", custom_textures_enabled);

    fprintf(f, "[graphics_3d_2d]\n");
    fprintf(f, "internal_resolution_scale = %d\n", internal_resolution_scale);
    fprintf(f, "texture_filter = %d\n", texture_filter);
    fprintf(f, "antialiasing_msaa = %d\n", antialiasing_msaa);
    fprintf(f, "anisotropic_filter = %d\n", anisotropic_filter);
    fprintf(f, "perspective_correction = %d\n", perspective_correction);
    fprintf(f, "subpixel_rasterizer = %d\n", subpixel_rasterizer);
    fprintf(f, "widescreen_3d_fov = %d\n", widescreen_3d_fov);
    fprintf(f, "color_mode = %d\n\n", color_mode);

    fprintf(f, "[touch_stylus]\n");
    fprintf(f, "stylus_mode = %d\n", stylus_mode);
    fprintf(f, "stylus_cursor_style = %d\n", stylus_cursor_style);
    fprintf(f, "stylus_quick_tap = %d\n", stylus_quick_tap);
    fprintf(f, "stylus_sensitivity = %d\n\n", stylus_sensitivity);

    fprintf(f, "[audio_spu]\n");
    fprintf(f, "volume_master = %d\n", volume_master);
    fprintf(f, "spu_interpolation = %d\n", spu_interpolation);
    fprintf(f, "spu_sample_rate = %d\n", spu_sample_rate);
    fprintf(f, "stereo_expansion = %d\n\n", stereo_expansion);

    fprintf(f, "[game]\n");
    fprintf(f, "rom_path = \"%s\"\n", rom_path.c_str());
    fprintf(f, "selected_language = %d\n\n", selected_language);

    fprintf(f, "[keyboard_controls]\n");
    fprintf(f, "key_up = %d\n", key_up);
    fprintf(f, "key_down = %d\n", key_down);
    fprintf(f, "key_left = %d\n", key_left);
    fprintf(f, "key_right = %d\n", key_right);
    fprintf(f, "key_a = %d\n", key_a);
    fprintf(f, "key_b = %d\n", key_b);
    fprintf(f, "key_x = %d\n", key_x);
    fprintf(f, "key_y = %d\n", key_y);
    fprintf(f, "key_l = %d\n", key_l);
    fprintf(f, "key_r = %d\n", key_r);
    fprintf(f, "key_start = %d\n", key_start);
    fprintf(f, "key_select = %d\n\n", key_select);

    fprintf(f, "[hotkeys]\n");
    fprintf(f, "hk_turbo = %d\n", hk_turbo);
    fprintf(f, "hk_swap_screens = %d\n", hk_swap_screens);
    fprintf(f, "hk_fullscreen = %d\n", hk_fullscreen);
    fprintf(f, "hk_pause = %d\n", hk_pause);

    fclose(f);
}
