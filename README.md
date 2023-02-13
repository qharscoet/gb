Simple Gameboy Emulator
=========================

This is a simple self-teaching project

Controls
---------

Gameboy controls mapping:

- Arrows : D-pad
- X/C : A/B Buttons
- Enter/Backspace : Start/Select

Misc controls:
- O to open a ROM file in the default display
- P to Pause
- R to Reset
- Press F1 to open the debug UI


![emu_screenshot](https://user-images.githubusercontent.com/6672352/218345596-6e626f31-7c9c-4182-a864-976357de42d0.png)

WHAT WORKS
----------

- CPU
- GPU
- Inputs
- MBC 1/3/5
- Sound
- GameBoy Color
- Link Cable (somewhat)

TODO :
-----------

- Refactor to get rid of duplicated code here and there
- Sprite overlapping and 10 sprite on one line limit
- Refactor Memory to not allocate the full 64Kb now that rom/ram and sound are allocated in other classes

IN PROGRESS
------------
- Cleaning up stuff
- Wasm frontend improvements
- Save States

Missing:
--------

- Link Cable still has some glitches on games like Pokémon
- Gameboy peripherals
- Lots of undiscovered bugs

Compile Dependencies:
---------------------
- SDL2
- GLEW

External librairies used:
---------------------

- SDL2 : https://www.libsdl.org/
- tinyfiledialogs : https://sourceforge.net/projects/tinyfiledialogs/

- Dear imgui : https://github.com/ocornut/imgui
	* Memory Editor plugin for imgui : https://github.com/ocornut/imgui_club
	* ImGuiFileDialog by aiekick : https://github.com/aiekick/ImGuiFileDialog

Ressources used for documentation about the gameboy:
--------------------

- The Ultimate Gameboy talk : https://www.youtube.com/watch?v=HyzD8pNlpwI

- Multiple docs and manuals:
	* http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
	* https://gekkio.fi/files/gb-docs/gbctr.pdf
	* https://ia803208.us.archive.org/9/items/GameBoyProgManVer1.1/GameBoyProgManVer1.1.pdf
	* http://bgb.bircd.org/pandocs.htm
	* https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware (For sound specifics)

- Multiple github repos (may link them later)
