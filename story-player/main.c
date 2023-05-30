/*******************************************************************************************
*
*   raygui - custom file dialog to load image
*
*   DEPENDENCIES:
*       raylib 4.0  - Windowing/input management and drawing.
*       raygui 3.0  - Immediate-mode GUI controls.
*
*   COMPILATION (Windows - MinGW):
*       gcc -o $(NAME_PART).exe $(FILE_NAME) -I../../src -lraylib -lopengl32 -lgdi32 -std=c99
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2023 Ramon Santamaria (@raysan5)
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui_file_dialog.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 560;

    InitWindow(screenWidth, screenHeight, "OpenStoryTeller - Player");
    SetExitKey(0);

    Font fontTtf = LoadFontEx("Inter-Regular.ttf", 14, 0, 0);

    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x133D42ff);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
    GuiSetIconScale(3);

    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x6DBFB2ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xffffffff);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0x2D7D85FF);


    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0x6DBFB2ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0xEA471Aff);

    GuiSetFont(fontTtf);


    // Custom file dialog
    GuiFileDialogState fileDialogState = InitGuiFileDialog(GetWorkingDirectory());
    Texture2D logoTexture = LoadTexture("logo-color2.png");        // Texture loading

    bool exitWindow = false;

    char fileNameToLoad[512] = { 0 };

    Texture texture = { 0 };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!exitWindow)    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        exitWindow = WindowShouldClose();

        if (fileDialogState.SelectFilePressed)
        {
            // Load image file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".c32"))
            {
                strcpy(fileNameToLoad, TextFormat("%s/%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                UnloadTexture(texture);
                texture = LoadTexture(fileNameToLoad);
            }

            fileDialogState.SelectFilePressed = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        DrawTextureEx(logoTexture,  (Vector2){ (float)10, (float)10 }, 0, 0.15, WHITE);

        DrawTexture(texture, GetScreenWidth()/2 - texture.width/2, GetScreenHeight()/2 - texture.height/2 - 5, WHITE);
//        DrawRectangleLines(GetScreenWidth()/2 - texture.width/2, GetScreenHeight()/2 - texture.height/2 - 5, texture.width, texture.height, BLACK);

//        DrawText(fileNameToLoad, 208, GetScreenHeight() - 20, 10, GRAY);

        // raygui: controls drawing
        //----------------------------------------------------------------------------------
        if (fileDialogState.windowActive) GuiLock();

        if (GuiButton((Rectangle){ 20, 140, 60, 60 }, "#05#")) fileDialogState.windowActive = true;

        // Pause ICON_PLAYER_PAUSE
        if (GuiButton((Rectangle){ 20 + 65, 140, 60, 60 }, "#132#"))
        {

        }

        // House ICON_HOUSE
        if (GuiButton((Rectangle){ 20 + 2*65, 140, 60, 60 }, "#185#"))
        {

        }


        GuiUnlock();

        // GUI: Dialog Window
        //--------------------------------------------------------------------------------
        GuiFileDialog(&fileDialogState);
        //--------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
