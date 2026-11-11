# Nintendo DS Recompilation - Golden Sun 3: Oscuro Amanecer (Dark Dawn)
**Suite Moderna de Recompilación Nativa para PC (64-Bit)**

Bienvenido al proyecto de recompilación nativa para **Nintendo DS**, enfocado en ***Golden Sun: Oscuro Amanecer (Golden Sun: Dark Dawn)*** (ROM BO5P). Este proyecto adopta la filosofía y arquitectura de los proyectos de recompilación de consola (*SNESRecomp*, *PS1-Recomp*, *Ps2-Recomp* y *Snatcher Mega-CD Recomp*), adaptado específicamente a las características únicas del hardware de doble pantalla y motor 3D de Nintendo DS.

---

## 🌟 Características y Mejoras Visuales Implementadas

### 1. 🖥️ Motor Gráfico 3D y Escalado de Resolución
- **Resolución Interna 3D**:
  - `1x`: Nativa DS (256 x 192).
  - `2x`: HD (512 x 384).
  - `3x`: 720p (768 x 576).
  - `4x`: 1080p Full HD (1024 x 768).
  - `6x`: 2K QHD (1536 x 1152).
  - `8x`: **4K Ultra HD (2048 x 1536)**.
- **Corrección de Perspectiva 3D y Subpíxeles**: Elimina la distorsión de texturas y el temblor de polígonos original.
- **Widescreen 3D y Pantalla Panorámica 16:9**: Modos Horizontal panorámico lado a lado, Vertical clásico DS, Híbrido Enfoque 3D (75%) y Pantalla Única.

### 2. 🎮 Ejecución Real y Transparente del Juego
- Al pulsar **INICIAR JUEGO (ENTER)** o **JUGAR AHORA**, la suite sincroniza todos los ajustes de gráficos, audio y controles directamente con el motor nativo x64 en `tools/engine/`.
- El juego arranca de forma inmediata con su cinemática de introducción, logotipo de Camelot, gráficos 3D en alta resolución y audio remasterizado.
- Al salir del juego, el control regresa de forma limpia al lanzador principal.

---

## 🚀 Cómo Jugar

Haz doble clic en:
```bat
JUGAR.bat
```
o ejecuta directamente:
```bat
GoldenSun3_Recomp.exe
```

---

## 🎮 Controles por Defecto

| Acción NDS | Teclado | Mando (Xbox / PS) |
|---|---|---|
| **Cruceta (D-Pad)** | Flechas de dirección | D-Pad / Stick Izquierdo |
| **Botón A (Confirmar)** | `X` | `A` (Xbox) / `Cruz` (PS) |
| **Botón B (Cancelar / Correr)** | `Z` | `B` (Xbox) / `Círculo` (PS) |
| **Botón X (Menú Principal)** | `S` | `X` (Xbox) / `Cuadrado` (PS) |
| **Botón Y (Psinergía)** | `A` | `Y` (Xbox) / `Triángulo` (PS) |
| **Gatillo L** | `Q` | `LB` / `L1` |
| **Gatillo R** | `W` | `RB` / `R1` |
| **Start** | `Enter` | `Start` / `Options` |
| **Select** | `Espacio` | `Back` / `Share` |
| **Intercambiar Pantalla** | `Tab` | `R3` |
| **Pantalla Completa** | `F11` | - |
| **Stylus (Toque)** | Ratón (Clic Izquierdo) | Stick Derecho |
| **Salir** | `Alt + F4` / `Esc` | - |
