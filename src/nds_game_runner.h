#ifndef NDS_GAME_RUNNER_H
#define NDS_GAME_RUNNER_H

#include "nds_config.h"
#include "nds_rom_parser.h"

// Inicia la ventana nativa de juego con el motor de render dual-screen y sonido
// Devuelve true si el usuario pide volver al lanzador, false si sale completamente al escritorio
bool run_nds_game(NdsConfig& config, const NdsRomInfo& rom_info);

#endif // NDS_GAME_RUNNER_H
