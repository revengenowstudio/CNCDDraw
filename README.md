# cnc-ddraw
cnc-ddraw can fix compatibility issues in older games, such as black screen, bad performance, crashes or defective Alt+Tab.

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
 
### Instructions

1. Download [cnc-ddraw.zip](https://github.com/CnCNet/cnc-ddraw/releases/latest/download/cnc-ddraw.zip) and extract it into your game folder
2. Disable all compatibility modes for all of the game executables
3. Start the game

Note: If you use cnc-ddraw with a game that got its own windowed mode built in then **make sure you disable the games own windowed mode** first. If you want to play in windowed mode then start the game once in fullscreen and then press Alt+Enter to enable the cnc-ddraw windowed mode (Or modify ddraw.ini without using Alt+Enter).

**If the game starts but it doesn't work perfectly** then open ddraw.ini and search for **Compatibility settings**, one of the settings will usually fix the problem.

Most common compatibility settings are the following 3:

- noactivateapp - Set this to true if there are issues on Alt+Tab.
- handlemouse - Set this to false if your cursor is invisible at some places in the game.
- maxgameticks - Set this to 60 if the game is running too fast or if it's flickering. If it still doesn't work, try a lower value.

**If the game doesn't start at all or it's crashing**, [then please generate a debug log file and upload it.](https://github.com/CnCNet/cnc-ddraw/issues/44)  

### Hotkeys
* [Alt] + [Enter]                  = Switch between windowed and fullscreen mode
* [Ctrl] + [Tab]                    = Unlock cursor
* [Right Alt] + [Right Ctrl]  = Unlock cursor

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
