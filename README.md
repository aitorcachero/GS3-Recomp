# Golden Sun 3: Oscuro Amanecer (Dark Dawn) - PC Native Recompilation Suite

[![Platform: Windows x64](https://img.shields.io/badge/Platform-Windows%20x64-blue.svg)](https://github.com/aitorcachero/GS3-Recomp)
[![Status: Released v1.0.1](https://img.shields.io/badge/Release-v1.0.1-brightgreen.svg)](https://github.com/aitorcachero/GS3-Recomp/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-orange.svg)](LICENSE)
[![Game: Nintendo DS](https://img.shields.io/badge/ROM-Golden%20Sun%203%20(BO5P)-gold.svg)](https://github.com/aitorcachero/GS3-Recomp)

Bienvenido a la **Suite de Recompilación y Modernización Nativa para PC** de **Golden Sun: Oscuro Amanecer (*Golden Sun: Dark Dawn*)** (Nintendo DS).

Este proyecto implementa la arquitectura moderna de decompilación / recompilación nativa para PC de 64 bits con interfaz gráfica basada en **Dear ImGui (Docking) + SDL2 + OpenGL 3.3 Core**, integrando un motor rasterizador por hardware optimizado con eliminación de artifacts de frame buffer, renderizado en alta definición hasta 4K UHD, soporte panorámico 16:9 y 16:10 (Steam Deck), filtrado anisotrópico 16x, soporte de paquetes de texturas personalizadas e inyección instantánea de partidas guardadas.

---

## 🌟 Características Principales

### 1. Resolución Interna 3D hasta 4K UHD
El rasterizador 3D por hardware permite escalar el juego desde la resolución original de Nintendo DS (256x192) hasta resolución 4K nativa con corrección de perspectiva y precisión de subpíxel:
* **1x**: Nativa NDS (256 × 192) - Fiel a la consola original.
* **2x**: HD (512 × 384).
* **3x**: 720p HD (768 × 576).
* **4x**: 1080p Full HD (1024 × 768) - Recomendado para portátiles y Steam Deck.
* **6x**: 2K QHD (1536 × 1152).
* **8x**: 4K Ultra HD (2048 × 1536) - Máxima nitidez en monitores 4K.

### 2. Soporte Panorámico Widescreen (16:9 y 16:10 para Steam Deck)
Se adapta la proyección geométrica del motor 3D y la disposición de las dos pantallas a formatos modernos:
* **Panorámico 16:9**: Pantallas lado a lado balanceadas con relación de aspecto cinematográfica.
* **Panorámico 16:10 (Steam Deck / Laptops)**: Optimizado para la pantalla de Steam Deck y portátiles 16:10 con máxima área útil.
* **Vertical Tradicional**: Disposición clásica estilo Nintendo DS (pantalla superior e inferior apiladas).
* **Híbrido Enfoque 3D (75% / 25%)**: Pantalla superior gigante para combate y exploración 3D, y pantalla táctil en esquina inferior para menú e invocaciones de Djinn.
* **Pantalla Única**: Conmutación instantánea de pantalla mediante una tecla rápida (`Tab` o `R3`).

### 3. Solución al Cuello de Botella del Framebuffer / Display Capture
Golden Sun: Dark Dawn es históricamente uno de los títulos más difíciles de emular en alta definición debido a su uso intensivo de efectos de captura de pantalla (`DISPCNT` display capture) para transiciones y magias de Psinergía (que en emuladores tradicionales provocan saltos constantes entre frames en HD y SD a 256x192). La suite implementa buffers de textura persistentes en VRAM para preservar la definición ultra-nítida durante todo el combate y las animaciones de invocación.

### 4. Filtrado Anisotrópico 16x, MSAA 8x y Texturas Personalizadas
* **MSAA (Multisampling Anti-Aliasing)**: Suavizado de bordes geométricos hasta 8x.
* **Filtrado Anisotrópico 16x**: Elimina el desenfoque en suelos inclinados, mapamundi y cámaras en ángulo bajo durante las batallas.
* **Carga de Texturas HD Modificadas**: Soporte de inyección desde la carpeta `textures/goldensun3/` para personajes, djinn, escenarios e interfaz de usuario.

### 5. Lanzador Moderno e Interfaz Gráfica Integrada
* Sin ventanas de consola ni comandos en segundo plano; cierre limpio y retorno al lanzador.
* Perfiles rápidos en un clic: **Ultra 4K**, **Equilibrado 1080p**, **Fiel Pixel-Art** y **Streamer / PiP**.
* Calibrador visual de Stylus / pantalla táctil con soporte completo para ratón y mando.

---

## 🎮 Controles por Defecto

| Acción Nintendo DS | Teclado | Mando (Xbox / PlayStation / Steam Deck) |
|---|---|---|
| **Cruceta (D-Pad)** | Flechas de dirección (`↑` `↓` `←` `→`) | D-Pad / Stick Izquierdo |
| **Botón A (Confirmar / Hablar)** | `X` | `A` (Xbox) / `Cruz` (PlayStation) |
| **Botón B (Cancelar / Correr)** | `Z` | `B` (Xbox) / `Círculo` (PlayStation) |
| **Botón X (Menú Principal / Djinn)** | `S` | `X` (Xbox) / `Cuadrado` (PlayStation) |
| **Botón Y (Psinergía de Campo)** | `A` | `Y` (Xbox) / `Triángulo` (PlayStation) |
| **Gatillo L** | `Q` | `LB` / `L1` |
| **Gatillo R** | `W` | `RB` / `R1` |
| **Start** | `Enter` | `Start` / `Options` |
| **Select** | `Espacio` | `Back` / `Select` / `Share` |
| **Intercambiar Pantallas** | `Tab` | `R3` (Pulsar Stick Derecho) |
| **Pantalla Completa** | `F11` | `Alt + Enter` |
| **Stylus (Pantalla Táctil)** | Botón Izquierdo del Ratón | Stick Derecho |
| **Salir del Juego** | `Esc` | `Guide` / `Home` |

---

## 🚀 Requisitos e Instalación

### Requisitos del Sistema
* **SO**: Windows 10 / Windows 11 (64-bit).
* **GPU**: Compatible con OpenGL 3.3 o superior (NVIDIA, AMD o Intel Iris Xe).
* **RAM**: 4 GB mínimo.
* **ROM**: Archivo original de *Golden Sun: Dark Dawn* / *Golden Sun: Oscuro Amanecer* (`.nds`). *(Por motivos legales de copyright, la ROM comercial no se distribuye con este software; debes extraer tu propia copia legal).*

### Instrucciones de Uso
1. Descarga la última versión empaquetada desde la sección [Releases](https://github.com/aitorcachero/GS3-Recomp/releases).
2. Descomprime el archivo `.zip` en la carpeta que prefieras.
3. Coloca tu archivo ROM (`Golden Sun - Dark Dawn.nds` o similar) en la carpeta raíz del juego o selecciónalo con el botón "Examinar ROM" del lanzador.
4. Ejecuta `GoldenSun3_Recomp.exe` o haz doble clic en `JUGAR.bat`.
5. Elige tu perfil gráfico preferido (ej. *Ultra 4K UHD* o *Equilibrado 1080p*) y pulsa **INICIAR JUEGO EN PC**.

---

## 🛠️ Compilación desde el Código Fuente

Para compilar el proyecto en Windows:
1. Clona el repositorio:
   ```bash
   git clone https://github.com/aitorcachero/GS3-Recomp.git
   cd GS3-Recomp
   ```
2. Requiere **CMake 3.20+** y el compilador **Visual Studio 2022 (MSVC v143)**.
3. Ejecuta el script automatizado:
   ```cmd
   COMPILAR.bat
   ```
   O compila manualmente mediante CMake:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

---

## 📜 Licencia y Agradecimientos

- Proyecto distribuido bajo licencia MIT.
- Agradecimientos especiales a la comunidad de emulación y decompilación de Nintendo DS y a Camelot Software Planning por crear esta legendaria saga de RPG.
