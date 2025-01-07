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

#include "chip32_vm.h"
#include <stdbool.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif


int set_filename_from_memory(chip32_ctx_t *ctx, uint32_t addr, char *filename_mem)
{
    int valid = 0;

    // Test if address is valid

    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam) {
        strcpy(&filename_mem[0], (const char *)&ctx->ram.mem[addr]);
    } else {
        strcpy(&filename_mem[0], (const char *)&ctx->rom.mem[addr]);
    }

    return valid;
}


chip32_result_t vm_load_script(chip32_ctx_t *ctx, const char *filename)
{
    chip32_result_t run_result = VM_FINISHED;
    FILE *fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        long int sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (sz <= ctx->rom.size)
        {
            fread(ctx->rom.mem, sz, 1, fp);
            run_result = VM_OK;
            chip32_initialize(ctx);
        }
        fclose(fp);
    }
    return run_result;
}

#define MAX_PATH 260

void get_home_path(char *homedir)
{
#ifdef _WIN32
    snprintf(homedir, MAX_PATH, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
    snprintf(homedir, MAX_PATH, "%s", getenv("HOME"));
#endif
}


void get_parent_dir(const char *path, char *parent)
{
    int parentLen;
    char* last = strrchr(path, '/');

    if (last != NULL) {

        parentLen = strlen(path) - strlen(last + 1);
        strncpy(parent, path, parentLen);
    }
}


static Music gMusic;
static char root_dir[260];
static bool gMusicLoaded = false;

static Texture texture = { 0 };

#define EV_BUTTON_OK        0x01
#define EV_BUTTON_LEFT      0x02
#define EV_BUTTON_RIGHT     0x04

uint8_t story_player_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;

    static char image_path[260];
    static char sound_path[260];

    if (code == 1) //  // Execute media
    {
        printf("SYSCALL 1\n");
        fflush(stdout);
//        UnloadTexture(*tex);
//        *tex =

        if (ctx->registers[R0] != 0)
        {
            // sound file name address is in R1
            char image[100];
            set_filename_from_memory(ctx, ctx->registers[R0], image);

            strcpy(image_path, root_dir);
            strcat(image_path, "images/");
            strcat(image_path, image);

            texture = LoadTexture(image_path);
        }
        else
        {
            UnloadTexture(texture);
        }


        if (ctx->registers[R1] != 0)
        {
            // sound file name address is in R1
            char sound[100];
            set_filename_from_memory(ctx, ctx->registers[R1], sound);

            strcpy(sound_path, root_dir);
            strcat(sound_path, "sounds/");
            strcat(sound_path, sound);

            gMusic = LoadMusicStream(sound_path);
            gMusic.looping = false;
            gMusicLoaded = true;

            if (IsMusicValid(gMusic))
            {
                PlayMusicStream(gMusic);
            }
        }
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    else if (code == 2) // Wait Event
    {
        printf("SYSCALL 2\n");
        fflush(stdout);
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    return retCode;
}

    // VM Stuff
    //---------------------------------------------------------------------------------------
    uint8_t rom_data[16*1024];
    uint8_t ram_data[16*1024];
    chip32_ctx_t chip32_ctx;

    
    chip32_result_t run_result = VM_FINISHED;

    // Directories
    //---------------------------------------------------------------------------------------
    char homedir[MAX_PATH];

    GuiFileDialogState fileDialogState;
    Texture2D logoTexture;

    int screenWidth = 400;
    int screenHeight = 560;
    bool exitWindow = false;

    char fileNameToLoad[512] = { 0 };


void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    exitWindow = WindowShouldClose();

    if (fileDialogState.SelectFilePressed)
    {
        if (IsFileExtension(fileDialogState.fileNameText, ".c32"))
        {
            strcpy(fileNameToLoad, TextFormat("%s/%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
            run_result = vm_load_script(&chip32_ctx, fileNameToLoad);
            get_parent_dir(fileNameToLoad, root_dir);
            printf("Root directory: %s\n", root_dir);
        }

        fileDialogState.SelectFilePressed = false;
    }

    // VM next instruction
    if (run_result == VM_OK)
    {
        run_result = chip32_step(&chip32_ctx);
    }

    if (gMusicLoaded)
    {
        UpdateMusicStream(gMusic);
        if (!IsMusicStreamPlaying(gMusic))
        {
            StopMusicStream(gMusic);
            UnloadMusicStream(gMusic);
            gMusicLoaded = false;
            run_result = VM_OK; // continue VM execution
        }
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    

    DrawTextureEx(logoTexture,  (Vector2){ (float)10, (float)450 }, 0, 0.15, WHITE);

    // Image de l'histoire
    DrawRectangle(40, 25, 320, 240, WHITE);
    DrawTexture(texture, 220, 25, WHITE);

    GuiSetIconScale(3);

    const int yBottomScreen = 275;




    // ICON_ARROW_LEFT
    if (GuiButton((Rectangle){ 40, yBottomScreen, 60, 60 }, "#114#"))
    {
        if (run_result == VM_WAIT_EVENT)
        {
            chip32_ctx.registers[R0] = EV_BUTTON_LEFT;
            run_result = VM_OK;
        }
    }

    // ICON_OK_TICK
    if (GuiButton((Rectangle){ 40 + 65, yBottomScreen, 60, 60 }, "#112#"))
    {
        if (run_result == VM_WAIT_EVENT)
        {
            chip32_ctx.registers[R0] = EV_BUTTON_OK;
            run_result = VM_OK;
        }
    }

    // ICON_ARROW_RIGHT
    if (GuiButton((Rectangle){ 40 + 2*65, yBottomScreen, 60, 60 }, "#115#"))
    {
        if (run_result == VM_WAIT_EVENT)
        {
            chip32_ctx.registers[R0] = EV_BUTTON_RIGHT;
            run_result = VM_OK;
        }
    }


        // ICON_PLAYER_PAUSE
    if (GuiButton((Rectangle){ 180 + 65, 450, 60, 60 }, "#132#"))
    {

    }

    // ICON_HOUSE
    if (GuiButton((Rectangle){ 180 + 2*65, 450, 60, 60 }, "#185#"))
    {

    }

    // raygui: controls drawing
    //----------------------------------------------------------------------------------

    if (fileDialogState.windowActive) GuiLock();

    GuiUnlock();

    // GUI: Dialog Window
    //--------------------------------------------------------------------------------
    GuiSetIconScale(1);
    GuiFileDialog(&fileDialogState);
    //--------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------

    EndDrawing();
    //----------------------------------------------------------------------------------
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{

    chip32_ctx.stack_size = 512;

    chip32_ctx.rom.mem = rom_data;
    chip32_ctx.rom.addr = 0;
    chip32_ctx.rom.size = sizeof(rom_data);

    chip32_ctx.ram.mem = ram_data;
    chip32_ctx.ram.addr = sizeof(rom_data);
    chip32_ctx.ram.size = sizeof(ram_data);

    chip32_ctx.syscall = story_player_syscall;


    get_home_path(homedir);

    // Initialization
    //---------------------------------------------------------------------------------------


    InitWindow(screenWidth, screenHeight, "OpenStoryTeller - Player");
    SetExitKey(0);

    Font fontTtf = LoadFontEx("assets/Inter-Regular.ttf", 14, 0, 0);

    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x133D42ff);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);

    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x6DBFB2ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xffffffff);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0x2D7D85FF);


    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0x6DBFB2ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0xEA471Aff);

    GuiSetFont(fontTtf);

    // Custom file dialog
    fileDialogState = InitGuiFileDialog(GetWorkingDirectory());
    strcpy(fileDialogState.filterExt, ".c32");
    strcpy(fileDialogState.dirPathText, homedir);

    logoTexture = LoadTexture("assets/logo-color.png");
   
      

    InitAudioDevice();
    UnloadMusicStream(gMusic);


#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
	SetTargetFPS(60); // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		UpdateDrawFrame();
	}
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(logoTexture);     // Unload texture
    UnloadTexture(texture);     // Unload texture

    CloseAudioDevice();
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
