#include <iostream>
#include <cmath>
#include <string>
#include <vector>

//the quickcg header, this really helps haha
#include "quickcg.h"

using namespace QuickCG;


#define screenWidth 640
#define screenHeight 480
#define texWidth 64 // must be power of two
#define texHeight 64 // must be power of two
#include "mapeditor/map.h"

struct Sprite
{
  double x;
  double y;
  int texture;
  bool isEnemy;
};

#define numSprites 20

bool spriteAlive[numSprites];

Sprite sprite[numSprites] =
{
  {20.5, 11.5, 10, false}, //green light in front of playerstart
  //green lights in every room
  {18.5,4.5, 10, false},
  {10.0,4.5, 10, false},
  {10.0,12.5,10, false},
  {3.5, 6.5, 10, false},
  {3.5, 20.5,10, false},
  {3.5, 14.5,10, false},
  {14.5,20.5,10, false},

  //row of pillars in front of wall: fisheye test
  {18.5, 10.5, 9, false},
  {18.5, 11.5, 9, false},
  {18.5, 12.5, 9, false},
  {5.5, 5.5, 11, true},

  //some barrels around the map
  {21.5, 1.5, 8, false},
  {15.5, 1.5, 8, false},
  {16.0, 1.8, 8, false},
  {16.2, 1.2, 8, false},
  {3.5,  2.5, 8, false},
  {9.5, 15.5, 8, false},
  {10.0, 15.1,8, false},
  {10.5, 15.8,8, false},
};

Uint32 buffer[screenHeight][screenWidth]; // y-coordinate first because it works per scanline

//1D Zbuffer
double ZBuffer[screenWidth];

//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteDistance[numSprites];

//function used to sort the sprites
void sortSprites(int* order, double* dist, int amount);

bool mousePressed(Uint8 button) {
    static Uint32 lastState = 0;
    Uint32 currentState = SDL_GetMouseState(NULL, NULL);

    bool wasDown = (lastState & SDL_BUTTON(button)) != 0;
    bool isDown = (currentState & SDL_BUTTON(button)) != 0;

    lastState = currentState;

    // Return true only if it just transitioned from up -> down
    return (!wasDown && isDown);
}


int main(int /*argc*/, char */*argv*/[])
{
  FLAGS[0] = false;
  for (int i = 0; i < numSprites; i++)
    spriteAlive[i] = true;

  double posX = 22.0, posY = 11.5; //x and y start position
  double dirX = -1.0, dirY = 0.0; //initial direction vector
  double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane

  double time = 0; //time of current frame
  double oldTime = 0; //time of previous frame

  std::vector<Uint32> texture[12];
  for(int i = 0; i < 11; i++) texture[i].resize(texWidth * texHeight);

  screen(screenWidth,screenHeight, 0, "Raycaster");
  SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_ShowCursor(SDL_DISABLE); // Optional: hides the mouse cursor



  //load some textures - REMEMBER TO CHANGE ARRAY LENGTH ^ it has to be one longer than the actuall amount of loaded images
  unsigned long tw, th, error = 0;
  error |= loadImage(texture[0], tw, th, "pics/eagle.png");
  error |= loadImage(texture[1], tw, th, "pics/redbrick.png");
  error |= loadImage(texture[2], tw, th, "pics/purplestone.png");
  error |= loadImage(texture[3], tw, th, "pics/greystone.png");
  error |= loadImage(texture[4], tw, th, "pics/bluestone.png");
  error |= loadImage(texture[5], tw, th, "pics/mossy.png");
  error |= loadImage(texture[6], tw, th, "pics/wood.png");
  error |= loadImage(texture[7], tw, th, "pics/colorstone.png");
  if(error) { std::cout << "error loading images" << std::endl; return 1; }

  //load some sprite textures
  error |= loadImage(texture[8], tw, th, "pics/barrel.png");
  error |= loadImage(texture[9], tw, th, "pics/pillar.png");
  error |= loadImage(texture[10], tw, th, "pics/greenlight.png");
  error |= loadImage(texture[11], tw, th, "pics/ENEMY.png");
  if(error) { std::cout << "error loading images" << std::endl; return 1; }

  const double moveSpeed = 0.05; // adjust for speed
  const double rotSpeed = 0.005;  // rotation speed per mouse motion unit

  int mouseLastX = screenWidth / 2;
  bool firstMouse = true;


  //start the main loop
  while(!done())
  {
    SDL_WM_GrabInput(SDL_GRAB_ON);   // keeps mouse inside window
SDL_ShowCursor(SDL_DISABLE);     // hides cursor

     SDL_Event event;
// Handle rotation with relative mouse movement
int mouseX, mouseY;
SDL_GetMouseState(&mouseX, &mouseY);

// Calculate delta from screen center
int deltaX = mouseX - (screenWidth / 2);

// Only rotate if there was mouse movement
if (deltaX != 0) {
    double angle = -deltaX * rotSpeed;

    // Rotate direction vector and camera plane
    double oldDirX = dirX;
    dirX = dirX * cos(angle) - dirY * sin(angle);
    dirY = oldDirX * sin(angle) + dirY * cos(angle);

    double oldPlaneX = planeX;
    planeX = planeX * cos(angle) - planeY * sin(angle);
    planeY = oldPlaneX * sin(angle) + planeY * cos(angle);

    // Re-center mouse to avoid pointer hitting screen edge
    SDL_WarpMouse(screenWidth / 2, screenHeight / 2);

    if (FLAGS[0] == false)
    {
      posX = playerSpawnX;
      posY = playerSpawnY; //x and y start position
      dirX = -1.0;
      dirY = 0.0; //initial direction vector
      planeX = 0.0; 
      planeY = 0.66; //the 2d raycaster version of camera plane

      FLAGS[0] = true;
    }
}


    //FLOOR CASTING
    for(int y = screenHeight / 2 + 1; y < screenHeight; ++y)
    {
      // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
      float rayDirX0 = dirX - planeX;
      float rayDirY0 = dirY - planeY;
      float rayDirX1 = dirX + planeX;
      float rayDirY1 = dirY + planeY;

      // Current y position compared to the center of the screen (the horizon)
      int p = y - screenHeight / 2;

      // Vertical position of the camera.
      float posZ = 0.5 * screenHeight;

      // Horizontal distance from the camera to the floor for the current row.
      // 0.5 is the z position exactly in the middle between floor and ceiling.
      float rowDistance = posZ / p;

      // calculate the real world step vector we have to add for each x (parallel to camera plane)
      // adding step by step avoids multiplications with a weight in the inner loop
      float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
      float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

      // real world coordinates of the leftmost column. This will be updated as we step to the right.
      float floorX = posX + rowDistance * rayDirX0;
      float floorY = posY + rowDistance * rayDirY0;

      for(int x = 0; x < screenWidth; ++x)
      {
        // the cell coord is simply got from the integer parts of floorX and floorY
        int cellX = (int)(floorX);
        int cellY = (int)(floorY);

        // get the texture coordinate from the fractional part
        int tx = (int)(texWidth * (floorX - cellX)) & (texWidth - 1);
        int ty = (int)(texHeight * (floorY - cellY)) & (texHeight - 1);

        floorX += floorStepX;
        floorY += floorStepY;

        // choose texture and draw the pixel
        int checkerBoardPattern = (int(cellX + cellY)) & 1;
        int floorTexture;
        if(checkerBoardPattern == 0) floorTexture = 3;
        else floorTexture = 4;
        int ceilingTexture = 6;
        Uint32 color;

        // floor
        color = texture[floorTexture][texWidth * ty + tx];
        color = (color >> 1) & 8355711; // make a bit darker
        buffer[y][x] = color;

        //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
        color = texture[ceilingTexture][texWidth * ty + tx];
        color = (color >> 1) & 8355711; // make a bit darker
        buffer[screenHeight - y - 1][x] = color;
      }
    }

    // WALL CASTING
    for(int x = 0; x < w; x++)
    {
      //calculate ray position and direction
      double cameraX = 2 * x / double(w) - 1; //x-coordinate in camera space
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;

      //which box of the map we're in
      int mapX = int(posX);
      int mapY = int(posY);

      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;

      //length of ray from one x or y-side to next x or y-side
      double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
      double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
      double perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?

      //calculate step and initial sideDist
      if(rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }
      if(rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, either in x-direction, or in y-direction
        if(sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if(worldMap[mapX][mapY] > 0) hit = 1;
      }

      //Calculate distance of perpendicular ray (Euclidean distance would give fisheye effect!)
      if(side == 0) perpWallDist = (sideDistX - deltaDistX);
      else          perpWallDist = (sideDistY - deltaDistY);

      //Calculate height of line to draw on screen
      int lineHeight = (int)(h / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + h / 2;
      if(drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + h / 2;
      if(drawEnd >= h) drawEnd = h - 1;
      //texturing calculations
      int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

      //calculate value of wallX
      double wallX; //where exactly the wall was hit
      if (side == 0) wallX = posY + perpWallDist * rayDirY;
      else           wallX = posX + perpWallDist * rayDirX;
      wallX -= floor((wallX));

      //x coordinate on the texture
      int texX = int(wallX * double(texWidth));
      if(side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      if(side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

      // TODO: an integer-only bresenham or DDA like algorithm could make the texture coordinate stepping faster
      // How much to increase the texture coordinate per screen pixel
      double step = 1.0 * texHeight / lineHeight;
      // Starting texture coordinate
      double texPos = (drawStart - h / 2 + lineHeight / 2) * step;
      for(int y = drawStart; y < drawEnd; y++)
      {
        // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
        int texY = (int)texPos & (texHeight - 1);
        texPos += step;
        Uint32 color = texture[texNum][texHeight * texY + texX];
        //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        if(side == 1) color = (color >> 1) & 8355711;
        buffer[y][x] = color;
      }

      //SET THE ZBUFFER FOR THE SPRITE CASTING
      ZBuffer[x] = perpWallDist; //perpendicular distance is used
    }

    //SPRITE CASTING
    //sort sprites from far to close
    for(int i = 0; i < numSprites; i++)
    {
      spriteOrder[i] = i;
      spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
    }
    sortSprites(spriteOrder, spriteDistance, numSprites);

    //after sorting the sprites, do the projection and draw them
    for(int i = 0; i < numSprites; i++)
    {
      int sprIdx = spriteOrder[i];
      if (!spriteAlive[sprIdx]) continue; // skip if dead

      double spriteX = sprite[sprIdx].x - posX;
      double spriteY = sprite[sprIdx].y - posY;
      //transform sprite with the inverse camera matrix
      // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
      // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
      // [ planeY   dirY ]                                          [ -planeY  planeX ]

      double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication

      double transformX = invDet * (dirY * spriteX - dirX * spriteY);
      double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D, the distance of sprite to player, matching sqrt(spriteDistance[i])

      int spriteScreenX = int((w / 2) * (1 + transformX / transformY));

      //parameters for scaling and moving the sprites
      #define uDiv 1
      #define vDiv 1
      #define vMove 0.0
      int vMoveScreen = int(vMove / transformY);

      //calculate height of the sprite on screen
      int spriteHeight = abs(int(h / (transformY))) / vDiv; //using "transformY" instead of the real distance prevents fisheye
      //calculate lowest and highest pixel to fill in current stripe
      int drawStartY = -spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawStartY < 0) drawStartY = 0;
      int drawEndY = spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawEndY >= h) drawEndY = h - 1;

      //calculate width of the sprite
      int spriteWidth = abs(int (h / (transformY))) / uDiv; // same as height of sprite, given that it's square
      int drawStartX = -spriteWidth / 2 + spriteScreenX;
      if(drawStartX < 0) drawStartX = 0;
      int drawEndX = spriteWidth / 2 + spriteScreenX;
      if(drawEndX > w) drawEndX = w;

      //loop through every vertical stripe of the sprite on screen
      for(int stripe = drawStartX; stripe < drawEndX; stripe++)
      {
        int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
        //the conditions in the if are:
        //1) it's in front of camera plane so you don't see things behind you
        //2) ZBuffer, with perpendicular distance
        if(transformY > 0 && transformY < ZBuffer[stripe])
        {
          for(int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
          {
            int d = (y - vMoveScreen) * 256 - h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
            int texY = ((d * texHeight) / spriteHeight) / 256;
            Uint32 color = texture[sprite[sprIdx].texture][texWidth * texY + texX]; //get current color from the texture
            if((color & 0x00FFFFFF) != 0) buffer[y][stripe] = color; //paint pixel if it isn't black, black is the invisible color
          }
        }
      }
    }

    //UI GOES HERE DUDE

    // UI DRAWING EXAMPLE - Crosshair
    int crosshairX = screenWidth / 2;
    int crosshairY = screenHeight / 2;
    Uint32 crosshairColor = 0xFFFFFFFF; // white

    // Vertical line
    for (int i = -5; i <= 5; ++i)
    {
        if (crosshairY + i >= 0 && crosshairY + i < screenHeight)
            buffer[crosshairY + i][crosshairX] = crosshairColor;
    }

    // Horizontal line
    for (int i = -5; i <= 5; ++i)
    {
        if (crosshairX + i >= 0 && crosshairX + i < screenWidth)
            buffer[crosshairY][crosshairX + i] = crosshairColor;
    }



    drawBuffer(buffer[0]);
    // No need to clear the screen here, since everything is overdrawn with floor and ceiling

    //timing for input and FPS counter
    oldTime = time;
    time = getTicks();
    double frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
    print(1.0 / frameTime); //FPS counter
    redraw();

    //speed modifiers
    double moveSpeed = frameTime * 3.0; //the constant value is in squares/second
    double rotSpeed = frameTime * 2.0; //the constant value is in radians/second
    readKeys();
    //move forward if no wall in front of you
    if (keyDown(SDLK_w))
    {
      if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
    }
    // strafe left (A key)
if (keyDown(SDLK_a)) {
    if (worldMap[int(posX - planeX * moveSpeed)][int(posY)] == false)
        posX -= planeX * moveSpeed;
    if (worldMap[int(posX)][int(posY - planeY * moveSpeed)] == false)
        posY -= planeY * moveSpeed;
}

// strafe right (D key)
if (keyDown(SDLK_d)) {
    if (worldMap[int(posX + planeX * moveSpeed)][int(posY)] == false)
        posX += planeX * moveSpeed;
    if (worldMap[int(posX)][int(posY + planeY * moveSpeed)] == false)
        posY += planeY * moveSpeed;
}

if (mousePressed(SDL_BUTTON_LEFT)) {
    double minDist = 1e30;
    int target = -1;

    // Loop through sprites to find the closest one in crosshair
    for (int i = 0; i < numSprites; i++) {
      if (!spriteAlive[i]) continue; // Skip dead sprites
      if (!sprite[i].isEnemy) continue; // Skip non-enemy sprites


        // Direction vector from player to sprite
        double dx = sprite[i].x - posX;
        double dy = sprite[i].y - posY;

        double distance = sqrt(dx * dx + dy * dy);
        double dirLength = sqrt(dirX * dirX + dirY * dirY);

        // Normalize dot product to get cosine of angle between player view and sprite
        double dot = (dx * dirX + dy * dirY) / (distance * dirLength);

        // If angle is small enough (crosshair aim), register as a hit
        if (dot > 0.99 && distance < 5.0) { // Field of view tolerance and distance check
            if (distance < minDist) {
                minDist = distance;
                target = i;
            }
        }
    }

    if (target != -1) {
        spriteAlive[target] = false;
        std::cout << "Hit enemy #" << target << std::endl;
    }
}
    //move backwards if no wall behind you
    if(keyDown(SDLK_s))
    {
      if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
    }
    //rotate to the right
    if(keyDown(SDLK_RIGHT))
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if(keyDown(SDLK_LEFT))
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
    if(keyDown(SDLK_ESCAPE))
    {
      break;
    }
  }
}

//sort the sprites based on distance
void sortSprites(int* order, double* dist, int amount)
{
  std::vector<std::pair<double, int>> sprites(amount);
  for(int i = 0; i < amount; i++) {
    sprites[i].first = dist[i];
    sprites[i].second = order[i];
  }
  std::sort(sprites.begin(), sprites.end());
  // restore in reverse order to go from farthest to nearest
  for(int i = 0; i < amount; i++) {
    dist[i] = sprites[amount - i - 1].first;
    order[i] = sprites[amount - i - 1].second;
  }
}
