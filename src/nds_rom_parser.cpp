#include "nds_rom_parser.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

static uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(int)(crc & 1)));
        }
    }
    return ~crc;
}

static std::string decode_utf16le(const uint8_t* ptr, size_t max_bytes) {
    std::string out;
    for (size_t i = 0; i + 1 < max_bytes; i += 2) {
        uint16_t ch = ptr[i] | (ptr[i + 1] << 8);
        if (ch == 0) break;
        if (ch == '\n' || ch == '\r') {
            out += " - ";
        } else if (ch < 128) {
            out += (char)ch;
        } else {
            // Basic UTF-8 encode for non-ASCII
            if (ch < 0x800) {
                out += (char)(0xC0 | (ch >> 6));
                out += (char)(0x80 | (ch & 0x3F));
            } else {
                out += (char)(0xE0 | (ch >> 12));
                out += (char)(0x80 | ((ch >> 6) & 0x3F));
                out += (char)(0x80 | (ch & 0x3F));
            }
        }
    }
    return out;
}

std::string NdsRomInfo::get_display_title(int lang_idx) const {
    if (banner.valid) {
        if (lang_idx >= 0 && lang_idx < 6 && !banner.titles[lang_idx].empty()) {
            return banner.titles[lang_idx];
        }
        // Fallback to English (idx 1) or Japanese (idx 0)
        if (!banner.titles[1].empty()) return banner.titles[1];
        if (!banner.titles[0].empty()) return banner.titles[0];
    }
    if (title[0] != '\0') return std::string(title);
    return file_name.empty() ? "Nintendo DS Title" : file_name;
}

std::string NdsRomInfo::get_region_string() const {
    if (game_code[3] == '\0') return "Unknown";
    char c = game_code[3];
    switch (c) {
        case 'E': return "USA / North America (NTSC-U)";
        case 'P': return "Europe / Australia (PAL - Multi5/6)";
        case 'J': return "Japan (NTSC-J)";
        case 'K': return "Korea";
        case 'S': return "Spain (Special Release)";
        case 'D': return "Germany";
        case 'F': return "France";
        case 'I': return "Italy";
        default:  return std::string("Region Code: ") + c;
    }
}

std::string NdsRomInfo::get_capacity_string() const {
    if (file_size == 0) return "0 MB";
    double mb = (double)file_size / (1024.0 * 1024.0);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.1f MB (Cartridge ROM)", mb);
    return std::string(buf);
}

bool parse_nds_rom(const std::string& path, NdsRomInfo& out_info) {
    out_info = NdsRomInfo();
    out_info.file_path = path;

    // Get filename
    size_t last_slash = path.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        out_info.file_name = path.substr(last_slash + 1);
    } else {
        out_info.file_name = path;
    }

    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    // Get file size
    fseek(f, 0, SEEK_END);
    out_info.file_size = (uint64_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t header[512] = { 0 };
    if (fread(header, 1, sizeof(header), f) != sizeof(header)) {
        fclose(f);
        return false;
    }

    memcpy(out_info.title, header, 12);
    out_info.title[12] = '\0';

    memcpy(out_info.game_code, header + 12, 4);
    out_info.game_code[4] = '\0';

    memcpy(out_info.maker_code, header + 16, 2);
    out_info.maker_code[2] = '\0';

    out_info.unit_code = header[18];
    out_info.device_capacity = header[19];

    out_info.arm9_rom_offset = *(uint32_t*)(header + 0x20);
    out_info.arm9_entry      = *(uint32_t*)(header + 0x24);
    out_info.arm9_ram_addr   = *(uint32_t*)(header + 0x28);
    out_info.arm9_size       = *(uint32_t*)(header + 0x2c);

    out_info.arm7_rom_offset = *(uint32_t*)(header + 0x30);
    out_info.arm7_entry      = *(uint32_t*)(header + 0x34);
    out_info.arm7_ram_addr   = *(uint32_t*)(header + 0x38);
    out_info.arm7_size       = *(uint32_t*)(header + 0x3c);

    out_info.fnt_offset    = *(uint32_t*)(header + 0x40);
    out_info.fat_offset    = *(uint32_t*)(header + 0x48);
    out_info.banner_offset = *(uint32_t*)(header + 0x68);
    out_info.header_crc16  = *(uint16_t*)(header + 0x15E);

    // Compute CRC32 of first 4MB for fast signature
    size_t sample_size = (size_t)std::min<uint64_t>(out_info.file_size, 4 * 1024 * 1024);
    std::vector<uint8_t> sample_buf(sample_size);
    fseek(f, 0, SEEK_SET);
    fread(sample_buf.data(), 1, sample_size, f);
    out_info.computed_crc32 = calculate_crc32(sample_buf.data(), sample_size);

    // Parse Banner if offset is valid
    if (out_info.banner_offset > 0 && out_info.banner_offset + 2048 <= out_info.file_size) {
        fseek(f, (long)out_info.banner_offset, SEEK_SET);
        uint8_t banner_raw[2048];
        if (fread(banner_raw, 1, sizeof(banner_raw), f) == sizeof(banner_raw)) {
            out_info.banner.valid = true;
            out_info.banner.version = *(uint16_t*)(banner_raw + 0);
            out_info.banner.crc16   = *(uint16_t*)(banner_raw + 2);

            // Decode 32x32 4bpp icon
            const uint8_t* tiles = banner_raw + 0x20;      // 512 bytes (4x4 tiles of 8x8)
            const uint8_t* pal_bytes = banner_raw + 0x220; // 32 bytes (16 colors BGR555)

            uint8_t palette_rgba[16][4];
            for (int i = 0; i < 16; ++i) {
                uint16_t c = pal_bytes[i * 2] | (pal_bytes[i * 2 + 1] << 8);
                uint8_t r = (c & 0x1F) << 3;
                uint8_t g = ((c >> 5) & 0x1F) << 3;
                uint8_t b = ((c >> 10) & 0x1F) << 3;
                uint8_t a = (i == 0) ? 0 : 255; // Color 0 is transparent in NDS banner
                palette_rgba[i][0] = r;
                palette_rgba[i][1] = g;
                palette_rgba[i][2] = b;
                palette_rgba[i][3] = a;
            }

            int tile_idx = 0;
            for (int ty = 0; ty < 4; ++ty) {
                for (int tx = 0; tx < 4; ++tx) {
                    const uint8_t* tile = tiles + (tile_idx * 32);
                    tile_idx++;
                    for (int py = 0; py < 8; ++py) {
                        for (int px = 0; px < 8; px += 2) {
                            uint8_t byte = tile[py * 4 + px / 2];
                            uint8_t c0 = byte & 0x0F;
                            uint8_t c1 = (byte >> 4) & 0x0F;

                            int img_y = ty * 8 + py;
                            int img_x = tx * 8 + px;

                            int pixel0_idx = (img_y * 32 + img_x) * 4;
                            memcpy(out_info.banner.icon_rgba + pixel0_idx, palette_rgba[c0], 4);

                            int pixel1_idx = (img_y * 32 + img_x + 1) * 4;
                            memcpy(out_info.banner.icon_rgba + pixel1_idx, palette_rgba[c1], 4);
                        }
                    }
                }
            }

            // Titles: 6 titles of 256 bytes UTF-16LE starting at 0x240
            for (int lang = 0; lang < 6; ++lang) {
                const uint8_t* tptr = banner_raw + 0x240 + (lang * 256);
                out_info.banner.titles[lang] = decode_utf16le(tptr, 256);
            }
        }
    }

    fclose(f);
    out_info.loaded = true;
    return true;
}
