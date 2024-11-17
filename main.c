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
unsigned char* LoadBinaryFile(const char* fileName, int* fileSize)
{
    FILE* file = fopen(fileName, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    unsigned char* fileContent = malloc(*fileSize);
    fread(fileContent, *fileSize, 1, file);

    fclose(file);
    return fileContent;
}

void FileParsing(GuiWindowFileDialogState * const state, char * const fileName, int * const fileSize, char ** fileContent)
{
    strcpy(fileName, TextFormat("%s" PATH_SEPERATOR "%s", state->dirPathText, state->fileNameText));
    // strncpy(fileNameToLoad, fileDialogState.fileNameText, 512 - 1);
    // fileNameToLoad[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
    *fileSize = GetFileLength(fileName);
    state->SelectFilePressed = false;
    // printf("%s\n", state->filterExt);
    // printf("%i\n", *fileSize);

    if (strcmp(state->filterExt, ".txt") == 0)
    {
        *fileContent = LoadTextFile(fileName);
    }
    else if (strcmp(state->filterExt, ".bin") == 0)
    {
        *fileContent = (void *)LoadBinaryFile(fileName, fileSize);
    }
    else if (strcmp(state->filterExt, ".hex") == 0)
    {
        *fileContent = (void *)LoadBinaryFile(fileName, fileSize);
    }

    // printf("%s\n", *fileContent);


}


//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void Button000(); //Key Gen
static void Button001(); //Encode
static void Button002(); //Decode
static void Button003(); //OK
static void Button004(); //Cancel


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

static char textBoxText[4096] = "";
static int dataSize = 0;
static int encodedDataSize = 0;
static int deltaDataSize = 0;
static int keySize = 0;
static int privateKeySize = 0;
static int cycleCount = 0;
static int timeCount = 0;
static int memoryCount = 0;


static char* dataContent = NULL;
static char* keyContent = NULL;

static char GroupBox003Text[15] = "Start window";
const char Button000Text[15] = "Key Gen";
const char Button001Text[15] = "Encode";
const char Button002Text[15] = "Decode";

const char *WindowBox000Text = "Text Dialog";
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

    const char *IncapProgressText = "";
    float IncapProgressValue = 10.0f;

    const char *EncodeProgressText = "";
    float EncodeProgressValue = 20.0f;

    const char *MemoryProgressTextLeft = "Using Memory: 0%";
    const char *MemoryProgressTextRight = "100%";
    float MemoryProgressValue = 20.0f;
    
    // Custom file dialog
    //----------------------------------------------------------------------------------
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    GuiWindowFileDialogState keyDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    char fileNameToLoad[512] = {0,};
    char keyNameToLoad[512] = {0,};
    
    // Custom text dialog
    //----------------------------------------------------------------------------------
    bool TextBox001EditMode = false;
    Rectangle textBoxRect = { 384, 208, 264, 176 };

    bool secretViewActive = false;
    Vector2 mouseOffset = { 0, 0 };
    bool isDragging = false;


    bool openKeyStatus = false;
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
            if (IsFileExtension(fileDialogState.fileNameText, ".txt"))
            {
                strcpy(fileDialogState.filterExt, ".txt");
                FileParsing(&fileDialogState, fileNameToLoad, &dataSize, &dataContent);
                // printf("%s\n", dataContent);
            }
            if (IsFileExtension(fileDialogState.fileNameText, ".bin"))
            {
                strcpy(fileDialogState.filterExt, ".bin");
                FileParsing(&fileDialogState, fileNameToLoad, &dataSize, &dataContent);
                // printf("%s\n", dataContent);
            }
            if (IsFileExtension(fileDialogState.fileNameText, ".hex"))
            {
                strcpy(fileDialogState.filterExt, ".hex");
                FileParsing(&fileDialogState, fileNameToLoad, &dataSize, &dataContent);
                // printf("%s\n", dataContent);
            }
        }
        if (keyDialogState.SelectFilePressed)
        {
            if (IsFileExtension(keyDialogState.fileNameText, ".txt"))
            {
                strcpy(keyDialogState.filterExt, ".bin");
                FileParsing(&keyDialogState, keyNameToLoad, &keySize, &keyContent);
            }

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
                GuiProgressBar((Rectangle){ 96, 336, 504, 24 }, ProgressBar004Text, NULL, &ProgressBar004Value, 0, 1);
            }
            if (winFlag == enc && winActive)
            {
                // GuiGroupBox((Rectangle){ 48, 110, 912, 539 }, GroupBox003Text);
                // GuiProgressBar((Rectangle){ 96, 336, 450, 24 }, ProgressBar004Text, NULL, &ProgressBar004Value, 0, 1);
                
                // Incapsulation
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 130, 872, 24}, "Incapsulation");
                
                if (GuiButton((Rectangle){68, 160, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Load Key"))) keyDialogState.windowActive = true;

                DrawText(TextFormat("Key File: %s\n", keyDialogState.fileNameText), 280, 160+2, 20, LIGHTGRAY);
                if (keySize % 1024 >= 1024) DrawText(TextFormat("Key Size: %4.4f MB", (double)((double)keySize / 1024 / 1024)), 550, 160+2, 20, LIGHTGRAY);
                else if (keySize % 1024 >= 100) DrawText(TextFormat("Key Size: %4.4f KB", (double)((double)keySize / 1024)), 550, 160+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Key Size: %i bytes", keySize), 550, 160+2, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){820, 160, 120, 24}, GuiIconText(ICON_FILE_DELETE, "Delete Key")))
                {
                    if (strcmp(keyDialogState.fileNameText, "") != 0)
                    {
                        keyDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(keyNameToLoad, 512, "", 512);
                        keySize = 0;
                        if (keyContent != NULL) free(keyContent);
                        keyContent = NULL;
                    }
                }
                if (GuiButton((Rectangle){68, 190, 200, 24}, GuiIconText(ICON_KEY, "Incapsulation")))
                {
                    // TODO: запуск функции инкапсуляции, получения и сохранения файла закрытого ключа, его размера. Можно сделать как загрузку файла тип чтобы ввели название для нового файла.
                }
                GuiProgressBar((Rectangle){ 600, 190, 340, 24 }, IncapProgressText, NULL, &IncapProgressValue, 0, 100);
                if (privateKeySize % 1024 >= 1024) DrawText(TextFormat("Private Key Size: %4.4f MB", (double)((double)privateKeySize / 1024 / 1024)), 280, 190+2, 20, LIGHTGRAY);
                else if (privateKeySize % 1024 >= 100) DrawText(TextFormat("Private Key Size: %4.4f KB", (double)((double)privateKeySize / 1024)), 280, 190+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Private Key Size: %i bytes", privateKeySize), 280, 190+2, 20, LIGHTGRAY);


                // File Work
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 220, 872, 24}, "Chose File");
                if (GuiButton((Rectangle){68, 250, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Open File"))) fileDialogState.windowActive = true;
                if (GuiButton((Rectangle){68, 280, 200, 24}, GuiIconText(ICON_TEXT_NOTES, "Enter Text"))) WindowBox000Active = true;

                if (strcmp(fileDialogState.fileNameText, "") != 0) DrawText(TextFormat("Data Content: %s\n", fileDialogState.fileNameText), 280, 250+2, 20, LIGHTGRAY);
                else if (strcmp(textBoxText, "") != 0) DrawText("Data Content: text", 280, 250+2, 20, LIGHTGRAY);
                else DrawText("Data Content: null", 280, 250+2, 20, LIGHTGRAY);

                if (dataSize % 1024 >= 1024) DrawText(TextFormat("Original Data Size: %4.4f MB", (double)((double)dataSize / 1024 / 1024)), 280, 280+2, 20, LIGHTGRAY);
                else if (dataSize % 1024 >= 100) DrawText(TextFormat("Original Data Size: %4.4f KB", (double)((double)dataSize / 1024)), 280, 280+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Original Data Size: %i bytes", dataSize), 280, 280+2, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){740, 250, 200, 54}, GuiIconText(ICON_FILE_DELETE, "Delete File")))
                {
                    if (strcmp(fileDialogState.fileNameText, "") != 0)
                    {
                        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512);
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                    else if(strcmp(textBoxText, "") != 0)
                    {
                        strncpy_s(textBoxText, 4096, "", 4096);
                        dataSize = 0;
                    }
                }

                // Encoding Data
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 310, 872, 24}, "Encoding Data");
                if (GuiButton((Rectangle){68, 340, 872, 50}, GuiIconText(ICON_SHIELD, "Encode Data")))
                {
                    // TODO: запуск функции шифрования ранее выбранного файла. Используется закрытый ключ, полученный ранее. Результат шифрования сохраняется в txt файл.
                }
                GuiProgressBar((Rectangle){ 68, 400, 872, 24 }, EncodeProgressText, NULL, &EncodeProgressValue, 0, 100);
                // Encode Methrics
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 430, 872, 24}, "Encode Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 460+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 300, 460+2, 20, LIGHTGRAY);
                GuiProgressBar((Rectangle){ 179, 500, 731, 24 }, MemoryProgressTextLeft, MemoryProgressTextRight, &MemoryProgressValue, 0, 100);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 535, 20, LIGHTGRAY);
                // Encode Results
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 555, 872, 24}, "Results");
                DrawText(TextFormat("Encoded Data Size: %i bytes", encodedDataSize), 68, 585, 20, LIGHTGRAY);
                DrawText(TextFormat("Delta Data Size: %i bytes", deltaDataSize), 68, 610, 20, LIGHTGRAY);

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
                if (keyDialogState.windowActive) GuiLock();
                GuiUnlock();
                GuiWindowFileDialog(&fileDialogState);
                GuiWindowFileDialog(&keyDialogState);
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
    if (dataContent) free(dataContent);
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
    dataSize = strlen(textBoxText);
}
static void Button004()
{
    WindowBox000Active = false;
    dataSize = 0;
    strncpy_s(textBoxText, 4096, "", 4096);
}
