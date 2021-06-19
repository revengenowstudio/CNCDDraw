# cnc-ddraw
cnc-ddraw can fix compatibility issues in older games, such as black screen, bad performance, crashes or defective Alt+Tab.

&nbsp;

### Features

 - Supports Windows XP, Vista, 7, 8, 10 and Wine
 - GDI / OpenGL / Direct3D 9 renderer (With automatic renderer selection)
 - Upscaling via glsl shaders - https://imgur.com/a/kxsM1oY | https://imgur.com/a/wjrhpFV
 - Windowed Mode / Fullscreen Exclusive Mode / Borderless Mode
 - Alt+Enter support to switch quickly between Fullscreen and Windowed mode
 - Automatically saves and restores window position/size/state
 - FPS Limiter
 - VSync
 - Optional mouse sensitivity scaling
 - Preliminary libretro shader support - https://github.com/libretro/glsl-shaders
 - ...
 
&nbsp;

### Instructions

1. Download [cnc-ddraw.zip](https://github.com/CnCNet/cnc-ddraw/releases/latest/download/cnc-ddraw.zip) and extract it into your game folder
2. Start the game


Note: If you use cnc-ddraw with a game that got its own windowed mode built in then **make sure you disable the games own windowed mode** first. 
If you want to play in windowed mode then start the game once in fullscreen and then press Alt+Enter to enable the cnc-ddraw windowed mode (Or modify ddraw.ini without using Alt+Enter).

&nbsp;

**If the game starts but it doesn't work perfectly** then open ddraw.ini and search for **Compatibility settings**, one of the settings will usually fix the problem.

&nbsp;

- If there are **problems on Alt+Tab** then try to set "noactivateapp=true" - If it still doesn't work also try "renderer=opengl" or "renderer=gdi".

- If the **game is running too fast** then try to set "maxgameticks=60" - If it's still too fast, try a lower value - If too slow, try a higher value.

- If **windowed mode or upscaling are not working properly** then try to set "hook=2" and "renderer=gdi" - opengl/direct3d could work as well in some games. 

- If **videos or other GUI elements are invisible** then try to set "nonexclusive=true".

- If some parts of the **screen are being displayed diagonally** then try to set "fixpitch=true".

&nbsp;

**If the game doesn't start at all or it's crashing**, [then please generate a debug log file and upload it.](https://github.com/CnCNet/cnc-ddraw/issues/44)  

&nbsp;

### Hotkeys
* [Alt] + [Enter]                  = Switch between windowed and fullscreen mode
* [Ctrl] + [Tab]                    = Unlock cursor
* [Right Alt] + [Right Ctrl]  = Unlock cursor

&nbsp;

### Supported Games

 - Command & Conquer Gold
 - Command & Conquer: Red Alert
 - Command & Conquer: Tiberian Sun
 - Command & Conquer: Red Alert 2
 - Carmageddon
 - Carmageddon 2
 - Warcraft 2
 - StarCraft
 - Diablo
 - Diablo 2
 - Age of Empires
 - Age of Empires II
 - Theme Hospital
 - Populous: The Beginning
 - Outlaws
 - Dungeon Keeper
 - Dark Reign: The Future of War
 - Star Wars: Galactic Battlegrounds
 - Atomic Bomberman
 - Dune 2000
 - Oddworld: Abe's Oddysee
 - Commandos
 - Red Baron 3D
 - F-16 Multirole Fighter
 - F-22 Raptor
 - Nox
 - ...

There are a lot more games supported but I don't usually update the list, just give it a try and if it doesn't work then check the instructions above.
