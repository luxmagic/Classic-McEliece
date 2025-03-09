#include "main.h"
#include <raylib.h>
#include <pthread.h>
#include <unistd.h>

#include <run.h>

#include "config_files.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "choice_file.h"
#include "file_handle.h"

#include "raygui/styles/dark/style_dark.h"

#define WITDH   1010
#define HEIGHT  697

#define MAX_FILE_NAME_LENGTH 256

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void Button000();        //Key Gen
static void Button001();        //Encode
static void Button002();        //Decode
static void Button003();        //OK
static void Button004();        //Cancel
static void setCFDec(void);
static void setCFEnc(void);
static void setCFKgn(void);

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

static parameters config;
static parameters *cnf = &config;
static config_file cf;
static config_file *cf_ptr = &cf;

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
static char textKEY_e[1024] = "Encaps Key: ";
static char textKEY_d[1024] = "Decaps Key: ";
static char fileName[100] = "";
static char *textLine = "";
static int dataSize = 0;
static int encodedDataSize = 0;
static int decodedDataSize = 0;
static int deltaDataSize = 0;

static int keySize = 0;
methrics *KGM = NULL;
methrics *ENCM = NULL;
methrics *DECM = NULL;

static int cycleCount = 0;
static int timeCount = 0;
static int usingProcessMemory = 0;
static int memoryCount = 0;

static bool startTime = false;
static bool startTimeInc = false;
static bool startTimeEnc = false;
static bool startTimeDec = false;

const char *KeyGenProgressText = "";
static float KeyGenProgressValue = 0.0f;

static char *configFile = NULL;

char* dataContent = NULL;
static char* keyContent = NULL;

static char GroupBox003Text[15] = "Start window";
const char Button000Text[15] = "Key Gen";
const char Button001Text[15] = "Incapsulation";
const char Button002Text[15] = "Decapsulation";

const char Button003Text[15] = "OK";
const char Button004Text[15] = "Cancel";

int securityLevel = 0;
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

    usingProcessMemory = get_using_memory();
    cf = GetDefaultConfigFile();
    
    // Classic McEliece: controls initialization
    //---------------------------------------------------------------------------------
    const char *ProgressBar004Text = "";
    float ProgressBar004Value = 0.3f;

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
    bool TextBoxKEYEncaps = false;
    bool TextBoxKEYDecaps = false;
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

    // Reaction Buttons
    //----------------------------------------------------------------------------------
    //KeyGen
    bool usedBtnSavePrivate = false;
    bool usedBtnSavePublic = false;

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
                if (GuiButton((Rectangle){68, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear Results")))
                {
                    strncpy(userName, "\0", 512);
                    if (strcmp(fileDialogState.fileNameText, "") != 0)
                    {
                        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512); 
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                    free(KGM);
                    cf = GetDefaultConfigFile();
                    KeyGenProgressValue = 0.0f;
                    timeCount = 0;
                    cycleCount = 0;
                    memoryCount = 0;
                    publicSize = 0;
                    privateSize = 0;
                    usedBtnSavePublic = false;
                    usedBtnSavePrivate = false;
                    clear_methrics();
                }
                if (GuiButton((Rectangle){280, 160, 208, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) WindowUserNameBox = true;
                if (GuiButton((Rectangle){500, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear User Name"))) strncpy(userName, "\0", 512);
                DrawText(TextFormat("User: %s\n", userName), 710, 160, 20, LIGHTGRAY);
                
                DrawText(TextFormat("Length of the code n: %i\n", config.n), 282, 190+1, 20, LIGHTGRAY);
                DrawText(TextFormat("Vector of length k: %i\n", config.k), 282, 220+1, 20, LIGHTGRAY);

                GuiPanel((Rectangle){600, 190, 1, 54}, NULL);

                DrawText(TextFormat("Corrective ability t: %i\n", config.t), 610, 190+1, 20, LIGHTGRAY);
                DrawText(TextFormat("Degree of the field m: %i\n", config.m), 610, 220+1, 20, LIGHTGRAY);

                // Generate Open Key
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 250, 872, 24}, "Generate Keys");
                if (WindowUserNameBox || WindowSavePublicBox || WindowSavePrivateBox || fileDialogState.windowActive || securityLevelList) GuiSetState(STATE_DISABLED);
                else GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 280, 872, 54}, GuiIconText(ICON_KEY, "Generate Keys")))
                {
                    setCFKgn();
                    create_methrics();
                    create_space(securityLevel);
                    if(securityLevel == 1) 
                    {
                        run_mce_348864(1);
                        
                    }
                    if (securityLevel == 2)
                    {
                        run_mce_460896(1);
                    }
                    if (securityLevel == 3)
                    {
                        run_mce_6688128(1);
                    }
                    methrics_keypair();
                    cpy_methrics(KGM);
                    startTime = true;
                }
                GuiSetState(STATE_NORMAL);

                if ((KeyGenProgressValue < 100.0f) && startTime) KeyGenProgressValue += 1.0;
                else if (startTime)
                {
                    
                    timeCount = trunc(KGM->time*1000);
                    cycleCount = KGM->cycle;
                    memoryCount = KGM->using_memory;
                    publicSize = KGM->one_obj_size;
                    privateSize = KGM->two_obj_size;
                    
                    startTime = false;
                }
                
                GuiProgressBar((Rectangle){ 68, 350, 872, 24 }, KeyGenProgressText, NULL, &KeyGenProgressValue, 0, 100);
                
                // KeyGen Methrics
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 380, 872, 24}, "KeyGen Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 410+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 400, 410+2, 20, LIGHTGRAY);

                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 440, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 470, 20, LIGHTGRAY);
                DrawText(TextFormat("Public Key Size: %i bytes", publicSize), 68, 500, 20, LIGHTGRAY);
                DrawText(TextFormat("Private Key Size: %i bytes", privateSize), 500, 500, 20, LIGHTGRAY);
                
                // Save Results
                //----------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 530, 872, 24}, "Save Results");
                if(usedBtnSavePublic) GuiSetState(STATE_PRESSED);
                if (GuiButton((Rectangle){68, 560, 420, 54}, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save Public Key")))
                {
                    WindowSavePublicBox = true;
                    usedBtnSavePublic = true;
                }
                GuiSetState(STATE_NORMAL);
                if(usedBtnSavePrivate) GuiSetState(STATE_PRESSED);
                if (GuiButton((Rectangle){520, 560, 420, 54}, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save Private Key")))
                {
                    WindowSavePrivateBox = true;
                    usedBtnSavePrivate = true;
                }
                GuiSetState(STATE_NORMAL);

                // Dropdown List
                //----------------------------------------------------------------------------------
                if(GuiDropdownBox((Rectangle){68, 190, 200, 54}, "#202#Security Level;Low;Medium;High", &securityLevel, securityLevelList))
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
                if (WindowSavePublicBox)
                {
                    TextCopy(textBoxText, "../keys/public_key_");
                    char fileExt[10] = ".bin";
                    strcat(textBoxText, userName);
                    strcat(textBoxText, fileExt);
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save file as..."), "Enter public key file name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1)
                    {
                        TextCopy(namePublic, textBoxText);
                        save(namePublic, 1, securityLevel);
                    }
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowSavePublicBox = false;
                        TextCopy(textBoxText, "\0");
                    }
                    if ((result == 0) || (result == 2)) usedBtnSavePublic = false;
                }
                if (WindowSavePrivateBox)
                {
                    TextCopy(textBoxText, "../keys/private_key_");
                    char fileExt[10] = ".bin";
                    strcat(textBoxText, userName);
                    strcat(textBoxText, fileExt);
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.5f));
                    int result = GuiTextInputBox((Rectangle){ (float)GetScreenWidth()/2 - 120, (float)GetScreenHeight()/2 - 60, 240, 140 }, GuiIconText(ICON_FILE_SAVE_CLASSIC, "Save file as..."), "Enter private key file name:", "Ok;Cancel", textBoxText, 512, NULL);
                    if (result == 1)
                    {
                        TextCopy(namePrivate, textBoxText);
                        save(namePrivate, 2, securityLevel);
                    }
                    if ((result == 0) || (result == 1) || (result == 2))
                    {
                        WindowSavePrivateBox = false;
                        TextCopy(textBoxText, "\0");
                    }
                    if ((result == 0) || (result == 2)) usedBtnSavePrivate = false;
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
                if (GuiButton((Rectangle){68, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear Results")))
                {
                    strncpy(userName, "\0", 512);
                    if (strcmp(keyDialogState.fileNameText, "") != 0)
                    {
                        keyDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512);
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                    if (strcmp(fileDialogState.fileNameText, "") != 0)
                    {
                        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
                        strncpy_s(fileNameToLoad, 512, "", 512); 
                        dataSize = 0;
                        if (dataContent != NULL) free(dataContent);
                        dataContent = NULL;
                    }
                    free(ENCM);
                    EncodeProgressValue = 0.0f;
                    // TextCopy(textKEY_e, "Encaps Key: ");
                    encodedDataSize = 0;
                    decodedDataSize = 0;
                    timeCount = 0;
                    cycleCount = 0;
                    memoryCount = 0;
                    publicSize = 0;
                    deltaDataSize = 0;
                    keySize = 0;
                    clear_methrics();
                }
                if (GuiButton((Rectangle){280, 160, 208, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) WindowUserNameBox = true;
                if (GuiButton((Rectangle){500, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear User Name"))) strncpy(userName, "\0", 512);
                DrawText(TextFormat("User: %s\n", userName), 710, 160, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){68, 190, 200, 24}, GuiIconText(ICON_FILE_OPEN, "Load Key"))) keyDialogState.windowActive = true;

                DrawText(TextFormat("Key File: %s\n", keyDialogState.fileNameText), 280, 190+2, 20, LIGHTGRAY);
                if (keySize % 1024 >= 1024) DrawText(TextFormat("Key Size: %4.2f MB", (double)((double)keySize / 1024 / 1024)), 600, 190+2, 20, LIGHTGRAY);
                else if (keySize % 1024 >= 100) DrawText(TextFormat("Key Size: %4.2f KB", (double)((double)keySize / 1024)), 600, 190+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Key Size: %i B", keySize), 600, 190+2, 20, LIGHTGRAY);

                if (GuiButton((Rectangle){860, 190, 80, 24}, GuiIconText(ICON_FILE_DELETE, "Close")))
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

                GuiTextBox((Rectangle){68, 220, 872, 24}, textKEY_e, 1024, TextBoxKEYEncaps);

                // Encoding Data
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 250, 872, 24}, "Encoding Data");
                if (WindowUserNameBox || keyDialogState.windowActive || fileDialogState.windowActive || WindowAddTextBox) GuiSetState(STATE_DISABLED);
                else GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 280, 872, 50}, GuiIconText(ICON_SHIELD, "Incapsulation Key")))
                {
                    TextCopy(fileName, "../data/ct_");
                    strcat(fileName, userName);
                    strcat(fileName, ".bin");

                    get_security_level(keyNameToLoad, &securityLevel);
                    create_space(securityLevel);
                    
                    setCFEnc();
                    create_methrics();
                    if(securityLevel == 1) 
                    {
                        load(keyNameToLoad, 1, &securityLevel);
                        run_mce_348864(2);
                    }
                    if (securityLevel == 2)
                    {
                        load(keyNameToLoad, 1, &securityLevel);
                        run_mce_460896(2);
                    }
                    if (securityLevel == 3)
                    {
                        load(keyNameToLoad, 1, &securityLevel);
                        run_mce_6688128(2);
                    }
                    
                    methrics_encaps();
                    cpy_methrics(ENCM);
                    get_text(textKEY_e, 1);
                    save(fileName, 3, securityLevel);
                    TextCopy(fileName, "\0");
                    startTimeEnc = true;
                }
                GuiSetState(STATE_NORMAL);
                GuiProgressBar((Rectangle){ 68, 340, 872, 24 }, EncodeProgressText, NULL, &EncodeProgressValue, 0, 100);

                if ((EncodeProgressValue < 100.0f) && startTimeEnc) EncodeProgressValue += 1.0;
                else if (startTimeEnc)
                {
                    timeCount = trunc(ENCM->time*1000);
                    cycleCount = ENCM->cycle;
                    memoryCount = ENCM->using_memory;
                    encodedDataSize = ENCM->one_obj_size;
                    deltaDataSize = ENCM->two_obj_size;
                    
                    startTimeEnc = false;
                }

                // Encode Methrics
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 370, 872, 24}, "Encode Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 400+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 300, 400+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 430+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 460+2, 20, LIGHTGRAY);
                // Encode Results
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 490, 872, 24}, "Results");
                DrawText(TextFormat("Encoded Data Size: %i bytes", encodedDataSize), 68, 520, 20, LIGHTGRAY);
                DrawText(TextFormat("Delta Data Size: %i bytes", deltaDataSize), 470, 520, 20, LIGHTGRAY);

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
                
                if (GuiButton((Rectangle){68, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear Results")))
                {
                    free(DECM);
                    timeCount = 0;
                    cycleCount = 0;
                    memoryCount = 0;
                    publicSize = 0;
                    deltaDataSize = 0;
                    decodedDataSize = 0;
                    keySize = 0;
                    DecodeProgressValue = 0.0f;
                    // TextCopy(textKEY_d, "Decaps Key: ");
                    // clear_methrics();
                }
                if (GuiButton((Rectangle){280, 160, 208, 24}, GuiIconText(ICON_PLAYER, "Create User Name"))) WindowUserNameBox = true;
                if (GuiButton((Rectangle){500, 160, 200, 24}, GuiIconText(ICON_FILE_DELETE, "Clear User Name"))) strncpy(userName, "\0", 512);
                
                DrawText(TextFormat("User: %s\n", userName), 710, 160, 20, LIGHTGRAY);
                
                if (GuiButton((Rectangle){68, 190, 300, 24}, GuiIconText(ICON_KEY, "Choice Private Key"))) keyDialogState.windowActive = true;
                DrawText(TextFormat("Key: %s", keyDialogState.fileNameText), 380, 190+2, 20, LIGHTGRAY);

                if (keySize % 1024 >= 1024) DrawText(TextFormat("Key Size: %4.2f MB", (double)((double)keySize / 1024 / 1024)), 650, 190+2, 20, LIGHTGRAY);
                else if (keySize % 1024 >= 100) DrawText(TextFormat("Key Size: %4.2f KB", (double)((double)keySize / 1024)), 650, 190+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Key Size: %i B", keySize), 650, 190+2, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){860, 190, 80, 24}, GuiIconText(ICON_KEY, "Close")))
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
                if (dataSize % 1024 >= 1024) DrawText(TextFormat("Data Size: %4.2f MB", (double)((double)dataSize / 1024 / 1024)), 650, 220+2, 20, LIGHTGRAY);
                else if (dataSize % 1024 >= 100) DrawText(TextFormat("Data Size: %4.2f KB", (double)((double)dataSize / 1024)), 650, 220+2, 20, LIGHTGRAY);
                else DrawText(TextFormat("Data Size: %i B", dataSize), 650, 220+2, 20, LIGHTGRAY);
                if (GuiButton((Rectangle){860, 220, 80, 24}, GuiIconText(ICON_FILE_DELETE, "Close"))) 
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

                GuiTextBox((Rectangle){68, 250, 872, 24}, textKEY_d, 1024, TextBoxKEYEncaps);

                // Decode Data
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 280, 872, 24}, "Decode Data");
                if (WindowUserNameBox || fileDialogState.windowActive || keyDialogState.windowActive) GuiSetState(STATE_DISABLED);
                else GuiSetState(STATE_FOCUSED);
                if (GuiButton((Rectangle){68, 310, 872, 50}, GuiIconText(ICON_SHIELD, "Decapsulation Key")))
                {
                    get_security_level(keyNameToLoad, &securityLevel);
                    create_space(securityLevel);

                    setCFDec();
                    create_methrics();

                    if(securityLevel == 1)
                    {
                        load(keyNameToLoad, 2, &securityLevel);
                        load(fileNameToLoad, 3, &securityLevel);
                        run_mce_348864(3);
                    }
                    if (securityLevel == 2)
                    {
                        load(keyNameToLoad, 2, &securityLevel);
                        load(fileNameToLoad, 3, &securityLevel);
                        run_mce_460896(3);
                    }
                    if (securityLevel == 3)
                    {
                        load(keyNameToLoad, 2, &securityLevel);
                        load(fileNameToLoad, 3, &securityLevel);
                        run_mce_6688128(3);
                    }
                    methrics_decaps();
                    cpy_methrics(DECM);
                    get_text(textKEY_d, 2);
                    startTimeDec = true;
                }
                GuiSetState(STATE_NORMAL);
                if ((DecodeProgressValue < 100.0f) && startTimeDec) DecodeProgressValue += 1.0;
                else if (startTimeDec)
                {
                    timeCount = trunc(DECM->time*1000);
                    cycleCount = DECM->cycle;
                    memoryCount = DECM->using_memory;
                    decodedDataSize = DECM->one_obj_size;
                    deltaDataSize = DECM->two_obj_size;
                    
                    startTimeDec = false;
                }

                GuiProgressBar((Rectangle){ 68, 370, 872, 24 }, DecodeProgressText, NULL, &DecodeProgressValue, 0, 100);

                // Decode Methrics
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 400, 872, 24}, "Decode Methrics");
                DrawText(TextFormat("Time: %i ms", timeCount), 68, 430+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Cycles: %i", cycleCount), 400, 430+2, 20, LIGHTGRAY);

                DrawText(TextFormat("Allocated memory: %i bytes", usingProcessMemory), 68, 460, 20, LIGHTGRAY);
                DrawText(TextFormat("Using Memory: %i bytes", memoryCount), 68, 490+2, 20, LIGHTGRAY);
                
                // Deccode Results
                //--------------------------------------------------------------------------------
                GuiLine((Rectangle){68, 520, 872, 24}, "Results");
                DrawText(TextFormat("Decoded Data Size: %i bytes", decodedDataSize), 68, 550+2, 20, LIGHTGRAY);
                DrawText(TextFormat("Delta Data Size: %i bytes", deltaDataSize), 68, 580+2, 20, LIGHTGRAY);

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

                textSize = MeasureTextEx(GetFontDefault(), TextFormat("Nedomolkin M.P. Bolshakova A.A. Shapovalov I.V."), 20, 2);
                textX = generalBorderBoxRect.x + (generalBorderBoxRect.width - textSize.x) / 2;
                textY = textY + textSize.y + 20;
                DrawText(TextFormat("Nedomolkin M.P. Bolshakova A.A. Shapovalov I.V."), textX, textY, 20, PURPLE);
            }
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (dataContent) free(dataContent);
    cleanup_heap();
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

    }
    if (winFlag == enc)
    {
        WindowAddTextBox = false;
        dataSize = strlen(textBoxText);
        dataContent = (char *)malloc(dataSize);
        strncpy(dataContent, textBoxText, dataSize);
    }
    if (winFlag == dec)
    {

    }

}
static void Button004()
{
    if (winFlag == key)
    {

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

static void setCFEnc(void)
{
    ENCM = malloc(sizeof(methrics));
    ENCM->cycle = 0;
    ENCM->time = 0;
    ENCM->using_memory = 0;
    ENCM->one_obj_size = 0;
    ENCM->two_obj_size = 0;
}

static void setCFDec(void)
{
    DECM = malloc(sizeof(methrics));
    DECM->cycle = 0;
    DECM->time = 0;
    DECM->using_memory = 0;
    DECM->one_obj_size = 0;
    DECM->two_obj_size = 0;
}

static void setCFKgn(void)
{
    KGM = malloc(sizeof(methrics));
    KGM->cycle = 0;
    KGM->time = 0;
    KGM->using_memory = 0;
    KGM->one_obj_size = 0;
    KGM->two_obj_size = 0;
}

