#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define UI_HEIGHT 50

int mapWidth = 24;
int mapHeight = 24;

int currentWallType = 1;
int spawnX = -1;
int spawnY = -1;

int **worldMap;
char outputFile[256] = "map.h";   // Default export filename

Color GetTileColor(int tileType)
{
    switch (tileType)
    {
        case 0: return LIGHTGRAY;
        case 1: return DARKGRAY;
        case 2: return GREEN;
        case 3: return BLUE;
        case 4: return ORANGE;
        case 5: return PURPLE;
        case 6: return YELLOW;
        case 7: return RED;
        case 8: return BROWN;
        default: return BLACK;
    }
}

void AllocateMap(int w, int h)
{
    worldMap = malloc(w * sizeof(int*));
    for (int i = 0; i < w; i++)
        worldMap[i] = malloc(h * sizeof(int));
}

void InitializeMap()
{
    for (int x = 0; x < mapWidth; x++)
    {
        for (int y = 0; y < mapHeight; y++)
        {
            if (x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1)
                worldMap[x][y] = 1;
            else
                worldMap[x][y] = 0;
        }
    }
}

void ExportMap(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        printf("Failed to open file for writing\n");
        return;
    }

    fprintf(f, "#define mapWidth %d\n", mapWidth);
    fprintf(f, "#define mapHeight %d\n", mapHeight);
    fprintf(f, "bool FLAGS[12];\n");
    fprintf(f, "int worldMap[mapWidth][mapHeight] = {\n");

    for (int y = 0; y < mapHeight; y++)
    {
        fprintf(f, "    {");
        for (int x = 0; x < mapWidth; x++)
        {
            fprintf(f, "%d", worldMap[x][y]);
            if (x < mapWidth - 1) fprintf(f, ", ");
        }
        fprintf(f, "}");
        if (y < mapHeight - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "};\n");

    if (spawnX >= 0 && spawnY >= 0)
        fprintf(f, "int playerSpawnX = %d;\nint playerSpawnY = %d;\n", spawnX, spawnY);
    else
        fprintf(f, "// No player spawn set\n");

    fclose(f);
    printf("Map exported to %s\n", filename);
}

bool LoadMap(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    fscanf(f, "#define mapWidth %d\n", &mapWidth);
    fscanf(f, "#define mapHeight %d\n", &mapHeight);

    AllocateMap(mapWidth, mapHeight);

    char buffer[1024];
    fgets(buffer, sizeof(buffer), f); // FLAGS
    fgets(buffer, sizeof(buffer), f); // worldMap line

    for (int y = 0; y < mapHeight; y++)
{
    fgets(buffer, sizeof(buffer), f);
    char *p = buffer;

    for (int x = 0; x < mapWidth; x++)
    {
        // Skip braces, commas, spaces
        while (*p && (*p == '{' || *p == ' ' || *p == '\t' || *p == ',')) p++;

        int val = 0;
        if (sscanf(p, "%d", &val) == 1)
            worldMap[x][y] = val;
        else
            worldMap[x][y] = 0;  // fallback if parsing fails

        // Move pointer past this number
        while (*p && *p != ',' && *p != '}' && *p != '\n') p++;
    }
}


    spawnX = spawnY = -1;
    while (fgets(buffer, sizeof(buffer), f))
    {
        if (sscanf(buffer, "int playerSpawnX = %d;", &spawnX) == 1) continue;
        if (sscanf(buffer, "int playerSpawnY = %d;", &spawnY) == 1) continue;
    }

    fclose(f);
    printf("Loaded map from %s\n", filename);
    return true;
}

int main(void)
{
    char inputFile[256];

    printf("Enter input map filename (or press Enter to create new): ");
    fgets(inputFile, sizeof(inputFile), stdin);
    inputFile[strcspn(inputFile, "\n")] = 0; // remove newline

    printf("Enter output map filename (default: map.h): ");
    fgets(outputFile, sizeof(outputFile), stdin);
    outputFile[strcspn(outputFile, "\n")] = 0; 
    if (strlen(outputFile) == 0) strcpy(outputFile, "map.h");

    if (strlen(inputFile) > 0)
    {
        if (!LoadMap(inputFile))
        {
            printf("Failed to load %s, creating new map.\n", inputFile);
            printf("Enter MAX MAP SIZE: ");
            int MPS; scanf("%d", &MPS);
            mapWidth = mapHeight = MPS;
            AllocateMap(mapWidth, mapHeight);
            InitializeMap();
        }
    }
    else
    {
        printf("Enter MAX MAP SIZE: ");
        int MPS; scanf("%d", &MPS);
        mapWidth = mapHeight = MPS;
        AllocateMap(mapWidth, mapHeight);
        InitializeMap();
    }

    const int windowMaxWidth = 800;
    const int windowMaxHeight = 600;
    InitWindow(windowMaxWidth, windowMaxHeight, "Raylib Map Editor");

    float scaleX = (float)windowMaxWidth / (float)mapWidth;
    float scaleY = (float)(windowMaxHeight - UI_HEIGHT) / (float)mapHeight;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    float tileSize = scale;

    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();

        for (int i = 0; i <= 8; i++)
        {
            int key = KEY_ZERO + i;
            if (IsKeyPressed(key)) currentWallType = i;
        }

        int tx = (int)(mousePos.x / tileSize);
        int ty = (int)(mousePos.y / tileSize);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (tx >= 0 && tx < mapWidth && ty >= 0 && ty < mapHeight)
                worldMap[tx][ty] = currentWallType;
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            if (tx >= 0 && tx < mapWidth && ty >= 0 && ty < mapHeight)
            {
                spawnX = tx;
                spawnY = ty;
            }
        }

        if (IsKeyPressed(KEY_E))
            ExportMap(outputFile);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int x = 0; x < mapWidth; x++)
        {
            for (int y = 0; y < mapHeight; y++)
            {
                Rectangle r = { x * tileSize, y * tileSize, tileSize, tileSize };
                Color col = GetTileColor(worldMap[x][y]);
                if (x == spawnX && y == spawnY) col = MAROON;
                DrawRectangleRec(r, col);
                DrawRectangleLines((int)r.x, (int)r.y, (int)tileSize, (int)tileSize, GRAY);
            }
        }

        DrawRectangle(0, windowMaxHeight - UI_HEIGHT, windowMaxWidth, UI_HEIGHT, LIGHTGRAY);
        DrawText("Click tiles to change. Keys 0-8 = tile type.", 10, windowMaxHeight - UI_HEIGHT + 5, 20, BLACK);
        DrawText(TextFormat("Current tile type: %d", currentWallType), 10, windowMaxHeight - UI_HEIGHT + 25, 20, BLACK);
        DrawText("Right click = spawn. Press E to export.", 400, windowMaxHeight - UI_HEIGHT + 25, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
