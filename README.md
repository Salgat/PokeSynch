PokeSynch
=================

![](https://github.com/Salgat/PokeSynch/blob/master/doc/preview.png)

PokeSynch allows multiple players to play Pokemon Red, Blue, or Yellow together online or through a LAN.

Building the project
------------------------------------------
If using CMAKE gui, set the source code to "C:/your_directory/PokeSynch" and where to build binaries to "C:/your_directory/PokeSynch/bin/Release". After configuring and generating the Makefile, change directory to the /bin/Release folder and type "make" to build (be sure to install both CMake-GUI and download Make.exe for Windows).

PokeSynch requires SFML. Libraries are available http://www.sfml-dev.org/download/sfml/2.3/ for your specific compiler. Extract the SFML header folder to /include and the libraries (.a) to /lib. Finally, extract the compiled library files (.dll) to your /bin/Release folder (I personally used "GCC 4.9.2 MinGW (SEH) - 64-bit", downloaded from MinGW-w64).

For those who want a more in depth tutorial on how to build a project like this, I created a detailed tutorial for a similar project that will also work for this at the below link.

https://github.com/Salgat/BubbleGrow/wiki/Building-from-source

Running the emulator
------------------------------------------
To run after building: 
 * Host: PokeSynch.exe -game="PokemonRed.gb" -save="PokemonRed.sav" -host
 * Client: PokeSynch.exe -game="PokemonRed.gb" -save="PokemonRed.sav" -connect=127.0.0.1

Note: The -save argument is optional and used to load and use save files.

Controls
------------------------------------------
Controls for the emulator are currently hard-coded.
* Z = B
* X = A
* Enter = Start
* Shift = Select
* 1 through 9 = Choose save state slot
* S = Save game state for current slot
* L = Load game state for current slot
* P = Save screenshot

Description
------------------------------------------
PokeSynch is based off [GBS](https://github.com/Salgat/GameBoyEmulator-GBS), a GameBoy emulator I developed. The emulator works fine for Pokemon but is still in need of a lot of polish (occasional minor graphical glitches, lack of proper config files for changing things like controls, and no sound).

PokeSynch is a specialization of this emulator that specifically targets certain memory addresses to manipulate the behavior of the game.

Progress
------------------------------------------
The following are minimal goals I'd like to complete as part of a "proof of concept",
 * Synchronize NPCs across all clients
   * This is currently the only feature implemented, although it only matches the host's NPCs. Eventually want to update this so that the NPCs match the person who was first in a map.
 * Synchronize players across all clients
   * Since Pokemon RBY only allows up to 16 sprites on a map, and this is sometimes already full, we can't use native code for this. Instead, players are drawn over the frame and "simulated" in the game.
 * Player battles between each other
   * Once players are synchronized on the map, need to have it so a trainer battle is initiated with the player and the trainer has both a copy of the other player's Pokemon and copies the player's commands on their local client.


