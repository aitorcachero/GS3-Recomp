#ifndef NDS_CONFIG_H
#define NDS_CONFIG_H

#include <string>

enum NdsScreenLayout {
    NDS_LAYOUT_VERTICAL = 0,    // Classic DS Top over Bottom
    NDS_LAYOUT_HORIZONTAL = 1,  // Side-by-Side Widescreen
    NDS_LAYOUT_HYBRID_TOP = 2,  // Large 3D Top Screen + Companion Touch PiP
    NDS_LAYOUT_HYBRID_BOT = 3,  // Large Touch Screen + Companion Top PiP
    NDS_LAYOUT_SINGLE_SWAP = 4  // Single Full Screen with Quick-Toggle
};

enum NdsAspectRatio {
    NDS_ASPECT_4_3 = 0,   // Standard DS 4:3
    NDS_ASPECT_16_9 = 1,  // Modern PC / TV 16:9
    NDS_ASPECT_16_10 = 2  // Steam Deck / Laptop 16:10
};

enum NdsTextureFilter {
    NDS_FILTER_SHARP_NEAREST = 0,
    NDS_FILTER_BILINEAR = 1,
    NDS_FILTER_DS_LCD_GRID = 2,
    NDS_FILTER_CRT_SCANLINES = 3,
    NDS_FILTER_XBRZ_SCALE2X = 4
};

enum NdsColorMode {
    NDS_COLOR_RAW = 0,
    NDS_COLOR_DS_LITE = 1,
    NDS_COLOR_DSI_VIBRANT = 2,
    NDS_COLOR_OLED_BOOST = 3
};

enum NdsSpuInterpolation {
    NDS_SPU_NONE = 0,
    NDS_SPU_LINEAR = 1,
    NDS_SPU_GAUSSIAN = 2,
    NDS_SPU_COSINE_HIFI = 3
};

struct NdsConfig {
    // Window & Display
    int fullscreen = 1;
    int window_scale = 3;
    int vsync = 1;
    int refresh_rate = 0;
    int integer_scaling = 0;

    // Dual-Screen System
    int screen_layout = NDS_LAYOUT_HORIZONTAL;
    int screen_gap = 16;
    int swap_screens = 0;
    int active_single_screen = 0;

    // Aspect Ratio (16:9, 16:10, 4:3)
    int aspect_ratio_mode = NDS_ASPECT_16_9;

    // Golden Sun 3 Specific Fixes & Enhancements
    int fix_hd_framebuffer = 1;      // Solves Golden Sun 3 HD/SD frame alternation bug
    int custom_textures_enabled = 1; // Allows HD texture replacement from textures/

    // 3D Engine & Resolution
    int internal_resolution_scale = 4; // 1x to 8x (4K)
    int texture_filter = NDS_FILTER_SHARP_NEAREST;
    int antialiasing_msaa = 4;        // 0, 2, 4, 8, 16
    int anisotropic_filter = 16;       // 0, 2, 4, 8, 16
    int perspective_correction = 1;
    int subpixel_rasterizer = 1;       // Subpixel float rasterizer
    int widescreen_3d_fov = 1;         // 16:9 / 16:10 FOV expansion
    int color_mode = NDS_COLOR_DSI_VIBRANT;

    // Touch & Stylus Controls
    int stylus_mode = 0;
    int stylus_cursor_style = 0;
    int stylus_quick_tap = 1;
    int stylus_sensitivity = 100;

    // Audio & HiFi SPU
    int volume_master = 95;
    int spu_interpolation = NDS_SPU_COSINE_HIFI;
    int spu_sample_rate = 2; // 48kHz
    int stereo_expansion = 1;

    // Game & Paths
    std::string rom_path = "Golden Sun - Dark Dawn.nds";
    int selected_language = 5; // 5: Spanish

    void apply_preset(int preset_id);
    void load_from_file(const char* filepath = "settings.toml");
    void save_to_file(const char* filepath = "settings.toml") const;
};

extern NdsConfig g_nds_cfg;

#endif // NDS_CONFIG_H
