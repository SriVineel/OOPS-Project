# Comic Strip Maker (OOPS Project)

## Short Description

Comic Strip Maker is a C++/SFML editor for composing comic panels with characters, speech/thought bubbles, and freehand brush strokes. The editor features a sidebar palette, color wheel and thickness slider for drawing, undo/redo, erasing objects, image export, and flipping any object horizontally.

---

## Key Features

- **Palette:** Side panel for choosing characters, fonts, and bubble styles.
- **Speech bubbles:** Procedural and image-based speech/thought/shout bubbles with word-wrapping and font size control.
- **Draw mode:** Freehand brush strokes with interpolation to avoid dotted lines at high mouse speed.
- **Erase:** Instantly erase any brush stroke, character, or bubble by switching to Erase mode and clicking/tapping on an object. Every erase is undoable.
- **Flip objects:** Flip any character, bubble, or stroke horizontally from the context menu or toolbar.
- **Export images:** Save your entire comic panel without all the UI elements as a PNG image with one click .
- **Undo/Redo:** All add, erase, flip and other actions are undoable and redoable (command pattern implementation).

---

## Repository Layout (Important Files)

- `main.cpp` — Application entry point, UI, event loop, and layout logic. Handles Erase, Export, and Flip actions.
- `AssetManager.*` — Loads textures and fonts from `Assets/`.
- `BrushStroke.*` — Freehand stroke representation and drawing, including erasing support.
- `SpeechBubble.*` — Bubble geometry, text wrapping/rendering, flipping support.
- `Character.*` — Sprite-based characters, supports horizontal flipping.
- `Command.*` — Undo/redo command implementations and `CommandManager` (supports erase and flip actions).
- `CanvasObject.*`, `VectorUtils.h` — Shared geometry/math utilities, base class for drawable/interactive objects.
- `Assets/` — Folders for all character, font, and speech bubble images.


## Build (Windows, MSYS2 / mingw/ucrt64)

1. Download SFML 3.0.2 and set your include/lib paths to its location.
2. To build from PowerShell:
    ```
    # from project root:
    C:/msys64/ucrt64/bin/g++.exe -std=c++17 -Wall -Wextra -Wpedantic ^
      main.cpp Character.cpp AssetManager.cpp SpeechBubble.cpp Command.cpp CanvasObject.cpp BrushStroke.cpp ^
      -I "C:/Users/SRI VINEEL/Downloads/Source/SFML-3.0.2/include" ^
      -L "C:/Users/SRI VINEEL/Downloads/Source/SFML-3.0.2/lib" ^
      -lsfml-graphics -lsfml-window -lsfml-system -o ComicStripMaker.exe

    # Run:
    .\\ComicStripMaker.exe
    ```
    > Or use the included VSCode task named `Build Comic Strip Maker`.

---

## Running and Testing

- Launch `ComicStripMaker.exe` from the root folder.
- Use the sidebar to place characters/bubbles, change fonts, and switch between Draw/Erase/Flip/Export tools.
- Right-click to flip objects. Use Export to save your panel as an image.

---

## Development Notes

- Place new assets in `Assets/Characters`, `Assets/Font`, or `Assets/SpeechBubbles` as needed.
- All code modules feature standardized top-of-file headers and clear section comments.
- Erase, Export Images, and Flip are fully integrated into command-based Undo/Redo.
- Font-size changes in bubbles are atomic/undoable actions.
- See source code and in-file documentation for more extensibility guidance.

---

## Contributing

Open issues or PRs at: [https://github.com/SriVineel/OOPS-PROJect](https://github.com/SriVineel/OOPS-PROJect)

---

## License

No license file is included. Please add a `LICENSE` file if you want to specify an open-source license.

---

## Contact

Repo owner: SriVineel

