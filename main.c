#include "main.h"
#include <keygen.h>
#include <raylib.h>
#include <pthread.h>
#include <unistd.h>

#include "config_files.h"


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
}


//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void Button000(); //Key Gen
static void Button001(); //Encode
static void Button002(); //Decode
static void Button003(); //OK
static void Button004(); //Cancel
static void SetCFThread();


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

static bool WindowAddTextBox = false;
const char *WindowAddTextBoxText = "Text Dialog";

static bool WindowConfigBox = false;
const char *WindowConfigText = "Config Dialog";
static char nParam[512] = "";
static char kParam[512] = "";
static char tParam[512] = "";
static parameters config;
static parameters *cnf = &config;
static config_file cf;
static config_file *cf_ptr = &cf;
cf_threads *cf_threads_ptr;

static bool WindowUserNameBox = false;
static char userName[512] = "";
static int userNameSize = 0;

static bool WindowSavePublicBox = false;
static char namePublic[512] = "";
static int publicSize = 0;

static bool WindowSavePrivateBox = false;
static char namePrivate[512] = "";
static int privateSize = 0;

static char textBoxText[4096] = "";
static int dataSize = 0;
static int encodedDataSize = 0;
static int decodedDataSize = 0;
static int deltaDataSize = 0;

static int keySize = 0;
static int privateKeySize = 0;
static int publicKeySize = 0;
static methrics KGM;
static methrics *KeyGenMethics = NULL;

static int cycleCount = 0;
static int timeCount = 0;
static float deltaTime = 0.0f;
static bool startTime = false;

static bool startThreadKeyGen = false;
static bool endThreadKeyGen = false;

static int usingProcessMemory = 0;
static int memoryCount = 0;

const char *KeyGenProgressText = "";
static float KeyGenProgressValue = 0.0f;

static char *configFile = NULL;

static char* dataContent = NULL;
static char* keyContent = NULL;

static char GroupBox003Text[15] = "Start window";
const char Button000Text[15] = "Key Gen";
const char Button001Text[15] = "Encode";
const char Button002Text[15] = "Decode";

const char Button003Text[15] = "OK";
const char Button004Text[15] = "Cancel";

static int securityLevel = 0;
static bool securityLevelList = false;

static pthread_t threadKeyGen, threadEncode, threadDecode;
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    InitWindow(WITDH, HEIGHT, "Classic McEliece");
    GuiLoadStyleDark();

    usingProcessMemory = GetUsingProcessMemory();
    cf = GetDefaultConfigFile();
    
    // Classic McEliece: controls initialization
    //---------------------------------------------------------------------------------
    const char *ProgressBar004Text = "";
    float ProgressBar004Value = 0.3f;

    const char *IncapProgressText = "";
    float IncapProgressValue = 0.0f;

    const char *EncodeProgressText = "";
    float EncodeProgressValue = 0.0f;

    const char *DecodeProgressText = "";
    float DecodeProgressValue = 0.0f;
    
    const char *MemoryProgressTextLeft = "Using Memory 0%:";
    const char *MemoryProgressTextRight = "100%";
    float MemoryProgressValue = 0.0f;

    // Label for Start Window
    //----------------------------------------------------------------------------------
    Vector2 textSize = { 0, 0 };
    float textX = 0.0f;
    float textY = 0.0f;

    // Custom file dialog
    //----------------------------------------------------------------------------------
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    GuiWindowFileDialogState keyDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    char fileNameToLoad[512] = {0,};
    char keyNameToLoad[512] = {0,};
    
    // Custom text dialog
    //----------------------------------------------------------------------------------
    bool TextBoxAddTextMode = false;
    bool TextBoxConfigOneMode = false;
    bool TextBoxConfigTwoMode = false;
    bool TextBoxConfigThreeMode = false;
    Rectangle textBoxRect = { 384, 208, 264, 176 };

    bool secretViewActive = false;
    Vector2 mouseOffset = { 0, 0 };
    bool isDragging = false;

    // Rect
    //----------------------------------------------------------------------------------
    Rectangle generalBorderBoxRect = { 48, 110, 912, 539 };
    Rectangle Button000BoxRect = { 168, 48, 168, 48 };
    Rectangle Button001BoxRect = { 408, 48, 192, 48 };
    Rectangle Button002BoxRect = { 672, 48, 168, 48 };

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
                if (winFlag == key)
                {
                    if (ReadBinaryFile(fileDialogState.fileNameText, cf_ptr))
                    {
                        if (securityLevel == 1) config = cf_ptr->low;
                        if (securityLevel == 2) config = cf_ptr->middle;
                        if (securityLevel == 3) config = cf_ptr->hight;
                    }
                } 
                //printf("%s\n", dataContent);
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
                strcpy(keyDialogState.filterExt, ".txt");
                FileParsing(&keyDialogState, keyNameToLoad, &keySize, &keyContent);
            }
            if (IsFileExtension(keyDialogState.fileNameText, ".bin"))
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
        
        if (startThreadKeyGen)
        {
            printf("1\n");
            cf_threads_ptr = malloc(sizeof(cf_threads));
            SetCFThread();
            pthread_create(&threadKeyGen, NULL, GeneratePublicKey, (void *)cf_threads_ptr);
            startThreadKeyGen = false;
            endThreadKeyGen = true;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            if (GuiButton(Button000BoxRect, Button000Text)) Button000(); 
            if (GuiButton(Button001BoxRect, Button001Text)) Button001(); 
            if (GuiButton(Button002BoxRect, Button002Text)) Button002();
            GuiGroupBox(generalBorderBoxRect, GroupBox003Text);
            
            if (winFlag == key && winActive)
            {
                // Security Settings
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 130, 872, 24}, "Settings Security");
                if (GuiButton((Rectangle){68, 160, 682, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) WindowUserNameBox = true;
                DrawText(TextFormat("User: %s\n", userName), 760, 160, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){68, 190, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Load Config"))) fileDialogState.windowActive = true;
                DrawText(TextFormat("File: %s\n", fileDialogState.fileNameText), 280, 190+2, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){630, 190, 120, 24}, GuiIconText(ICON_FILE_DELETE, "Close Config")))
                {
                    if (strcmp(fileDialogState.fileNameText, "") != 0)
                    {
                        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512);
                        cf = GetDefaultConfigFile();
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                }

                if (GuiButton((Rectangle){280, 220, 260, 24}, GuiIconText(ICON_FILE_SAVE, "Save Config"))) WriteConfigFile(cf, userName);
                if (GuiButton((Rectangle){550, 220, 200, 24}, GuiIconText(ICON_GEAR, "Configure"))) WindowConfigBox = true;
                DrawText(TextFormat("Length of the code n: %i\n", config.n), 760, 190+1, 10, LIGHTGRAY);
                DrawText(TextFormat("Vector of length k: %i\n", config.k), 760, 210+1, 10, LIGHTGRAY);
                DrawText(TextFormat("Corrective ability t: %i\n", config.t), 760, 230+1, 10, LIGHTGRAY);

                // Generate Open Key
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 250, 872, 24}, "Generate Public Key");
                if (WindowUserNameBox || WindowConfigBox || WindowSavePublicBox || WindowSavePrivateBox || fileDialogState.windowActive || securityLevelList) GuiSetState(STATE_DISABLED);
                else GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 280, 872, 54}, GuiIconText(ICON_KEY, "Generate Key")))
                {
                    startThreadKeyGen = true;
                    deltaTime = 1.0f;//(float)(trunc(keygen_time*1000)) / 100.0f;
                    // printf("%f\n", deltaTime);
                }
                GuiSetState(STATE_NORMAL);
                if (endThreadKeyGen)
                {
                    pthread_mutex_lock(&cf_threads_ptr->mutex);
                    if(!cf_threads_ptr->is_done)
                    {
                        pthread_cond_wait(&cf_threads_ptr->cond, &cf_threads_ptr->mutex);
                    }
                    //if (cf_threads_ptr->is_done)
                    else
                    {
                        // printf("good\n");
                        endThreadKeyGen = false;
                        startTime = true;
                    }
                    pthread_mutex_unlock(&cf_threads_ptr->mutex);
                }

                
                if ((KeyGenProgressValue < 100.0f) && startTime) KeyGenProgressValue += deltaTime;
                else if (startTime)
                {
                    timeCount = trunc(cf_threads_ptr->KeyGenMethics.keygen_time*1000);
                    cycleCount = cf_threads_ptr->KeyGenMethics.keygen_cycle;
                    memoryCount = cf_threads_ptr->KeyGenMethics.keygen_using_memory;
                    publicKeySize = cf_threads_ptr->KeyGenMethics.key_size;
                    
                    startTime != startTime;
                    deltaTime = 0.0f;
                }
                
                GuiProgressBar((Rectangle){ 68, 350, 872, 24 }, KeyGenProgressText, NULL, &KeyGenProgressValue, 0, 100);
                
                // KeyGen Methrics
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 380, 872, 24}, "KeyGen Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 410+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 400, 410+2, 20, LIGHTGRAY);

                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 440, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 470, 20, LIGHTGRAY);
                DrawText(TextFormat("Public Key Size: %i bytes", publicKeySize), 68, 500, 20, LIGHTGRAY);
                
                // Save Results
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 530, 872, 24}, "Save Results");
                if (GuiButton((Rectangle){68, 560, 420, 54}, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save Public Key"))) WindowSavePublicBox = true;
                if (GuiButton((Rectangle){520, 560, 420, 54}, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save Private Key"))) WindowSavePrivateBox = true;
                
                // Dropdown List
                //----------------------------------------------------------------------------------
                if(GuiDropdownBox((Rectangle){68, 220, 200, 24}, "#202#Security Level;Low;Medium;High", &securityLevel, securityLevelList))
                {
                    config = ReadConfigFile(securityLevel, cf);
                    securityLevelList = !securityLevelList;
                }
                
                // GUI: Dialog Window
                //--------------------------------------------------------------------------------
                if (WindowUserNameBox) // User name input box
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_PLAYER_PLAY, "Create User"), "Enter User Name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1) TextCopy(userName, textBoxText);
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowUserNameBox = false;
                        TextCopy(textBoxText, "\0");
                    }
                }
                if (WindowConfigBox) // Config input box
                {
                    WindowConfigBox = !GuiWindowBox(textBoxRect, WindowConfigText);
                    DrawText(TextFormat("n:"), textBoxRect.x + 24, textBoxRect.y + 40 + 2, 20, LIGHTGRAY);
                    DrawText(TextFormat("k:"), textBoxRect.x + 24, textBoxRect.y + 70 + 2, 20, LIGHTGRAY);
                    DrawText(TextFormat("t:"), textBoxRect.x + 24, textBoxRect.y + 100 + 2, 20, LIGHTGRAY);

                    if (GuiTextBox((Rectangle){textBoxRect.x + 60, textBoxRect.y + 40, 180, 24}, nParam, 512, TextBoxConfigOneMode)) TextBoxConfigOneMode = !TextBoxConfigOneMode;
                    if (GuiTextBox((Rectangle){textBoxRect.x + 60, textBoxRect.y + 70, 180, 24}, kParam, 512, TextBoxConfigTwoMode)) TextBoxConfigTwoMode = !TextBoxConfigTwoMode;
                    if (GuiTextBox((Rectangle){textBoxRect.x + 60, textBoxRect.y + 100, 180, 24}, tParam, 512, TextBoxConfigThreeMode)) TextBoxConfigThreeMode = !TextBoxConfigThreeMode;
                    
                    if (GuiButton((Rectangle){textBoxRect.x + 24, textBoxRect.y + 130, 96, 32}, Button003Text)) Button003();
                    if (GuiButton((Rectangle){textBoxRect.x + 144, textBoxRect.y + 130, 96, 32}, Button004Text)) Button004();
                }
                if (WindowSavePublicBox)
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save file as..."), "Enter public key file name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1) TextCopy(namePublic, textBoxText);
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowSavePublicBox = false;
                        TextCopy(textBoxText, "\0");
                    } 
                }
                if (WindowSavePrivateBox)
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save file as..."), "Enter private key file name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1) TextCopy(namePrivate, textBoxText);
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowSavePrivateBox = false;
                        TextCopy(textBoxText, "\0");
                    }
                }

                if (fileDialogState.windowActive) GuiLock();
                GuiUnlock();
                GuiWindowFileDialog(&fileDialogState);
            }
            if (winFlag == enc && winActive)
            {
                // Incapsulation
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 130, 872, 24}, "Incapsulation");
                if (GuiButton((Rectangle){68, 160, 682, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) {}
                DrawText(TextFormat("User: %s\n", userName), 760, 160, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){68, 190, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Load Key"))) keyDialogState.windowActive = true;

                DrawText(TextFormat("Key File: %s\n", keyDialogState.fileNameText), 280, 190+2, 20, LIGHTGRAY);
                if (keySize % 1024 >= 1024) DrawText(TextFormat("Key Size: %4.2f MB", (double)((double)keySize / 1024 / 1024)), 550, 190+2, 20, LIGHTGRAY);
                else if (keySize % 1024 >= 100) DrawText(TextFormat("Key Size: %4.2f KB", (double)((double)keySize / 1024)), 550, 190+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Key Size: %i B", keySize), 550, 190+2, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){820, 190, 120, 24}, GuiIconText(ICON_FILE_DELETE, "Close Key File")))
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
                if (GuiButton((Rectangle){68, 220, 200, 24}, GuiIconText(ICON_KEY, "Incapsulation")))
                {
                    // TODO: запуск функции инкапсуляции, получения и сохранения файла закрытого ключа, его размера. Можно сделать как загрузку файла тип чтобы ввели название для нового файла.
                }
                GuiProgressBar((Rectangle){ 280, 220, 660, 24 }, IncapProgressText, NULL, &IncapProgressValue, 0, 100);

                // File Work
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 250, 872, 24}, "Choice File");
                if (GuiButton((Rectangle){68, 280, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Open File"))) fileDialogState.windowActive = true;
                if (GuiButton((Rectangle){68, 310, 200, 24}, GuiIconText(ICON_TEXT_NOTES, "Enter Text"))) WindowAddTextBox = true;

                if (strcmp(fileDialogState.fileNameText, "") != 0) DrawText(TextFormat("Data Content: %s\n", fileDialogState.fileNameText), 280, 280+2, 20, LIGHTGRAY);
                else if (strcmp(textBoxText, "") != 0) DrawText("Data Content: text", 280, 280+2, 20, LIGHTGRAY);
                else DrawText("Data Content: null", 280, 280+2, 20, LIGHTGRAY);

                if (dataSize % 1024 >= 1024) DrawText(TextFormat("Original Data Size: %4.2f MB", (double)((double)dataSize / 1024 / 1024)), 280, 310+2, 20, LIGHTGRAY);
                else if (dataSize % 1024 >= 100) DrawText(TextFormat("Original Data Size: %4.2f KB", (double)((double)dataSize / 1024)), 280, 310+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Original Data Size: %i bytes", dataSize), 280, 310+2, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){740, 280, 200, 54}, GuiIconText(ICON_FILE_DELETE, "Close Data")))
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
                GuiLine((Rectangle){68, 340, 872, 24}, "Encoding Data");
                // if (WindowBox001Active) GuiSetState(STATE_DISABLED);
                GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 370, 872, 50}, GuiIconText(ICON_SHIELD, "Encode Data")))
                {
                    // TODO: запуск функции шифрования ранее выбранного файла. Используется закрытый ключ, полученный ранее. Результат шифрования сохраняется в txt файл.
                }
                GuiSetState(STATE_NORMAL);
                GuiProgressBar((Rectangle){ 68, 430, 872, 24 }, EncodeProgressText, NULL, &EncodeProgressValue, 0, 100);
                // Encode Methrics
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 460, 872, 24}, "Encode Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 490+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 300, 490+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 520+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 550+2, 20, LIGHTGRAY);
                // Encode Results
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 580, 872, 24}, "Results");
                DrawText(TextFormat("Encoded Data Size: %i bytes", encodedDataSize), 68, 610, 20, LIGHTGRAY);
                DrawText(TextFormat("Delta Data Size: %i bytes", deltaDataSize), 470, 610, 20, LIGHTGRAY);

                // GUI: Dialog Window
                //--------------------------------------------------------------------------------
                if (WindowAddTextBox)
                {
                    WindowAddTextBox = !GuiWindowBox(textBoxRect, WindowAddTextBoxText);
                    if (GuiTextBox((Rectangle){textBoxRect.x + 24, textBoxRect.y + 40, 216, 48}, textBoxText, 4096, TextBoxAddTextMode)) TextBoxAddTextMode = !TextBoxAddTextMode;
                    if (GuiButton((Rectangle){textBoxRect.x + 24, textBoxRect.y + 112, 96, 32}, Button003Text)) Button003();
                    if (GuiButton((Rectangle){textBoxRect.x + 144, textBoxRect.y + 112, 96, 32}, Button004Text)) Button004();
                }

                if (fileDialogState.windowActive) GuiLock();
                if (keyDialogState.windowActive) GuiLock();
                GuiUnlock();
                GuiWindowFileDialog(&fileDialogState);
                GuiWindowFileDialog(&keyDialogState);
            }
            if (winFlag == dec && winActive)
            {
                // Decapsulation
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 130, 872, 24}, "Decapsulation");
                if (GuiButton((Rectangle){68, 160, 682, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) WindowUserNameBox = true;
                DrawText(TextFormat("User: %s\n", userName), 760, 160, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){68, 190, 300, 24}, GuiIconText(ICON_KEY, "Choice Private Key"))) keyDialogState.windowActive = true;
                DrawText(TextFormat("Key: %s", keyDialogState.fileNameText), 380, 190+2, 20, LIGHTGRAY);
                if (keySize % 1024 >= 1024) DrawText(TextFormat("Key Size: %4.2f MB", (double)((double)keySize / 1024 / 1024)), 620, 190+2, 20, LIGHTGRAY);
                else if (keySize % 1024 >= 100) DrawText(TextFormat("Key Size: %4.2f KB", (double)((double)keySize / 1024)), 620, 190+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Key Size: %i B", keySize), 620, 190+2, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){820, 190, 120, 24}, GuiIconText(ICON_KEY, "Close Key File"))) 
                {
                    if (strcmp(keyDialogState.fileNameText, "") != 0)
                    {
                        keyDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(keyNameToLoad, 512, "", 512);
                        keySize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                }

                if (GuiButton((Rectangle){68, 220, 300, 24}, GuiIconText(ICON_FILE_OPEN, "Choice Chiper File"))) fileDialogState.windowActive = true;
                DrawText(TextFormat("File: %s", fileDialogState.fileNameText), 380, 220+2, 20, LIGHTGRAY);
                if (dataSize % 1024 >= 1024) DrawText(TextFormat("Data Size: %4.2f MB", (double)((double)dataSize / 1024 / 1024)), 620, 220+2, 20, LIGHTGRAY);
                else if (dataSize % 1024 >= 100) DrawText(TextFormat("Data Size: %4.2f KB", (double)((double)dataSize / 1024)), 620, 220+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Data Size: %i B", dataSize), 620, 220+2, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){820, 220, 120, 24}, GuiIconText(ICON_FILE_DELETE, "Close File"))) 
                {
                    if (strcmp(fileDialogState.fileNameText, "") != 0)
                    {
                        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512);
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                }

                // Decode Data
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 250, 872, 24}, "Decode Data");
                if (WindowUserNameBox || fileDialogState.windowActive || keyDialogState.windowActive) GuiSetState(STATE_DISABLED);
                else GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 280, 872, 50}, GuiIconText(ICON_SHIELD, "Decode Data")))
                {
                    // TODO: запуск функции декодирования
                }
                GuiSetState(STATE_NORMAL);
                GuiProgressBar((Rectangle){ 68, 340, 872, 24 }, DecodeProgressText, NULL, &DecodeProgressValue, 0, 100);

                // Decode Methrics
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 370, 872, 24}, "Decode Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 400+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 400, 400+2, 20, LIGHTGRAY);

                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 430, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 460+2, 20, LIGHTGRAY);
                
                // Deccode Results
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 520, 872, 24}, "Results");
                DrawText(TextFormat("Decoded Data Size: %i bytes", publicKeySize), 68, 550+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Delta Data Size: %i bytes", publicKeySize), 68, 580+2, 20, LIGHTGRAY);

                // GUI: Dialog Window
                //--------------------------------------------------------------------------------
                if (WindowUserNameBox) // User name input box
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_PLAYER_PLAY, "Create User"), "Enter User Name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1) TextCopy(userName, textBoxText);
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowUserNameBox = false; 
                        TextCopy(textBoxText, "\0");
                    }
                }

                if (fileDialogState.windowActive) GuiLock();
                if (keyDialogState.windowActive) GuiLock();
                GuiUnlock();
                GuiWindowFileDialog(&fileDialogState);
                GuiWindowFileDialog(&keyDialogState);
            }
            if (!winActive)
            {
                winFlag = start;
                strncpy_s(GroupBox003Text, 15, "Start window", 15);

                textSize = MeasureTextEx(GetFontDefault(), TextFormat("WELCOME"), 40, 2);
                textX = generalBorderBoxRect.x + (generalBorderBoxRect.width - textSize.x) / 2;
                textY = generalBorderBoxRect.y + (generalBorderBoxRect.height - textSize.y) / 2 - textSize.y;
                DrawText(TextFormat("WELCOME"), textX, textY, 40, GRAY);

                textSize = MeasureTextEx(GetFontDefault(), TextFormat("Classic McEliece"), 30, 2);
                textX = generalBorderBoxRect.x + (generalBorderBoxRect.width - textSize.x) / 2;
                textY = textY + textSize.y + 20;
                DrawText(TextFormat("Classic McEliece"), textX, textY, 30, GOLD);

                textSize = MeasureTextEx(GetFontDefault(), TextFormat("Nedomolkin M.P. Bolshakova A.A. Shapovalov I."), 20, 2);
                textX = generalBorderBoxRect.x + (generalBorderBoxRect.width - textSize.x) / 2;
                textY = textY + textSize.y + 20;
                DrawText(TextFormat("Nedomolkin M.P. Bolshakova A.A. Shapovalov I."), textX, textY, 20, PURPLE);
            }
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (dataContent) free(dataContent);
    pthread_join(threadKeyGen, NULL);
    // Освобождаем ресурсы
    pthread_mutex_destroy(&cf_threads_ptr->mutex);
    pthread_cond_destroy(&cf_threads_ptr->cond);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
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
    if (winFlag == key)
    {
        WindowConfigBox = false;
        sscanf_s(nParam, "%i", &config.n);
        sscanf_s(kParam, "%i", &config.k);
        sscanf_s(tParam, "%i", &config.t);
        if (securityLevel == 1) cf.low = config;
        if (securityLevel == 2) cf.middle = config;
        if (securityLevel == 3) cf.hight = config;
    }
    if (winFlag == enc)
    {
        WindowAddTextBox = false;
        dataSize = strlen(textBoxText);
    }
    if (winFlag == dec)
    {

    }

}
static void Button004()
{
    if (winFlag == key)
    {
        WindowConfigBox = false;
        TextCopy(nParam, '\0');
        TextCopy(kParam, '\0');
        TextCopy(tParam, '\0');
    }
    if (winFlag == enc)
    {
        WindowAddTextBox = false;
        dataSize = 0;
        strncpy_s(textBoxText, 4096, "", 4096);
    }
    if (winFlag == dec)
    {

    }
}

static void SetCFThread(void)
{
    cf_threads_ptr->cf = cnf;
    cf_threads_ptr->KeyGenMethics.key_size = 0;
    cf_threads_ptr->KeyGenMethics.keygen_cycle = 0;
    cf_threads_ptr->KeyGenMethics.keygen_time = 0;
    cf_threads_ptr->KeyGenMethics.keygen_using_memory = 0;
    cf_threads_ptr->is_done = false;
    pthread_mutex_init(&cf_threads_ptr->mutex, NULL);
    pthread_mutex_init(&cf_threads_ptr->cond, NULL);
}