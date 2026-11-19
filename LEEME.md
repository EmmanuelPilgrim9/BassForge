# BassForge

Realzador psicoacústico de graves (VST3 para FL Studio). Genera armónicos de la banda grave para que el sub se escuche en audífonos, laptops y celulares.

## Controles

| Knob | Qué hace |
|---|---|
| **Frequency** (30–200 Hz) | Corte de la banda grave que se procesa. Ponlo un poco arriba de la fundamental de tu kick/bajo (55–80 Hz suele funcionar en big room). |
| **Intensity** (0–100 %) | Cantidad de armónicos agregados. |
| **Warm / Bite** (0–100 %) | 0 = armónicos pares (2º, 4º): cálido y redondo. 100 = impares (3º, 5º): más agresivo y presente. |
| **Fundamental** (0–100 %) | Nivel del sub original. Bájalo para ganar headroom y dejar que los armónicos hagan el trabajo. |
| **Output** (−24 a +12 dB) | Ganancia de salida. |
| **Solo Harmonics** | Escuchar solo lo que el plugin agrega. |

## Compilar (opción A: en tu PC con Windows)

1. Instala, gratis:
   - [Visual Studio 2022 Community](https://visualstudio.microsoft.com/es/vs/community/) y marca la carga de trabajo **"Desarrollo para el escritorio con C++"**.
   - [CMake](https://cmake.org/download/) (marca "Add CMake to PATH").
   - [Git](https://git-scm.com/download/win).
2. Doble clic en `build-windows.bat`. La primera vez descarga JUCE y tarda unos minutos.
3. Copia `build\BassForge_artefacts\Release\VST3\BassForge.vst3` a `C:\Program Files\Common Files\VST3\`.

## Compilar (opción B: en la nube con GitHub, sin instalar nada)

1. Sube esta carpeta a un repositorio de GitHub.
2. Ve a la pestaña **Actions** y espera a que termine "Build BassForge" (unos 10 minutos).
3. Descarga el artefacto **BassForge-VST3-Windows**, descomprímelo y copia `BassForge.vst3` a `C:\Program Files\Common Files\VST3\`.

## Cargar en FL Studio

1. *Options → Manage plugins → Find installed plugins*.
2. Busca **BassForge** en la lista y marca la estrella para agregarlo a favoritos.
3. Ponlo en un slot del Mixer, en la pista del kick, del bajo o en un bus de graves.

## Consejos

- Empieza con Intensity en 30–50 % y activa *Solo Harmonics* para ajustar Frequency hasta que los armónicos sigan bien la nota.
- Compara en audífonos o en la bocina del celular: ahí se nota el efecto.
- Si el master se satura, baja Fundamental un 20–40 % en lugar de bajar Intensity.
