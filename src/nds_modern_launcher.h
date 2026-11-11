#ifndef NDS_MODERN_LAUNCHER_H
#define NDS_MODERN_LAUNCHER_H

#include "nds_config.h"
#include "nds_rom_parser.h"

enum NdsLauncherAction {
    NDS_ACTION_NONE = 0,
    NDS_ACTION_LAUNCH = 1,
    NDS_ACTION_QUIT = 2
};

// Main launcher window loop
NdsLauncherAction run_nds_modern_launcher(NdsConfig& config, NdsRomInfo& rom_info);

#endif // NDS_MODERN_LAUNCHER_H
