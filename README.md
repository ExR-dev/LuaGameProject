# LuaGameProject

## Prerequisites

1. Download **Vcpkg** (https://github.com/microsoft/vcpkg).
2. Open a command prompt in the extracted vcpkg folder.
3. Install it by running **_bootstrap-vcpkg.bat_**.
4. Run the command “ **_vcpkg integrate install_** ”.
5. Install RayLib by running “ **vcpkg install raylib** ”.

## Run Game

In the **Build/** folder, run the **LuaGameProject.exe** executable file.

## Build Game

1. Open the visual studio by running the **LuaGameProject.sln** solution file, found in
    the **LuaGameProject/** folder.
2. Press **Ctrl + Shift + B** to build the game. Alternatively press **Build > Build Solution**
    found in the menu bar.
3. Run the game by clicking **Local Windows Debugger**

## How To Use

### Game

To play the game, press “Play” in the main menu. Use WASD to move the player. When the
player is close to a weapon (black rotating rectangles) press E to pick it up. Aim with the
mouse and shoot with the left mouse button. The enemies (moving cats) can be hit, and
have their health displayed above them. The game ends when the player collides with the
cave.

The command prompt open in the background can be used to execute Lua commands, try
writing “help” for a list of some available commands. Any command input that is not
recognized as a hard-coded command will be executed as Lua code. This means that any
function available from Lua can also run from the command prompt. The most useful
functions are located within the tables named ‘game’, ‘dev’ and ‘data’. Try writing


“PrintTable([table])”, replacing [table] with game, dev or data to see what commands you
have access to.

### Editor

Entering the editor, you will see a scene view along with some other windows. You can
move around in the scene view by holding right click and dragging. You can also zoom
using the scroll wheel. There are two colored buttons in the top right corner of the view. The
red button will reset the scene and the yellow will reload mods.

There are a few different editing modes. These can be selected in the ‘Editor Settings’
window and are the following: ‘Sandbox’, ‘LevelCreator’, ‘PresetCreator’, ‘PrefabCreator’
and ‘DungeonCreator’. Out of these modes, the most relevant ones are the preset, prefab
and dungeon creators.

#### Preset Creator

The preset creator lets you create new stat presets. The presets currently implemented are
weapons and ammo. Each type of weapon and ammo functions in the same way, the only
difference is which preset they get their stats from.

After entering the preset creator a window opens, letting you pick if you want to create/edit
weapons or ammo. After picking one, a new window will open. This window lists all loaded
variants of the selected type, and a button to create a new variant at the bottom. After
either selecting an existing variant or creating a new one, yet another window will open,
displaying all of the editable stats and descriptions. After making your desired changes to
the stats, press the confirm button located below the stats (if not visible, try resizing the
game window). There may be a warning message instead of a confirmation button. If so,
read the warning and do as it asks. After pressing confirm, and satisfying any other
potential popup, the edited preset will be saved to an .lts file, located within src/Mods/
folder. From now on, whenever a scene is loaded, this new preset is loaded as well.

If it is a weapon you created, you can now spawn an instance of the weapon in the game
scene by first ensuring you have reloaded the game scene since the preset was added,
then going over to the command prompt and writing ‘dev.MakeWeap(“[preset name]”)’.
Note that the weapon will be spawned at the location of the cursor, so make sure to
position your cursor appropriately before pressing enter.


#### Prefab Creator

The prefab creator lets you create and edit prefabs. A prefab is an entity with components
added to it, that can later be instantiated, for example in the game or in the DungronEditor.

In the “Prefab Creator” window you can load an existing prefab. You can also reset or save
the current one. In order to create a prefab, start by adding components to it in the “Entity
Editor” window. When you are done press “Save as prefab”, give it a name and select
which values should be saved. Any values left unchecked will use the default value
instead.

Prefabs can be spawned in the game using the console command
‘game.SpawnPrefab(“[prefab name]”)’.

#### Dungeon Creator

In the dungeon creator you can create rooms that can be used to generate a dungeon. By
selecting a room in the “Room Selection” window, you can start placing entities in the
room. Do this by either selecting a prefab in the “Prefab Collection” window or by creating
a new entity by clicking “Create Entity” in the “Scene Hierarchy” window. Components can
be added and modified in the “Entity Editor” window.

In order to generate a dungeon, press the green button labeled “Generate” in the “View”
window. You will get a few options for generating your dungeon, such as selecting which
rooms should be included and how many. It’s recommended to generate the dungeon by
pressing “Spawn Rooms”, then pressing “Separate Rooms” until all rooms are separated
and green lines are displayed. Finaly press “Complete” to see your final dungeon. If you’re
happy with your dungeon and want to use it in the game, press save. The next time you play
the game the new dungeon will be used, note that you probably must press “Restart” in the
main menu first.

Some additional details about this mode can be found in the “Info” window.



