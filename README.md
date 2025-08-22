# WOLFTECH-1
(v0.0.1) a raycaster game engine with basic shooting , textures and sprites a crosshair, a map editor all inspired by wolfenstein3d

# COMPILATION
  1. THE ENGINE REQUIRES SDL 1.2, i know its old as shit but.... fuck you.
  2. THE MAP EDITOR USES RAYLIB 5.5.

  just run the makefiles for the game and the map editor

  make and then make run.

# USAGE
  unfortunatelly most of the code is crammed into main.cpp, sorry about that but next version im splitting the files,
  for now the game only loads map.h no other maps its hard coded. to make or edit a map, you can use the map editor

  if you read the code you can see how textures are added and controlls and if you read the map.h you can see how it works.

  again there isnt really a propper documentation FOR NOW. i will be fixing that soon.

  REMEMBER, whenever you make a new map.h, you need to re compile the main.cpp. USE THE MAP EDITOR IN A TERMINAL.
  IT WILL ASK YOU FOR INPUTS THERE, ONLY THEN DOES THE GUI MODE OPEN.

  stuff like: Enter input map filename (or press Enter to create new): 
              Enter output map filename (default: map.h): 
              Enter MAX MAP SIZE: 

  TO EXPORT THE MAPS PRESS E, TO SET PLAYER POSITIONS RIGHT CLICK ON A SQUARE AND TO PLACE WALLS, PRESS THE NUMBER KEYS 0 - 8 FOR A WALL. 0 IS NOTHING.

# things to add
A FUCKING FULL SCREEN GOD DAMN IT.
oh and a propper health system and enemy health system but u can do that if you want, ui shouldent be TOO hard but just in case i will make a header fo it for the next version
multiple weapons and weapon switching.
and enemies that can shoot back, again most of these things i have listed you can do if you know SDL 1.2, i will be implementing these things later though of course 

# controlls
WASD - MOVEMENT
LEFT MB - INSTA KILL SHOOT (havent implemented a proper enemy health system yet haha but it shouldent be too hard for you to do, dont worry i will do it tho anyway)
MOUSE LOOK - LOOK LEFT TO RIGHT



