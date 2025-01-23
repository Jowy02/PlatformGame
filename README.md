# Platform Game
## Description
This first approach of the project uses Mario Bros mechanics, sprites and Level design. The goal is to test the engine and ensure its implementation and procedures have a solid base. Once we get that, we'll use said base to detail and polish the game to make it more personal.

## Installation
**_Unzip the [RELEASE FOLDER](https://github.com/Jowy02/PlatformGame/releases) and execute the .exe file_**

## Game Controls
### Player Controls
     * A = Move player left
     * D = Move player right
     * SPACE = Jump

### Debug Mode
     * H = Shows/hides a help menu showing the keys
     * F4 = Respawn Enemies
     * F5 = Save Game
     * F6 = Load Game from last Save
     * F7 = Move between the different checkpoints
     * F9 = Visualize the colliders/logic
     * F10 = God Mode (fly around, cannot be killed)
     * F11 = Enable/Disable FPS cap to 30
 
## Features implemented in this version
### 0.7 Version (27/10/2024)
* Provisional Map done on Tiled for Stage 1, simple to show features
* Tiled TMX map implemented with logic working on the code
* Map layer defines collisions and these are dynamically loaded using Box2D
* Player walk/jump/die animations
* Player movement:
     * **_WASD_** keys for movement
     * **_SPACE_** for jump
* Player parameters, Animation sets and other configuration parameters such as audio and textures can be read from external files (coin audio is implemented)
* Game is capped at 60 stable FPS and info appears on Window Title as demanded
### 0.9 Version (09/12/2024)
* Enemies added, with PathFinding to search for Player and animations when moving
* Save/Load features coded to use from Debug
* Checkpoints implemented with visual and SFX feedback for Player
* Map layer now allows to position different checkpoints just by the use of a tile
* Player can jump on Enemies to "kill" them
* Collectable coins distributed on map, with SFX feedback for Player
* BG music implemented

## Credits
_**Code and Research + QA**: Joel Vicente_
* **Github**: [Jowy02](https://github.com/Jowy02)

_**Art/Design and Management + QA**: Arthur Cordoba_
* **Github**: [000Arthur](https://github.com/000Arthur)
