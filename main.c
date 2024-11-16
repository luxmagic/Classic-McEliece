#include "main.h"
#include <pthread.h>
#include <keygen.h>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "choice_file.h"

#include "raygui/styles/dark/style_dark.h"

#define WITDH   1010
#define HEIGHT  697

#define MAX_FILE_NAME_LENGTH 256



// Загрузка текстового файла
char* LoadTextFile(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    rewind(file);

    char* fileContent = (char*)malloc(fileSize + 1);
    fread(fileContent, fileSize, 1, file);
    fileContent[fileSize] = '\0';

    fclose(file);
    return fileContent;
}

// Загрузка двоичного файла
void* LoadBinaryFile(const char* fileName, int* fileSize)
{
    FILE* file = fopen(fileName, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    void* fileContent = malloc(*fileSize);
    fread(fileContent, *fileSize, 1, file);

    fclose(file);
    return fileContent;
}
//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void Button000();
static void Button001();
static void Button002();
static void Button003();
static void Button004();


//----------------------------------------------------------------------------------
// Variables Declaration
//----------------------------------------------------------------------------------
typedef enum
{
    key,
    enc,
    dec,
    start
} win;


static bool winActive = false;
static win winFlag = start;
static bool WindowBox000Active = false;
char textBoxText[4096] = "";

static char GroupBox003Text[15] = "Start window";
const char Button000Text[15] = "Key Gen";
const char Button001Text[15] = "Encode";
const char Button002Text[15] = "Decode";

const char Button003Text[15] = "OK";
const char Button004Text[15] = "Cancel";


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    run();
    // Initialization
    //---------------------------------------------------------------------------------------
    InitWindow(WITDH, HEIGHT, "Classic McEliece");
    GuiLoadStyleDark();
    // Classic McEliece: controls initialization
    //---------------------------------------------------------------------------------

    const char *ProgressBar004Text = "";
    float ProgressBar004Value = 0.3f;
    
    // Custom file dialog
    //----------------------------------------------------------------------------------
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    char fileNameToLoad[512] = { 0 };
    char* textFileContent = NULL;
    void* binaryFileContent = NULL;
    int binaryFileSize = 0;

    // Custom text dialog
    //----------------------------------------------------------------------------------
    const char *WindowBox000Text = "Text Dialog";
    bool TextBox001EditMode = false;
    Rectangle textBoxRect = { 384, 208, 264, 176 };

    bool secretViewActive = false;
    Vector2 mouseOffset = { 0, 0 };
    bool isDragging = false;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Implement required update logic
        //----------------------------------------------------------------------------------
        
        if (fileDialogState.SelectFilePressed)
        {
            // printf("6\n");
            
            if (IsFileExtension(fileDialogState.fileNameText, ".txt"))
            {
                strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                // strncpy(fileNameToLoad, fileDialogState.fileNameText, 512 - 1);
                // fileNameToLoad[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
                binaryFileSize = GetFileLength(fileNameToLoad);
                fileDialogState.SelectFilePressed = false;
            }
            else if (IsFileExtension(fileDialogState.fileNameText, ".bin"))
            {
                strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                // strncpy(fileNameToLoad, fileDialogState.fileNameText, 512 - 1);
                // fileNameToLoad[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
                binaryFileSize = GetFileLength(fileNameToLoad);
                fileDialogState.SelectFilePressed = false;
            }
            else printf("8\n");
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Check if the mouse is within the bounds of the text input box
            if (CheckCollisionPointRec(GetMousePosition(), textBoxRect))
            {
                // Calculate the offset of the mouse cursor from the top-left corner of the window
                mouseOffset.x = GetMouseX() - textBoxRect.x;
                mouseOffset.y = GetMouseY() - textBoxRect.y;

                // Set the flag to indicate that the window is being dragged
                isDragging = true;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isDragging)
        {
            // Update the position of the text input box based on the mouse movement
            textBoxRect.x = GetMouseX() - mouseOffset.x;
            textBoxRect.y = GetMouseY() - mouseOffset.y;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            // Reset the flag to indicate that the window is no longer being dragged
            isDragging = false;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            if (GuiButton((Rectangle){ 168, 48, 168, 48 }, Button000Text)) Button000(); 
            if (GuiButton((Rectangle){ 408, 48, 192, 48 }, Button001Text)) Button001(); 
            if (GuiButton((Rectangle){ 672, 48, 168, 48 }, Button002Text)) Button002(); 
            GuiGroupBox((Rectangle){ 48, 110, 912, 539 }, GroupBox003Text);
            
            if (winFlag == key && winActive)
            {
                // GuiGroupBox((Rectangle){ 48, 110, 912, 539}, GroupBox003Text);
                // DrawTextEx(font1, "hello", (Vector2){ 100, 150 }, 32.0, 2.0, VIOLET); 
                GuiProgressBar((Rectangle){ 96, 336, 504, 24 }, ProgressBar004Text, NULL, &ProgressBar004Value, 0, 1);
            }
            if (winFlag == enc && winActive)
            {
                // GuiGroupBox((Rectangle){ 48, 110, 912, 539 }, GroupBox003Text);
                GuiProgressBar((Rectangle){ 96, 336, 450, 24 }, ProgressBar004Text, NULL, &ProgressBar004Value, 0, 1);

                if (GuiButton((Rectangle){68, 130, 200, 50}, GuiIconText(ICON_FILE_OPEN, "Open File")))
                {
                    fileDialogState.windowActive = true;
                }

                if (GuiButton((Rectangle){68, 190, 200, 50}, GuiIconText(ICON_TEXT_NOTES, "Enter Text")))
                {
                    WindowBox000Active = true;
                }

                DrawText("Data Content:", 100, 400, 20, LIGHTGRAY);
                if (textFileContent)
                {
                    DrawText(textFileContent, 100, 420, 20, LIGHTGRAY);
                }

                DrawText(TextFormat("Data Size: %i bytes", binaryFileSize), 100, 440, 20, LIGHTGRAY);

                // GUI: Dialog Window
                //--------------------------------------------------------------------------------
                if (WindowBox000Active)
                {
                    WindowBox000Active = !GuiWindowBox(textBoxRect, WindowBox000Text);
                    if (GuiTextBox((Rectangle){textBoxRect.x + 24, textBoxRect.y + 40, 216, 48}, textBoxText, 4096, TextBox001EditMode))
                        TextBox001EditMode = !TextBox001EditMode;
                    if (GuiButton((Rectangle){textBoxRect.x + 24, textBoxRect.y + 112, 96, 32}, Button003Text))
                        Button003();
                    if (GuiButton((Rectangle){textBoxRect.x + 144, textBoxRect.y + 112, 96, 32}, Button004Text))
                        Button004();
                }

                // GUI: Dialog Window
                //--------------------------------------------------------------------------------
                if (fileDialogState.windowActive) GuiLock();
                GuiUnlock();
                GuiWindowFileDialog(&fileDialogState);
            }
            if (winFlag == dec && winActive)
            {
                // GuiGroupBox((Rectangle){ 48, 110, 912, 539 }, GroupBox003Text);
                GuiProgressBar((Rectangle){ 96, 336, 430, 24 }, ProgressBar004Text, NULL, &ProgressBar004Value, 0, 1);
            }
            if (!winActive)
            {
                winFlag = start;
                strncpy_s(GroupBox003Text, 15, "Start window", 15);
            }

            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    if (textFileContent) free(textFileContent);
    if (binaryFileContent) free(binaryFileContent);
    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------
void checkFlag(win flag)
{
    if (winFlag != flag && !winActive) 
    {
        winActive = !winActive;
        winFlag = flag;
    }
    else if (winFlag != flag && winActive)
    {
        winFlag = flag;
    }
    else if (winFlag == flag && winActive)
    {
        winActive = !winActive;
    }
}

static void Button000()
{
    checkFlag(key);
    strncpy_s(GroupBox003Text, 15, Button000Text, 15);
}
static void Button001()
{
    checkFlag(enc);
    strncpy_s(GroupBox003Text, 15, Button001Text, 15);
}
static void Button002()
{
    checkFlag(dec);
    strncpy_s(GroupBox003Text, 15, Button002Text, 15);
}

static void Button003()
{
    WindowBox000Active = false;
    printf("%s\n", textBoxText);
    strncpy_s(textBoxText, 4096, "", 4096);
}
static void Button004()
{
    WindowBox000Active = false;
    strncpy_s(textBoxText, 4096, "", 4096);
}
