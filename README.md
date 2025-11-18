# Comic Strip Maker (OOPS Project)

Short description
- Comic Strip Maker is a small C++/SFML editor for composing comic panels with characters, speech/thought bubbles, and freehand brush strokes. It provides a sidebar palette for assets, a color wheel and thickness slider for drawing, and an undo/redo system.

Key features
- Palette: characters, fonts, and bubble styles.
- Speech bubbles: procedural shapes and optional image-based bubbles, with word-wrapping and font controls.
- Draw mode: freehand brush strokes with interpolation to avoid dotted lines at high mouse speed.
- Undo/Redo: command pattern implemented to support undoable actions (add/delete/change).

Repository layout (important files)
- `main.cpp` — application entry point, UI, event loop, and layout logic.
- `AssetManager.*` — loads textures and fonts from `Assets/`.
- `BrushStroke.*` — freehand stroke representation and drawing.
- `SpeechBubble.*` — bubble geometry, text wrapping, and rendering.
- `Character.*` — sprite-based characters.
- `Command.*` — undo/redo command implementations and `CommandManager`.
- `CanvasObject.*`, `VectorUtils.h` — shared geometry utilities.
- `Assets/` — bundled assets (characters, fonts, speech bubble images).

Build (Windows, MSYS2 / mingw/ucrt64)
1. Ensure SFML 3.0.2 is downloaded and the include/lib paths are available.
2. From PowerShell (example):

```powershell
# from project root
C:/msys64/ucrt64/bin/g++.exe -std=c++17 -Wall -Wextra -Wpedantic \
  main.cpp Character.cpp AssetManager.cpp SpeechBubble.cpp Command.cpp CanvasObject.cpp BrushStroke.cpp \
  -I "C:/Users/SRI VINEEL/Downloads/Source/SFML-3.0.2/include" \
  -L "C:/Users/SRI VINEEL/Downloads/Source/SFML-3.0.2/lib" \
  -lsfml-graphics -lsfml-window -lsfml-system -o ComicStripMaker.exe

# Run
.\ComicStripMaker.exe
```

Note: This repository includes a VSCode task named `Build Comic Strip Maker` that runs the same command.

Running and testing
- Launch `ComicStripMaker.exe` from the workspace root.
- Use the left sidebar to add characters/bubbles, switch to `Fonts` to use the text-size slider, and toggle `Draw Mode` to paint on the canvas.

Development notes
- To add assets, place images and font files under `Assets/Characters`, `Assets/Font`, or `Assets/SpeechBubbles` and update `AssetManager` if more extensions are needed.
- Top-of-file headers have been standardized; these changes are non-functional.
- The `ChangeBubbleFontSizeCommand` records font-size changes as a single undoable action when the slider drag completes.

Contributing
- Please open issues or PRs on the repository: https://github.com/SriVineel/OOPS-PROJect

License
- No license file is included in the repo. Add a `LICENSE` file if you want to apply an open-source license.

Contact
- Repo owner: SriVineel

