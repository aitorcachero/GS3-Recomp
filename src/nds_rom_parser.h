#ifndef NDS_ROM_PARSER_H
#define NDS_ROM_PARSER_H

#include <string>
#include <vector>
#include <cstdint>

struct NdsRomBanner {
    bool valid = false;
    uint16_t version = 0;
    uint16_t crc16 = 0;
    std::string titles[6]; // 0: Japanese, 1: English, 2: French, 3: German, 4: Italian, 5: Spanish
    uint8_t icon_rgba[32 * 32 * 4]; // 32x32 32-bit RGBA
};

struct NdsRomInfo {
    bool loaded = false;
    std::string file_path;
    std::string file_name;
    uint64_t file_size = 0;

    // Header fields
    char title[13] = { 0 };
    char game_code[5] = { 0 };
    char maker_code[3] = { 0 };
    uint8_t unit_code = 0; // 00=NDS, 02=NDS+DSi, 03=DSi
    uint8_t device_capacity = 0;

    uint32_t arm9_rom_offset = 0;
    uint32_t arm9_entry = 0;
    uint32_t arm9_ram_addr = 0;
    uint32_t arm9_size = 0;

    uint32_t arm7_rom_offset = 0;
    uint32_t arm7_entry = 0;
    uint32_t arm7_ram_addr = 0;
    uint32_t arm7_size = 0;

    uint32_t fnt_offset = 0;
    uint32_t fat_offset = 0;
    uint32_t banner_offset = 0;

    uint16_t header_crc16 = 0;
    uint32_t computed_crc32 = 0;

    NdsRomBanner banner;

    // Derived helpers
    std::string get_display_title(int lang_idx = 5) const; // 5 = Spanish default if available
    std::string get_region_string() const;
    std::string get_capacity_string() const;
};

bool parse_nds_rom(const std::string& path, NdsRomInfo& out_info);

#endif // NDS_ROM_PARSER_H
