#include "RetroEngine.hpp"
#include <cmath>

#if RETRO_PLATFORM == RETRO_WIN
#include <windows.h>
#endif

ObjectScript objectScriptList[OBJECT_COUNT];
ScriptPtr functionScriptList[FUNCTION_COUNT];

int scriptData[SCRIPTDATA_COUNT];
int jumpTableData[JUMPTABLE_COUNT];
int jumpTableStack[JUMPSTACK_COUNT];
int functionStack[FUNCSTACK_COUNT];
int foreachStack[FORSTACK_COUNT];

int scriptCodePos     = 0;
int jumpTablePos      = 0;
int jumpTableStackPos = 0;
int functionStackPos  = 0;
int foreachStackPos   = 0;

ScriptEngine scriptEng = ScriptEngine();
char scriptText[0x4000];

int scriptDataPos       = 0;
int scriptDataOffset    = 0;
int jumpTableDataPos    = 0;
int jumpTableDataOffset = 0;

#if RETRO_USE_COMPILER
#if !RETRO_REV00
#define COMMONALIAS_COUNT (0x75)
#else
#define COMMONALIAS_COUNT (0x74)
#endif
#define ALIAS_COUNT_TRIM (0xE0)
#define ALIAS_COUNT      (COMMONALIAS_COUNT + ALIAS_COUNT_TRIM + 0xFFF)
int lineID = 0;

struct AliasInfo {
    AliasInfo()
    {
        StrCopy(name, "");
        StrCopy(value, "");
    }
    AliasInfo(const char *aliasName, const char *aliasVal)
    {
        StrCopy(name, aliasName);
        StrCopy(value, aliasVal);
    }

    char name[0x20];
    char value[0x20];
};

#define STATICVAR_COUNT (0x200)
#endif

struct FunctionInfo {
    FunctionInfo()
    {
        StrCopy(name, "");
        opcodeSize = 0;
    }
    FunctionInfo(const char *functionName, int opSize)
    {
        StrCopy(name, functionName);
        opcodeSize = opSize;
    }

    char name[0x20];
    int opcodeSize;
};

#if RETRO_USE_COMPILER
const char variableNames[][0x20] = {
    // Internal Script Values
    "temp0",
    "temp1",
    "temp2",
    "temp3",
    "temp4",
    "temp5",
    "temp6",
    "temp7",
    "temp8",
    "temp9",
    "temp10",
    "checkResult",
    "arrayPos0",
    "arrayPos1",
    "arrayPos2",
    "arrayPos3",
    "arrayPos4",
    "arrayPos5",
    "arrayPos6",
    "arrayPos7",
    "global",
    "local",

    // Object Properties
    "object.entityPos",
    "object.groupID",
    "object.type",
    "object.propertyValue",
    "object.xpos",
    "object.ypos",
    "object.ixpos",
    "object.iypos",
    "object.xvel",
    "object.yvel",
    "object.speed",
    "object.state",
    "object.rotation",
    "object.scale",
    "object.priority",
    "object.drawOrder",
    "object.direction",
    "object.inkEffect",
    "object.alpha",
    "object.frame",
    "object.animation",
    "object.prevAnimation",
    "object.animationSpeed",
    "object.animationTimer",
    "object.angle",
    "object.lookPosX",
    "object.lookPosY",
    "object.collisionMode",
    "object.collisionPlane",
    "object.controlMode",
    "object.controlLock",
    "object.pushing",
    "object.visible",
    "object.tileCollisions",
    "object.interaction",
    "object.gravity",
    "object.up",
    "object.down",
    "object.left",
    "object.right",
    "object.jumpPress",
    "object.jumpHold",
    "object.scrollTracking",
    "object.floorSensorL",
    "object.floorSensorC",
    "object.floorSensorR",
#if !RETRO_REV00
    "object.floorSensorLC",
    "object.floorSensorRC",
#endif
    "object.collisionLeft",
    "object.collisionTop",
    "object.collisionRight",
    "object.collisionBottom",
    "object.outOfBounds",
    "object.spriteSheet",

    // Object Values
    "object.value0",
    "object.value1",
    "object.value2",
    "object.value3",
    "object.value4",
    "object.value5",
    "object.value6",
    "object.value7",
    "object.value8",
    "object.value9",
    "object.value10",
    "object.value11",
    "object.value12",
    "object.value13",
    "object.value14",
    "object.value15",
    "object.value16",
    "object.value17",
    "object.value18",
    "object.value19",
    "object.value20",
    "object.value21",
    "object.value22",
    "object.value23",
    "object.value24",
    "object.value25",
    "object.value26",
    "object.value27",
    "object.value28",
    "object.value29",
    "object.value30",
    "object.value31",
    "object.value32",
    "object.value33",
    "object.value34",
    "object.value35",
    "object.value36",
    "object.value37",
    "object.value38",
    "object.value39",
    "object.value40",
    "object.value41",
    "object.value42",
    "object.value43",
    "object.value44",
    "object.value45",
    "object.value46",
    "object.value47",

    // Stage Properties
    "stage.state",
    "stage.activeList",
    "stage.listPos",
    "stage.timeEnabled",
    "stage.milliSeconds",
    "stage.seconds",
    "stage.minutes",
    "stage.actNum",
    "stage.pauseEnabled",
    "stage.listSize",
    "stage.newXBoundary1",
    "stage.newXBoundary2",
    "stage.newYBoundary1",
    "stage.newYBoundary2",
    "stage.curXBoundary1",
    "stage.curXBoundary2",
    "stage.curYBoundary1",
    "stage.curYBoundary2",
    "stage.deformationData0",
    "stage.deformationData1",
    "stage.deformationData2",
    "stage.deformationData3",
    "stage.waterLevel",
    "stage.activeLayer",
    "stage.midPoint",
    "stage.playerListPos",
    "stage.debugMode",
    "stage.entityPos",

    // Screen Properties
    "screen.cameraEnabled",
    "screen.cameraTarget",
    "screen.cameraStyle",
    "screen.cameraX",
    "screen.cameraY",
    "screen.drawListSize",
    "screen.xcenter",
    "screen.ycenter",
    "screen.xsize",
    "screen.ysize",
    "screen.xoffset",
    "screen.yoffset",
    "screen.shakeX",
    "screen.shakeY",
    "screen.adjustCameraY",

    "touchscreen.down",
    "touchscreen.xpos",
    "touchscreen.ypos",

    // Sound Properties
    "music.volume",
    "music.currentTrack",
    "music.position",

    // Input Properties
    "inputDown.up",
    "inputDown.down",
    "inputDown.left",
    "inputDown.right",
    "inputDown.buttonA",
    "inputDown.buttonB",
    "inputDown.buttonC",
    "inputDown.buttonX",
    "inputDown.buttonY",
    "inputDown.buttonZ",
    "inputDown.buttonL",
    "inputDown.buttonR",
    "inputDown.start",
    "inputDown.select",
    "inputPress.up",
    "inputPress.down",
    "inputPress.left",
    "inputPress.right",
    "inputPress.buttonA",
    "inputPress.buttonB",
    "inputPress.buttonC",
    "inputPress.buttonX",
    "inputPress.buttonY",
    "inputPress.buttonZ",
    "inputPress.buttonL",
    "inputPress.buttonR",
    "inputPress.start",
    "inputPress.select",

    // Menu Properties
    "menu1.selection",
    "menu2.selection",

    // Tile Layer Properties
    "tileLayer.xsize",
    "tileLayer.ysize",
    "tileLayer.type",
    "tileLayer.angle",
    "tileLayer.xpos",
    "tileLayer.ypos",
    "tileLayer.zpos",
    "tileLayer.parallaxFactor",
    "tileLayer.scrollSpeed",
    "tileLayer.scrollPos",
    "tileLayer.deformationOffset",
    "tileLayer.deformationOffsetW",
    "hParallax.parallaxFactor",
    "hParallax.scrollSpeed",
    "hParallax.scrollPos",
    "vParallax.parallaxFactor",
    "vParallax.scrollSpeed",
    "vParallax.scrollPos",

    // 3D Scene Properties
    "scene3D.vertexCount",
    "scene3D.faceCount",
    "scene3D.projectionX",
    "scene3D.projectionY",
#if !RETRO_REV00
    "scene3D.fogColor",
    "scene3D.fogStrength",
#endif

    "vertexBuffer.x",
    "vertexBuffer.y",
    "vertexBuffer.z",
    "vertexBuffer.u",
    "vertexBuffer.v",

    "faceBuffer.a",
    "faceBuffer.b",
    "faceBuffer.c",
    "faceBuffer.d",
    "faceBuffer.flag",
    "faceBuffer.color",

    "saveRAM",
    "engine.state",
#if RETRO_REV00
    "engine.message",
#endif
    "engine.language",
    "engine.onlineActive",
    "engine.sfxVolume",
    "engine.bgmVolume",
    "engine.platformID",
    "engine.trialMode",
    "engine.deviceType",
#if RETRO_USE_HAPTICS
    "engine.hapticsEnabled",
#endif

// Custom
    "engine.timer",
    "ERM_WHAT_THE_SIGMA",
    "object.value48",
    "object.value49",
    "object.value50",
    "object.value51",
    "object.width",
    "object.height",
    "object.inkred",
    "object.inkgreen",
    "object.inkblue",
    "menu3.selection",
    "current.unix",
    "local.unix",
};
#endif

const FunctionInfo functions[] = {
    FunctionInfo("End", 0),      // End of Script
    FunctionInfo("Equal", 2),    // Equal
    FunctionInfo("Add", 2),      // Add
    FunctionInfo("Sub", 2),      // Subtract
    FunctionInfo("Inc", 1),      // Increment
    FunctionInfo("Dec", 1),      // Decrement
    FunctionInfo("Mul", 2),      // Multiply
    FunctionInfo("Div", 2),      // Divide
    FunctionInfo("ShR", 2),      // Bit Shift Right
    FunctionInfo("ShL", 2),      // Bit Shift Left
    FunctionInfo("And", 2),      // Bitwise And
    FunctionInfo("Or", 2),       // Bitwise Or
    FunctionInfo("Xor", 2),      // Bitwise Xor
    FunctionInfo("Mod", 2),      // Mod
    FunctionInfo("FlipSign", 1), // Flips the Sign of the value

    FunctionInfo("CheckEqual", 2),    // compare a=b, return result in CheckResult Variable
    FunctionInfo("CheckGreater", 2),  // compare a>b, return result in CheckResult Variable
    FunctionInfo("CheckLower", 2),    // compare a<b, return result in CheckResult Variable
    FunctionInfo("CheckNotEqual", 2), // compare a!=b, return result in CheckResult Variable

    FunctionInfo("IfEqual", 3),          // compare a=b, jump if condition met
    FunctionInfo("IfGreater", 3),        // compare a>b, jump if condition met
    FunctionInfo("IfGreaterOrEqual", 3), // compare a>=b, jump if condition met
    FunctionInfo("IfLower", 3),          // compare a<b, jump if condition met
    FunctionInfo("IfLowerOrEqual", 3),   // compare a<=b, jump if condition met
    FunctionInfo("IfNotEqual", 3),       // compare a!=b, jump if condition met
    FunctionInfo("else", 0),             // The else for an if statement
    FunctionInfo("endif", 0),            // The end if

    FunctionInfo("WEqual", 3),          // compare a=b, loop if condition met
    FunctionInfo("WGreater", 3),        // compare a>b, loop if condition met
    FunctionInfo("WGreaterOrEqual", 3), // compare a>=b, loop if condition met
    FunctionInfo("WLower", 3),          // compare a<b, loop if condition met
    FunctionInfo("WLowerOrEqual", 3),   // compare a<=b, loop if condition met
    FunctionInfo("WNotEqual", 3),       // compare a!=b, loop if condition met
    FunctionInfo("loop", 0),            // While Loop marker

    FunctionInfo("ForEachActive", 3), // foreach loop, iterates through object group lists only if they are active and interaction is true
    FunctionInfo("ForEachAll", 3),    // foreach loop, iterates through objects matching type
    FunctionInfo("next", 0),          // foreach loop, next marker

    FunctionInfo("switch", 2),    // Switch Statement
    FunctionInfo("break", 0),     // break
    FunctionInfo("endswitch", 0), // endswitch

    // Math Functions
    FunctionInfo("Rand", 2),
    FunctionInfo("Sin", 2),
    FunctionInfo("Cos", 2),
    FunctionInfo("Sin256", 2),
    FunctionInfo("Cos256", 2),
    FunctionInfo("ATan2", 3),
    FunctionInfo("Interpolate", 4),
    FunctionInfo("InterpolateXY", 7),

    // Graphics Functions
    FunctionInfo("LoadSpriteSheet", 1),
    FunctionInfo("RemoveSpriteSheet", 1),
    FunctionInfo("DrawSprite", 1),
    FunctionInfo("DrawSpriteXY", 3),
    FunctionInfo("DrawSpriteScreenXY", 3),
    FunctionInfo("DrawTintRect", 4),
    FunctionInfo("DrawNumbers", 7),
    FunctionInfo("DrawActName", 7),
    FunctionInfo("DrawMenu", 3),
    FunctionInfo("SpriteFrame", 6),
    FunctionInfo("EditFrame", 7),
    FunctionInfo("LoadPalette", 5),
    FunctionInfo("RotatePalette", 4),
    FunctionInfo("SetScreenFade", 4),
    FunctionInfo("SetClassicFade", 4),
    FunctionInfo("SetActivePalette", 3),
    FunctionInfo("SetPaletteFade", RETRO_REV00 ? 7 : 6),
    FunctionInfo("SetPaletteEntry", 3),
    FunctionInfo("GetPaletteEntry", 3),
    FunctionInfo("CopyPalette", 5),
    FunctionInfo("ClearScreen", 1),
    FunctionInfo("DrawSpriteFX", 4),
    FunctionInfo("DrawSpriteScreenFX", 4),

    // More Useful Stuff
    FunctionInfo("LoadAnimation", 1),
    FunctionInfo("SetupMenu", 4),
    FunctionInfo("AddMenuEntry", 3),
    FunctionInfo("EditMenuEntry", 4),
    FunctionInfo("LoadStage", 0),
    FunctionInfo("DrawRect", 8),
    FunctionInfo("ClassicTint", 8),
    FunctionInfo("ResetObjectEntity", 5),
    FunctionInfo("BoxCollisionTest", 11),
    FunctionInfo("CreateTempObject", 4),

    // Player and Animation Functions
    FunctionInfo("ProcessObjectMovement", 0),
    FunctionInfo("ProcessObjectControl", 0),
    FunctionInfo("ProcessAnimation", 0),
    FunctionInfo("DrawObjectAnimation", 0),

    // Music
    FunctionInfo("SetMusicTrack", 3),
    FunctionInfo("PlayMusic", 1),
    FunctionInfo("StopMusic", 0),
    FunctionInfo("PauseMusic", 0),
    FunctionInfo("ResumeMusic", 0),
    FunctionInfo("SwapMusicTrack", 4),

    // Sound FX
    FunctionInfo("PlaySfx", 2),
    FunctionInfo("StopSfx", 1),
    FunctionInfo("SetSfxAttributes", 3),

    // More Collision Stuff
    FunctionInfo("ObjectTileCollision", 4),
    FunctionInfo("ObjectTileGrip", 4),

    // Video
    FunctionInfo("LoadVideo", 1),
    FunctionInfo("NextVideoFrame", 0),

    // Bitwise Not
    FunctionInfo("Not", 1),

    // 3D Stuff
    FunctionInfo("Draw3DScene", 0),
    FunctionInfo("SetIdentityMatrix", 1),
    FunctionInfo("MatrixMultiply", 2),
    FunctionInfo("MatrixTranslateXYZ", 4),
    FunctionInfo("MatrixScaleXYZ", 4),
    FunctionInfo("MatrixRotateX", 2),
    FunctionInfo("MatrixRotateY", 2),
    FunctionInfo("MatrixRotateZ", 2),
    FunctionInfo("MatrixRotateXYZ", 4),
#if !RETRO_REV00
    FunctionInfo("MatrixInverse", 1),
#endif
    FunctionInfo("TransformVertices", 3),

    FunctionInfo("CallFunction", 1),
    FunctionInfo("return", 0),

    FunctionInfo("SetLayerDeformation", 6),
    FunctionInfo("CheckTouchRect", 4),
    FunctionInfo("GetTileLayerEntry", 4),
    FunctionInfo("SetTileLayerEntry", 4),

    FunctionInfo("GetBit", 3),
    FunctionInfo("SetBit", 3),

    FunctionInfo("ClearDrawList", 1),
    FunctionInfo("AddDrawListEntityRef", 2),
    FunctionInfo("GetDrawListEntityRef", 3),
    FunctionInfo("SetDrawListEntityRef", 3),

    FunctionInfo("Get16x16TileInfo", 4),
    FunctionInfo("Set16x16TileInfo", 4),
    FunctionInfo("Copy16x16Tile", 2),
    FunctionInfo("GetAnimationByName", 2),
    FunctionInfo("ReadSaveRAM", 0),
    FunctionInfo("WriteSaveRAM", 0),

#if RETRO_REV00 || RETRO_REV01
    FunctionInfo("LoadFontFile", 1),
#endif
    FunctionInfo("LoadTextFile", RETRO_REV02 ? 2 : 3),
    FunctionInfo("GetTextInfo", 5),
#if RETRO_REV00 || RETRO_REV01
    FunctionInfo("DrawText", 7),
#endif
    FunctionInfo("GetVersionNumber", 2),

    FunctionInfo("GetTableValue", 3),
    FunctionInfo("SetTableValue", 3),

    FunctionInfo("CheckCurrentStageFolder", 1),
    FunctionInfo("Abs", 1),

    FunctionInfo("CallNativeFunction", 1),
    FunctionInfo("CallNativeFunction2", 3),
    FunctionInfo("CallNativeFunction4", 5),

    FunctionInfo("SetObjectRange", 1),
#if !RETRO_REV00 || !RETRO_REV01
    FunctionInfo("GetObjectValue", 3),
    FunctionInfo("SetObjectValue", 3),
    FunctionInfo("CopyObject", 3),
#endif
    FunctionInfo("Print", 3),
    FunctionInfo("CalculateObjectRotation", 0),
    FunctionInfo("LoadWebsite", 1),
    FunctionInfo("ProcessFlippedObjectControl", 0)
};

#if RETRO_USE_COMPILER
AliasInfo publicAliases[ALIAS_COUNT] = {
    AliasInfo("true", "1"),
    AliasInfo("false", "0"),
    AliasInfo("FX_FLIP", "1"),
    AliasInfo("FX_ROTATE", "2"),
    AliasInfo("FX_SCALE", "4"),
    AliasInfo("FX_ROTOZOOM", "6"),
    AliasInfo("FX_INK", "8"),
    AliasInfo("PRESENTATION_STAGE", "0"),
    AliasInfo("REGULAR_STAGE", "1"),
    AliasInfo("BONUS_STAGE", "2"),
    AliasInfo("SPECIAL_STAGE", "3"),
    AliasInfo("MENU_1", "0"),
    AliasInfo("MENU_2", "1"),
    AliasInfo("MENU_3", "2"),
    AliasInfo("MENU_4", "3"),
    AliasInfo("C_TOUCH", "0"),
    AliasInfo("C_BOX", "1"),
    AliasInfo("C_BOX2", "2"),
    AliasInfo("C_PLATFORM", "3"),
    AliasInfo("MAT_WORLD", "0"),
    AliasInfo("MAT_VIEW", "1"),
    AliasInfo("MAT_TEMP", "2"),
    AliasInfo("FACING_LEFT", "1"),
    AliasInfo("FACING_RIGHT", "0"),
    AliasInfo("FLIP_NONE", "0"),
    AliasInfo("FLIP_X", "1"),
    AliasInfo("FLIP_Y", "2"),
    AliasInfo("FLIP_XY", "3"),
    AliasInfo("STAGE_RUNNING", "1"),
    AliasInfo("STAGE_PAUSED", "2"),
    AliasInfo("STAGE_FROZEN", "3"),
#if !RETRO_REV00
    AliasInfo("STAGE_2P", "4"),
#endif
    AliasInfo("ENGINE_DEVMENU", "0"),
    AliasInfo("ENGINE_MAINGAME", "1"),
    AliasInfo("ENGINE_INITDEVMENU", "2"),
    AliasInfo("ENGINE_WAIT", "3"),
    AliasInfo("ENGINE_SCRIPTERROR", "4"),
    AliasInfo("ENGINE_INITPAUSE", "5"),
    AliasInfo("ENGINE_EXITPAUSE", "6"),
    AliasInfo("ENGINE_ENDGAME", "7"),
    AliasInfo("ENGINE_RESETGAME", "8"),
    AliasInfo("RESET_GAME", "2"),
    AliasInfo("RETRO_STANDARD", "0"),
    AliasInfo("RETRO_MOBILE", "1"),
    AliasInfo("INK_NONE", "0"),
    AliasInfo("INK_BLEND", "1"),
    AliasInfo("INK_ALPHA", "2"),
    AliasInfo("INK_ADD", "3"),
    AliasInfo("INK_SUB", "4"),
    AliasInfo("INK_PINKIFIED", "5"),
    AliasInfo("INK_BLANK", "6"),
    AliasInfo("INK_CUSTOM", "7"),
    AliasInfo("CSIDE_FLOOR", "0"),
    AliasInfo("CSIDE_LWALL", "1"),
    AliasInfo("CSIDE_RWALL", "2"),
    AliasInfo("CSIDE_ROOF", "3"),
    AliasInfo("CMODE_FLOOR", "0"),
    AliasInfo("CMODE_LWALL", "1"),
    AliasInfo("CMODE_ROOF", "2"),
    AliasInfo("CMODE_RWALL", "3"),
    AliasInfo("COL_NONE", "0"),
    AliasInfo("COL_TOP", "1"),
    AliasInfo("COL_LEFT", "2"),
    AliasInfo("COL_RIGHT", "3"),
    AliasInfo("COL_BOTTOM", "4"),
    AliasInfo("PATH_A", "0"),
    AliasInfo("PATH_B", "1"),
    AliasInfo("GRAVITY_GROUND", "0"),
    AliasInfo("GRAVITY_AIR", "1"),
    AliasInfo("FACE_TEXTURED_3D", "0"),
    AliasInfo("FACE_TEXTURED_2D", "1"),
    AliasInfo("FACE_COLOURED_3D", "2"),
    AliasInfo("FACE_COLOURED_2D", "3"),
    AliasInfo("FACE_FADED", "4"),
    AliasInfo("FACE_TEXTURED_C", "5"),
    AliasInfo("FACE_TEXTURED_C_BLEND", "6"),
    AliasInfo("FACE_TEXTURED_D", "6"), // identical to "FACE_TEXTURED_C_BLEND", but kept here for backwards compat purposes
    AliasInfo("FACE_SPRITE_3D", "7"),
    AliasInfo("PRIORITY_ACTIVE_BOUNDS", "0"),
    AliasInfo("PRIORITY_ACTIVE", "1"),
    AliasInfo("PRIORITY_ACTIVE_PAUSED", "2"),
    AliasInfo("PRIORITY_XBOUNDS", "3"),
    AliasInfo("PRIORITY_XBOUNDS_DESTROY", "4"),
    AliasInfo("PRIORITY_INACTIVE", "5"),
    AliasInfo("PRIORITY_BOUNDS_SMALL", "6"),
    AliasInfo("PRIORITY_ACTIVE_SMALL", "7"),
    AliasInfo("PRIORITY_UNKNOWN", "7"), // identical to "PRIORITY_ACTIVE_SMALL", but kept here for backwards compat purposes
    AliasInfo("CONTROLMODE_NONE", "-1"),
    AliasInfo("CONTROLMODE_NORMAL", "0"),
    AliasInfo("CAMERASTYLE_FOLLOW", "0"),
    AliasInfo("CAMERASTYLE_EXTENDED", "1"),
    AliasInfo("CAMERASTYLE_EXTENDED_OFFSET_L", "2"),
    AliasInfo("CAMERASTYLE_EXTENDED_OFFSET_R", "3"),
    AliasInfo("CAMERASTYLE_HLOCKED", "4"),
    AliasInfo("TILEINFO_INDEX", "0"),
    AliasInfo("TILEINFO_DIRECTION", "1"),
    AliasInfo("TILEINFO_VISUALPLANE", "2"),
    AliasInfo("TILEINFO_SOLIDITYA", "3"),
    AliasInfo("TILEINFO_SOLIDITYB", "4"),
    AliasInfo("TILEINFO_FLAGSA", "5"),
    AliasInfo("TILEINFO_ANGLEA", "6"),
    AliasInfo("TILEINFO_FLAGSB", "7"),
    AliasInfo("TILEINFO_ANGLEB", "8"),
    AliasInfo("TEXTINFO_TEXTDATA", "0"),
    AliasInfo("TEXTINFO_TEXTSIZE", "1"),
    AliasInfo("TEXTINFO_ROWCOUNT", "2"),
    AliasInfo("TILELAYER_NOSCROLL", "0"),
    AliasInfo("TILELAYER_HSCROLL", "1"),
    AliasInfo("TILELAYER_VSCROLL", "2"),
    AliasInfo("TILELAYER_3DFLOOR", "3"),
    AliasInfo("TILELAYER_3DSKY", "4"),
    AliasInfo("GROUP_ALL", "0"),
    // These are for Engine.PlatformID
    AliasInfo("RETRO_WIN", "0"),
    AliasInfo("RETRO_OSX", "1"),
    AliasInfo("RETRO_XBOX_360", "2"),
    AliasInfo("RETRO_PS3", "3"),
    AliasInfo("RETRO_iOS", "4"),
    AliasInfo("RETRO_ANDROID", "5"),
    AliasInfo("RETRO_WP7", "6"),
    AliasInfo("RETRO_UWP", "7"),
    AliasInfo("RETRO_LINUX", "8"),
    AliasInfo("RETRO_SWITCH", "9"),
    AliasInfo("SCREENFLIP_NONE", "0"),
    AliasInfo("SCREENFLIP_X", "1"),
    AliasInfo("SCREENFLIP_Y", "2"),
    AliasInfo("SCREENFLIP_XY", "3")

    // custom
};
AliasInfo privateAliases[ALIAS_COUNT_TRIM];
int publicAliasCount  = 0;
int privateAliasCount = 0;

StaticInfo publicStaticVariables[STATICVAR_COUNT];
StaticInfo privateStaticVariables[STATICVAR_COUNT];
int publicStaticVarCount  = 0;
int privateStaticVarCount = 0;

TableInfo *currentTable = NULL;
bool curTablePublic     = false;

TableInfo publicTables[TABLE_COUNT];
TableInfo privateTables[TABLE_COUNT];
int publicTableCount  = 0;
int privateTableCount = 0;

const char scriptEvaluationTokens[][0x4] = { "=",  "+=", "-=", "++", "--", "*=", "/=", ">>=", "<<=", "&=",
                                             "|=", "^=", "%=", "==", ">",  ">=", "<",  "<=",  "!=" };

int scriptFunctionCount = 0;
char scriptFunctionNames[FUNCTION_COUNT][0x40];

enum ScriptReadModes { READMODE_NORMAL = 0, READMODE_STRING = 1, READMODE_COMMENTLINE = 2, READMODE_ENDLINE = 3, READMODE_EOF = 4 };
enum ScriptParseModes {
    PARSEMODE_SCOPELESS    = 0,
    PARSEMODE_PLATFORMSKIP = 1,
    PARSEMODE_FUNCTION     = 2,
    PARSEMODE_SWITCHREAD   = 3,
    PARSEMODE_TABLEREAD    = 4,
    PARSEMODE_ERROR        = 0xFF
};
#endif

enum ScriptVarTypes { SCRIPTVAR_VAR = 1, SCRIPTVAR_INTCONST = 2, SCRIPTVAR_STRCONST = 3 };
enum ScriptVarArrTypes { VARARR_NONE = 0, VARARR_ARRAY = 1, VARARR_ENTNOPLUS1 = 2, VARARR_ENTNOMINUS1 = 3 };

enum ScrVar {
    VAR_TEMP0,
    VAR_TEMP1,
    VAR_TEMP2,
    VAR_TEMP3,
    VAR_TEMP4,
    VAR_TEMP5,
    VAR_TEMP6,
    VAR_TEMP7,
    VAR_TEMP8,
    VAR_TEMP9,
    VAR_TEMP10,
    VAR_CHECKRESULT,
    VAR_ARRAYPOS0,
    VAR_ARRAYPOS1,
    VAR_ARRAYPOS2,
    VAR_ARRAYPOS3,
    VAR_ARRAYPOS4,
    VAR_ARRAYPOS5,
    VAR_ARRAYPOS6,
    VAR_ARRAYPOS7,
    VAR_GLOBAL,
    VAR_LOCAL,
    VAR_OBJECTENTITYPOS,
    VAR_OBJECTGROUPID,
    VAR_OBJECTTYPE,
    VAR_OBJECTPROPERTYVALUE,
    VAR_OBJECTXPOS,
    VAR_OBJECTYPOS,
    VAR_OBJECTIXPOS,
    VAR_OBJECTIYPOS,
    VAR_OBJECTXVEL,
    VAR_OBJECTYVEL,
    VAR_OBJECTSPEED,
    VAR_OBJECTSTATE,
    VAR_OBJECTROTATION,
    VAR_OBJECTSCALE,
    VAR_OBJECTPRIORITY,
    VAR_OBJECTDRAWORDER,
    VAR_OBJECTDIRECTION,
    VAR_OBJECTINKEFFECT,
    VAR_OBJECTALPHA,
    VAR_OBJECTFRAME,
    VAR_OBJECTANIMATION,
    VAR_OBJECTPREVANIMATION,
    VAR_OBJECTANIMATIONSPEED,
    VAR_OBJECTANIMATIONTIMER,
    VAR_OBJECTANGLE,
    VAR_OBJECTLOOKPOSX,
    VAR_OBJECTLOOKPOSY,
    VAR_OBJECTCOLLISIONMODE,
    VAR_OBJECTCOLLISIONPLANE,
    VAR_OBJECTCONTROLMODE,
    VAR_OBJECTCONTROLLOCK,
    VAR_OBJECTPUSHING,
    VAR_OBJECTVISIBLE,
    VAR_OBJECTTILECOLLISIONS,
    VAR_OBJECTINTERACTION,
    VAR_OBJECTGRAVITY,
    VAR_OBJECTUP,
    VAR_OBJECTDOWN,
    VAR_OBJECTLEFT,
    VAR_OBJECTRIGHT,
    VAR_OBJECTJUMPPRESS,
    VAR_OBJECTJUMPHOLD,
    VAR_OBJECTSCROLLTRACKING,
    VAR_OBJECTFLOORSENSORL,
    VAR_OBJECTFLOORSENSORC,
    VAR_OBJECTFLOORSENSORR,
#if !RETRO_REV00
    VAR_OBJECTFLOORSENSORLC,
    VAR_OBJECTFLOORSENSORRC,
#endif
    VAR_OBJECTCOLLISIONLEFT,
    VAR_OBJECTCOLLISIONTOP,
    VAR_OBJECTCOLLISIONRIGHT,
    VAR_OBJECTCOLLISIONBOTTOM,
    VAR_OBJECTOUTOFBOUNDS,
    VAR_OBJECTSPRITESHEET,
    VAR_OBJECTVALUE0,
    VAR_OBJECTVALUE1,
    VAR_OBJECTVALUE2,
    VAR_OBJECTVALUE3,
    VAR_OBJECTVALUE4,
    VAR_OBJECTVALUE5,
    VAR_OBJECTVALUE6,
    VAR_OBJECTVALUE7,
    VAR_OBJECTVALUE8,
    VAR_OBJECTVALUE9,
    VAR_OBJECTVALUE10,
    VAR_OBJECTVALUE11,
    VAR_OBJECTVALUE12,
    VAR_OBJECTVALUE13,
    VAR_OBJECTVALUE14,
    VAR_OBJECTVALUE15,
    VAR_OBJECTVALUE16,
    VAR_OBJECTVALUE17,
    VAR_OBJECTVALUE18,
    VAR_OBJECTVALUE19,
    VAR_OBJECTVALUE20,
    VAR_OBJECTVALUE21,
    VAR_OBJECTVALUE22,
    VAR_OBJECTVALUE23,
    VAR_OBJECTVALUE24,
    VAR_OBJECTVALUE25,
    VAR_OBJECTVALUE26,
    VAR_OBJECTVALUE27,
    VAR_OBJECTVALUE28,
    VAR_OBJECTVALUE29,
    VAR_OBJECTVALUE30,
    VAR_OBJECTVALUE31,
    VAR_OBJECTVALUE32,
    VAR_OBJECTVALUE33,
    VAR_OBJECTVALUE34,
    VAR_OBJECTVALUE35,
    VAR_OBJECTVALUE36,
    VAR_OBJECTVALUE37,
    VAR_OBJECTVALUE38,
    VAR_OBJECTVALUE39,
    VAR_OBJECTVALUE40,
    VAR_OBJECTVALUE41,
    VAR_OBJECTVALUE42,
    VAR_OBJECTVALUE43,
    VAR_OBJECTVALUE44,
    VAR_OBJECTVALUE45,
    VAR_OBJECTVALUE46,
    VAR_OBJECTVALUE47,
    VAR_STAGESTATE,
    VAR_STAGEACTIVELIST,
    VAR_STAGELISTPOS,
    VAR_STAGETIMEENABLED,
    VAR_STAGEMILLISECONDS,
    VAR_STAGESECONDS,
    VAR_STAGEMINUTES,
    VAR_STAGEACTNUM,
    VAR_STAGEPAUSEENABLED,
    VAR_STAGELISTSIZE,
    VAR_STAGENEWXBOUNDARY1,
    VAR_STAGENEWXBOUNDARY2,
    VAR_STAGENEWYBOUNDARY1,
    VAR_STAGENEWYBOUNDARY2,
    VAR_STAGECURXBOUNDARY1,
    VAR_STAGECURXBOUNDARY2,
    VAR_STAGECURYBOUNDARY1,
    VAR_STAGECURYBOUNDARY2,
    VAR_STAGEDEFORMATIONDATA0,
    VAR_STAGEDEFORMATIONDATA1,
    VAR_STAGEDEFORMATIONDATA2,
    VAR_STAGEDEFORMATIONDATA3,
    VAR_STAGEWATERLEVEL,
    VAR_STAGEACTIVELAYER,
    VAR_STAGEMIDPOINT,
    VAR_STAGEPLAYERLISTPOS,
    VAR_STAGEDEBUGMODE,
    VAR_STAGEENTITYPOS,
    VAR_SCREENCAMERAENABLED,
    VAR_SCREENCAMERATARGET,
    VAR_SCREENCAMERASTYLE,
    VAR_SCREENCAMERAX,
    VAR_SCREENCAMERAY,
    VAR_SCREENDRAWLISTSIZE,
    VAR_SCREENXCENTER,
    VAR_SCREENYCENTER,
    VAR_SCREENXSIZE,
    VAR_SCREENYSIZE,
    VAR_SCREENXOFFSET,
    VAR_SCREENYOFFSET,
    VAR_SCREENSHAKEX,
    VAR_SCREENSHAKEY,
    VAR_SCREENADJUSTCAMERAY,
    VAR_TOUCHSCREENDOWN,
    VAR_TOUCHSCREENXPOS,
    VAR_TOUCHSCREENYPOS,
    VAR_MUSICVOLUME,
    VAR_MUSICCURRENTTRACK,
    VAR_MUSICPOSITION,
    VAR_INPUTDOWNUP,
    VAR_INPUTDOWNDOWN,
    VAR_INPUTDOWNLEFT,
    VAR_INPUTDOWNRIGHT,
    VAR_INPUTDOWNBUTTONA,
    VAR_INPUTDOWNBUTTONB,
    VAR_INPUTDOWNBUTTONC,
    VAR_INPUTDOWNBUTTONX,
    VAR_INPUTDOWNBUTTONY,
    VAR_INPUTDOWNBUTTONZ,
    VAR_INPUTDOWNBUTTONL,
    VAR_INPUTDOWNBUTTONR,
    VAR_INPUTDOWNSTART,
    VAR_INPUTDOWNSELECT,
    VAR_INPUTPRESSUP,
    VAR_INPUTPRESSDOWN,
    VAR_INPUTPRESSLEFT,
    VAR_INPUTPRESSRIGHT,
    VAR_INPUTPRESSBUTTONA,
    VAR_INPUTPRESSBUTTONB,
    VAR_INPUTPRESSBUTTONC,
    VAR_INPUTPRESSBUTTONX,
    VAR_INPUTPRESSBUTTONY,
    VAR_INPUTPRESSBUTTONZ,
    VAR_INPUTPRESSBUTTONL,
    VAR_INPUTPRESSBUTTONR,
    VAR_INPUTPRESSSTART,
    VAR_INPUTPRESSSELECT,
    VAR_MENU1SELECTION,
    VAR_MENU2SELECTION,
    VAR_TILELAYERXSIZE,
    VAR_TILELAYERYSIZE,
    VAR_TILELAYERTYPE,
    VAR_TILELAYERANGLE,
    VAR_TILELAYERXPOS,
    VAR_TILELAYERYPOS,
    VAR_TILELAYERZPOS,
    VAR_TILELAYERPARALLAXFACTOR,
    VAR_TILELAYERSCROLLSPEED,
    VAR_TILELAYERSCROLLPOS,
    VAR_TILELAYERDEFORMATIONOFFSET,
    VAR_TILELAYERDEFORMATIONOFFSETW,
    VAR_HPARALLAXPARALLAXFACTOR,
    VAR_HPARALLAXSCROLLSPEED,
    VAR_HPARALLAXSCROLLPOS,
    VAR_VPARALLAXPARALLAXFACTOR,
    VAR_VPARALLAXSCROLLSPEED,
    VAR_VPARALLAXSCROLLPOS,
    VAR_SCENE3DVERTEXCOUNT,
    VAR_SCENE3DFACECOUNT,
    VAR_SCENE3DPROJECTIONX,
    VAR_SCENE3DPROJECTIONY,
#if !RETRO_REV00
    VAR_SCENE3DFOGCOLOR,
    VAR_SCENE3DFOGSTRENGTH,
#endif
    VAR_VERTEXBUFFERX,
    VAR_VERTEXBUFFERY,
    VAR_VERTEXBUFFERZ,
    VAR_VERTEXBUFFERU,
    VAR_VERTEXBUFFERV,
    VAR_FACEBUFFERA,
    VAR_FACEBUFFERB,
    VAR_FACEBUFFERC,
    VAR_FACEBUFFERD,
    VAR_FACEBUFFERFLAG,
    VAR_FACEBUFFERCOLOR,
    VAR_SAVERAM,
    VAR_ENGINESTATE,
#if RETRO_REV00
    VAR_ENGINEMESSAGE,
#endif
    VAR_ENGINELANGUAGE,
    VAR_ENGINEONLINEACTIVE,
    VAR_ENGINESFXVOLUME,
    VAR_ENGINEBGMVOLUME,
    VAR_ENGINEPLATFORMID, // v3-style device type aka Windows/Mac/Android/etc
    VAR_ENGINETRIALMODE,
    VAR_ENGINEDEVICETYPE,
#if RETRO_USE_HAPTICS
    VAR_HAPTICSENABLED,
#endif
    VAR_ENGINETIMER,
    VAR_ERM_WHAT_THE_SIGMA,
    VAR_OBJECTVALUE48,
    VAR_OBJECTVALUE49,
    VAR_OBJECTVALUE50,
    VAR_OBJECTVALUE51,
    VAR_OBJECTWIDTH,
    VAR_OBJECTHEIGHT,
    VAR_OBJECTINKRED,
    VAR_OBJECTINKGREEN,
    VAR_OBJECTINKBLUE,
    VAR_MENU3SELECTION,
    VAR_CURRENTUNIX,
    VAR_CURRENTLOCALUNIX,
    VAR_MAX_CNT
};

enum ScrFunc {
    FUNC_END,
    FUNC_EQUAL,
    FUNC_ADD,
    FUNC_SUB,
    FUNC_INC,
    FUNC_DEC,
    FUNC_MUL,
    FUNC_DIV,
    FUNC_SHR,
    FUNC_SHL,
    FUNC_AND,
    FUNC_OR,
    FUNC_XOR,
    FUNC_MOD,
    FUNC_FLIPSIGN,
    FUNC_CHECKEQUAL,
    FUNC_CHECKGREATER,
    FUNC_CHECKLOWER,
    FUNC_CHECKNOTEQUAL,
    FUNC_IFEQUAL,
    FUNC_IFGREATER,
    FUNC_IFGREATEROREQUAL,
    FUNC_IFLOWER,
    FUNC_IFLOWEROREQUAL,
    FUNC_IFNOTEQUAL,
    FUNC_ELSE,
    FUNC_ENDIF,
    FUNC_WEQUAL,
    FUNC_WGREATER,
    FUNC_WGREATEROREQUAL,
    FUNC_WLOWER,
    FUNC_WLOWEROREQUAL,
    FUNC_WNOTEQUAL,
    FUNC_LOOP,
    FUNC_FOREACHACTIVE,
    FUNC_FOREACHALL,
    FUNC_NEXT,
    FUNC_SWITCH,
    FUNC_BREAK,
    FUNC_ENDSWITCH,
    FUNC_RAND,
    FUNC_SIN,
    FUNC_COS,
    FUNC_SIN256,
    FUNC_COS256,
    FUNC_ATAN2,
    FUNC_INTERPOLATE,
    FUNC_INTERPOLATEXY,
    FUNC_LOADSPRITESHEET,
    FUNC_REMOVESPRITESHEET,
    FUNC_DRAWSPRITE,
    FUNC_DRAWSPRITEXY,
    FUNC_DRAWSPRITESCREENXY,
    FUNC_DRAWTINTRECT,
    FUNC_DRAWNUMBERS,
    FUNC_DRAWACTNAME,
    FUNC_DRAWMENU,
    FUNC_SPRITEFRAME,
    FUNC_EDITFRAME,
    FUNC_LOADPALETTE,
    FUNC_ROTATEPALETTE,
    FUNC_SETSCREENFADE,
    FUNC_SETCLASSICFADE,
    FUNC_SETACTIVEPALETTE,
    FUNC_SETPALETTEFADE,
    FUNC_SETPALETTEENTRY,
    FUNC_GETPALETTEENTRY,
    FUNC_COPYPALETTE,
    FUNC_CLEARSCREEN,
    FUNC_DRAWSPRITEFX,
    FUNC_DRAWSPRITESCREENFX,
    FUNC_LOADANIMATION,
    FUNC_SETUPMENU,
    FUNC_ADDMENUENTRY,
    FUNC_EDITMENUENTRY,
    FUNC_LOADSTAGE,
    FUNC_DRAWRECT,
    FUNC_CLASSICTINT,
    FUNC_RESETOBJECTENTITY,
    FUNC_BOXCOLLISIONTEST,
    FUNC_CREATETEMPOBJECT,
    FUNC_PROCESSOBJECTMOVEMENT,
    FUNC_PROCESSOBJECTCONTROL,
    FUNC_PROCESSANIMATION,
    FUNC_DRAWOBJECTANIMATION,
    FUNC_SETMUSICTRACK,
    FUNC_PLAYMUSIC,
    FUNC_STOPMUSIC,
    FUNC_PAUSEMUSIC,
    FUNC_RESUMEMUSIC,
    FUNC_SWAPMUSICTRACK,
    FUNC_PLAYSFX,
    FUNC_STOPSFX,
    FUNC_SETSFXATTRIBUTES,
    FUNC_OBJECTTILECOLLISION,
    FUNC_OBJECTTILEGRIP,
    FUNC_LOADVIDEO,
    FUNC_NEXTVIDEOFRAME,
    FUNC_NOT,
    FUNC_DRAW3DSCENE,
    FUNC_SETIDENTITYMATRIX,
    FUNC_MATRIXMULTIPLY,
    FUNC_MATRIXTRANSLATEXYZ,
    FUNC_MATRIXSCALEXYZ,
    FUNC_MATRIXROTATEX,
    FUNC_MATRIXROTATEY,
    FUNC_MATRIXROTATEZ,
    FUNC_MATRIXROTATEXYZ,
#if !RETRO_REV00
    FUNC_MATRIXINVERSE,
#endif
    FUNC_TRANSFORMVERTICES,
    FUNC_CALLFUNCTION,
    FUNC_RETURN,
    FUNC_SETLAYERDEFORMATION,
    FUNC_CHECKTOUCHRECT,
    FUNC_GETTILELAYERENTRY,
    FUNC_SETTILELAYERENTRY,
    FUNC_GETBIT,
    FUNC_SETBIT,
    FUNC_CLEARDRAWLIST,
    FUNC_ADDDRAWLISTENTITYREF,
    FUNC_GETDRAWLISTENTITYREF,
    FUNC_SETDRAWLISTENTITYREF,
    FUNC_GET16X16TILEINFO,
    FUNC_SET16X16TILEINFO,
    FUNC_COPY16X16TILE,
    FUNC_GETANIMATIONBYNAME,
    FUNC_READSAVERAM,
    FUNC_WRITESAVERAM,
#if RETRO_REV00 || RETRO_REV01
    FUNC_LOADTEXTFONT,
#endif
    FUNC_LOADTEXTFILE,
    FUNC_GETTEXTINFO,
#if RETRO_REV00 || RETRO_REV01
    FUNC_DRAWTEXT,
#endif
    FUNC_GETVERSIONNUMBER,
    FUNC_GETTABLEVALUE,
    FUNC_SETTABLEVALUE,
    FUNC_CHECKCURRENTSTAGEFOLDER,
    FUNC_ABS,
    FUNC_CALLNATIVEFUNCTION,
    FUNC_CALLNATIVEFUNCTION2,
    FUNC_CALLNATIVEFUNCTION4,
    FUNC_SETOBJECTRANGE,
#if !RETRO_REV00 && !RETRO_REV01
    FUNC_GETOBJECTVALUE,
    FUNC_SETOBJECTVALUE,
    FUNC_COPYOBJECT,
#endif
    FUNC_PRINT,
    FUNC_CALCULATEOBJECTROTATION,
    FUNC_LOADWEBSITE,
    FUNC_PROCESSFLIPPEDOBJECTCONTROL,
    FUNC_MAX_CNT
};

#if RETRO_USE_COMPILER
void CheckAliasText(char *text)
{
    sizeof(publicTables);
    if (FindStringToken(text, "publicalias", 1) == 0) {
        
        if (publicAliasCount >= ALIAS_COUNT) {
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PUBLIC ALIASES");
            Engine.gameMode = ENGINE_SCRIPTERROR;									// public alias & we reached the cap
            return;
        }

        AliasInfo *a = &publicAliases[publicAliasCount];
        int *cnt     = &publicAliasCount;
        bool priv    = false;
        MEM_ZEROP(a);
        int textPos     = 11;
        int aliasStrPos = 0;
        int aliasMatch  = 0;

        
        while (aliasMatch < 2) {
            if (aliasMatch) {
                if (aliasMatch == 1) {
                    a->name[aliasStrPos] = text[textPos];
                    if (text[textPos]) {
                        aliasStrPos++;
                    }
                    else {
                        aliasStrPos = 0;
                        ++aliasMatch;
                    }
                }
            }
            else if (text[textPos] == ':') {
                a->value[aliasStrPos] = 0;
                aliasStrPos           = 0;
                aliasMatch            = 1;
            }
            else {
                a->value[aliasStrPos++] = text[textPos];
            }
            ++textPos;
        }

        for (int v = 0; v < *cnt; ++v) {
            if (StrComp(publicAliases[v].name, a->name)) {
                PrintLog("Warning: Public Alias %s has already been used!", a->name);
            }
        }

        ++*cnt;
    }
    else if (FindStringToken(text, "privatealias", 1) == 0) {
        
        if (privateAliasCount >= ALIAS_COUNT) {
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PRIVATE ALIASES");
            Engine.gameMode = ENGINE_SCRIPTERROR;									// public alias & we reached the cap
            return;
        }
        
        AliasInfo *a = &privateAliases[privateAliasCount];
        int *cnt     = &privateAliasCount;
        bool priv    = false;
        MEM_ZEROP(a);
        int textPos     = 12;
        int aliasStrPos = 0;
        int aliasMatch  = 0;
        

        while (aliasMatch < 2) {
            if (aliasMatch) {
                if (aliasMatch == 1) {
                    a->name[aliasStrPos] = text[textPos];
                    if (text[textPos]) {
                        aliasStrPos++;
                    }
                    else {
                        aliasStrPos = 0;
                        ++aliasMatch;
                    }
                }
            }
            else if (text[textPos] == ':') {
                a->value[aliasStrPos] = 0;
                aliasStrPos           = 0;
                aliasMatch            = 1;
            }
            else {
                a->value[aliasStrPos++] = text[textPos];
            }
            ++textPos;
        }

        for (int v = 0; v < *cnt; ++v) {
            if (StrComp(privateAliases[v].name, a->name)) {
                PrintLog("Warning: Private Alias %s has already been used!", a->name);
            }
        }

        ++*cnt;
    }
}
void CheckStaticText(char *text)
{
    if (FindStringToken(text, "publicvalue", 1) == 0) {
         if (publicStaticVarCount >= STATICVAR_COUNT) {
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PUBLIC VALUES");
            Engine.gameMode = ENGINE_SCRIPTERROR;
            return;
         }
        
        StaticInfo *var = &publicStaticVariables[publicStaticVarCount];
        int *cnt        = &publicStaticVarCount;
        MEM_ZEROP(var);
        int textPos      = 11;
        int staticStrPos = 0;
        int staticMatch  = 0;
        char strBuffer[0x10];	
        
        var->dataPos = -1;

        while (staticMatch < 2) {
            if (staticMatch == 1) {
                if (text[staticStrPos] != ';' && text[textPos]) {
                    strBuffer[staticStrPos++] = text[textPos];
                }
                else {
                    strBuffer[staticStrPos] = 0;

                    if (!ConvertStringToInteger(strBuffer, &var->value)) {
                        bool flag = false;

                        for (int a = 0; a < publicAliasCount && !flag; ++a) {
                            if (StrComp(publicAliases[a].name, strBuffer)) {
                                StrCopy(strBuffer, publicAliases[a].value);
                                break;
                            }
                        }

                        if (!ConvertStringToInteger(strBuffer, &var->value)) {
                            PrintLog("WARNING: unable to convert static var value \"%s\" to int, on line %d", strBuffer, lineID);
                        }
                    }

                    var->dataPos                = scriptDataPos;
                    scriptData[scriptDataPos++] = var->value;
                    staticStrPos                = 0;
                    ++staticMatch;
                }
            }
            else if (text[textPos] == '=') {
                var->name[staticStrPos] = 0;
                staticStrPos            = 0;
                staticMatch             = 1;
            }
            else if (!text[textPos]) {
                break;
            }
            else {
                var->name[staticStrPos++] = text[textPos];
            }
            ++textPos;
        }

        // no assigned value, default to 0
        if (var->dataPos == -1) {
            var->dataPos                = scriptDataPos;
            var->value                  = 0;
            scriptData[scriptDataPos++] = var->value;
        }

        for (int v = 0; v < *cnt; ++v) {
            if (StrComp(publicStaticVariables[v].name, var->name)) {
                PrintLog("Warning: Public Variable %s has already been used!", var->name);
            }
        }

        ++*cnt;
    }
    else if (FindStringToken(text, "privatevalue", 1) == 0) {
        if (privateStaticVarCount >= STATICVAR_COUNT) {
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PRIVATE VALUES");
            Engine.gameMode = ENGINE_SCRIPTERROR;
            return;
        }
        
        StaticInfo *var = &privateStaticVariables[privateStaticVarCount];
        int *cnt        = &privateStaticVarCount;
        MEM_ZEROP(var);
        int textPos      = 12;
        int staticStrPos = 0;
        int staticMatch  = 0;
        char strBuffer[0x10];	
        
        var->dataPos = -1;

        while (staticMatch < 2) {
            if (staticMatch == 1) {
                if (text[staticStrPos] != ';' && text[textPos]) {
                    strBuffer[staticStrPos++] = text[textPos];
                }
                else {
                    strBuffer[staticStrPos] = 0;

                    if (!ConvertStringToInteger(strBuffer, &var->value)) {
                        bool flag = false;
                        for (int a = 0; a < privateAliasCount; ++a) {
                            if (StrComp(privateAliases[a].name, strBuffer)) {
                                StrCopy(strBuffer, privateAliases[a].value);
                                flag = true;
                                break;
                            }
                        }

                        if (!ConvertStringToInteger(strBuffer, &var->value)) {
                            PrintLog("WARNING: unable to convert static var value \"%s\" to int, on line %d", strBuffer, lineID);
                        }
                    }

                    var->dataPos                = scriptDataPos;
                    scriptData[scriptDataPos++] = var->value;
                    staticStrPos                = 0;
                    ++staticMatch;
                }
            }
            else if (text[textPos] == '=') {
                var->name[staticStrPos] = 0;
                staticStrPos            = 0;
                staticMatch             = 1;
            }
            else if (!text[textPos]) {
                break;
            }
            else {
                var->name[staticStrPos++] = text[textPos];
            }
            ++textPos;
        }

        // no assigned value, default to 0
        if (var->dataPos == -1) {
            var->dataPos                = scriptDataPos;
            var->value                  = 0;
            scriptData[scriptDataPos++] = var->value;
        }

        for (int v = 0; v < *cnt; ++v) {
            if (StrComp(privateStaticVariables[v].name, var->name)) {
                PrintLog("Warning: Private Variable %s has already been used!", var->name);
            }
        }

        ++*cnt;
    }
}
TableInfo *CheckTableText(char *text)
{
    if (FindStringToken(text, "publictable", 1) && FindStringToken(text, "privatetable", 1))
        return NULL;
    
    if (FindStringToken(text, "publictable", 1) == 0) { // public table & we reached the cap
        if (publicTableCount >= TABLE_COUNT) {
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PUBLIC TABLES");
            Engine.gameMode = ENGINE_SCRIPTERROR;
            return NULL;
        }
    }

    TableInfo *table = &publicTables[publicTableCount];
    int strPos       = 11;
    curTablePublic   = true;
    if (FindStringToken(text, "privatetable", 1) == 0) {
        if (privateTableCount >= TABLE_COUNT) { // private table and we reached the cap
            SetupTextMenu(&gameMenu[0], 0);
            AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
            AddTextMenuEntry(&gameMenu[0], " ");
            AddTextMenuEntry(&gameMenu[0], "TOO MANY PRIVATE TABLES");
            Engine.gameMode = ENGINE_SCRIPTERROR;
            return NULL;
        }	
        table          = &privateTables[privateTableCount];
        strPos         = 12;
        curTablePublic = false;
    }
    MEM_ZEROP(table);

    int namePos = 0;
    while (text[strPos]) {
        table->name[namePos++] = text[strPos++];
    }
    table->name[namePos] = 0;
    return table;
}
void ConvertArithmaticSyntax(char *text)
{
    int token  = 0;
    int offset = 0;
    int findID = 0;
    char dest[260];

    for (int i = FUNC_EQUAL; i <= FUNC_MOD; ++i) {
        findID = FindStringToken(text, scriptEvaluationTokens[i - 1], 1);
        if (findID > -1) {
            offset = findID;
            token  = i;
        }
    }
    if (token > 0) {
        StrCopy(dest, functions[token].name);
        StrAdd(dest, "(");
        findID = StrLength(dest);
        for (int i = 0; i < offset; ++i) dest[findID++] = text[i];
        if (functions[token].opcodeSize > 1) {
            dest[findID] = ',';
            int len      = StrLength(scriptEvaluationTokens[token - 1]);
            offset += len;
            ++findID;
            while (text[offset]) dest[findID++] = text[offset++];
        }
        dest[findID] = 0;
        StrAdd(dest, ")");
        StrCopy(text, dest);
    }
}
void ConvertIfWhileStatement(char *text)
{
    char dest[260];
    int compareOp  = -1;
    int strPos     = 0;
    int destStrPos = 0;
    if (FindStringToken(text, "if", 1)) {
        if (!FindStringToken(text, "while", 1)) {
            for (int i = 0; i < 6; ++i) {
                destStrPos = FindStringToken(text, scriptEvaluationTokens[i + FUNC_MOD], 1);
                if (destStrPos > -1) {
                    strPos    = destStrPos;
                    compareOp = i;
                }
            }
            if (compareOp > -1) {
                text[strPos] = ',';
                StrCopy(dest, functions[compareOp + FUNC_WEQUAL].name);
                StrAdd(dest, "(");
                AppendIntegerToString(dest, jumpTableDataPos - jumpTableDataOffset);
                StrAdd(dest, ",");
                destStrPos = StrLength(dest);
                for (int i = 5; text[i]; ++i) {
                    if (text[i] != '=' && text[i] != '(' && text[i] != ')')
                        dest[destStrPos++] = text[i];
                }
                dest[destStrPos] = 0;
                StrAdd(dest, ")");
                StrCopy(text, dest);
                jumpTableStack[++jumpTableStackPos] = jumpTableDataPos;
                jumpTableData[jumpTableDataPos++]   = scriptDataPos - scriptDataOffset;
                jumpTableData[jumpTableDataPos++]   = 0;
            }
        }
    }
    else {
        for (int i = 0; i < 6; ++i) {
            destStrPos = FindStringToken(text, scriptEvaluationTokens[i + FUNC_MOD], 1);
            if (destStrPos > -1) {
                strPos    = destStrPos;
                compareOp = i;
            }
        }
        if (compareOp > -1) {
            text[strPos] = ',';
            StrCopy(dest, functions[compareOp + FUNC_IFEQUAL].name);
            StrAdd(dest, "(");
            AppendIntegerToString(dest, jumpTableDataPos - jumpTableDataOffset);
            StrAdd(dest, ",");
            destStrPos = StrLength(dest);
            for (int i = 2; text[i]; ++i) {
                if (text[i] != '=' && text[i] != '(' && text[i] != ')')
                    dest[destStrPos++] = text[i];
            }
            dest[destStrPos] = 0;
            StrAdd(dest, ")");
            StrCopy(text, dest);
            jumpTableStack[++jumpTableStackPos] = jumpTableDataPos;
            jumpTableData[jumpTableDataPos++]   = -1;
            jumpTableData[jumpTableDataPos++]   = 0;
        }
    }
}
void ConvertForeachStatement(char *text)
{
    if (FindStringToken(text, "foreach", 1))
        return;
    char dest[260];
    int destStrPos = 0;

    if (FindStringToken(text, "ACTIVE_ENTITIES", 1) > 0) { // foreach (just actively interacting entities)
        StrCopy(dest, functions[FUNC_FOREACHACTIVE].name);
        StrAdd(dest, "(");
        AppendIntegerToString(dest, jumpTableDataPos - jumpTableDataOffset);
        StrAdd(dest, ",");
        destStrPos = StrLength(dest);
        int cnt    = 0;
        for (int i = 7; text[i]; ++i) {
            if (text[i] != '(' && text[i] != ')' && text[i] != ',') {
                dest[destStrPos++] = text[i];
            }
            else if (text[i] == ',') {
                if (!cnt)
                    dest[destStrPos++] = text[i];
                else
                    break;
                ++cnt;
            }
        }
        dest[destStrPos] = 0;
        StrAdd(dest, ")");
        StrCopy(text, dest);
        jumpTableStack[++jumpTableStackPos] = jumpTableDataPos;
        jumpTableData[jumpTableDataPos++]   = scriptDataPos - scriptDataOffset;
        jumpTableData[jumpTableDataPos++]   = 0;
    }
    else if (FindStringToken(text, "ALL_ENTITIES", 1) > 0) { // foreach (all entities)
        StrCopy(dest, functions[FUNC_FOREACHALL].name);
        StrAdd(dest, "(");
        AppendIntegerToString(dest, jumpTableDataPos - jumpTableDataOffset);
        StrAdd(dest, ",");
        destStrPos = StrLength(dest);
        int cnt    = 0;
        for (int i = 7; text[i] && cnt < 2; ++i) {
            if (text[i] != '(' && text[i] != ')' && text[i] != ',') {
                dest[destStrPos++] = text[i];
            }
            else if (text[i] == ',') {
                if (!cnt)
                    dest[destStrPos++] = text[i];
                else
                    break;
                ++cnt;
            }
        }
        dest[destStrPos] = 0;
        StrAdd(dest, ")");
        StrCopy(text, dest);
        jumpTableStack[++jumpTableStackPos] = jumpTableDataPos;
        jumpTableData[jumpTableDataPos++]   = scriptDataPos - scriptDataOffset;
        jumpTableData[jumpTableDataPos++]   = 0;
    }
}
bool ConvertSwitchStatement(char *text)
{
    if (FindStringToken(text, "switch", 1))
        return false;
    char switchText[260];
    StrCopy(switchText, "switch");
    StrAdd(switchText, "(");
    AppendIntegerToString(switchText, jumpTableDataPos - jumpTableDataOffset);
    StrAdd(switchText, ",");
    int pos = StrLength(switchText);
    for (int i = 6; text[i]; ++i) {
        if (text[i] != '=' && text[i] != '(' && text[i] != ')')
            switchText[pos++] = text[i];
    }
    switchText[pos] = 0;
    StrAdd(switchText, ")");
    StrCopy(text, switchText);
    jumpTableStack[++jumpTableStackPos] = jumpTableDataPos;
    jumpTableData[jumpTableDataPos++]   = 0x10000;
    jumpTableData[jumpTableDataPos++]   = -0x10000;
    jumpTableData[jumpTableDataPos++]   = -1;
    jumpTableData[jumpTableDataPos++]   = 0;
    return true;
}
void ConvertFunctionText(char *text)
{
    char arrayStr[0x80];
    char funcName[132];
    int opcode     = 0;
    int opcodeSize = 0;
    int textPos    = 0;
    int namePos    = 0;
    for (namePos = 0; text[namePos] != '(' && text[namePos]; ++namePos) funcName[namePos] = text[namePos];
    funcName[namePos] = 0;
    for (int i = 0; i < FUNC_MAX_CNT; ++i) {
        if (StrComp(funcName, functions[i].name)) {
            opcode     = i;
            opcodeSize = functions[i].opcodeSize;
            textPos    = StrLength(functions[i].name);
            i          = FUNC_MAX_CNT;
        }
    }
    if (opcode <= 0) {
        SetupTextMenu(&gameMenu[0], 0);
        AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
        AddTextMenuEntry(&gameMenu[0], " ");
        AddTextMenuEntry(&gameMenu[0], "OPCODE NOT FOUND");
        AddTextMenuEntry(&gameMenu[0], funcName);
#if !RETRO_USE_ORIGINAL_CODE
        AddTextMenuEntry(&gameMenu[0], " ");
        AddTextMenuEntry(&gameMenu[0], "LINE NUMBER");
        char buffer[0x10];
        buffer[0] = 0;
        AppendIntegerToString(buffer, lineID);
        AddTextMenuEntry(&gameMenu[0], buffer);
#endif
        Engine.gameMode = ENGINE_SCRIPTERROR;
    }
    else {
        scriptData[scriptDataPos++] = opcode;
        if (StrComp("else", functions[opcode].name))
            jumpTableData[jumpTableStack[jumpTableStackPos]] = scriptDataPos - scriptDataOffset;

        if (StrComp("endif", functions[opcode].name) == 1) {
            int jPos                = jumpTableStack[jumpTableStackPos];
            jumpTableData[jPos + 1] = scriptDataPos - scriptDataOffset;
            if (jumpTableData[jPos] == -1)
                jumpTableData[jPos] = (scriptDataPos - scriptDataOffset) - 1;
            --jumpTableStackPos;
        }

        if (StrComp("endswitch", functions[opcode].name)) {
            int jPos                = jumpTableStack[jumpTableStackPos];
            jumpTableData[jPos + 3] = scriptDataPos - scriptDataOffset;
            if (jumpTableData[jPos + 2] == -1) {
                jumpTableData[jPos + 2] = (scriptDataPos - scriptDataOffset) - 1;
                int caseCnt             = abs(jumpTableData[jPos + 1] - jumpTableData[jPos]) + 1;

                int jOffset = jPos + 4;
                for (int c = 0; c < caseCnt; ++c) {
                    if (jumpTableData[jOffset + c] < 0)
                        jumpTableData[jOffset + c] = jumpTableData[jPos + 2];
                }
            }
            --jumpTableStackPos;
        }

        if (StrComp("loop", functions[opcode].name) || StrComp("next", functions[opcode].name)) {
            jumpTableData[jumpTableStack[jumpTableStackPos--] + 1] = scriptDataPos - scriptDataOffset;
        }

        for (int i = 0; i < opcodeSize; ++i) {
            ++textPos;
            int funcNamePos = 0;
            int mode        = 0;
            int prevMode    = 0;
            int arrayStrPos = 0;
            while (((text[textPos] != ',' && text[textPos] != ')') || mode == 2) && text[textPos]) {
                switch (mode) {
                    case 0: // normal
                        if (text[textPos] == '[')
                            mode = 1;
                        else if (text[textPos] == '"') {
                            prevMode                = mode;
                            mode                    = 2;
                            funcName[funcNamePos++] = '"';
                        }
                        else
                            funcName[funcNamePos++] = text[textPos];
                        ++textPos;
                        break;
                    case 1: // array val
                        if (text[textPos] == ']')
                            mode = 0;
                        else if (text[textPos] == '"') {
                            prevMode = mode;
                            mode     = 2;
                        }
                        else
                            arrayStr[arrayStrPos++] = text[textPos];
                        ++textPos;
                        break;
                    case 2: // string
                        if (text[textPos] == '"') {
                            mode                    = prevMode;
                            funcName[funcNamePos++] = '"';
                        }
                        else
                            funcName[funcNamePos++] = text[textPos];
                        ++textPos;
                        break;
                }
            }
            funcName[funcNamePos] = 0;
            arrayStr[arrayStrPos] = 0;

            int value = 0;
            // Eg: temp0 = FX_SCALE
            // Private (this script only)
            for (int a = 0; a < privateAliasCount; ++a) {
                if (StrComp(funcName, privateAliases[a].name)) {
                    CopyAliasStr(funcName, privateAliases[a].value, 0);
                    if (FindStringToken(privateAliases[a].value, "[", 1) > -1)
                        CopyAliasStr(arrayStr, privateAliases[a].value, 1);
                }
            }
            // Public (this script & all following scripts)
            for (int a = 0; a < publicAliasCount; ++a) {
                if (StrComp(funcName, publicAliases[a].name)) {
                    CopyAliasStr(funcName, publicAliases[a].value, 0);
                    if (FindStringToken(publicAliases[a].value, "[", 1) > -1)
                        CopyAliasStr(arrayStr, publicAliases[a].value, 1);
                }
            }

            if (arrayStr[0]) {
                char arrStrBuf[0x80];
                int arrPos = 0;
                int bufPos = 0;
                if (arrayStr[0] == '+' || arrayStr[0] == '-')
                    ++arrPos;
                while (arrayStr[arrPos]) arrStrBuf[bufPos++] = arrayStr[arrPos++];
                arrStrBuf[bufPos] = 0;

                // Private (this script only)
                for (int a = 0; a < privateAliasCount; ++a) {
                    if (StrComp(arrStrBuf, privateAliases[a].name)) {
                        char pref = arrayStr[0];
                        CopyAliasStr(arrayStr, privateAliases[a].value, 0);

                        if (pref == '+' || pref == '-') {
                            int len = StrLength(arrayStr);
                            for (int i = len; i >= 0; --i) arrayStr[i + 1] = arrayStr[i];
                            arrayStr[0] = pref;
                        }
                    }
                }
                // Public (this script & all following scripts)
                for (int a = 0; a < publicAliasCount; ++a) {
                    if (StrComp(arrStrBuf, publicAliases[a].name)) {
                        char pref = arrayStr[0];
                        CopyAliasStr(arrayStr, publicAliases[a].value, 0);

                        if (pref == '+' || pref == '-') {
                            int len = StrLength(arrayStr);
                            for (int i = len; i >= 0; --i) arrayStr[i + 1] = arrayStr[i];
                            arrayStr[0] = pref;
                        }
                    }
                }
            }

            // Eg: temp0 = value0
            // Private (this script only)
            for (int s = 0; s < privateStaticVarCount; ++s) {
                if (StrComp(funcName, privateStaticVariables[s].name)) {
                    StrCopy(funcName, "local");
                    arrayStr[0] = 0;
                    AppendIntegerToString(arrayStr, privateStaticVariables[s].dataPos);
                }
            }
            // Public (this script & all following scripts)
            for (int s = 0; s < publicStaticVarCount; ++s) {
                if (StrComp(funcName, publicStaticVariables[s].name)) {
                    StrCopy(funcName, "local");
                    arrayStr[0] = 0;
                    AppendIntegerToString(arrayStr, publicStaticVariables[s].dataPos);
                }
            }

            // Eg: GetTableValue(temp0, 1, arrayPos0)
            // Private (this script only)
            for (int a = 0; a < privateTableCount; ++a) {
                if (StrComp(funcName, privateTables[a].name)) {
                    funcName[0] = 0;
                    AppendIntegerToString(funcName, privateTables[a].dataPos);
                    arrayStr[0] = 0;
                }
            }
            // Public (this script & all following scripts)
            for (int a = 0; a < publicTableCount; ++a) {
                if (StrComp(funcName, publicTables[a].name)) {
                    funcName[0] = 0;
                    AppendIntegerToString(funcName, publicTables[a].dataPos);
                    arrayStr[0] = 0;
                }
            }

            // Eg: temp0 = game.variable
            for (int v = 0; v < globalVariablesCount; ++v) {
                if (StrComp(funcName, globalVariableNames[v])) {
                    StrCopy(funcName, "global");
                    arrayStr[0] = 0;
                    AppendIntegerToString(arrayStr, v);
                }
            }
            // Eg: temp0 = Function1
            for (int f = 0; f < scriptFunctionCount; ++f) {
                if (StrComp(funcName, scriptFunctionNames[f])) {
                    funcName[0] = 0;
                    AppendIntegerToString(funcName, f);
                }
            }

            // Eg: temp0 = TypeName[Player Object]
            if (StrComp(funcName, "TypeName")) {
                funcName[0] = 0;
                AppendIntegerToString(funcName, 0);
                int o = 0;
                for (; o < OBJECT_COUNT; ++o) {
                    if (StrComp(arrayStr, typeNames[o])) {
                        funcName[0] = 0;
                        AppendIntegerToString(funcName, o);
                        break;
                    }
                }

                if (o == OBJECT_COUNT) {
                    PrintLog("WARNING: Unknown typename \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = SfxName[Jump]
            if (StrComp(funcName, "SfxName")) {
                funcName[0] = 0;
                AppendIntegerToString(funcName, 0);
                int s = 0;
                for (; s < SFX_COUNT; ++s) {
                    if (StrComp(arrayStr, sfxNames[s])) {
                        funcName[0] = 0;
                        AppendIntegerToString(funcName, s);
                        break;
                    }
                }

                if (s == SFX_COUNT) {
                    PrintLog("WARNING: Unknown sfxName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = VarName[player.lives]
            if (StrComp(funcName, "VarName")) {
                funcName[0] = 0;
                AppendIntegerToString(funcName, 0);
                int v = 0;
                for (; v < globalVariablesCount; ++v) {
                    if (StrComp(arrayStr, globalVariableNames[v])) {
                        funcName[0] = 0;
                        AppendIntegerToString(funcName, v);
                        break;
                    }
                }

                if (v == globalVariablesCount) {
                    PrintLog("WARNING: Unknown varName \"%s\", on line %d", arrayStr, lineID);
                }
            }

#if !RETRO_USE_ORIGINAL_CODE
            // Eg: temp0 = AchievementName[Ring King]
            if (StrComp(funcName, "AchievementName")) {
                funcName[0] = 0;
                AppendIntegerToString(funcName, 0);
                int a = 0;
                for (; a < achievementCount; ++a) {
                    char buf[0x40];
                    char *str = achievements[a].name;
                    int pos   = 0;

                    while (*str) {
                        if (*str != ' ')
                            buf[pos++] = *str;
                        str++;
                    }
                    buf[pos] = 0;

                    if (StrComp(arrayStr, buf)) {
                        funcName[0] = 0;
                        AppendIntegerToString(funcName, a);
                        break;
                    }
                }

                if (a == achievementCount) {
                    PrintLog("WARNING: Unknown AchievementName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = PlayerName[SONIC]
            if (StrComp(funcName, "PlayerName")) {
                funcName[0] = 0;
                AppendIntegerToString(funcName, 0);
                int p = 0;
                for (; p < PLAYER_COUNT; ++p) {
                    char buf[0x40];
                    char *str = playerNames[p];
                    int pos   = 0;

                    while (*str) {
                        if (*str != ' ')
                            buf[pos++] = *str;
                        str++;
                    }
                    buf[pos] = 0;

                    if (StrComp(arrayStr, buf)) {
                        funcName[0] = 0;
                        AppendIntegerToString(funcName, p);
                        break;
                    }
                }

                if (p == PLAYER_COUNT) {
                    PrintLog("WARNING: Unknown PlayerName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = StageName[R - GREEN HILL ZONE 1]
            if (StrComp(funcName, "StageName")) {
                funcName[0] = 0;
                int s       = -1;
                if (StrLength(arrayStr) >= 2) {
                    char list = arrayStr[0];
                    switch (list) {
                        case 'P': list = STAGELIST_PRESENTATION; break;
                        case 'R': list = STAGELIST_REGULAR; break;
                        case 'S': list = STAGELIST_SPECIAL; break;
                        case 'B': list = STAGELIST_BONUS; break;
                    }
                    s = GetSceneID(list, &arrayStr[2]);
                }

                if (s == -1) {
                    PrintLog("WARNING: Unknown StageName \"%s\", on line %d", arrayStr, lineID);
                    s = 0;
                }
                funcName[0] = 0;
                AppendIntegerToString(funcName, s);
            }
#endif

            // Storing Values
            if (ConvertStringToInteger(funcName, &value)) {
                scriptData[scriptDataPos++] = SCRIPTVAR_INTCONST;
                scriptData[scriptDataPos++] = value;
            }
            else if (funcName[0] == '"') {
                scriptData[scriptDataPos++] = SCRIPTVAR_STRCONST;
                scriptData[scriptDataPos++] = StrLength(funcName) - 2;
                int scriptTextPos           = 1;
                arrayStrPos                 = 0;
                while (scriptTextPos > -1) {
                    switch (arrayStrPos) {
                        case 0:
                            scriptData[scriptDataPos] = funcName[scriptTextPos] << 24;
                            ++arrayStrPos;
                            break;
                        case 1:
                            scriptData[scriptDataPos] += funcName[scriptTextPos] << 16;
                            ++arrayStrPos;
                            break;
                        case 2:
                            scriptData[scriptDataPos] += funcName[scriptTextPos] << 8;
                            ++arrayStrPos;
                            break;
                        case 3:
                            scriptData[scriptDataPos++] += funcName[scriptTextPos];
                            arrayStrPos = 0;
                            break;
                        default: break;
                    }
                    if (funcName[scriptTextPos] == '"') {
                        if (arrayStrPos > 0)
                            ++scriptDataPos;
                        scriptTextPos = -1;
                    }
                    else {
                        scriptTextPos++;
                    }
                }
            }
            else {
                scriptData[scriptDataPos++] = SCRIPTVAR_VAR;
                if (arrayStr[0]) {
                    scriptData[scriptDataPos] = VARARR_ARRAY;
                    if (arrayStr[0] == '+')
                        scriptData[scriptDataPos] = VARARR_ENTNOPLUS1;
                    if (arrayStr[0] == '-')
                        scriptData[scriptDataPos] = VARARR_ENTNOMINUS1;
                    ++scriptDataPos;
                    if (arrayStr[0] == '-' || arrayStr[0] == '+') {
                        for (int i = 0; i < StrLength(arrayStr); ++i) arrayStr[i] = arrayStr[i + 1];
                    }
                    if (ConvertStringToInteger(arrayStr, &value) == 1) {
                        scriptData[scriptDataPos++] = 0;
                        scriptData[scriptDataPos++] = value;
                    }
                    else {
                        if (StrComp(arrayStr, "arrayPos0"))
                            value = 0;
                        if (StrComp(arrayStr, "arrayPos1"))
                            value = 1;
                        if (StrComp(arrayStr, "arrayPos2"))
                            value = 2;
                        if (StrComp(arrayStr, "arrayPos3"))
                            value = 3;
                        if (StrComp(arrayStr, "arrayPos4"))
                            value = 4;
                        if (StrComp(arrayStr, "arrayPos5"))
                            value = 5;
                        if (StrComp(arrayStr, "arrayPos6"))
                            value = 6;
                        if (StrComp(arrayStr, "arrayPos7"))
                            value = 7;
                        if (StrComp(arrayStr, "tempObjectPos"))
                            value = 8;
                        scriptData[scriptDataPos++] = 1;
                        scriptData[scriptDataPos++] = value;
                    }
                }
                else {
                    scriptData[scriptDataPos++] = VARARR_NONE;
                }
                value = -1;
                for (int i = 0; i < VAR_MAX_CNT; ++i) {
                    if (StrComp(funcName, variableNames[i]))
                        value = i;
                }

                if (value == -1 && Engine.gameMode != ENGINE_SCRIPTERROR) {
                    SetupTextMenu(&gameMenu[0], 0);
                    AddTextMenuEntry(&gameMenu[0], "SCRIPT PARSING FAILED");
                    AddTextMenuEntry(&gameMenu[0], " ");
                    AddTextMenuEntry(&gameMenu[0], "OPERAND NOT FOUND");
                    AddTextMenuEntry(&gameMenu[0], funcName);
                    AddTextMenuEntry(&gameMenu[0], " ");
                    AddTextMenuEntry(&gameMenu[0], "LINE NUMBER");
                    funcName[0] = 0;
                    AppendIntegerToString(funcName, lineID);
                    AddTextMenuEntry(&gameMenu[0], funcName);
                    Engine.gameMode = ENGINE_SCRIPTERROR;
                    value           = 0;
                }
                scriptData[scriptDataPos++] = value;
            }
        }
    }
}
void CheckCaseNumber(char *text)
{
    if (FindStringToken(text, "case", 1))
        return;

    char caseString[128];
    int caseStrPos = 0;
    char caseChar  = text[4];
    if (text[4]) {
        int textPos = 5;
        do {
            if (caseChar != ':')
                caseString[caseStrPos++] = caseChar;
            caseChar = text[textPos++];
        } while (caseChar);
    }
    else {
        caseStrPos = 0;
    }
    caseString[caseStrPos] = 0;

    bool flag = false;

    if (FindStringToken(caseString, "[", 1) >= 0) {
        char caseValue[0x80];
        char arrayStr[0x80];

        int textPos     = 0;
        int funcNamePos = 0;
        int mode        = 0;
        int arrayStrPos = 0;
        while (caseString[textPos] != ':' && caseString[textPos]) {
            if (mode) {
                if (caseString[textPos] == ']')
                    mode = 0;
                else
                    arrayStr[arrayStrPos++] = caseString[textPos];
                ++textPos;
            }
            else {
                if (caseString[textPos] == '[')
                    mode = 1;
                else
                    caseValue[funcNamePos++] = caseString[textPos];
                ++textPos;
            }
        }
        caseValue[funcNamePos] = 0;
        arrayStr[arrayStrPos]  = 0;

        // Eg: temp0 = TypeName[Player Object]
        if (StrComp(caseValue, "TypeName")) {
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, 0);
            int o = 0;
            for (; o < OBJECT_COUNT; ++o) {
                if (StrComp(arrayStr, typeNames[o])) {
                    caseValue[0] = 0;
                    AppendIntegerToString(caseValue, o);
                    break;
                }
            }

            if (o == OBJECT_COUNT) {
                PrintLog("WARNING: Unknown typename \"%s\", on line %d", arrayStr, lineID);
            }
        }

        // Eg: temp0 = SfxName[Jump]
        if (StrComp(caseValue, "SfxName")) {
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, 0);
            int s = 0;
            for (; s < SFX_COUNT; ++s) {
                if (StrComp(arrayStr, sfxNames[s])) {
                    caseValue[0] = 0;
                    AppendIntegerToString(caseValue, s);
                    break;
                }
            }

            if (s == SFX_COUNT) {
                PrintLog("WARNING: Unknown sfxName \"%s\", on line %d", arrayStr, lineID);
            }
        }

        // Eg: temp0 = VarName[player.lives]
        if (StrComp(caseValue, "VarName")) {
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, 0);
            int v = 0;
            for (; v < globalVariablesCount; ++v) {
                if (StrComp(arrayStr, globalVariableNames[v])) {
                    caseValue[0] = 0;
                    AppendIntegerToString(caseValue, v);
                    break;
                }
            }

            if (v == globalVariablesCount) {
                PrintLog("WARNING: Unknown varName \"%s\", on line %d", arrayStr, lineID);
            }
        }

#if !RETRO_USE_ORIGINAL_CODE
        // Eg: temp0 = AchievementName[Ring King]
        if (StrComp(caseValue, "AchievementName")) {
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, 0);
            int a = 0;
            for (; a < achievementCount; ++a) {
                char buf[0x40];
                char *str = achievements[a].name;
                int pos   = 0;

                while (*str) {
                    if (*str != ' ')
                        buf[pos++] = *str;
                    str++;
                }
                buf[pos] = 0;

                if (StrComp(arrayStr, buf)) {
                    caseValue[0] = 0;
                    AppendIntegerToString(caseValue, a);
                    break;
                }
            }

            if (a == achievementCount) {
                PrintLog("WARNING: Unknown AchievementName \"%s\", on line %d", arrayStr, lineID);
            }
        }

        // Eg: temp0 = PlayerName[SONIC]
        if (StrComp(caseValue, "PlayerName")) {
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, 0);
            int p = 0;
            for (; p < PLAYER_COUNT; ++p) {
                char buf[0x40];
                char *str = playerNames[p];
                int pos   = 0;

                while (*str) {
                    if (*str != ' ')
                        buf[pos++] = *str;
                    str++;
                }
                buf[pos] = 0;

                if (StrComp(arrayStr, buf)) {
                    caseValue[0] = 0;
                    AppendIntegerToString(caseValue, p);
                    break;
                }
            }

            if (p == PLAYER_COUNT) {
                PrintLog("WARNING: Unknown PlayerName \"%s\", on line %d", arrayStr, lineID);
            }
        }

        // Eg: temp0 = StageName[R - GREEN HILL ZONE 1]
        if (StrComp(caseValue, "StageName")) {
            caseValue[0] = 0;
            int s        = -1;
            if (StrLength(arrayStr) >= 2) {
                char list = arrayStr[0];
                switch (list) {
                    case 'P': list = STAGELIST_PRESENTATION; break;
                    case 'R': list = STAGELIST_REGULAR; break;
                    case 'S': list = STAGELIST_SPECIAL; break;
                    case 'B': list = STAGELIST_BONUS; break;
                }
                s = GetSceneID(list, &arrayStr[2]);
            }

            if (s == -1) {
                PrintLog("WARNING: Unknown StageName \"%s\", on line %d", arrayStr, lineID);
                s = 0;
            }
            caseValue[0] = 0;
            AppendIntegerToString(caseValue, s);
        }
#endif
        StrCopy(caseString, caseValue);
        flag = true;
    }

    for (int a = 0; a < privateAliasCount && !flag; ++a) {
        if (StrComp(privateAliases[a].name, caseString)) {
            StrCopy(caseString, privateAliases[a].value);
            flag = true;
            break;
        }
    }

    for (int a = 0; a < publicAliasCount && !flag; ++a) {
        if (StrComp(publicAliases[a].name, caseString)) {
            StrCopy(caseString, publicAliases[a].value);
            break;
        }
    }

    int caseID = 0;
    if (ConvertStringToInteger(caseString, &caseID)) {
        int stackValue = jumpTableStack[jumpTableStackPos];
        if (caseID < jumpTableData[stackValue])
            jumpTableData[stackValue] = caseID;
        stackValue++;
        if (caseID > jumpTableData[stackValue])
            jumpTableData[stackValue] = caseID;
    }
    else {
        PrintLog("WARNING: unable to convert case string \"%s\" to int, on line %d", caseString, lineID);
    }
}
bool ReadSwitchCase(char *text)
{
    char caseText[0x80];
    if (FindStringToken(text, "case", 1)) {
        if (FindStringToken(text, "default", 1)) {
            return false;
        }
        else {
            int jumpTablepos                = jumpTableStack[jumpTableStackPos];
            jumpTableData[jumpTablepos + 2] = scriptDataPos - scriptDataOffset;
            int cnt                         = abs(jumpTableData[jumpTablepos + 1] - jumpTableData[jumpTablepos]) + 1;

            int jOffset = jumpTablepos + 4;
            for (int i = 0; i < cnt; ++i) {
                if (jumpTableData[jOffset + i] < 0)
                    jumpTableData[jOffset + i] = scriptDataPos - scriptDataOffset;
            }
            return true;
        }
    }
    else {
        int textPos       = 4;
        int caseStringPos = 0;
        while (text[textPos]) {
            if (text[textPos] != ':')
                caseText[caseStringPos++] = text[textPos];
            ++textPos;
        }
        caseText[caseStringPos] = 0;

        bool flag = false;
        if (FindStringToken(caseText, "[", 1) >= 0) {
            char caseValue[0x80];
            char arrayStr[0x80];

            int textPos     = 0;
            int funcNamePos = 0;
            int mode        = 0;
            int arrayStrPos = 0;
            while (caseText[textPos] != ':' && caseText[textPos]) {
                if (mode) {
                    if (caseText[textPos] == ']')
                        mode = 0;
                    else
                        arrayStr[arrayStrPos++] = caseText[textPos];
                    ++textPos;
                }
                else {
                    if (caseText[textPos] == '[')
                        mode = 1;
                    else
                        caseValue[funcNamePos++] = caseText[textPos];
                    ++textPos;
                }
            }
            caseValue[funcNamePos] = 0;
            arrayStr[arrayStrPos]  = 0;

            // Eg: temp0 = TypeName[Player Object]
            if (StrComp(caseValue, "TypeName")) {
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, 0);
                int o = 0;
                for (; o < OBJECT_COUNT; ++o) {
                    if (StrComp(arrayStr, typeNames[o])) {
                        caseValue[0] = 0;
                        AppendIntegerToString(caseValue, o);
                        break;
                    }
                }

                if (o == OBJECT_COUNT) {
                    PrintLog("WARNING: Unknown typename \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = SfxName[Jump]
            if (StrComp(caseValue, "SfxName")) {
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, 0);
                int s = 0;
                for (; s < SFX_COUNT; ++s) {
                    if (StrComp(arrayStr, sfxNames[s])) {
                        caseValue[0] = 0;
                        AppendIntegerToString(caseValue, s);
                        break;
                    }
                }

                if (s == SFX_COUNT) {
                    PrintLog("WARNING: Unknown sfxName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = VarName[player.lives]
            if (StrComp(caseValue, "VarName")) {
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, 0);
                int v = 0;
                for (; v < globalVariablesCount; ++v) {
                    if (StrComp(arrayStr, globalVariableNames[v])) {
                        caseValue[0] = 0;
                        AppendIntegerToString(caseValue, v);
                        break;
                    }
                }

                if (v == globalVariablesCount) {
                    PrintLog("WARNING: Unknown varName \"%s\", on line %d", arrayStr, lineID);
                }
            }

#if !RETRO_USE_ORIGINAL_CODE
            // Eg: temp0 = AchievementName[Ring King]
            if (StrComp(caseValue, "AchievementName")) {
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, 0);
                int a = 0;
                for (; a < achievementCount; ++a) {
                    char buf[0x40];
                    char *str = achievements[a].name;
                    int pos   = 0;

                    while (*str) {
                        if (*str != ' ')
                            buf[pos++] = *str;
                        str++;
                    }
                    buf[pos] = 0;

                    if (StrComp(arrayStr, buf)) {
                        caseValue[0] = 0;
                        AppendIntegerToString(caseValue, a);
                        break;
                    }
                }

                if (a == achievementCount) {
                    PrintLog("WARNING: Unknown AchievementName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = PlayerName[SONIC]
            if (StrComp(caseValue, "PlayerName")) {
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, 0);
                int p = 0;
                for (; p < PLAYER_COUNT; ++p) {
                    char buf[0x40];
                    char *str = playerNames[p];
                    int pos   = 0;

                    while (*str) {
                        if (*str != ' ')
                            buf[pos++] = *str;
                        str++;
                    }
                    buf[pos] = 0;

                    if (StrComp(arrayStr, buf)) {
                        caseValue[0] = 0;
                        AppendIntegerToString(caseValue, p);
                        break;
                    }
                }

                if (p == PLAYER_COUNT) {
                    PrintLog("WARNING: Unknown PlayerName \"%s\", on line %d", arrayStr, lineID);
                }
            }

            // Eg: temp0 = StageName[R - GREEN HILL ZONE 1]
            if (StrComp(caseValue, "StageName")) {
                caseValue[0] = 0;
                int s        = -1;
                if (StrLength(arrayStr) >= 2) {
                    char list = arrayStr[0];
                    switch (list) {
                        case 'P': list = STAGELIST_PRESENTATION; break;
                        case 'R': list = STAGELIST_REGULAR; break;
                        case 'S': list = STAGELIST_SPECIAL; break;
                        case 'B': list = STAGELIST_BONUS; break;
                    }
                    s = GetSceneID(list, &arrayStr[2]);
                }

                if (s == -1) {
                    PrintLog("WARNING: Unknown StageName \"%s\", on line %d", arrayStr, lineID);
                    s = 0;
                }
                caseValue[0] = 0;
                AppendIntegerToString(caseValue, s);
            }
#endif
            StrCopy(caseText, caseValue);
            flag = true;
        }

        for (int a = 0; a < privateAliasCount && !flag; ++a) {
            if (StrComp(caseText, privateAliases[a].name)) {
                StrCopy(caseText, privateAliases[a].value);
                flag = true;
                break;
            }
        }

        for (int a = 0; a < publicAliasCount && !flag; ++a) {
            if (StrComp(caseText, publicAliases[a].name)) {
                StrCopy(caseText, publicAliases[a].value);
                break;
            }
        }

        int val = 0;

        int jPos    = jumpTableStack[jumpTableStackPos];
        int jOffset = jPos + 4;
        if (ConvertStringToInteger(caseText, &val))
            jumpTableData[val - jumpTableData[jPos] + jOffset] = scriptDataPos - scriptDataOffset;
        else
            PrintLog("WARNING: unable to read case string \"%s\" as an int, on line %d", caseText, lineID);
        return true;
    }
    return false;
}
void ReadTableValues(char *text)
{
    int textPos = 0;
    char strBuffer[0x20];
    int strPos = 0;
    while (true) {
        if (text[textPos] == ',' || !text[textPos]) {
            strBuffer[strPos] = 0;
            if (strBuffer[0]) { // only try if something exists
                int cnt = currentTable->valueCount;

                if (!ConvertStringToInteger(strBuffer, &currentTable->values[cnt].value)) {
                    bool flag = false;
                    for (int a = 0; a < privateAliasCount; ++a) {
                        if (StrComp(privateAliases[a].name, strBuffer)) {
                            StrCopy(strBuffer, privateAliases[a].value);
                            flag = true;
                            break;
                        }
                    }

                    for (int a = 0; a < publicAliasCount && !flag; ++a) {
                        if (StrComp(publicAliases[a].name, strBuffer)) {
                            StrCopy(strBuffer, publicAliases[a].value);
                            break;
                        }
                    }

                    if (!ConvertStringToInteger(strBuffer, &currentTable->values[cnt].value)) {
                        PrintLog("WARNING: unable to convert table var %d value \"%s\" to int, on line %d", cnt, strBuffer, lineID);
                    }
                }
                currentTable->valueCount++;
            }

            strPos = 0;

            if (!text[textPos])
                break;
        }
        else {
            strBuffer[strPos++] = text[textPos];
        }
        textPos++;
    }
}
void AppendIntegerToString(char *text, int value)
{
    int textPos = 0;
    while (true) {
        if (!text[textPos])
            break;
        ++textPos;
    }

    int cnt = 0;
    int v   = value;
    while (v != 0) {
        v /= 10;
        cnt++;
    }

    v = 0;
    for (int i = cnt - 1; i >= 0; --i) {
        v = value / pow(10, i);
        v %= 10;

        int strValue = v + '0';
        if (strValue < '0' || strValue > '9') {
            // what
        }
        text[textPos++] = strValue;
    }
    if (value == 0)
        text[textPos++] = '0';
    text[textPos] = 0;
}
void AppendIntegerToStringW(ushort *text, int value)
{
    int textPos = 0;
    while (true) {
        if (!text[textPos])
            break;
        ++textPos;
    }

    int cnt = 0;
    int v   = value;
    while (v != 0) {
        v /= 10;
        cnt++;
    }

    v = 0;
    for (int i = cnt - 1; i >= 0; --i) {
        v = value / pow(10, i);
        v %= 10;

        int strValue = v + '0';
        if (strValue < '0' || strValue > '9') {
            // what
        }
        text[textPos++] = strValue;
    }
    if (value == 0)
        text[textPos++] = '0';
    text[textPos] = 0;
}
#endif

bool ConvertStringToInteger(const char *text, int *value)
{
    int charID    = 0;
    bool negative = false;
    int base      = 10;
    *value        = 0;
    if (*text != '+' && !(*text >= '0' && *text <= '9') && *text != '-')
        return false;
    int strLength = StrLength(text) - 1;
    uint charVal  = 0;
    if (*text == '-') {
        negative = true;
        charID   = 1;
        --strLength;
    }
    else if (*text == '+') {
        charID = 1;
        --strLength;
    }

    if (text[charID] == '0') {
        if (text[charID + 1] == 'x' || text[charID + 1] == 'X')
            base = 0x10;
        else if (text[charID + 1] == 'b' || text[charID + 1] == 'B')
            base = 0b10;
        else if (text[charID + 1] == 'o' || text[charID + 1] == 'O')
            base = 0010; // base 8

        if (base != 10) {
            charID += 2;
            strLength -= 2;
        }
    }

    while (strLength > -1) {
        bool flag = text[charID] < '0';
        if (!flag) {
            if (base == 0x10 && text[charID] > 'f')
                flag = true;
            if (base == 0010 && text[charID] > '7')
                flag = true;
            if (base == 0b10 && text[charID] > '1')
                flag = true;
        }

        if (flag) {
            return 0;
        }
        if (strLength <= 0) {
            if (text[charID] >= '0' && text[charID] <= '9') {
                *value = text[charID] + *value - '0';
            }
            else if (text[charID] >= 'a' && text[charID] <= 'f') {
                charVal = text[charID] - 'a';
                charVal += 10;
                *value += charVal;
            }
            else if (text[charID] >= 'A' && text[charID] <= 'F') {
                charVal = text[charID] - 'A';
                charVal += 10;
                *value += charVal;
            }
        }
        else {
            int strlen = strLength + 1;
            charVal    = 0;
            if (text[charID] >= '0' && text[charID] <= '9') {
                charVal = text[charID] - '0';
            }
            else if (text[charID] >= 'a' && text[charID] <= 'f') {
                charVal = text[charID] - 'a';
                charVal += 10;
            }
            else if (text[charID] >= 'A' && text[charID] <= 'F') {
                charVal = text[charID] - 'A';
                charVal += 10;
            }
            for (; --strlen; charVal *= base)
                ;
            *value += charVal;
        }
        --strLength;
        ++charID;
    }
    if (negative)
        *value = -*value;
    return true;
}

#if RETRO_USE_COMPILER
void CopyAliasStr(char *dest, char *text, bool arrayIndex)
{
    int textPos     = 0;
    int destPos     = 0;
    bool arrayValue = false;
    if (arrayIndex) {
        while (text[textPos]) {
            if (arrayValue) {
                if (text[textPos] == ']')
                    arrayValue = false;
                else
                    dest[destPos++] = text[textPos];
                ++textPos;
            }
            else {
                if (text[textPos] == '[')
                    arrayValue = true;
                ++textPos;
            }
        }
    }
    else {
        while (text[textPos]) {
            if (arrayValue) {
                if (text[textPos] == ']')
                    arrayValue = false;
                ++textPos;
            }
            else {
                if (text[textPos] == '[')
                    arrayValue = true;
                else
                    dest[destPos++] = text[textPos];
                ++textPos;
            }
        }
    }
    dest[destPos] = 0;
}
bool CheckOpcodeType(char *text)
{
    while (true) {
        int c = *text;
        if (!*text)
            break;
        ++text;
        if (c == '(')
            return false;
    }
    return true;
}

void ParseScriptFile(char *scriptName, int scriptID)
{
    jumpTableStackPos = 0;
    lineID            = 0;

    privateAliasCount     = 0;
    privateStaticVarCount = 0;
    privateTableCount     = 0;

    char scriptPath[0x40];
    StrCopy(scriptPath, "Scripts/");
    StrAdd(scriptPath, scriptName);
    FileInfo info;
    if (LoadFile(scriptPath, &info)) {
        int readMode   = READMODE_NORMAL;
        int parseMode  = PARSEMODE_SCOPELESS;
        char prevChar  = 0;
        char curChar   = 0;
        int switchDeep = 0;

        while (readMode < READMODE_EOF) {
            int textPos   = 0;
            readMode      = READMODE_NORMAL;
            bool semiFlag = false;
            while (readMode < READMODE_ENDLINE) {
                prevChar = curChar;
                FileRead(&curChar, 1);
                if (readMode == READMODE_STRING) {
                    if (curChar == '\t' || curChar == '\r' || curChar == '\n' || curChar == ';' || readMode >= READMODE_COMMENTLINE) {
                        if ((curChar == '\n' && prevChar != '\r') || (curChar == '\n' && prevChar == '\r')) {
                            readMode            = READMODE_ENDLINE;
                            scriptText[textPos] = 0;
                            if (curChar == ';')
                                semiFlag = true;
                        }
                    }
                    else if (curChar != '/' || textPos <= 0) {
                        scriptText[textPos++] = curChar;
                        if (curChar == '"')
                            readMode = READMODE_NORMAL;
                    }
                    else if (curChar == '/' && prevChar == '/') {
                        readMode              = READMODE_COMMENTLINE;
                        scriptText[--textPos] = 0;
                    }
                    else {
                        scriptText[textPos++] = curChar;
                    }
                }
                else if (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n' || curChar == ';'
                         || readMode >= READMODE_COMMENTLINE) {
                    if ((curChar == '\n' && prevChar != '\r') || (curChar == '\n' && prevChar == '\r') || curChar == ';') {
                        readMode            = READMODE_ENDLINE;
                        scriptText[textPos] = 0;
                        if (curChar == ';')
                            semiFlag = true;
                    }
                }
                else if (curChar != '/' || textPos <= 0) {
                    scriptText[textPos++] = curChar;
                    if (curChar == '"' && !readMode)
                        readMode = READMODE_STRING;
                }
                else if (curChar == '/' && prevChar == '/') {
                    readMode              = READMODE_COMMENTLINE;
                    scriptText[--textPos] = 0;
                }
                else {
                    scriptText[textPos++] = curChar;
                }
                if (ReachedEndOfFile()) {
                    scriptText[textPos] = 0;
                    readMode            = READMODE_EOF;
                }
            }

            switch (parseMode) {
                case PARSEMODE_SCOPELESS:
                    if (!semiFlag)
                        ++lineID;
                    CheckAliasText(scriptText);
                    CheckStaticText(scriptText);

                    currentTable = CheckTableText(scriptText);
                    if (currentTable) {
                        parseMode = PARSEMODE_TABLEREAD;
                        StrCopy(scriptText, "");
                    }

                    if (StrComp(scriptText, "eventObjectMain")) {
                        parseMode                                          = PARSEMODE_FUNCTION;
                        objectScriptList[scriptID].eventMain.scriptCodePtr = scriptDataPos;
                        objectScriptList[scriptID].eventMain.jumpTablePtr  = jumpTableDataPos;
                        scriptDataOffset                                   = scriptDataPos;
                        jumpTableDataOffset                                = jumpTableDataPos;
                    }
                    if (StrComp(scriptText, "eventObjectDraw")) {
                        parseMode                                          = PARSEMODE_FUNCTION;
                        objectScriptList[scriptID].eventDraw.scriptCodePtr = scriptDataPos;
                        objectScriptList[scriptID].eventDraw.jumpTablePtr  = jumpTableDataPos;
                        scriptDataOffset                                   = scriptDataPos;
                        jumpTableDataOffset                                = jumpTableDataPos;
                    }
                    if (StrComp(scriptText, "eventObjectStartup")) {
                        parseMode                                             = PARSEMODE_FUNCTION;
                        objectScriptList[scriptID].eventStartup.scriptCodePtr = scriptDataPos;
                        objectScriptList[scriptID].eventStartup.jumpTablePtr  = jumpTableDataPos;
                        scriptDataOffset                                      = scriptDataPos;
                        jumpTableDataOffset                                   = jumpTableDataPos;
                    }
                    if (!FindStringToken(scriptText, "reservefunction", 1)) { // forward decl
                        char funcName[0x40];
                        for (textPos = 15; scriptText[textPos]; ++textPos) funcName[textPos - 15] = scriptText[textPos];
                        funcName[textPos - 15] = 0;
                        int funcID             = -1;
                        for (int f = 0; f < scriptFunctionCount; ++f) {
                            if (StrComp(funcName, scriptFunctionNames[f]))
                                funcID = f;
                        }
                        if (scriptFunctionCount < FUNCTION_COUNT && funcID == -1) {
                            StrCopy(scriptFunctionNames[scriptFunctionCount++], funcName);
                        }
                        else {
                            PrintLog("Warning: Function %s has already been reserved!", funcName);
                        }
                        parseMode = PARSEMODE_SCOPELESS;
                    }
                    else if (!FindStringToken(scriptText, "function", 1)) { // regular decl
                        char funcName[0x40];
                        for (textPos = 8; scriptText[textPos]; ++textPos) funcName[textPos - 8] = scriptText[textPos];
                        funcName[textPos - 8] = 0;
                        int funcID            = -1;
                        for (int f = 0; f < scriptFunctionCount; ++f) {
                            if (StrComp(funcName, scriptFunctionNames[f]))
                                funcID = f;
                        }
                        if (funcID <= -1) {
                            if (scriptFunctionCount >= FUNCTION_COUNT) {
                                parseMode = PARSEMODE_SCOPELESS;
                            }
                            else {
                                StrCopy(scriptFunctionNames[scriptFunctionCount], funcName);
                                functionScriptList[scriptFunctionCount].scriptCodePtr = scriptDataPos;
                                functionScriptList[scriptFunctionCount].jumpTablePtr  = jumpTableDataPos;
                                scriptDataOffset                                      = scriptDataPos;
                                jumpTableDataOffset                                   = jumpTableDataPos;
                                parseMode                                             = PARSEMODE_FUNCTION;
                                ++scriptFunctionCount;
                            }
                        }
                        else {
                            StrCopy(scriptFunctionNames[funcID], funcName);
                            functionScriptList[funcID].scriptCodePtr = scriptDataPos;
                            functionScriptList[funcID].jumpTablePtr  = jumpTableDataPos;
                            scriptDataOffset                         = scriptDataPos;
                            jumpTableDataOffset                      = jumpTableDataPos;
                            parseMode                                = PARSEMODE_FUNCTION;
                        }
                    }
                    break;
                case PARSEMODE_PLATFORMSKIP:
                    if (!semiFlag)
                        ++lineID;
                    if (!FindStringToken(scriptText, "#endplatform", 1))
                        parseMode = PARSEMODE_FUNCTION;
                    break;
                case PARSEMODE_FUNCTION:
                    if (!semiFlag)
                        ++lineID;
                    if (scriptText[0]) {
                        if (StrComp(scriptText, "endevent")) {
                            scriptData[scriptDataPos++] = FUNC_END;
                            parseMode                   = PARSEMODE_SCOPELESS;
                        }
                        else if (StrComp(scriptText, "endfunction")) {
                            scriptData[scriptDataPos++] = FUNC_RETURN;
                            parseMode                   = PARSEMODE_SCOPELESS;
                        }
                        else if (FindStringToken(scriptText, "#platform:", 1)) {
                            if (FindStringToken(scriptText, "#endplatform", 1) == -1) {
                                ConvertIfWhileStatement(scriptText);
                                ConvertForeachStatement(scriptText);
                                if (ConvertSwitchStatement(scriptText)) {
                                    parseMode    = PARSEMODE_SWITCHREAD;
                                    info.readPos = (int)GetFilePosition();
                                    switchDeep   = 0;
                                }
                                ConvertArithmaticSyntax(scriptText);
                                if (!ReadSwitchCase(scriptText)) {
                                    ConvertFunctionText(scriptText);
                                    if (Engine.gameMode == ENGINE_SCRIPTERROR) {
                                        AddTextMenuEntry(&gameMenu[0], " ");
                                        AddTextMenuEntry(&gameMenu[0], "ERROR IN");
                                        AddTextMenuEntry(&gameMenu[0], scriptName);
                                        parseMode = PARSEMODE_ERROR;
                                    }
                                }
                            }
                        }
                        else if (FindStringToken(scriptText, Engine.gamePlatform, 1) == -1
                                 && FindStringToken(scriptText, Engine.gameRenderType, 1) == -1
#if RETRO_USE_HAPTICS
                                 && FindStringToken(scriptText, Engine.gameHapticSetting, 1) == -1
#endif
#if !RETRO_USE_ORIGINAL_CODE
                                 && FindStringToken(scriptText, "USE_DECOMP", 1) == -1 // general flag for decomp-only stuff
#endif
#if RETRO_USE_NETWORKING
                                 && FindStringToken(scriptText, "USE_NETWORKING", 1) == -1
#endif
#if RETRO_USE_MOD_LOADER
                                 && FindStringToken(scriptText, "USE_MOD_LOADER", 1) == -1
#endif
                        ) {
                            parseMode = PARSEMODE_PLATFORMSKIP;
                        }
                    }
                    break;
                case PARSEMODE_SWITCHREAD:
                    if (!FindStringToken(scriptText, "switch", 1))
                        ++switchDeep;
                    if (switchDeep) {
                        if (!FindStringToken(scriptText, "endswitch", 1))
                            --switchDeep;
                    }
                    else if (FindStringToken(scriptText, "endswitch", 1)) {
                        CheckCaseNumber(scriptText);
                    }
                    else {
                        SetFilePosition(info.readPos);
                        parseMode  = PARSEMODE_FUNCTION;
                        int jPos   = jumpTableStack[jumpTableStackPos];
                        switchDeep = abs(jumpTableData[jPos + 1] - jumpTableData[jPos]) + 1;
                        for (textPos = 0; textPos < switchDeep; ++textPos) jumpTableData[jumpTableDataPos++] = -1;
                    }
                    break;
                case PARSEMODE_TABLEREAD:
                    if (!semiFlag)
                        ++lineID;
                    if (FindStringToken(scriptText, "endtable", 1)) {
                        ReadTableValues(scriptText);
                    }
                    else {
                        currentTable->dataPos       = scriptDataPos;
                        scriptData[scriptDataPos++] = currentTable->valueCount;
                        for (int v = 0; v < currentTable->valueCount; ++v) {
                            currentTable->values[v].dataPos = scriptDataPos;
                            scriptData[scriptDataPos++]     = currentTable->values[v].value;
                        }

                        if (curTablePublic) {
                            for (int t = 0; t < publicTableCount; ++t) {
                                if (StrComp(publicTables[t].name, currentTable->name)) {
                                    PrintLog("Warning: Table %s has already been used!", currentTable->name);
                                }
                            }

                            publicTables[publicTableCount] = *currentTable;
                            ++publicTableCount;
                        }
                        else {
                            for (int t = 0; t < privateTableCount; ++t) {
                                if (StrComp(privateTables[t].name, currentTable->name)) {
                                    PrintLog("Warning: Table %s has already been used!", currentTable->name);
                                }
                            }

                            privateTables[privateTableCount] = *currentTable;
                            ++privateTableCount;
                        }

                        currentTable = NULL;
                        parseMode    = PARSEMODE_SCOPELESS;
                    }
                    break;
                default: break;
            }
        }

        CloseFile();
    }
}
#endif

void LoadBytecode(int stageListID, int scriptID)
{
    char scriptPath[0x40];
    switch (stageListID) {
        case STAGELIST_PRESENTATION:
        case STAGELIST_REGULAR:
        case STAGELIST_BONUS:
        case STAGELIST_SPECIAL:
            StrCopy(scriptPath, "Bytecode/");
            StrAdd(scriptPath, stageList[stageListID][stageListPosition].folder);
            StrAdd(scriptPath, ".bin");
            break;
        case 4: StrCopy(scriptPath, "Bytecode/GlobalCode.bin"); break;
        default: break;
    }

    FileInfo info;
    if (LoadFile(scriptPath, &info)) {
        byte fileBuffer = 0;
        int *scrData    = &scriptData[scriptCodePos];
        FileRead(&fileBuffer, 1);
        int scriptCodeCount = fileBuffer;
        FileRead(&fileBuffer, 1);
        scriptCodeCount += (fileBuffer << 8);
        FileRead(&fileBuffer, 1);
        scriptCodeCount += (fileBuffer << 16);
        FileRead(&fileBuffer, 1);
        scriptCodeCount += (fileBuffer << 24);

        while (scriptCodeCount > 0) {
            FileRead(&fileBuffer, 1);
            int blockSize = fileBuffer & 0x7F;
            if (fileBuffer >= 0x80) {
                while (blockSize > 0) {
                    FileRead(&fileBuffer, 1);
                    int data = fileBuffer;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 8;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 16;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 24;
                    *scrData = data;
                    ++scrData;
                    ++scriptCodePos;
                    --scriptCodeCount;
                    --blockSize;
                }
            }
            else {
                while (blockSize > 0) {
                    FileRead(&fileBuffer, 1);
                    *scrData = fileBuffer;
                    ++scrData;
                    ++scriptCodePos;
                    --scriptCodeCount;
                    --blockSize;
                }
            }
        }

        int *jumpPtr = &jumpTableData[jumpTablePos];
        FileRead(&fileBuffer, 1);
        int jumpDataCnt = fileBuffer;
        FileRead(&fileBuffer, 1);
        jumpDataCnt += fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        jumpDataCnt += fileBuffer << 16;
        FileRead(&fileBuffer, 1);
        jumpDataCnt += fileBuffer << 24;

        while (jumpDataCnt > 0) {
            FileRead(&fileBuffer, 1);
            int blockSize = fileBuffer & 0x7F;
            if (fileBuffer >= 0x80) {
                while (blockSize > 0) {
                    FileRead(&fileBuffer, 1);
                    int data = fileBuffer;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 8;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 16;
                    FileRead(&fileBuffer, 1);
                    data += fileBuffer << 24;
                    *jumpPtr = data;
                    ++jumpPtr;
                    ++jumpTablePos;
                    --jumpDataCnt;
                    --blockSize;
                }
            }
            else {
                while (blockSize > 0) {
                    FileRead(&fileBuffer, 1);
                    *jumpPtr = fileBuffer;
                    ++jumpPtr;
                    ++jumpTablePos;
                    --jumpDataCnt;
                    --blockSize;
                }
            }
        }
        FileRead(&fileBuffer, 1);
        int scriptCount = fileBuffer;
        FileRead(&fileBuffer, 1);
        scriptCount += fileBuffer << 8;

        int objType = scriptID;
        for (int i = 0; i < scriptCount; ++i) {

            FileRead(&fileBuffer, 1);
            int buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType].eventMain.scriptCodePtr = buf + (fileBuffer << 24);

            FileRead(&fileBuffer, 1);
            buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType].eventDraw.scriptCodePtr = buf + (fileBuffer << 24);

            FileRead(&fileBuffer, 1);
            buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType++].eventStartup.scriptCodePtr = buf + (fileBuffer << 24);
        }

        objType = scriptID;
        for (int i = 0; i < scriptCount; ++i) {
            FileRead(&fileBuffer, 1);
            int buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType].eventMain.jumpTablePtr = buf + (fileBuffer << 24);

            FileRead(&fileBuffer, 1);
            buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType].eventDraw.jumpTablePtr = buf + (fileBuffer << 24);

            FileRead(&fileBuffer, 1);
            buf = fileBuffer;
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            buf += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            objectScriptList[objType++].eventStartup.jumpTablePtr = buf + (fileBuffer << 24);
        }

        FileRead(&fileBuffer, 1);
        int functionCount = fileBuffer;
        FileRead(&fileBuffer, 1);
        functionCount += fileBuffer << 8;

        for (int i = 0; i < functionCount; ++i) {
            FileRead(&fileBuffer, 1);
            int scrPos = fileBuffer;
            FileRead(&fileBuffer, 1);
            scrPos += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            scrPos += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            functionScriptList[i].scriptCodePtr = scrPos + (fileBuffer << 24);
        }

        for (int i = 0; i < functionCount; ++i) {
            FileRead(&fileBuffer, 1);
            int jmpPos = fileBuffer;
            FileRead(&fileBuffer, 1);
            jmpPos += (fileBuffer << 8);
            FileRead(&fileBuffer, 1);
            jmpPos += (fileBuffer << 16);
            FileRead(&fileBuffer, 1);
            functionScriptList[i].jumpTablePtr = jmpPos + (fileBuffer << 24);
        }

        CloseFile();
    }
}

void ClearScriptData()
{
    memset(scriptData, 0, sizeof(scriptData));
    memset(jumpTableData, 0, sizeof(jumpTableData));

    memset(foreachStack, -1, sizeof(foreachStack));
    memset(jumpTableStack, 0, sizeof(jumpTableStack));
    memset(functionStack, 0, sizeof(functionStack));

    scriptFrameCount = 0;

    scriptCodePos     = 0;
    jumpTablePos      = 0;
    jumpTableStackPos = 0;
    functionStackPos  = 0;

    scriptDataPos       = 0;
    scriptDataOffset    = 0;
    jumpTableDataPos    = 0;
    jumpTableDataOffset = 0;

#if RETRO_USE_COMPILER
    scriptFunctionCount = 0;

    lineID = 0;

    publicAliasCount  = COMMONALIAS_COUNT;
    privateAliasCount = 0;

    publicStaticVarCount  = 0;
    privateStaticVarCount = 0;

    publicTableCount  = 0;
    privateTableCount = 0;

    for (int i = COMMONALIAS_COUNT; i < ALIAS_COUNT; ++i) {
        StrCopy(publicAliases[i].name, "");
        StrCopy(publicAliases[i].value, "");
    }

    for (int i = 0; i < ALIAS_COUNT_TRIM; ++i) {
        StrCopy(privateAliases[i].name, "");
        StrCopy(privateAliases[i].value, "");
    }

    memset(publicStaticVariables, 0, sizeof(publicStaticVariables));
    memset(privateStaticVariables, 0, sizeof(privateStaticVariables));

    memset(publicTables, 0, sizeof(publicTables));
    memset(privateTables, 0, sizeof(privateTables));
#endif

    ClearAnimationData();

    for (int o = 0; o < OBJECT_COUNT; ++o) {
        ObjectScript *scriptInfo               = &objectScriptList[o];
        scriptInfo->eventMain.scriptCodePtr    = SCRIPTDATA_COUNT - 1;
        scriptInfo->eventMain.jumpTablePtr     = JUMPTABLE_COUNT - 1;
        scriptInfo->eventDraw.scriptCodePtr    = SCRIPTDATA_COUNT - 1;
        scriptInfo->eventDraw.jumpTablePtr     = JUMPTABLE_COUNT - 1;
        scriptInfo->eventStartup.scriptCodePtr = SCRIPTDATA_COUNT - 1;
        scriptInfo->eventStartup.jumpTablePtr  = JUMPTABLE_COUNT - 1;
        scriptInfo->frameListOffset            = 0;
        scriptInfo->spriteSheetID              = 0;
        scriptInfo->animFile                   = GetDefaultAnimationRef();
        typeNames[o][0]                        = 0;
    }

    for (int s = globalSFXCount; s < globalSFXCount + stageSFXCount; ++s) {
        sfxNames[s][0] = 0;
    }

    for (int f = 0; f < FUNCTION_COUNT; ++f) {
        functionScriptList[f].scriptCodePtr = SCRIPTDATA_COUNT - 1;
        functionScriptList[f].jumpTablePtr  = JUMPTABLE_COUNT - 1;
    }

    SetObjectTypeName("Blank Object", OBJ_TYPE_BLANKOBJECT);
}

void ProcessScript(int scriptCodePtr, int jumpTablePtr, byte scriptEvent)
{
    bool running      = true;
    int scriptDataPtr = scriptCodePtr;
    // int jumpTableDataPtr = jumpTablePtr;
    jumpTableStackPos = 0;
    functionStackPos  = 0;
    foreachStackPos   = 0;

    while (running) {
        int opcode           = scriptData[scriptDataPtr++];
        int opcodeSize       = functions[opcode].opcodeSize;
        int scriptCodeOffset = scriptDataPtr;

        scriptText[0] = '\0';

        // Get Values
        for (int i = 0; i < opcodeSize; ++i) {
            int opcodeType = scriptData[scriptDataPtr++];

            if (opcodeType == SCRIPTVAR_VAR) {
                int arrayVal = 0;
                switch (scriptData[scriptDataPtr++]) {
                    case VARARR_NONE: arrayVal = objectEntityPos; break;
                    case VARARR_ARRAY:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = scriptEng.arrayPosition[scriptData[scriptDataPtr++]];
                        else
                            arrayVal = scriptData[scriptDataPtr++];
                        break;
                    case VARARR_ENTNOPLUS1:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = scriptEng.arrayPosition[scriptData[scriptDataPtr++]] + objectEntityPos;
                        else
                            arrayVal = scriptData[scriptDataPtr++] + objectEntityPos;
                        break;
                    case VARARR_ENTNOMINUS1:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = objectEntityPos - scriptEng.arrayPosition[scriptData[scriptDataPtr++]];
                        else
                            arrayVal = objectEntityPos - scriptData[scriptDataPtr++];
                        break;
                    default: break;
                }

                // Variables
                switch (scriptData[scriptDataPtr++]) {
                    default: break;
                    case VAR_TEMP0: scriptEng.operands[i] = scriptEng.temp[0]; break;
                    case VAR_TEMP1: scriptEng.operands[i] = scriptEng.temp[1]; break;
                    case VAR_TEMP2: scriptEng.operands[i] = scriptEng.temp[2]; break;
                    case VAR_TEMP3: scriptEng.operands[i] = scriptEng.temp[3]; break;
                    case VAR_TEMP4: scriptEng.operands[i] = scriptEng.temp[4]; break;
                    case VAR_TEMP5: scriptEng.operands[i] = scriptEng.temp[5]; break;
                    case VAR_TEMP6: scriptEng.operands[i] = scriptEng.temp[6]; break;
                    case VAR_TEMP7: scriptEng.operands[i] = scriptEng.temp[7]; break;
                    case VAR_TEMP8: scriptEng.operands[i] = scriptEng.temp[8]; break;
                    case VAR_TEMP9: scriptEng.operands[i] = scriptEng.temp[9]; break;
                    case VAR_TEMP10: scriptEng.operands[i] = scriptEng.temp[10]; break;
                    case VAR_CHECKRESULT: scriptEng.operands[i] = scriptEng.checkResult; break;
                    case VAR_ARRAYPOS0: scriptEng.operands[i] = scriptEng.arrayPosition[0]; break;
                    case VAR_ARRAYPOS1: scriptEng.operands[i] = scriptEng.arrayPosition[1]; break;
                    case VAR_ARRAYPOS2: scriptEng.operands[i] = scriptEng.arrayPosition[2]; break;
                    case VAR_ARRAYPOS3: scriptEng.operands[i] = scriptEng.arrayPosition[3]; break;
                    case VAR_ARRAYPOS4: scriptEng.operands[i] = scriptEng.arrayPosition[4]; break;
                    case VAR_ARRAYPOS5: scriptEng.operands[i] = scriptEng.arrayPosition[5]; break;
                    case VAR_ARRAYPOS6: scriptEng.operands[i] = scriptEng.arrayPosition[6]; break;
                    case VAR_ARRAYPOS7: scriptEng.operands[i] = scriptEng.arrayPosition[7]; break;
                    case VAR_GLOBAL: scriptEng.operands[i] = globalVariables[arrayVal]; break;
                    case VAR_LOCAL: scriptEng.operands[i] = scriptData[arrayVal]; break;
                    case VAR_OBJECTENTITYPOS: scriptEng.operands[i] = arrayVal; break;
                    case VAR_OBJECTGROUPID: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].groupID;
                        break;
                    }
                    case VAR_OBJECTTYPE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].type;
                        break;
                    }
                    case VAR_OBJECTPROPERTYVALUE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].propertyValue;
                        break;
                    }
                    case VAR_OBJECTXPOS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].xpos;
                        break;
                    }
                    case VAR_OBJECTYPOS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].ypos;
                        break;
                    }
                    case VAR_OBJECTIXPOS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].xpos >> 16;
                        break;
                    }
                    case VAR_OBJECTIYPOS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].ypos >> 16;
                        break;
                    }
                    case VAR_OBJECTXVEL: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].xvel;
                        break;
                    }
                    case VAR_OBJECTYVEL: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].yvel;
                        break;
                    }
                    case VAR_OBJECTSPEED: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].speed;
                        break;
                    }
                    case VAR_OBJECTSTATE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].state;
                        break;
                    }
                    case VAR_OBJECTROTATION: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].rotation;
                        break;
                    }
                    case VAR_OBJECTSCALE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].scale;
                        break;
                    }
                    case VAR_OBJECTPRIORITY: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].priority;
                        break;
                    }
                    case VAR_OBJECTDRAWORDER: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].drawOrder;
                        break;
                    }
                    case VAR_OBJECTDIRECTION: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].direction;
                        break;
                    }
                    case VAR_OBJECTINKEFFECT: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].inkEffect;
                        break;
                    }
                    case VAR_OBJECTALPHA: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].alpha;
                        break;
                    }
                    case VAR_OBJECTFRAME: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].frame;
                        break;
                    }
                    case VAR_OBJECTANIMATION: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].animation;
                        break;
                    }
                    case VAR_OBJECTPREVANIMATION: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].prevAnimation;
                        break;
                    }
                    case VAR_OBJECTANIMATIONSPEED: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].animationSpeed;
                        break;
                    }
                    case VAR_OBJECTANIMATIONTIMER: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].animationTimer;
                        break;
                    }
                    case VAR_OBJECTANGLE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].angle;
                        break;
                    }
                    case VAR_OBJECTLOOKPOSX: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].lookPosX;
                        break;
                    }
                    case VAR_OBJECTLOOKPOSY: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].lookPosY;
                        break;
                    }
                    case VAR_OBJECTCOLLISIONMODE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].collisionMode;
                        break;
                    }
                    case VAR_OBJECTCOLLISIONPLANE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].collisionPlane;
                        break;
                    }
                    case VAR_OBJECTCONTROLMODE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].controlMode;
                        break;
                    }
                    case VAR_OBJECTCONTROLLOCK: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].controlLock;
                        break;
                    }
                    case VAR_OBJECTPUSHING: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].pushing;
                        break;
                    }
                    case VAR_OBJECTVISIBLE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].visible;
                        break;
                    }
                    case VAR_OBJECTTILECOLLISIONS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].tileCollisions;
                        break;
                    }
                    case VAR_OBJECTINTERACTION: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].objectInteractions;
                        break;
                    }
                    case VAR_OBJECTGRAVITY: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].gravity;
                        break;
                    }
                    case VAR_OBJECTUP: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].up;
                        break;
                    }
                    case VAR_OBJECTDOWN: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].down;
                        break;
                    }
                    case VAR_OBJECTLEFT: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].left;
                        break;
                    }
                    case VAR_OBJECTRIGHT: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].right;
                        break;
                    }
                    case VAR_OBJECTJUMPPRESS: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].jumpPress;
                        break;
                    }
                    case VAR_OBJECTJUMPHOLD: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].jumpHold;
                        break;
                    }
                    case VAR_OBJECTSCROLLTRACKING: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].scrollTracking;
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORL: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].floorSensors[0];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORC: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].floorSensors[1];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORR: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].floorSensors[2];
                        break;
                    }
#if !RETRO_REV00
                    case VAR_OBJECTFLOORSENSORLC: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].floorSensors[3];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORRC: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].floorSensors[4];
                        break;
                    }
#endif
                    case VAR_OBJECTCOLLISIONLEFT: {
                        AnimationFile *animFile = objectScriptList[objectEntityList[arrayVal].type].animFile;
                        Entity *ent             = &objectEntityList[arrayVal];
                        if (animFile) {
                            int h = animFrames[animationList[animFile->aniListOffset + ent->animation].frameListOffset + ent->frame].hitboxID;

                            scriptEng.operands[i] = hitboxList[animFile->hitboxListOffset + h].left[0];
                        }
                        else {
                            scriptEng.operands[i] = 0;
                        }
                        break;
                    }
                    case VAR_OBJECTCOLLISIONTOP: {
                        AnimationFile *animFile = objectScriptList[objectEntityList[arrayVal].type].animFile;
                        Entity *ent             = &objectEntityList[arrayVal];
                        if (animFile) {
                            int h = animFrames[animationList[animFile->aniListOffset + ent->animation].frameListOffset + ent->frame].hitboxID;

                            scriptEng.operands[i] = hitboxList[animFile->hitboxListOffset + h].top[0];
                        }
                        else {
                            scriptEng.operands[i] = 0;
                        }
                        break;
                    }
                    case VAR_OBJECTCOLLISIONRIGHT: {
                        AnimationFile *animFile = objectScriptList[objectEntityList[arrayVal].type].animFile;
                        Entity *ent             = &objectEntityList[arrayVal];
                        if (animFile) {
                            int h = animFrames[animationList[animFile->aniListOffset + ent->animation].frameListOffset + ent->frame].hitboxID;

                            scriptEng.operands[i] = hitboxList[animFile->hitboxListOffset + h].right[0];
                        }
                        else {
                            scriptEng.operands[i] = 0;
                        }
                        break;
                    }
                    case VAR_OBJECTCOLLISIONBOTTOM: {
                        AnimationFile *animFile = objectScriptList[objectEntityList[arrayVal].type].animFile;
                        Entity *ent             = &objectEntityList[arrayVal];
                        if (animFile) {
                            int h = animFrames[animationList[animFile->aniListOffset + ent->animation].frameListOffset + ent->frame].hitboxID;

                            scriptEng.operands[i] = hitboxList[animFile->hitboxListOffset + h].bottom[0];
                        }
                        else {
                            scriptEng.operands[i] = 0;
                        }
                        break;
                    }
                    case VAR_OBJECTOUTOFBOUNDS: {
#if !RETRO_REV00
                        int boundX1_2P = -(0x200 << 16);
                        int boundX2_2P = (0x200 << 16);
                        int boundX3_2P = -(0x180 << 16);
                        int boundX4_2P = (0x180 << 16);

                        int boundY1_2P = -(0x180 << 16);
                        int boundY2_2P = (0x180 << 16);
                        int boundY3_2P = -(0x100 << 16);
                        int boundY4_2P = (0x100 << 16);

                        Entity *entPtr = &objectEntityList[arrayVal];
                        int x          = entPtr->xpos >> 16;
                        int y          = entPtr->ypos >> 16;

                        if (entPtr->priority == PRIORITY_ACTIVE_BOUNDS_SMALL || entPtr->priority == PRIORITY_ACTIVE_SMALL) {
                            if (stageMode == STAGEMODE_2P) {
                                x = entPtr->xpos;
                                y = entPtr->ypos;

                                int boundL_P1 = objectEntityList[0].xpos + boundX3_2P;
                                int boundR_P1 = objectEntityList[0].xpos + boundX4_2P;
                                int boundT_P1 = objectEntityList[0].ypos + boundY3_2P;
                                int boundB_P1 = objectEntityList[0].ypos + boundY4_2P;

                                int boundL_P2 = objectEntityList[1].xpos + boundX3_2P;
                                int boundR_P2 = objectEntityList[1].xpos + boundX4_2P;
                                int boundT_P2 = objectEntityList[1].ypos + boundY3_2P;
                                int boundB_P2 = objectEntityList[1].ypos + boundY4_2P;

                                bool oobP1 = scriptEng.operands[i] = x <= boundL_P1 || x >= boundR_P1 || y <= boundT_P1 || y >= boundB_P1;
                                bool oobP2 = scriptEng.operands[i] = x <= boundL_P2 || x >= boundR_P2 || y <= boundT_P2 || y >= boundB_P2;

                                scriptEng.operands[i] = oobP1 && oobP2;
                            }
                            else {
                                int boundL = xScrollOffset - OBJECT_BORDER_X3;
                                int boundR = xScrollOffset + OBJECT_BORDER_X4;
                                int boundT = yScrollOffset - OBJECT_BORDER_Y3;
                                int boundB = yScrollOffset + OBJECT_BORDER_Y4;

                                scriptEng.operands[i] = x <= boundL || x >= boundR || y <= boundT || y >= boundB;
                            }
                        }
                        else {
                            if (stageMode == STAGEMODE_2P) {
                                x = entPtr->xpos;
                                y = entPtr->ypos;

                                int boundL_P1 = objectEntityList[0].xpos + boundX1_2P;
                                int boundR_P1 = objectEntityList[0].xpos + boundX2_2P;
                                int boundT_P1 = objectEntityList[0].ypos + boundY1_2P;
                                int boundB_P1 = objectEntityList[0].ypos + boundY2_2P;

                                int boundL_P2 = objectEntityList[1].xpos + boundX1_2P;
                                int boundR_P2 = objectEntityList[1].xpos + boundX2_2P;
                                int boundT_P2 = objectEntityList[1].ypos + boundY1_2P;
                                int boundB_P2 = objectEntityList[1].ypos + boundY2_2P;

                                bool oobP1 = scriptEng.operands[i] = x <= boundL_P1 || x >= boundR_P1 || y <= boundT_P1 || y >= boundB_P1;
                                bool oobP2 = scriptEng.operands[i] = x <= boundL_P2 || x >= boundR_P2 || y <= boundT_P2 || y >= boundB_P2;

                                scriptEng.operands[i] = oobP1 && oobP2;
                            }
                            else {
                                int boundL = xScrollOffset - OBJECT_BORDER_X1;
                                int boundR = xScrollOffset + OBJECT_BORDER_X2;
                                int boundT = yScrollOffset - OBJECT_BORDER_Y1;
                                int boundB = yScrollOffset + OBJECT_BORDER_Y2;

                                scriptEng.operands[i] = x <= boundL || x >= boundR || y <= boundT || y >= boundB;
                            }
                        }
#else
                        int x = objectEntityList[arrayVal].xpos >> 16;
                        int y = objectEntityList[arrayVal].ypos >> 16;

                        int boundL = xScrollOffset - OBJECT_BORDER_X1;
                        int boundR = xScrollOffset + OBJECT_BORDER_X2;
                        int boundT = yScrollOffset - OBJECT_BORDER_Y1;
                        int boundB = yScrollOffset + OBJECT_BORDER_Y2;

                        scriptEng.operands[i] = x <= boundL || x >= boundR || y <= boundT || y >= boundB;
#endif
                        break;
                    }
                    case VAR_OBJECTSPRITESHEET: {
                        scriptEng.operands[i] = objectScriptList[objectEntityList[arrayVal].type].spriteSheetID;
                        break;
                    }
                    case VAR_OBJECTVALUE0: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[0];
                        break;
                    }
                    case VAR_OBJECTVALUE1: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[1];
                        break;
                    }
                    case VAR_OBJECTVALUE2: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[2];
                        break;
                    }
                    case VAR_OBJECTVALUE3: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[3];
                        break;
                    }
                    case VAR_OBJECTVALUE4: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[4];
                        break;
                    }
                    case VAR_OBJECTVALUE5: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[5];
                        break;
                    }
                    case VAR_OBJECTVALUE6: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[6];
                        break;
                    }
                    case VAR_OBJECTVALUE7: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[7];
                        break;
                    }
                    case VAR_OBJECTVALUE8: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[8];
                        break;
                    }
                    case VAR_OBJECTVALUE9: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[9];
                        break;
                    }
                    case VAR_OBJECTVALUE10: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[10];
                        break;
                    }
                    case VAR_OBJECTVALUE11: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[11];
                        break;
                    }
                    case VAR_OBJECTVALUE12: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[12];
                        break;
                    }
                    case VAR_OBJECTVALUE13: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[13];
                        break;
                    }
                    case VAR_OBJECTVALUE14: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[14];
                        break;
                    }
                    case VAR_OBJECTVALUE15: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[15];
                        break;
                    }
                    case VAR_OBJECTVALUE16: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[16];
                        break;
                    }
                    case VAR_OBJECTVALUE17: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[17];
                        break;
                    }
                    case VAR_OBJECTVALUE18: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[18];
                        break;
                    }
                    case VAR_OBJECTVALUE19: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[19];
                        break;
                    }
                    case VAR_OBJECTVALUE20: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[20];
                        break;
                    }
                    case VAR_OBJECTVALUE21: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[21];
                        break;
                    }
                    case VAR_OBJECTVALUE22: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[22];
                        break;
                    }
                    case VAR_OBJECTVALUE23: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[23];
                        break;
                    }
                    case VAR_OBJECTVALUE24: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[24];
                        break;
                    }
                    case VAR_OBJECTVALUE25: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[25];
                        break;
                    }
                    case VAR_OBJECTVALUE26: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[26];
                        break;
                    }
                    case VAR_OBJECTVALUE27: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[27];
                        break;
                    }
                    case VAR_OBJECTVALUE28: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[28];
                        break;
                    }
                    case VAR_OBJECTVALUE29: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[29];
                        break;
                    }
                    case VAR_OBJECTVALUE30: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[30];
                        break;
                    }
                    case VAR_OBJECTVALUE31: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[31];
                        break;
                    }
                    case VAR_OBJECTVALUE32: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[32];
                        break;
                    }
                    case VAR_OBJECTVALUE33: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[33];
                        break;
                    }
                    case VAR_OBJECTVALUE34: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[34];
                        break;
                    }
                    case VAR_OBJECTVALUE35: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[35];
                        break;
                    }
                    case VAR_OBJECTVALUE36: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[36];
                        break;
                    }
                    case VAR_OBJECTVALUE37: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[37];
                        break;
                    }
                    case VAR_OBJECTVALUE38: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[38];
                        break;
                    }
                    case VAR_OBJECTVALUE39: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[39];
                        break;
                    }
                    case VAR_OBJECTVALUE40: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[40];
                        break;
                    }
                    case VAR_OBJECTVALUE41: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[41];
                        break;
                    }
                    case VAR_OBJECTVALUE42: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[42];
                        break;
                    }
                    case VAR_OBJECTVALUE43: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[43];
                        break;
                    }
                    case VAR_OBJECTVALUE44: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[44];
                        break;
                    }
                    case VAR_OBJECTVALUE45: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[45];
                        break;
                    }
                    case VAR_OBJECTVALUE46: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[46];
                        break;
                    }
                    case VAR_OBJECTVALUE47: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[47];
                        break;
                    }
                    case VAR_STAGESTATE: scriptEng.operands[i] = stageMode; break;
                    case VAR_STAGEACTIVELIST: scriptEng.operands[i] = activeStageList; break;
                    case VAR_STAGELISTPOS: scriptEng.operands[i] = stageListPosition; break;
                    case VAR_STAGETIMEENABLED: scriptEng.operands[i] = timeEnabled; break;
                    case VAR_STAGEMILLISECONDS: scriptEng.operands[i] = stageMilliseconds; break;
                    case VAR_STAGESECONDS: scriptEng.operands[i] = stageSeconds; break;
                    case VAR_STAGEMINUTES: scriptEng.operands[i] = stageMinutes; break;
                    case VAR_STAGEACTNUM: scriptEng.operands[i] = actID; break;
                    case VAR_STAGEPAUSEENABLED: scriptEng.operands[i] = pauseEnabled; break;
                    case VAR_STAGELISTSIZE: scriptEng.operands[i] = stageListCount[activeStageList]; break;
                    case VAR_STAGENEWXBOUNDARY1: scriptEng.operands[i] = newXBoundary1; break;
                    case VAR_STAGENEWXBOUNDARY2: scriptEng.operands[i] = newXBoundary2; break;
                    case VAR_STAGENEWYBOUNDARY1: scriptEng.operands[i] = newYBoundary1; break;
                    case VAR_STAGENEWYBOUNDARY2: scriptEng.operands[i] = newYBoundary2; break;
                    case VAR_STAGECURXBOUNDARY1: scriptEng.operands[i] = curXBoundary1; break;
                    case VAR_STAGECURXBOUNDARY2: scriptEng.operands[i] = curXBoundary2; break;
                    case VAR_STAGECURYBOUNDARY1: scriptEng.operands[i] = curYBoundary1; break;
                    case VAR_STAGECURYBOUNDARY2: scriptEng.operands[i] = curYBoundary2; break;
                    case VAR_STAGEDEFORMATIONDATA0: scriptEng.operands[i] = bgDeformationData0[arrayVal]; break;
                    case VAR_STAGEDEFORMATIONDATA1: scriptEng.operands[i] = bgDeformationData1[arrayVal]; break;
                    case VAR_STAGEDEFORMATIONDATA2: scriptEng.operands[i] = bgDeformationData2[arrayVal]; break;
                    case VAR_STAGEDEFORMATIONDATA3: scriptEng.operands[i] = bgDeformationData3[arrayVal]; break;
                    case VAR_STAGEWATERLEVEL: scriptEng.operands[i] = waterLevel; break;
                    case VAR_STAGEACTIVELAYER: scriptEng.operands[i] = activeTileLayers[arrayVal]; break;
                    case VAR_STAGEMIDPOINT: scriptEng.operands[i] = tLayerMidPoint; break;
                    case VAR_STAGEPLAYERLISTPOS: scriptEng.operands[i] = playerListPos; break;
                    case VAR_STAGEDEBUGMODE: scriptEng.operands[i] = debugMode; break;
                    case VAR_STAGEENTITYPOS: scriptEng.operands[i] = objectEntityPos; break;
                    case VAR_SCREENCAMERAENABLED: scriptEng.operands[i] = cameraEnabled; break;
                    case VAR_SCREENCAMERATARGET: scriptEng.operands[i] = cameraTarget; break;
                    case VAR_SCREENCAMERASTYLE: scriptEng.operands[i] = cameraStyle; break;
                    case VAR_SCREENCAMERAX: scriptEng.operands[i] = cameraXPos; break;
                    case VAR_SCREENCAMERAY: scriptEng.operands[i] = cameraYPos; break;
                    case VAR_SCREENDRAWLISTSIZE: scriptEng.operands[i] = drawListEntries[arrayVal].listSize; break;
                    case VAR_SCREENXCENTER: scriptEng.operands[i] = SCREEN_CENTERX; break;
                    case VAR_SCREENYCENTER: scriptEng.operands[i] = SCREEN_CENTERY; break;
                    case VAR_SCREENXSIZE: scriptEng.operands[i] = SCREEN_XSIZE; break;
                    case VAR_SCREENYSIZE: scriptEng.operands[i] = SCREEN_YSIZE; break;
                    case VAR_SCREENXOFFSET: scriptEng.operands[i] = xScrollOffset; break;
                    case VAR_SCREENYOFFSET: scriptEng.operands[i] = yScrollOffset; break;
                    case VAR_SCREENSHAKEX: scriptEng.operands[i] = cameraShakeX; break;
                    case VAR_SCREENSHAKEY: scriptEng.operands[i] = cameraShakeY; break;
                    case VAR_SCREENADJUSTCAMERAY: scriptEng.operands[i] = cameraAdjustY; break;
                    case VAR_TOUCHSCREENDOWN: scriptEng.operands[i] = touchDown[arrayVal]; break;
                    case VAR_TOUCHSCREENXPOS: scriptEng.operands[i] = touchX[arrayVal]; break;
                    case VAR_TOUCHSCREENYPOS: scriptEng.operands[i] = touchY[arrayVal]; break;
                    case VAR_MUSICVOLUME: scriptEng.operands[i] = masterVolume; break;
                    case VAR_MUSICCURRENTTRACK: scriptEng.operands[i] = trackID; break;
                    case VAR_MUSICPOSITION: scriptEng.operands[i] = musicPosition; break;
                    case VAR_INPUTDOWNUP: scriptEng.operands[i] = inputDown.up; break;
                    case VAR_INPUTDOWNDOWN: scriptEng.operands[i] = inputDown.down; break;
                    case VAR_INPUTDOWNLEFT: scriptEng.operands[i] = inputDown.left; break;
                    case VAR_INPUTDOWNRIGHT: scriptEng.operands[i] = inputDown.right; break;
                    case VAR_INPUTDOWNBUTTONA: scriptEng.operands[i] = inputDown.A; break;
                    case VAR_INPUTDOWNBUTTONB: scriptEng.operands[i] = inputDown.B; break;
                    case VAR_INPUTDOWNBUTTONC: scriptEng.operands[i] = inputDown.C; break;
                    case VAR_INPUTDOWNBUTTONX: scriptEng.operands[i] = inputDown.X; break;
                    case VAR_INPUTDOWNBUTTONY: scriptEng.operands[i] = inputDown.Y; break;
                    case VAR_INPUTDOWNBUTTONZ: scriptEng.operands[i] = inputDown.Z; break;
                    case VAR_INPUTDOWNBUTTONL: scriptEng.operands[i] = inputDown.L; break;
                    case VAR_INPUTDOWNBUTTONR: scriptEng.operands[i] = inputDown.R; break;
                    case VAR_INPUTDOWNSTART: scriptEng.operands[i] = inputDown.start; break;
                    case VAR_INPUTDOWNSELECT: scriptEng.operands[i] = inputDown.select; break;
                    case VAR_INPUTPRESSUP: scriptEng.operands[i] = inputPress.up; break;
                    case VAR_INPUTPRESSDOWN: scriptEng.operands[i] = inputPress.down; break;
                    case VAR_INPUTPRESSLEFT: scriptEng.operands[i] = inputPress.left; break;
                    case VAR_INPUTPRESSRIGHT: scriptEng.operands[i] = inputPress.right; break;
                    case VAR_INPUTPRESSBUTTONA: scriptEng.operands[i] = inputPress.A; break;
                    case VAR_INPUTPRESSBUTTONB: scriptEng.operands[i] = inputPress.B; break;
                    case VAR_INPUTPRESSBUTTONC: scriptEng.operands[i] = inputPress.C; break;
                    case VAR_INPUTPRESSBUTTONX: scriptEng.operands[i] = inputPress.X; break;
                    case VAR_INPUTPRESSBUTTONY: scriptEng.operands[i] = inputPress.Y; break;
                    case VAR_INPUTPRESSBUTTONZ: scriptEng.operands[i] = inputPress.Z; break;
                    case VAR_INPUTPRESSBUTTONL: scriptEng.operands[i] = inputPress.L; break;
                    case VAR_INPUTPRESSBUTTONR: scriptEng.operands[i] = inputPress.R; break;
                    case VAR_INPUTPRESSSTART: scriptEng.operands[i] = inputPress.start; break;
                    case VAR_INPUTPRESSSELECT: scriptEng.operands[i] = inputPress.select; break;
                    case VAR_MENU1SELECTION: scriptEng.operands[i] = gameMenu[0].selection1; break;
                    case VAR_MENU2SELECTION: scriptEng.operands[i] = gameMenu[1].selection1; break;
                    case VAR_MENU3SELECTION: scriptEng.operands[i] = gameMenu[2].selection1; break;
                    case VAR_TILELAYERXSIZE: scriptEng.operands[i] = stageLayouts[arrayVal].xsize; break;
                    case VAR_TILELAYERYSIZE: scriptEng.operands[i] = stageLayouts[arrayVal].ysize; break;
                    case VAR_TILELAYERTYPE: scriptEng.operands[i] = stageLayouts[arrayVal].type; break;
                    case VAR_TILELAYERANGLE: scriptEng.operands[i] = stageLayouts[arrayVal].angle; break;
                    case VAR_TILELAYERXPOS: scriptEng.operands[i] = stageLayouts[arrayVal].xpos; break;
                    case VAR_TILELAYERYPOS: scriptEng.operands[i] = stageLayouts[arrayVal].ypos; break;
                    case VAR_TILELAYERZPOS: scriptEng.operands[i] = stageLayouts[arrayVal].zpos; break;
                    case VAR_TILELAYERPARALLAXFACTOR: scriptEng.operands[i] = stageLayouts[arrayVal].parallaxFactor; break;
                    case VAR_TILELAYERSCROLLSPEED: scriptEng.operands[i] = stageLayouts[arrayVal].scrollSpeed; break;
                    case VAR_TILELAYERSCROLLPOS: scriptEng.operands[i] = stageLayouts[arrayVal].scrollPos; break;
                    case VAR_TILELAYERDEFORMATIONOFFSET: scriptEng.operands[i] = stageLayouts[arrayVal].deformationOffset; break;
                    case VAR_TILELAYERDEFORMATIONOFFSETW: scriptEng.operands[i] = stageLayouts[arrayVal].deformationOffsetW; break;
                    case VAR_HPARALLAXPARALLAXFACTOR: scriptEng.operands[i] = hParallax.parallaxFactor[arrayVal]; break;
                    case VAR_HPARALLAXSCROLLSPEED: scriptEng.operands[i] = hParallax.scrollSpeed[arrayVal]; break;
                    case VAR_HPARALLAXSCROLLPOS: scriptEng.operands[i] = hParallax.scrollPos[arrayVal]; break;
                    case VAR_VPARALLAXPARALLAXFACTOR: scriptEng.operands[i] = vParallax.parallaxFactor[arrayVal]; break;
                    case VAR_VPARALLAXSCROLLSPEED: scriptEng.operands[i] = vParallax.scrollSpeed[arrayVal]; break;
                    case VAR_VPARALLAXSCROLLPOS: scriptEng.operands[i] = vParallax.scrollPos[arrayVal]; break;
                    case VAR_SCENE3DVERTEXCOUNT: scriptEng.operands[i] = vertexCount; break;
                    case VAR_SCENE3DFACECOUNT: scriptEng.operands[i] = faceCount; break;
                    case VAR_SCENE3DPROJECTIONX: scriptEng.operands[i] = projectionX; break;
                    case VAR_SCENE3DPROJECTIONY: scriptEng.operands[i] = projectionY; break;
#if !RETRO_REV00
                    case VAR_SCENE3DFOGCOLOR: scriptEng.operands[i] = fogColor; break;
                    case VAR_SCENE3DFOGSTRENGTH: scriptEng.operands[i] = fogStrength; break;
#endif
                    case VAR_VERTEXBUFFERX: scriptEng.operands[i] = vertexBuffer[arrayVal].x; break;
                    case VAR_VERTEXBUFFERY: scriptEng.operands[i] = vertexBuffer[arrayVal].y; break;
                    case VAR_VERTEXBUFFERZ: scriptEng.operands[i] = vertexBuffer[arrayVal].z; break;
                    case VAR_VERTEXBUFFERU: scriptEng.operands[i] = vertexBuffer[arrayVal].u; break;
                    case VAR_VERTEXBUFFERV: scriptEng.operands[i] = vertexBuffer[arrayVal].v; break;
                    case VAR_FACEBUFFERA: scriptEng.operands[i] = faceBuffer[arrayVal].a; break;
                    case VAR_FACEBUFFERB: scriptEng.operands[i] = faceBuffer[arrayVal].b; break;
                    case VAR_FACEBUFFERC: scriptEng.operands[i] = faceBuffer[arrayVal].c; break;
                    case VAR_FACEBUFFERD: scriptEng.operands[i] = faceBuffer[arrayVal].d; break;
                    case VAR_FACEBUFFERFLAG: scriptEng.operands[i] = faceBuffer[arrayVal].flag; break;
                    case VAR_FACEBUFFERCOLOR: scriptEng.operands[i] = faceBuffer[arrayVal].color; break;
                    case VAR_SAVERAM: scriptEng.operands[i] = saveRAM[arrayVal]; break;
                    case VAR_ENGINESTATE: scriptEng.operands[i] = Engine.gameMode; break;
#if RETRO_REV00
                    case VAR_ENGINEMESSAGE: scriptEng.operands[i] = Engine.message; break;
#endif
                    case VAR_ENGINELANGUAGE: scriptEng.operands[i] = Engine.language; break;
                    case VAR_ENGINEONLINEACTIVE: scriptEng.operands[i] = Engine.onlineActive; break;
                    case VAR_ENGINESFXVOLUME: scriptEng.operands[i] = sfxVolume; break;
                    case VAR_ENGINEBGMVOLUME: scriptEng.operands[i] = bgmVolume; break;
                    case VAR_ENGINEPLATFORMID: scriptEng.operands[i] = RETRO_GAMEPLATFORMID; break;
                    case VAR_ENGINETRIALMODE: scriptEng.operands[i] = Engine.trialMode; break;
                    case VAR_ENGINEDEVICETYPE: scriptEng.operands[i] = RETRO_DEVICETYPE; break;
#if RETRO_USE_HAPTICS
                    case VAR_HAPTICSENABLED: scriptEng.operands[i] = Engine.hapticsEnabled; break;
#endif
                    case VAR_ERM_WHAT_THE_SIGMA: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[100];
                        break;
                    }
                    case VAR_OBJECTVALUE48: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[48];
                        break;
                    }
                    case VAR_OBJECTVALUE49: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[49];
                        break;
                    }
                    case VAR_OBJECTVALUE50: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[50];
                        break;
                    }
                    case VAR_OBJECTVALUE51: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].values[51];
                        break;
                    }
                    case VAR_ENGINETIMER: {
                        time_t initialTimer = time(NULL);
                        struct tm *actualTimer = localtime(&initialTimer);

                        scriptEng.operands[i] = (int)(((actualTimer->tm_year + 1900 - 2000) & 0x3F) << 26 | 
                                     ((actualTimer->tm_mon + 1) & 0xF) << 22 |
                                     (actualTimer->tm_mday & 0x1F) << 17 |
                                     (actualTimer->tm_hour & 0x1F) << 12 |
                                     (actualTimer->tm_min & 0x3F) << 6 |
                                     (actualTimer->tm_sec & 0x3F));
                         break;
                    }
                    case VAR_OBJECTWIDTH: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].width;
                        break;
                    }
                    case VAR_OBJECTHEIGHT: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].height;
                        break;
                    }
                    case VAR_OBJECTINKRED: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].inkred;
                        break;
                    }
                    case VAR_OBJECTINKGREEN: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].inkgreen;
                        break;
                    }
                    case VAR_OBJECTINKBLUE: {
                        scriptEng.operands[i] = objectEntityList[arrayVal].inkblue;
                        break;
                    }
                    case VAR_CURRENTUNIX: { 
						time_t currentTime = time(NULL);
                        scriptEng.operands[i] = (int)currentTime;
						break;
                    }
                    case VAR_CURRENTLOCALUNIX: { 
						time_t currentTime = time(NULL);
						struct tm *localTime = localtime(&currentTime);
						struct tm *utcTime = gmtime(&currentTime);
						time_t localEpoch = mktime(localTime);
						time_t utcEpoch = mktime(utcTime);
						time_t localOffset = localEpoch - utcEpoch;
						time_t localUnixTime = currentTime + localOffset;
						scriptEng.operands[i] = (int)localUnixTime; 
						break;
                    }
                }
            }
            else if (opcodeType == SCRIPTVAR_INTCONST) { // int constant
                scriptEng.operands[i] = scriptData[scriptDataPtr++];
            }
            else if (opcodeType == SCRIPTVAR_STRCONST) { // string constant
                int strLen         = scriptData[scriptDataPtr++];
                scriptText[strLen] = 0;
                for (int c = 0; c < strLen; ++c) {
                    switch (c % 4) {
                        case 0: {
                            scriptText[c] = scriptData[scriptDataPtr] >> 24;
                            break;
                        }
                        case 1: {
                            scriptText[c] = (0xFFFFFF & scriptData[scriptDataPtr]) >> 16;
                            break;
                        }
                        case 2: {
                            scriptText[c] = (0xFFFF & scriptData[scriptDataPtr]) >> 8;
                            break;
                        }
                        case 3: {
                            scriptText[c] = scriptData[scriptDataPtr++];
                            break;
                        }
                        default: break;
                    }
                }
                scriptDataPtr++;
            }
        }

        ObjectScript *scriptInfo = &objectScriptList[objectEntityList[objectEntityPos].type];
        Entity *entity           = &objectEntityList[objectEntityPos];
        SpriteFrame *spriteFrame = nullptr;

        // Functions
        switch (opcode) {
            default: break;
            case FUNC_END: running = false; break;
            case FUNC_EQUAL: scriptEng.operands[0] = scriptEng.operands[1]; break;
            case FUNC_ADD: scriptEng.operands[0] += scriptEng.operands[1]; break;
            case FUNC_SUB: scriptEng.operands[0] -= scriptEng.operands[1]; break;
            case FUNC_INC: ++scriptEng.operands[0]; break;
            case FUNC_DEC: --scriptEng.operands[0]; break;
            case FUNC_MUL: scriptEng.operands[0] *= scriptEng.operands[1]; break;
            case FUNC_DIV: scriptEng.operands[0] /= scriptEng.operands[1]; break;
            case FUNC_SHR: scriptEng.operands[0] >>= scriptEng.operands[1]; break;
            case FUNC_SHL: scriptEng.operands[0] <<= scriptEng.operands[1]; break;
            case FUNC_AND: scriptEng.operands[0] &= scriptEng.operands[1]; break;
            case FUNC_OR: scriptEng.operands[0] |= scriptEng.operands[1]; break;
            case FUNC_XOR: scriptEng.operands[0] ^= scriptEng.operands[1]; break;
            case FUNC_MOD: scriptEng.operands[0] %= scriptEng.operands[1]; break;
            case FUNC_FLIPSIGN: scriptEng.operands[0] = -scriptEng.operands[0]; break;
            case FUNC_CHECKEQUAL:
                scriptEng.checkResult = scriptEng.operands[0] == scriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKGREATER:
                scriptEng.checkResult = scriptEng.operands[0] > scriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKLOWER:
                scriptEng.checkResult = scriptEng.operands[0] < scriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKNOTEQUAL:
                scriptEng.checkResult = scriptEng.operands[0] != scriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_IFEQUAL:
                if (scriptEng.operands[1] != scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFGREATER:
                if (scriptEng.operands[1] <= scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFGREATEROREQUAL:
                if (scriptEng.operands[1] < scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFLOWER:
                if (scriptEng.operands[1] >= scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFLOWEROREQUAL:
                if (scriptEng.operands[1] > scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFNOTEQUAL:
                if (scriptEng.operands[1] == scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0]];
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_ELSE:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + jumpTableStack[jumpTableStackPos--] + 1];
                break;
            case FUNC_ENDIF:
                opcodeSize = 0;
                --jumpTableStackPos;
                break;
            case FUNC_WEQUAL:
                if (scriptEng.operands[1] != scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WGREATER:
                if (scriptEng.operands[1] <= scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WGREATEROREQUAL:
                if (scriptEng.operands[1] < scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WLOWER:
                if (scriptEng.operands[1] >= scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WLOWEROREQUAL:
                if (scriptEng.operands[1] > scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WNOTEQUAL:
                if (scriptEng.operands[1] == scriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                else
                    jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_LOOP:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + jumpTableStack[jumpTableStackPos--]];
                break;
            case FUNC_FOREACHACTIVE: {
                int groupID = scriptEng.operands[1];
                if (groupID < TYPEGROUP_COUNT) {
                    int loop                      = foreachStack[++foreachStackPos] + 1;
                    foreachStack[foreachStackPos] = loop;
                    if (loop >= objectTypeGroupList[groupID].listSize) {
                        opcodeSize                      = 0;
                        foreachStack[foreachStackPos--] = -1;
                        scriptDataPtr                   = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                        break;
                    }
                    else {
                        scriptEng.operands[2]               = objectTypeGroupList[groupID].entityRefs[loop];
                        jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                    }
                }
                else {
                    opcodeSize    = 0;
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                }
                break;
            }
            case FUNC_FOREACHALL: {
                int objType = scriptEng.operands[1];
                if (objType < OBJECT_COUNT) {
                    int loop                      = foreachStack[++foreachStackPos] + 1;
                    foreachStack[foreachStackPos] = loop;

                    if (scriptEvent == EVENT_SETUP) {
                        while (true) {
                            if (loop >= TEMPENTITY_START) {
                                opcodeSize                      = 0;
                                foreachStack[foreachStackPos--] = -1;
                                int off                         = jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                                scriptDataPtr                   = scriptCodePtr + off;
                                break;
                            }
                            else if (objType == objectEntityList[loop].type) {
                                scriptEng.operands[2]               = loop;
                                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                                break;
                            }
                            else {
                                foreachStack[foreachStackPos] = ++loop;
                            }
                        }
                    }
                    else {
                        while (true) {
                            if (loop >= ENTITY_COUNT) {
                                opcodeSize                      = 0;
                                foreachStack[foreachStackPos--] = -1;
                                scriptDataPtr                   = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                                break;
                            }
                            else if (objType == objectEntityList[loop].type) {
                                scriptEng.operands[2]               = loop;
                                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                                break;
                            }
                            else {
                                foreachStack[foreachStackPos] = ++loop;
                            }
                        }
                    }
                }
                else {
                    opcodeSize    = 0;
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1];
                }
                break;
            }
            case FUNC_NEXT:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + jumpTableStack[jumpTableStackPos--]];
                --foreachStackPos;
                break;
            case FUNC_SWITCH:
                jumpTableStack[++jumpTableStackPos] = scriptEng.operands[0];
                if (scriptEng.operands[1] < jumpTableData[jumpTablePtr + scriptEng.operands[0]]
                    || scriptEng.operands[1] > jumpTableData[jumpTablePtr + scriptEng.operands[0] + 1])
                    scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 2];
                else
                    scriptDataPtr = scriptCodePtr
                                    + jumpTableData[jumpTablePtr + scriptEng.operands[0] + 4
                                                    + (scriptEng.operands[1] - jumpTableData[jumpTablePtr + scriptEng.operands[0]])];
                opcodeSize = 0;
                break;
            case FUNC_BREAK:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + jumpTableData[jumpTablePtr + jumpTableStack[jumpTableStackPos--] + 3];
                break;
            case FUNC_ENDSWITCH:
                opcodeSize = 0;
                --jumpTableStackPos;
                break;
            case FUNC_RAND: scriptEng.operands[0] = rand() % scriptEng.operands[1]; break;
            case FUNC_SIN: {
                scriptEng.operands[0] = Sin512(scriptEng.operands[1]);
                break;
            }
            case FUNC_COS: {
                scriptEng.operands[0] = Cos512(scriptEng.operands[1]);
                break;
            }
            case FUNC_SIN256: {
                scriptEng.operands[0] = Sin256(scriptEng.operands[1]);
                break;
            }
            case FUNC_COS256: {
                scriptEng.operands[0] = Cos256(scriptEng.operands[1]);
                break;
            }
            case FUNC_ATAN2: {
                scriptEng.operands[0] = ArcTanLookup(scriptEng.operands[1], scriptEng.operands[2]);
                break;
            }
            case FUNC_INTERPOLATE:
                scriptEng.operands[0] =
                    (scriptEng.operands[2] * (0x100 - scriptEng.operands[3]) + scriptEng.operands[3] * scriptEng.operands[1]) >> 8;
                break;
            case FUNC_INTERPOLATEXY:
                scriptEng.operands[0] =
                    (scriptEng.operands[3] * (0x100 - scriptEng.operands[6]) >> 8) + ((scriptEng.operands[6] * scriptEng.operands[2]) >> 8);
                scriptEng.operands[1] =
                    (scriptEng.operands[5] * (0x100 - scriptEng.operands[6]) >> 8) + (scriptEng.operands[6] * scriptEng.operands[4] >> 8);
                break;
            case FUNC_LOADSPRITESHEET:
                opcodeSize                = 0;
                scriptInfo->spriteSheetID = AddGraphicsFile(scriptText);
                break;
            case FUNC_REMOVESPRITESHEET:
                opcodeSize = 0;
                RemoveGraphicsFile(scriptText, -1);
                break;
            case FUNC_DRAWSPRITE:
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];
                DrawSprite((entity->xpos >> 16) - xScrollOffset + spriteFrame->pivotX, (entity->ypos >> 16) - yScrollOffset + spriteFrame->pivotY,
                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWSPRITEXY:
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];
                DrawSprite((scriptEng.operands[1] >> 16) - xScrollOffset + spriteFrame->pivotX,
                           (scriptEng.operands[2] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width, spriteFrame->height,
                           spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWSPRITESCREENXY:
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];
                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY, spriteFrame->width,
                           spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWTINTRECT:
                opcodeSize = 0;
                DrawTintRectangle(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
                break;
            case FUNC_DRAWNUMBERS: {
                opcodeSize = 0;
                int i      = 10;
                if (scriptEng.operands[6]) {
                    while (scriptEng.operands[4] > 0) {
                        int frameID = scriptEng.operands[3] % i / (i / 10) + scriptEng.operands[0];
                        spriteFrame = &scriptFrames[scriptInfo->frameListOffset + frameID];
                        DrawSprite(spriteFrame->pivotX + scriptEng.operands[1], spriteFrame->pivotY + scriptEng.operands[2], spriteFrame->width,
                                   spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        scriptEng.operands[1] -= scriptEng.operands[5];
                        i *= 10;
                        --scriptEng.operands[4];
                    }
                }
                else {
                    int extra = 10;
                    if (scriptEng.operands[3])
                        extra = 10 * scriptEng.operands[3];
                    while (scriptEng.operands[4] > 0) {
                        if (extra >= i) {
                            int frameID = scriptEng.operands[3] % i / (i / 10) + scriptEng.operands[0];
                            spriteFrame = &scriptFrames[scriptInfo->frameListOffset + frameID];
                            DrawSprite(spriteFrame->pivotX + scriptEng.operands[1], spriteFrame->pivotY + scriptEng.operands[2], spriteFrame->width,
                                       spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        scriptEng.operands[1] -= scriptEng.operands[5];
                        i *= 10;
                        --scriptEng.operands[4];
                    }
                }
                break;
            }
            case FUNC_DRAWACTNAME: {
                opcodeSize = 0;
                int charID = 0;
                switch (scriptEng.operands[3]) { // Draw Mode
                    case 0:                      // Draw Word 1 (but aligned from the right instead of left)
                        charID = 0;

                        for (charID = 0;; ++charID) {
                            int nextChar = titleCardText[charID + 1];
                            if (nextChar == '-' || !nextChar)
                                break;
                        }

                        while (charID >= 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = -1; // special space char
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                scriptEng.operands[1] -= scriptEng.operands[5] + scriptEng.operands[6]; // spaceWidth + spacing
                            }
                            else {
                                character += scriptEng.operands[0];
                                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + character];

                                scriptEng.operands[1] -= spriteFrame->width + scriptEng.operands[6];

                                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                            }
                            charID--;
                        }
                        break;

                    case 1: // Draw Word 1
                        charID = 0;

                        // Draw the first letter as a capital letter, the rest are lowercase (if scriptEng.operands[4] is true, otherwise they're all
                        // uppercase)
                        if (scriptEng.operands[4] == 1 && titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                scriptEng.operands[1] += scriptEng.operands[5] + scriptEng.operands[6]; // spaceWidth + spacing
                            }
                            else {
                                character += scriptEng.operands[0];
                                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + character];
                                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                scriptEng.operands[1] += spriteFrame->width + scriptEng.operands[6];
                            }

                            scriptEng.operands[0] += 26;
                            charID++;
                        }

                        while (titleCardText[charID] != 0 && titleCardText[charID] != '-') {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character > '/' && character < ':')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                scriptEng.operands[1] += scriptEng.operands[5] + scriptEng.operands[6]; // spaceWidth + spacing
                            }
                            else {
                                character += scriptEng.operands[0];
                                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + character];
                                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                scriptEng.operands[1] += spriteFrame->width + scriptEng.operands[6];
                            }
                            charID++;
                        }
                        break;

                    case 2: // Draw Word 2
                        charID = titleCardWord2;

                        // Draw the first letter as a capital letter, the rest are lowercase (if scriptEng.operands[4] is true, otherwise they're all
                        // uppercase)
                        if (scriptEng.operands[4] == 1 && titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                scriptEng.operands[1] += scriptEng.operands[5] + scriptEng.operands[6]; // spaceWidth + spacing
                            }
                            else {
                                character += scriptEng.operands[0];
                                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + character];
                                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                scriptEng.operands[1] += spriteFrame->width + scriptEng.operands[6];
                            }
                            scriptEng.operands[0] += 26;
                            charID++;
                        }

                        while (titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                scriptEng.operands[1] += scriptEng.operands[5] + scriptEng.operands[6]; // spaceWidth + spacing
                            }
                            else {
                                character += scriptEng.operands[0];
                                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + character];
                                DrawSprite(scriptEng.operands[1] + spriteFrame->pivotX, scriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                scriptEng.operands[1] += spriteFrame->width + scriptEng.operands[6];
                            }
                            charID++;
                        }
                        break;
                }
                break;
            }
            case FUNC_DRAWMENU:
                opcodeSize        = 0;
                textMenuSurfaceNo = scriptInfo->spriteSheetID;
                DrawTextMenu(&gameMenu[scriptEng.operands[0]], scriptEng.operands[1], scriptEng.operands[2]);
                break;
            case FUNC_SPRITEFRAME:
                opcodeSize = 0;
                if (scriptEvent == EVENT_SETUP && scriptFrameCount < SPRITEFRAME_COUNT) {
                    scriptFrames[scriptFrameCount].pivotX = scriptEng.operands[0];
                    scriptFrames[scriptFrameCount].pivotY = scriptEng.operands[1];
                    scriptFrames[scriptFrameCount].width  = scriptEng.operands[2];
                    scriptFrames[scriptFrameCount].height = scriptEng.operands[3];
                    scriptFrames[scriptFrameCount].sprX   = scriptEng.operands[4];
                    scriptFrames[scriptFrameCount].sprY   = scriptEng.operands[5];
                    ++scriptFrameCount;
                }
                break;
            case FUNC_EDITFRAME: {
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];

                spriteFrame->pivotX = scriptEng.operands[1];
                spriteFrame->pivotY = scriptEng.operands[2];
                spriteFrame->width  = scriptEng.operands[3];
                spriteFrame->height = scriptEng.operands[4];
                spriteFrame->sprX   = scriptEng.operands[5];
                spriteFrame->sprY   = scriptEng.operands[6];
            } break;
            case FUNC_LOADPALETTE:
                opcodeSize = 0;
                LoadPalette(scriptText, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4]);
                break;
            case FUNC_ROTATEPALETTE:
                opcodeSize = 0;
                RotatePalette(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
                break;
            case FUNC_SETSCREENFADE:
                opcodeSize = 0;
                SetFade(1, scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
                break;
            case FUNC_SETCLASSICFADE:
                opcodeSize = 0;
                SetFade(2, scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
                break;
            case FUNC_SETACTIVEPALETTE:
                opcodeSize = 0;
                SetActivePalette(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2]);
                break;
            case FUNC_SETPALETTEFADE:
#if RETRO_REV00
                SetLimitedFade(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                               scriptEng.operands[5], scriptEng.operands[6]);
#else
                SetPaletteFade(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                               scriptEng.operands[5]);
#endif
                break;
            case FUNC_SETPALETTEENTRY: SetPaletteEntryPacked(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2]); break;
            case FUNC_GETPALETTEENTRY: scriptEng.operands[2] = GetPaletteEntryPacked(scriptEng.operands[0], scriptEng.operands[1]); break;
            case FUNC_COPYPALETTE:
                opcodeSize = 0;
                CopyPalette(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4]);
                break;
            case FUNC_CLEARSCREEN:
                opcodeSize = 0;
                ClearScreen(scriptEng.operands[0]);
                break;
            case FUNC_DRAWSPRITEFX:
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];
				if (entity->inkEffect == 7) {
					DrawScaledTintMask(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
									   (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
									   entity->scale, entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX,
									   spriteFrame->sprY, scriptInfo->spriteSheetID);
				}
				else {
					DrawSpriteAllEffect(entity, entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
											   (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
											   spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
											   entity->scale, scriptInfo->spriteSheetID, entity->alpha, entity->inkEffect, scriptEng.operands[1]);
				}	
                break;
                /*
                switch (scriptEng.operands[1]) {
					case 0: //No FX Flags, just draw the sprite
						DrawSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
								   (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
								   spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
						break;
						//If only one FX flag (or rotozoom) use the pre-existing functions
                    case FX_SCALE:
                        DrawSpriteScaled(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
                                         (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY, entity->scale,
                                         entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                         scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTATE:
                        DrawSpriteRotated(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
                                          (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
                                          spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
                                          scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTOZOOM:
                        DrawSpriteRotozoom(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
                                           (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
                                           spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
                                           entity->scale, scriptInfo->spriteSheetID);
                        break;
                    case FX_INK:
                        switch (entity->inkEffect) {
                            case INK_NONE:
                                DrawSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                           (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                           spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                            case INK_BLEND:
                                DrawBlendedSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                  (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                            case INK_ALPHA:
                                DrawAlphaBlendedSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                       (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                       spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, entity->alpha,
                                                       scriptInfo->spriteSheetID);
                                break;
                            case INK_ADD:
                                DrawAdditiveBlendedSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                          (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                          spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, entity->alpha,
                                                          scriptInfo->spriteSheetID);
                                break;
                            case INK_SUB:
                                DrawSubtractiveBlendedSprite((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                             (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                             spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, entity->alpha,
                                                             scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    case FX_TINT:
                        if (entity->inkEffect == INK_ALPHA) {
                            DrawScaledTintMask(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
                                               (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
                                               entity->scale, entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX,
                                               spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        else {
                            DrawSpriteScaled(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
                                             (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY, entity->scale,
                                             entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                             scriptInfo->spriteSheetID);
                        }
                        break;
                    case FX_FLIP:
                        switch (entity->direction) {
                            default:
                            case FLIP_NONE:
                                DrawSpriteFlipped((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                  (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_NONE, scriptInfo->spriteSheetID);
                                break;
                            case FLIP_X:
                                DrawSpriteFlipped((scriptEng.operands[2] >> 16) - xScrollOffset - spriteFrame->width - spriteFrame->pivotX,
                                                  (scriptEng.operands[3] >> 16) - yScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_X, scriptInfo->spriteSheetID);
                                break;
                            case FLIP_Y:
                                DrawSpriteFlipped((scriptEng.operands[2] >> 16) - xScrollOffset + spriteFrame->pivotX,
                                                  (scriptEng.operands[3] >> 16) - yScrollOffset - spriteFrame->height - spriteFrame->pivotY,
                                                  spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_Y,
                                                  scriptInfo->spriteSheetID);
                                break;
                            case FLIP_XY:
                                DrawSpriteFlipped((scriptEng.operands[2] >> 16) - xScrollOffset - spriteFrame->width - spriteFrame->pivotX,
                                                  (scriptEng.operands[3] >> 16) - yScrollOffset - spriteFrame->height - spriteFrame->pivotY,
                                                  spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_XY,
                                                  scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    default: //Anything those can't handle goes to the all-in-one function
						DrawSpriteAllEffect(entity->direction, (scriptEng.operands[2] >> 16) - xScrollOffset,
									     (scriptEng.operands[3] >> 16) - yScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
									     spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
									     entity->scale, scriptInfo->spriteSheetID, entity->alpha, entity->inkEffect, scriptEng.operands[1]);
					    break;
                }
                break;
                */
            case FUNC_DRAWSPRITESCREENFX:
                opcodeSize  = 0;
                spriteFrame = &scriptFrames[scriptInfo->frameListOffset + scriptEng.operands[0]];		   
				if (entity->inkEffect == 7) {
					DrawScaledTintMask(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
									   -spriteFrame->pivotY, entity->scale, entity->scale, spriteFrame->width, spriteFrame->height,
									   spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
				}
				else {
					DrawSpriteAllEffect(entity, entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
											   -spriteFrame->pivotY, spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height,
											   entity->rotation, entity->scale, scriptInfo->spriteSheetID, entity->alpha, entity->inkEffect, scriptEng.operands[1]);
				}				   
                break;
                /*
                switch (scriptEng.operands[1]) {
					case 0: //No FX Flags, just draw the sprite
						DrawSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
								   spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                    case FX_SCALE:
                        DrawSpriteScaled(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX, -spriteFrame->pivotY,
                                         entity->scale, entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                         scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTATE:
                        DrawSpriteRotated(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX, -spriteFrame->pivotY,
                                          spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
                                          scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTOZOOM:
                        DrawSpriteRotozoom(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
                                           -spriteFrame->pivotY, spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height,
                                           entity->rotation, entity->scale, scriptInfo->spriteSheetID);
                        break;
                    case FX_INK:
                        switch (entity->inkEffect) {
                            case INK_NONE:
                                DrawSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                            case INK_BLEND:
                                DrawBlendedSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                                  spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                                  scriptInfo->spriteSheetID);
                                break;
                            case INK_ALPHA:
                                DrawAlphaBlendedSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                                       spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, entity->alpha,
                                                       scriptInfo->spriteSheetID);
                                break;
                            case INK_ADD:
                                DrawAdditiveBlendedSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                                          spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                                          entity->alpha, scriptInfo->spriteSheetID);
                                break;
                            case INK_SUB:
                                DrawSubtractiveBlendedSprite(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                                             spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                                             entity->alpha, scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    case FX_TINT:
                        if (entity->inkEffect == INK_ALPHA) {
                            DrawScaledTintMask(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
                                               -spriteFrame->pivotY, entity->scale, entity->scale, spriteFrame->width, spriteFrame->height,
                                               spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        else {
                            DrawSpriteScaled(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
                                             -spriteFrame->pivotY, entity->scale, entity->scale, spriteFrame->width, spriteFrame->height,
                                             spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        break;
                    case FX_FLIP:
                        switch (entity->direction) {
                            default:
                            case FLIP_NONE:
                                DrawSpriteFlipped(scriptEng.operands[2] + spriteFrame->pivotX, scriptEng.operands[3] + spriteFrame->pivotY,
                                                  spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_NONE,
                                                  scriptInfo->spriteSheetID);
                                break;
                            case FLIP_X:
                                DrawSpriteFlipped(scriptEng.operands[2] - spriteFrame->width - spriteFrame->pivotX,
                                                  scriptEng.operands[3] + spriteFrame->pivotY, spriteFrame->width, spriteFrame->height,
                                                  spriteFrame->sprX, spriteFrame->sprY, FLIP_X, scriptInfo->spriteSheetID);
                                break;
                            case FLIP_Y:
                                DrawSpriteFlipped(scriptEng.operands[2] + spriteFrame->pivotX,
                                                  scriptEng.operands[3] - spriteFrame->height - spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_Y, scriptInfo->spriteSheetID);
                                break;
                            case FLIP_XY:
                                DrawSpriteFlipped(scriptEng.operands[2] - spriteFrame->width - spriteFrame->pivotX,
                                                  scriptEng.operands[3] - spriteFrame->height - spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, FLIP_XY, scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    default: //Anything those can't handle goes to the all-in-one function
						DrawSpriteAllEffect(entity->direction, scriptEng.operands[2], scriptEng.operands[3], -spriteFrame->pivotX,
									     -spriteFrame->pivotY, spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height,
									     entity->rotation, entity->scale, scriptInfo->spriteSheetID, entity->alpha, entity->inkEffect, scriptEng.operands[1]);
						break;
                }
                break;
                */
            case FUNC_LOADANIMATION:
                opcodeSize           = 0;
                scriptInfo->animFile = AddAnimationFile(scriptText);
                break;
            case FUNC_SETUPMENU: {
                opcodeSize     = 0;
                TextMenu *menu = &gameMenu[scriptEng.operands[0]];
                SetupTextMenu(menu, scriptEng.operands[1]);
                menu->selectionCount = scriptEng.operands[2];
                menu->alignment      = scriptEng.operands[3];
                break;
            }
            case FUNC_ADDMENUENTRY: {
                opcodeSize                           = 0;
                TextMenu *menu                       = &gameMenu[scriptEng.operands[0]];
                menu->entryHighlight[menu->rowCount] = scriptEng.operands[2];
                AddTextMenuEntry(menu, scriptText);
                break;
            }
            case FUNC_EDITMENUENTRY: {
                opcodeSize     = 0;
                TextMenu *menu = &gameMenu[scriptEng.operands[0]];
                EditTextMenuEntry(menu, scriptText, scriptEng.operands[2]);
                menu->entryHighlight[scriptEng.operands[2]] = scriptEng.operands[3];
                break;
            }
            case FUNC_LOADSTAGE:
                opcodeSize = 0;
                stageMode  = STAGEMODE_LOAD;
                break;
            case FUNC_DRAWRECT:
                opcodeSize = 0;
                DrawRectangle(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                              scriptEng.operands[5], scriptEng.operands[6], scriptEng.operands[7]);
                break;
			case FUNC_CLASSICTINT:
                opcodeSize = 0;
                DrawClassicFade(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                              scriptEng.operands[5], scriptEng.operands[6], scriptEng.operands[7]);
                break;	
            case FUNC_RESETOBJECTENTITY: {
                opcodeSize     = 0;
                Entity *newEnt = &objectEntityList[scriptEng.operands[0]];
                memset(newEnt, 0, sizeof(Entity));
                newEnt->type               = scriptEng.operands[1];
                newEnt->propertyValue      = scriptEng.operands[2];
                newEnt->xpos               = scriptEng.operands[3];
                newEnt->ypos               = scriptEng.operands[4];
                newEnt->direction          = FLIP_NONE;
                newEnt->priority           = PRIORITY_ACTIVE_BOUNDS;
                newEnt->drawOrder          = 3;
                newEnt->scale              = 512;
                newEnt->inkEffect          = INK_NONE;
                newEnt->objectInteractions = true;
                newEnt->visible            = true;
                newEnt->tileCollisions     = true;
                break;
            }
            case FUNC_BOXCOLLISIONTEST:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    default: break;
                    case C_TOUCH:
                        TouchCollision(&objectEntityList[scriptEng.operands[1]], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                                       scriptEng.operands[5], &objectEntityList[scriptEng.operands[6]], scriptEng.operands[7], scriptEng.operands[8],
                                       scriptEng.operands[9], scriptEng.operands[10]);
                        break;
                    case C_BOX:
                        BoxCollision(&objectEntityList[scriptEng.operands[1]], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                                     scriptEng.operands[5], &objectEntityList[scriptEng.operands[6]], scriptEng.operands[7], scriptEng.operands[8],
                                     scriptEng.operands[9], scriptEng.operands[10]);
                        break;
                    case C_BOX2:
                        BoxCollision2(&objectEntityList[scriptEng.operands[1]], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                                      scriptEng.operands[5], &objectEntityList[scriptEng.operands[6]], scriptEng.operands[7], scriptEng.operands[8],
                                      scriptEng.operands[9], scriptEng.operands[10]);
                        break;
                    case C_PLATFORM:
                        PlatformCollision(&objectEntityList[scriptEng.operands[1]], scriptEng.operands[2], scriptEng.operands[3],
                                          scriptEng.operands[4], scriptEng.operands[5], &objectEntityList[scriptEng.operands[6]],
                                          scriptEng.operands[7], scriptEng.operands[8], scriptEng.operands[9], scriptEng.operands[10]);
                        break;
                }
                break;
            case FUNC_CREATETEMPOBJECT: {
                opcodeSize = 0;
                if (objectEntityList[scriptEng.arrayPosition[8]].type > OBJ_TYPE_BLANKOBJECT && ++scriptEng.arrayPosition[8] == ENTITY_COUNT)
                    scriptEng.arrayPosition[8] = TEMPENTITY_START;
                Entity *temp = &objectEntityList[scriptEng.arrayPosition[8]];
                memset(temp, 0, sizeof(Entity));
                temp->type               = scriptEng.operands[0];
                temp->propertyValue      = scriptEng.operands[1];
                temp->xpos               = scriptEng.operands[2];
                temp->ypos               = scriptEng.operands[3];
                temp->direction          = FLIP_NONE;
                temp->priority           = PRIORITY_ACTIVE;
                temp->drawOrder          = 3;
                temp->scale              = 512;
                temp->inkEffect          = INK_NONE;
                temp->objectInteractions = true;
                temp->visible            = true;
                temp->tileCollisions     = true;
                break;
            }
            case FUNC_PROCESSOBJECTMOVEMENT:
                opcodeSize = 0;
                if (entity->tileCollisions) {
                    ProcessTileCollisions(entity);
                }
                else {
                    entity->xpos += entity->xvel;
                    entity->ypos += entity->yvel;
                }
                break;
            case FUNC_PROCESSOBJECTCONTROL:
                opcodeSize = 0;
                ProcessObjectControl(entity);
                break;
            case FUNC_PROCESSANIMATION:
                opcodeSize = 0;
                ProcessObjectAnimation(scriptInfo, entity);
                break;
            case FUNC_DRAWOBJECTANIMATION:
                opcodeSize = 0;
                if (entity->visible)
                    DrawObjectAnimation(scriptInfo, entity, (entity->xpos >> 16) - xScrollOffset, (entity->ypos >> 16) - yScrollOffset);
                break;
            case FUNC_SETMUSICTRACK:
                opcodeSize = 0;
                if (scriptEng.operands[2] <= 1)
                    SetMusicTrack(scriptText, scriptEng.operands[1], scriptEng.operands[2], 0);
                else
                    SetMusicTrack(scriptText, scriptEng.operands[1], true, scriptEng.operands[2]);
                break;
            case FUNC_PLAYMUSIC:
                opcodeSize = 0;
                PlayMusic(scriptEng.operands[0], 0);
                break;
            case FUNC_STOPMUSIC:
                opcodeSize = 0;
                StopMusic(true);
                break;
            case FUNC_PAUSEMUSIC:
                opcodeSize = 0;
                PauseSound();
                break;
            case FUNC_RESUMEMUSIC:
                opcodeSize = 0;
                ResumeSound();
                break;
            case FUNC_SWAPMUSICTRACK:
                opcodeSize = 0;
                if (scriptEng.operands[2] <= 1)
                    SwapMusicTrack(scriptText, scriptEng.operands[1], 0, scriptEng.operands[3]);
                else
                    SwapMusicTrack(scriptText, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
                break;
            case FUNC_PLAYSFX:
                opcodeSize = 0;
                PlaySfx(scriptEng.operands[0], scriptEng.operands[1]);
                break;
            case FUNC_STOPSFX:
                opcodeSize = 0;
                StopSfx(scriptEng.operands[0]);
                break;
            case FUNC_SETSFXATTRIBUTES:
                opcodeSize = 0;
                SetSfxAttributes(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2]);
                break;
            case FUNC_OBJECTTILECOLLISION:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    default: break;
                    case CSIDE_FLOOR: ObjectFloorCollision(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_LWALL: ObjectLWallCollision(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_RWALL: ObjectRWallCollision(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_ROOF: ObjectRoofCollision(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                }
                break;
            case FUNC_OBJECTTILEGRIP:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    default: break;
                    case CSIDE_FLOOR: ObjectFloorGrip(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_LWALL: ObjectLWallGrip(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_RWALL: ObjectRWallGrip(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case CSIDE_ROOF: ObjectRoofGrip(scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                }
                break;
            case FUNC_LOADVIDEO:
                opcodeSize = 0;
                PauseSound();

                if (FindStringToken(scriptText, ".rsv", 1) <= -1) {
                    PlayVideoFile(scriptText);
                } else {
                    scriptInfo->spriteSheetID = AddGraphicsFile(scriptText);
                }

                ResumeSound();
                break;
            case FUNC_NEXTVIDEOFRAME:
                opcodeSize = 0;
                UpdateVideoFrame();
                break;
            case FUNC_NOT: scriptEng.operands[0] = ~scriptEng.operands[0]; break;
            case FUNC_DRAW3DSCENE:
                opcodeSize = 0;
                TransformVertexBuffer();
                Sort3DDrawList();
                Draw3DScene(scriptInfo->spriteSheetID);
                break;
            case FUNC_SETIDENTITYMATRIX:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: SetIdentityMatrix(&matWorld); break;
                    case MAT_VIEW: SetIdentityMatrix(&matView); break;
                    case MAT_TEMP: SetIdentityMatrix(&matTemp); break;
                }
                break;
            case FUNC_MATRIXMULTIPLY:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD:
                        switch (scriptEng.operands[1]) {
                            case MAT_WORLD: MatrixMultiply(&matWorld, &matWorld); break;
                            case MAT_VIEW: MatrixMultiply(&matWorld, &matView); break;
                            case MAT_TEMP: MatrixMultiply(&matWorld, &matTemp); break;
                        }
                        break;
                    case MAT_VIEW:
                        switch (scriptEng.operands[1]) {
                            case MAT_WORLD: MatrixMultiply(&matView, &matWorld); break;
                            case MAT_VIEW: MatrixMultiply(&matView, &matView); break;
                            case MAT_TEMP: MatrixMultiply(&matView, &matTemp); break;
                        }
                        break;
                    case MAT_TEMP:
                        switch (scriptEng.operands[1]) {
                            case MAT_WORLD: MatrixMultiply(&matTemp, &matWorld); break;
                            case MAT_VIEW: MatrixMultiply(&matTemp, &matView); break;
                            case MAT_TEMP: MatrixMultiply(&matTemp, &matTemp); break;
                        }
                        break;
                }
                break;
            case FUNC_MATRIXTRANSLATEXYZ:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixTranslateXYZ(&matWorld, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_VIEW: MatrixTranslateXYZ(&matView, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_TEMP: MatrixTranslateXYZ(&matTemp, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                }
                break;
            case FUNC_MATRIXSCALEXYZ:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixScaleXYZ(&matWorld, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_VIEW: MatrixScaleXYZ(&matView, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_TEMP: MatrixScaleXYZ(&matTemp, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                }
                break;
            case FUNC_MATRIXROTATEX:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixRotateX(&matWorld, scriptEng.operands[1]); break;
                    case MAT_VIEW: MatrixRotateX(&matView, scriptEng.operands[1]); break;
                    case MAT_TEMP: MatrixRotateX(&matTemp, scriptEng.operands[1]); break;
                }
                break;
            case FUNC_MATRIXROTATEY:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixRotateY(&matWorld, scriptEng.operands[1]); break;
                    case MAT_VIEW: MatrixRotateY(&matView, scriptEng.operands[1]); break;
                    case MAT_TEMP: MatrixRotateY(&matTemp, scriptEng.operands[1]); break;
                }
                break;
            case FUNC_MATRIXROTATEZ:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixRotateZ(&matWorld, scriptEng.operands[1]); break;
                    case MAT_VIEW: MatrixRotateZ(&matView, scriptEng.operands[1]); break;
                    case MAT_TEMP: MatrixRotateZ(&matTemp, scriptEng.operands[1]); break;
                }
                break;
            case FUNC_MATRIXROTATEXYZ:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixRotateXYZ(&matWorld, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_VIEW: MatrixRotateXYZ(&matView, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                    case MAT_TEMP: MatrixRotateXYZ(&matTemp, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]); break;
                }
                break;
#if !RETRO_REV00
            case FUNC_MATRIXINVERSE:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: MatrixInverse(&matWorld); break;
                    case MAT_VIEW: MatrixInverse(&matView); break;
                    case MAT_TEMP: MatrixInverse(&matTemp); break;
                }
                break;
#endif
            case FUNC_TRANSFORMVERTICES:
                opcodeSize = 0;
                switch (scriptEng.operands[0]) {
                    case MAT_WORLD: TransformVertices(&matWorld, scriptEng.operands[1], scriptEng.operands[2]); break;
                    case MAT_VIEW: TransformVertices(&matView, scriptEng.operands[1], scriptEng.operands[2]); break;
                    case MAT_TEMP: TransformVertices(&matTemp, scriptEng.operands[1], scriptEng.operands[2]); break;
                }
                break;
            case FUNC_CALLFUNCTION: {
                opcodeSize                        = 0;
                functionStack[functionStackPos++] = scriptDataPtr;
                functionStack[functionStackPos++] = jumpTablePtr;
                functionStack[functionStackPos++] = scriptCodePtr;
                scriptCodePtr                     = functionScriptList[scriptEng.operands[0]].scriptCodePtr;
                jumpTablePtr                      = functionScriptList[scriptEng.operands[0]].jumpTablePtr;
                scriptDataPtr                     = scriptCodePtr;
                break;
            }
            case FUNC_RETURN:
                opcodeSize = 0;
                if (!functionStackPos) { // event, stop running
                    running = false;
                }
                else { // function, jump out
                    scriptCodePtr = functionStack[--functionStackPos];
                    jumpTablePtr  = functionStack[--functionStackPos];
                    scriptDataPtr = functionStack[--functionStackPos];
                }
                break;
            case FUNC_SETLAYERDEFORMATION:
                opcodeSize = 0;
                SetLayerDeformation(scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                                    scriptEng.operands[5]);
                break;
            case FUNC_CHECKTOUCHRECT: opcodeSize = 0; scriptEng.checkResult = -1;
#if !RETRO_USE_ORIGINAL_CODE
                addDebugHitbox(H_TYPE_FINGER, NULL, scriptEng.operands[0], scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3]);
#endif
                for (int f = 0; f < touches; ++f) {
                    if (touchDown[f] && touchX[f] > scriptEng.operands[0] && touchX[f] < scriptEng.operands[2] && touchY[f] > scriptEng.operands[1]
                        && touchY[f] < scriptEng.operands[3]) {
                        scriptEng.checkResult = f;
                    }
                }
                break;
            case FUNC_GETTILELAYERENTRY:
                scriptEng.operands[0] = stageLayouts[scriptEng.operands[1]].tiles[scriptEng.operands[2] + 0x100 * scriptEng.operands[3]];
                break;
            case FUNC_SETTILELAYERENTRY:
                stageLayouts[scriptEng.operands[1]].tiles[scriptEng.operands[2] + 0x100 * scriptEng.operands[3]] = scriptEng.operands[0];
                break;
            case FUNC_GETBIT: scriptEng.operands[0] = (scriptEng.operands[1] & (1 << scriptEng.operands[2])) >> scriptEng.operands[2]; break;
            case FUNC_SETBIT:
                if (scriptEng.operands[2] <= 0)
                    scriptEng.operands[0] &= ~(1 << scriptEng.operands[1]);
                else
                    scriptEng.operands[0] |= 1 << scriptEng.operands[1];
                break;
            case FUNC_CLEARDRAWLIST:
                opcodeSize                                      = 0;
                drawListEntries[scriptEng.operands[0]].listSize = 0;
                break;
            case FUNC_ADDDRAWLISTENTITYREF: {
                opcodeSize                                                                                           = 0;
                drawListEntries[scriptEng.operands[0]].entityRefs[drawListEntries[scriptEng.operands[0]].listSize++] = scriptEng.operands[1];
                break;
            }
            case FUNC_GETDRAWLISTENTITYREF: scriptEng.operands[0] = drawListEntries[scriptEng.operands[1]].entityRefs[scriptEng.operands[2]]; break;
            case FUNC_SETDRAWLISTENTITYREF:
                opcodeSize                                                               = 0;
                drawListEntries[scriptEng.operands[1]].entityRefs[scriptEng.operands[2]] = scriptEng.operands[0];
                break;
            case FUNC_GET16X16TILEINFO: {
                scriptEng.operands[4] = scriptEng.operands[1] >> 7;
                scriptEng.operands[5] = scriptEng.operands[2] >> 7;
                scriptEng.operands[6] = stageLayouts[0].tiles[scriptEng.operands[4] + (scriptEng.operands[5] << 8)] << 6;
                scriptEng.operands[6] += ((scriptEng.operands[1] & 0x7F) >> 4) + 8 * ((scriptEng.operands[2] & 0x7F) >> 4);
                int index = tiles128x128.tileIndex[scriptEng.operands[6]];
                switch (scriptEng.operands[3]) {
                    case TILEINFO_INDEX: scriptEng.operands[0] = tiles128x128.tileIndex[scriptEng.operands[6]]; break;
                    case TILEINFO_DIRECTION: scriptEng.operands[0] = tiles128x128.direction[scriptEng.operands[6]]; break;
                    case TILEINFO_VISUALPLANE: scriptEng.operands[0] = tiles128x128.visualPlane[scriptEng.operands[6]]; break;
                    case TILEINFO_SOLIDITYA: scriptEng.operands[0] = tiles128x128.collisionFlags[0][scriptEng.operands[6]]; break;
                    case TILEINFO_SOLIDITYB: scriptEng.operands[0] = tiles128x128.collisionFlags[1][scriptEng.operands[6]]; break;
                    case TILEINFO_FLAGSA: scriptEng.operands[0] = collisionMasks[0].flags[index]; break;
                    case TILEINFO_ANGLEA: scriptEng.operands[0] = collisionMasks[0].angles[index]; break;
                    case TILEINFO_FLAGSB: scriptEng.operands[0] = collisionMasks[1].flags[index]; break;
                    case TILEINFO_ANGLEB: scriptEng.operands[0] = collisionMasks[1].angles[index]; break;
                    default: break;
                }
                break;
            }
            case FUNC_SET16X16TILEINFO: {
                scriptEng.operands[4] = scriptEng.operands[1] >> 7;
                scriptEng.operands[5] = scriptEng.operands[2] >> 7;
                scriptEng.operands[6] = stageLayouts[0].tiles[scriptEng.operands[4] + (scriptEng.operands[5] << 8)] << 6;
                scriptEng.operands[6] += ((scriptEng.operands[1] & 0x7F) >> 4) + 8 * ((scriptEng.operands[2] & 0x7F) >> 4);
                switch (scriptEng.operands[3]) {
                    case TILEINFO_INDEX:
                        tiles128x128.tileIndex[scriptEng.operands[6]]  = scriptEng.operands[0];
                        tiles128x128.gfxDataPos[scriptEng.operands[6]] = scriptEng.operands[0] << 8;
                        break;
                    case TILEINFO_DIRECTION: tiles128x128.direction[scriptEng.operands[6]] = scriptEng.operands[0]; break;
                    case TILEINFO_VISUALPLANE: tiles128x128.visualPlane[scriptEng.operands[6]] = scriptEng.operands[0]; break;
                    case TILEINFO_SOLIDITYA: tiles128x128.collisionFlags[0][scriptEng.operands[6]] = scriptEng.operands[0]; break;
                    case TILEINFO_SOLIDITYB: tiles128x128.collisionFlags[1][scriptEng.operands[6]] = scriptEng.operands[0]; break;
                    case TILEINFO_FLAGSA: collisionMasks[1].flags[tiles128x128.tileIndex[scriptEng.operands[6]]] = scriptEng.operands[0]; break;
                    case TILEINFO_ANGLEA: collisionMasks[1].angles[tiles128x128.tileIndex[scriptEng.operands[6]]] = scriptEng.operands[0]; break;
                    default: break;
                }
                break;
            }
            case FUNC_COPY16X16TILE:
                opcodeSize = 0;
                Copy16x16Tile(scriptEng.operands[0], scriptEng.operands[1]);
                break;
            case FUNC_GETANIMATIONBYNAME: {
                AnimationFile *animFile = scriptInfo->animFile;
                scriptEng.operands[0]   = -1;
                int id                  = 0;
                while (scriptEng.operands[0] == -1) {
                    SpriteAnimation *anim = &animationList[animFile->aniListOffset + id];
                    if (StrComp(scriptText, anim->name))
                        scriptEng.operands[0] = id;
                    else if (++id == animFile->animCount)
                        scriptEng.operands[0] = 0;
                }
                break;
            }
            case FUNC_READSAVERAM:
                opcodeSize            = 0;
                scriptEng.checkResult = ReadSaveRAMData();
                break;
            case FUNC_WRITESAVERAM:
                opcodeSize            = 0;
                scriptEng.checkResult = WriteSaveRAMData();
                break;
#if RETRO_REV00 || RETRO_REV01
            case FUNC_LOADTEXTFONT: {
                opcodeSize = 0;
                LoadFontFile(scriptText);
                break;
            }
#endif
            case FUNC_LOADTEXTFILE: {
                opcodeSize     = 0;
                TextMenu *menu = &gameMenu[scriptEng.operands[0]];
#if RETRO_REV00 || RETRO_REV01
                LoadTextFile(menu, scriptText, scriptEng.operands[2] != 0);
#else
                LoadTextFile(menu, scriptText, false);
#endif
                break;
            }
            case FUNC_GETTEXTINFO: {
                TextMenu *menu = &gameMenu[scriptEng.operands[1]];
                switch (scriptEng.operands[2]) {
                    case TEXTINFO_TEXTDATA:
                        scriptEng.operands[0] = menu->textData[menu->entryStart[scriptEng.operands[3]] + scriptEng.operands[4]];
                        break;
                    case TEXTINFO_TEXTSIZE: scriptEng.operands[0] = menu->entrySize[scriptEng.operands[3]]; break;
                    case TEXTINFO_ROWCOUNT: scriptEng.operands[0] = menu->rowCount; break;
                }
                break;
            }
#if RETRO_REV00 || RETRO_REV01
            case FUNC_DRAWTEXT: {
                opcodeSize        = 0;
                textMenuSurfaceNo = scriptInfo->spriteSheetID;
                TextMenu *menu    = &gameMenu[scriptEng.operands[0]];
                DrawBitmapText(menu, scriptEng.operands[1], scriptEng.operands[2], scriptEng.operands[3], scriptEng.operands[4],
                               scriptEng.operands[5], scriptEng.operands[6]);
                break;
            }
#endif
            case FUNC_GETVERSIONNUMBER: {
                opcodeSize                           = 0;
                TextMenu *menu                       = &gameMenu[scriptEng.operands[0]];
                menu->entryHighlight[menu->rowCount] = scriptEng.operands[1];
                AddTextMenuEntry(menu, Engine.gameVersion);
                break;
            }
            case FUNC_GETTABLEVALUE: {
                int arrPos = scriptEng.operands[1];
                if (arrPos >= 0) {
                    int pos     = scriptEng.operands[2];
                    int arrSize = scriptData[pos];
                    if (arrPos < arrSize)
                        scriptEng.operands[0] = scriptData[pos + arrPos + 1];
                }
                break;
            }
            case FUNC_SETTABLEVALUE: {
                opcodeSize = 0;
                int arrPos = scriptEng.operands[1];
                if (arrPos >= 0) {
                    int pos     = scriptEng.operands[2];
                    int arrSize = scriptData[pos];
                    if (arrPos < arrSize)
                        scriptData[pos + arrPos + 1] = scriptEng.operands[0];
                }
                break;
            }
            case FUNC_CHECKCURRENTSTAGEFOLDER:
                opcodeSize            = 0;
                scriptEng.checkResult = StrComp(stageList[activeStageList][stageListPosition].folder, scriptText);
                break;
            case FUNC_ABS: {
                scriptEng.operands[0] = abs(scriptEng.operands[0]);
                break;
            }
            case FUNC_CALLNATIVEFUNCTION:
                opcodeSize = 0;
                if (scriptEng.operands[0] >= 0 && scriptEng.operands[0] < NATIIVEFUNCTION_COUNT) {
                    void (*func)(void) = (void (*)(void))nativeFunction[scriptEng.operands[0]];
                    if (func)
                        func();
                }
                break;
            case FUNC_CALLNATIVEFUNCTION2:
                if (scriptEng.operands[0] >= 0 && scriptEng.operands[0] < NATIIVEFUNCTION_COUNT) {
                    if (StrLength(scriptText)) {
                        void (*func)(int *, char *) = (void (*)(int *, char *))nativeFunction[scriptEng.operands[0]];
                        if (func)
                            func(&scriptEng.operands[2], scriptText);
                    }
                    else {
                        void (*func)(int *, int *) = (void (*)(int *, int *))nativeFunction[scriptEng.operands[0]];
                        if (func)
                            func(&scriptEng.operands[1], &scriptEng.operands[2]);
                    }
                }
                break;
            case FUNC_CALLNATIVEFUNCTION4:
                if (scriptEng.operands[0] >= 0 && scriptEng.operands[0] < NATIIVEFUNCTION_COUNT) {
                    if (StrLength(scriptText)) {
                        void (*func)(int *, char *, int *, int *) = (void (*)(int *, char *, int *, int *))nativeFunction[scriptEng.operands[0]];
                        if (func)
                            func(&scriptEng.operands[1], scriptText, &scriptEng.operands[3], &scriptEng.operands[4]);
                    }
                    else {
                        void (*func)(int *, int *, int *, int *) = (void (*)(int *, int *, int *, int *))nativeFunction[scriptEng.operands[0]];
                        if (func)
                            func(&scriptEng.operands[1], &scriptEng.operands[2], &scriptEng.operands[3], &scriptEng.operands[4]);
                    }
                }
                break;
            case FUNC_SETOBJECTRANGE: {
                opcodeSize       = 0;
                int offset       = (scriptEng.operands[0] >> 1) - SCREEN_CENTERX;
                OBJECT_BORDER_X1 = offset + 0x80;
                OBJECT_BORDER_X2 = scriptEng.operands[0] + 0x80 - offset;
                OBJECT_BORDER_X3 = offset + 0x20;
                OBJECT_BORDER_X4 = scriptEng.operands[0] + 0x20 - offset;
                break;
            }
#if !RETRO_REV00 && !RETRO_REV01
            case FUNC_GETOBJECTVALUE: {
                if (scriptEng.operands[1] < 52)
                    scriptEng.operands[0] = objectEntityList[scriptEng.operands[2]].values[scriptEng.operands[1]];
                break;
            }
            case FUNC_SETOBJECTVALUE: {
                opcodeSize = 0;
                if (scriptEng.operands[1] < 52)
                    objectEntityList[scriptEng.operands[2]].values[scriptEng.operands[1]] = scriptEng.operands[0];
                break;
            }
            case FUNC_COPYOBJECT: {
                // dstID, srcID, count
                Entity *dstList = &objectEntityList[scriptEng.operands[0]];
                Entity *srcList = &objectEntityList[scriptEng.operands[1]];
                for (int i = 0; i < scriptEng.operands[2]; ++i) memcpy(&dstList[i], &srcList[i], sizeof(Entity));
                break;
            }
#endif
            case FUNC_PRINT: {
                endLine = false;
                if (scriptEng.operands[1])
                    PrintLog("%d", scriptEng.operands[0]);
                else
                    PrintLog("%s", scriptText);

                if (scriptEng.operands[2])
                    PrintLog("\n");
                endLine = true;
                break;
            }
            case FUNC_CALCULATEOBJECTROTATION: {
                opcodeSize = 0;
                ProcessTileRotation(entity);
                break;
            }
            case FUNC_LOADWEBSITE: {
            	opcodeSize = 0;
		char temporar[100];
		if (scriptText == "google.com") {
		}
		else if (scriptText == "soniccd.vercel.app") {
		}
		else if (scriptText == "youtube.com/watch?v=dQw4w9WgXcQ") {
		}
		else {
			strcpy(temporar, "Loading unknown website: https://");
			strcat(temporar, scriptText);
			PrintLog(temporar);
		}
		strcpy(temporar, "https://");
		strcat(temporar, scriptText);
		SDL_OpenURL(temporar);
                break;
            }
            case FUNC_PROCESSFLIPPEDOBJECTCONTROL:
                opcodeSize = 0;
                ProcessFlippedObjectControl(entity);
                break;
        }

        // Set Values
        if (opcodeSize > 0)
            scriptDataPtr -= scriptDataPtr - scriptCodeOffset;
        for (int i = 0; i < opcodeSize; ++i) {
            int opcodeType = scriptData[scriptDataPtr++];
            if (opcodeType == SCRIPTVAR_VAR) {
                int arrayVal = 0;
                switch (scriptData[scriptDataPtr++]) { // variable
                    case VARARR_NONE: arrayVal = objectEntityPos; break;
                    case VARARR_ARRAY:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = scriptEng.arrayPosition[scriptData[scriptDataPtr++]];
                        else
                            arrayVal = scriptData[scriptDataPtr++];
                        break;
                    case VARARR_ENTNOPLUS1:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = objectEntityPos + scriptEng.arrayPosition[scriptData[scriptDataPtr++]];
                        else
                            arrayVal = objectEntityPos + scriptData[scriptDataPtr++];
                        break;
                    case VARARR_ENTNOMINUS1:
                        if (scriptData[scriptDataPtr++] == 1)
                            arrayVal = objectEntityPos - scriptEng.arrayPosition[scriptData[scriptDataPtr++]];
                        else
                            arrayVal = objectEntityPos - scriptData[scriptDataPtr++];
                        break;
                    default: break;
                }

                // Variables
                switch (scriptData[scriptDataPtr++]) {
                    default: break;
                    case VAR_TEMP0: scriptEng.temp[0] = scriptEng.operands[i]; break;
                    case VAR_TEMP1: scriptEng.temp[1] = scriptEng.operands[i]; break;
                    case VAR_TEMP2: scriptEng.temp[2] = scriptEng.operands[i]; break;
                    case VAR_TEMP3: scriptEng.temp[3] = scriptEng.operands[i]; break;
                    case VAR_TEMP4: scriptEng.temp[4] = scriptEng.operands[i]; break;
                    case VAR_TEMP5: scriptEng.temp[5] = scriptEng.operands[i]; break;
                    case VAR_TEMP6: scriptEng.temp[6] = scriptEng.operands[i]; break;
                    case VAR_TEMP7: scriptEng.temp[7] = scriptEng.operands[i]; break;
                    case VAR_TEMP8: scriptEng.temp[8] = scriptEng.operands[i]; break;
                    case VAR_TEMP9: scriptEng.temp[9] = scriptEng.operands[i]; break;
                    case VAR_TEMP10: scriptEng.temp[10] = scriptEng.operands[i]; break;
                    case VAR_CHECKRESULT: scriptEng.checkResult = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS0: scriptEng.arrayPosition[0] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS1: scriptEng.arrayPosition[1] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS2: scriptEng.arrayPosition[2] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS3: scriptEng.arrayPosition[3] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS4: scriptEng.arrayPosition[4] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS5: scriptEng.arrayPosition[5] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS6: scriptEng.arrayPosition[6] = scriptEng.operands[i]; break;
                    case VAR_ARRAYPOS7: scriptEng.arrayPosition[7] = scriptEng.operands[i]; break;
                    case VAR_GLOBAL: globalVariables[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_LOCAL: scriptData[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_OBJECTENTITYPOS: break;
                    case VAR_OBJECTGROUPID: {
                        objectEntityList[arrayVal].groupID = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTTYPE: {
                        objectEntityList[arrayVal].type = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPROPERTYVALUE: {
                        objectEntityList[arrayVal].propertyValue = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTXPOS: {
                        objectEntityList[arrayVal].xpos = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTYPOS: {
                        objectEntityList[arrayVal].ypos = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTIXPOS: {
                        objectEntityList[arrayVal].xpos = scriptEng.operands[i] << 16;
                        break;
                    }
                    case VAR_OBJECTIYPOS: {
                        objectEntityList[arrayVal].ypos = scriptEng.operands[i] << 16;
                        break;
                    }
                    case VAR_OBJECTXVEL: {
                        objectEntityList[arrayVal].xvel = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTYVEL: {
                        objectEntityList[arrayVal].yvel = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTSPEED: {
                        objectEntityList[arrayVal].speed = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTSTATE: {
                        objectEntityList[arrayVal].state = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTROTATION: {
                        objectEntityList[arrayVal].rotation = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTSCALE: {
                        objectEntityList[arrayVal].scale = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPRIORITY: {
                        objectEntityList[arrayVal].priority = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTDRAWORDER: {
                        objectEntityList[arrayVal].drawOrder = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTDIRECTION: {
                        objectEntityList[arrayVal].direction = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINKEFFECT: {
                        objectEntityList[arrayVal].inkEffect = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTALPHA: {
                        objectEntityList[arrayVal].alpha = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFRAME: {
                        objectEntityList[arrayVal].frame = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTANIMATION: {
                        objectEntityList[arrayVal].animation = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPREVANIMATION: {
                        objectEntityList[arrayVal].prevAnimation = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTANIMATIONSPEED: {
                        objectEntityList[arrayVal].animationSpeed = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTANIMATIONTIMER: {
                        objectEntityList[arrayVal].animationTimer = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTANGLE: {
                        objectEntityList[arrayVal].angle = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTLOOKPOSX: {
                        objectEntityList[arrayVal].lookPosX = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTLOOKPOSY: {
                        objectEntityList[arrayVal].lookPosY = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTCOLLISIONMODE: {
                        objectEntityList[arrayVal].collisionMode = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTCOLLISIONPLANE: {
                        objectEntityList[arrayVal].collisionPlane = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTCONTROLMODE: {
                        objectEntityList[arrayVal].controlMode = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTCONTROLLOCK: {
                        objectEntityList[arrayVal].controlLock = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPUSHING: {
                        objectEntityList[arrayVal].pushing = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVISIBLE: {
                        objectEntityList[arrayVal].visible = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTTILECOLLISIONS: {
                        objectEntityList[arrayVal].tileCollisions = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINTERACTION: {
                        objectEntityList[arrayVal].objectInteractions = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTGRAVITY: {
                        objectEntityList[arrayVal].gravity = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTUP: {
                        objectEntityList[arrayVal].up = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTDOWN: {
                        objectEntityList[arrayVal].down = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTLEFT: {
                        objectEntityList[arrayVal].left = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTRIGHT: {
                        objectEntityList[arrayVal].right = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTJUMPPRESS: {
                        objectEntityList[arrayVal].jumpPress = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTJUMPHOLD: {
                        objectEntityList[arrayVal].jumpHold = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTSCROLLTRACKING: {
                        objectEntityList[arrayVal].scrollTracking = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORL: {
                        objectEntityList[arrayVal].floorSensors[0] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORC: {
                        objectEntityList[arrayVal].floorSensors[1] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORR: {
                        objectEntityList[arrayVal].floorSensors[2] = scriptEng.operands[i];
                        break;
                    }
#if !RETRO_REV00
                    case VAR_OBJECTFLOORSENSORLC: {
                        objectEntityList[arrayVal].floorSensors[3] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFLOORSENSORRC: {
                        objectEntityList[arrayVal].floorSensors[4] = scriptEng.operands[i];
                        break;
                    }
#endif
                    case VAR_OBJECTCOLLISIONLEFT: {
                        break;
                    }
                    case VAR_OBJECTCOLLISIONTOP: {
                        break;
                    }
                    case VAR_OBJECTCOLLISIONRIGHT: {
                        break;
                    }
                    case VAR_OBJECTCOLLISIONBOTTOM: {
                        break;
                    }
                    case VAR_OBJECTOUTOFBOUNDS: {
                        break;
                    }
                    case VAR_OBJECTSPRITESHEET: {
                        objectScriptList[objectEntityList[arrayVal].type].spriteSheetID = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE0: {
                        objectEntityList[arrayVal].values[0] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE1: {
                        objectEntityList[arrayVal].values[1] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE2: {
                        objectEntityList[arrayVal].values[2] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE3: {
                        objectEntityList[arrayVal].values[3] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE4: {
                        objectEntityList[arrayVal].values[4] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE5: {
                        objectEntityList[arrayVal].values[5] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE6: {
                        objectEntityList[arrayVal].values[6] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE7: {
                        objectEntityList[arrayVal].values[7] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE8: {
                        objectEntityList[arrayVal].values[8] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE9: {
                        objectEntityList[arrayVal].values[9] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE10: {
                        objectEntityList[arrayVal].values[10] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE11: {
                        objectEntityList[arrayVal].values[11] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE12: {
                        objectEntityList[arrayVal].values[12] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE13: {
                        objectEntityList[arrayVal].values[13] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE14: {
                        objectEntityList[arrayVal].values[14] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE15: {
                        objectEntityList[arrayVal].values[15] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE16: {
                        objectEntityList[arrayVal].values[16] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE17: {
                        objectEntityList[arrayVal].values[17] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE18: {
                        objectEntityList[arrayVal].values[18] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE19: {
                        objectEntityList[arrayVal].values[19] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE20: {
                        objectEntityList[arrayVal].values[20] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE21: {
                        objectEntityList[arrayVal].values[21] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE22: {
                        objectEntityList[arrayVal].values[22] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE23: {
                        objectEntityList[arrayVal].values[23] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE24: {
                        objectEntityList[arrayVal].values[24] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE25: {
                        objectEntityList[arrayVal].values[25] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE26: {
                        objectEntityList[arrayVal].values[26] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE27: {
                        objectEntityList[arrayVal].values[27] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE28: {
                        objectEntityList[arrayVal].values[28] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE29: {
                        objectEntityList[arrayVal].values[29] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE30: {
                        objectEntityList[arrayVal].values[30] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE31: {
                        objectEntityList[arrayVal].values[31] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE32: {
                        objectEntityList[arrayVal].values[32] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE33: {
                        objectEntityList[arrayVal].values[33] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE34: {
                        objectEntityList[arrayVal].values[34] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE35: {
                        objectEntityList[arrayVal].values[35] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE36: {
                        objectEntityList[arrayVal].values[36] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE37: {
                        objectEntityList[arrayVal].values[37] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE38: {
                        objectEntityList[arrayVal].values[38] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE39: {
                        objectEntityList[arrayVal].values[39] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE40: {
                        objectEntityList[arrayVal].values[40] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE41: {
                        objectEntityList[arrayVal].values[41] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE42: {
                        objectEntityList[arrayVal].values[42] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE43: {
                        objectEntityList[arrayVal].values[43] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE44: {
                        objectEntityList[arrayVal].values[44] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE45: {
                        objectEntityList[arrayVal].values[45] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE46: {
                        objectEntityList[arrayVal].values[46] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE47: {
                        objectEntityList[arrayVal].values[47] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_STAGESTATE: stageMode = scriptEng.operands[i]; break;
                    case VAR_STAGEACTIVELIST: activeStageList = scriptEng.operands[i]; break;
                    case VAR_STAGELISTPOS: stageListPosition = scriptEng.operands[i]; break;
                    case VAR_STAGETIMEENABLED: timeEnabled = scriptEng.operands[i]; break;
                    case VAR_STAGEMILLISECONDS: stageMilliseconds = scriptEng.operands[i]; break;
                    case VAR_STAGESECONDS: stageSeconds = scriptEng.operands[i]; break;
                    case VAR_STAGEMINUTES: stageMinutes = scriptEng.operands[i]; break;
                    case VAR_STAGEACTNUM: actID = scriptEng.operands[i]; break;
                    case VAR_STAGEPAUSEENABLED: pauseEnabled = scriptEng.operands[i]; break;
                    case VAR_STAGELISTSIZE: break;
                    case VAR_STAGENEWXBOUNDARY1: newXBoundary1 = scriptEng.operands[i]; break;
                    case VAR_STAGENEWXBOUNDARY2: newXBoundary2 = scriptEng.operands[i]; break;
                    case VAR_STAGENEWYBOUNDARY1: newYBoundary1 = scriptEng.operands[i]; break;
                    case VAR_STAGENEWYBOUNDARY2: newYBoundary2 = scriptEng.operands[i]; break;
                    case VAR_STAGECURXBOUNDARY1:
                        if (curXBoundary1 != scriptEng.operands[i]) {
                            curXBoundary1 = scriptEng.operands[i];
                            newXBoundary1 = scriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGECURXBOUNDARY2:
                        if (curXBoundary2 != scriptEng.operands[i]) {
                            curXBoundary2 = scriptEng.operands[i];
                            newXBoundary2 = scriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGECURYBOUNDARY1:
                        if (curYBoundary1 != scriptEng.operands[i]) {
                            curYBoundary1 = scriptEng.operands[i];
                            newYBoundary1 = scriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGECURYBOUNDARY2:
                        if (curYBoundary2 != scriptEng.operands[i]) {
                            curYBoundary2 = scriptEng.operands[i];
                            newYBoundary2 = scriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGEDEFORMATIONDATA0: bgDeformationData0[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_STAGEDEFORMATIONDATA1: bgDeformationData1[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_STAGEDEFORMATIONDATA2: bgDeformationData2[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_STAGEDEFORMATIONDATA3: bgDeformationData3[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_STAGEWATERLEVEL: waterLevel = scriptEng.operands[i]; break;
                    case VAR_STAGEACTIVELAYER: activeTileLayers[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_STAGEMIDPOINT: tLayerMidPoint = scriptEng.operands[i]; break;
                    case VAR_STAGEPLAYERLISTPOS: playerListPos = scriptEng.operands[i]; break;
                    case VAR_STAGEDEBUGMODE: debugMode = scriptEng.operands[i]; break;
                    case VAR_STAGEENTITYPOS: objectEntityPos = scriptEng.operands[i]; break;
                    case VAR_SCREENCAMERAENABLED: cameraEnabled = scriptEng.operands[i]; break;
                    case VAR_SCREENCAMERATARGET: cameraTarget = scriptEng.operands[i]; break;
                    case VAR_SCREENCAMERASTYLE: cameraStyle = scriptEng.operands[i]; break;
                    case VAR_SCREENCAMERAX: cameraXPos = scriptEng.operands[i]; break;
                    case VAR_SCREENCAMERAY: cameraYPos = scriptEng.operands[i]; break;
                    case VAR_SCREENDRAWLISTSIZE: drawListEntries[arrayVal].listSize = scriptEng.operands[i]; break;
                    case VAR_SCREENXCENTER: break;
                    case VAR_SCREENYCENTER: break;
                    case VAR_SCREENXSIZE: break;
                    case VAR_SCREENYSIZE: break;
                    case VAR_SCREENXOFFSET: xScrollOffset = scriptEng.operands[i]; break;
                    case VAR_SCREENYOFFSET: yScrollOffset = scriptEng.operands[i]; break;
                    case VAR_SCREENSHAKEX: cameraShakeX = scriptEng.operands[i]; break;
                    case VAR_SCREENSHAKEY: cameraShakeY = scriptEng.operands[i]; break;
                    case VAR_SCREENADJUSTCAMERAY: cameraAdjustY = scriptEng.operands[i]; break;
                    case VAR_TOUCHSCREENDOWN: break;
                    case VAR_TOUCHSCREENXPOS: break;
                    case VAR_TOUCHSCREENYPOS: break;
                    case VAR_MUSICVOLUME: SetMusicVolume(scriptEng.operands[i]); break;
                    case VAR_MUSICCURRENTTRACK: break;
                    case VAR_MUSICPOSITION: break;
                    case VAR_INPUTDOWNUP: inputDown.up = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNDOWN: inputDown.down = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNLEFT: inputDown.left = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNRIGHT: inputDown.right = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONA: inputDown.A = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONB: inputDown.B = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONC: inputDown.C = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONX: inputDown.X = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONY: inputDown.Y = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONZ: inputDown.Z = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONL: inputDown.L = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNBUTTONR: inputDown.R = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNSTART: inputDown.start = scriptEng.operands[i]; break;
                    case VAR_INPUTDOWNSELECT: inputDown.select = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSUP: inputPress.up = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSDOWN: inputPress.down = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSLEFT: inputPress.left = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSRIGHT: inputPress.right = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONA: inputPress.A = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONB: inputPress.B = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONC: inputPress.C = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONX: inputPress.X = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONY: inputPress.Y = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONZ: inputPress.Z = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONL: inputPress.L = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSBUTTONR: inputPress.R = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSSTART: inputPress.start = scriptEng.operands[i]; break;
                    case VAR_INPUTPRESSSELECT: inputPress.select = scriptEng.operands[i]; break;
                    case VAR_MENU1SELECTION: gameMenu[0].selection1 = scriptEng.operands[i]; break;
                    case VAR_MENU2SELECTION: gameMenu[1].selection1 = scriptEng.operands[i]; break;
                    case VAR_MENU3SELECTION: gameMenu[2].selection1 = scriptEng.operands[i]; break;
                    case VAR_TILELAYERXSIZE: stageLayouts[arrayVal].xsize = scriptEng.operands[i]; break;
                    case VAR_TILELAYERYSIZE: stageLayouts[arrayVal].ysize = scriptEng.operands[i]; break;
                    case VAR_TILELAYERTYPE: stageLayouts[arrayVal].type = scriptEng.operands[i]; break;
                    case VAR_TILELAYERANGLE: {
                        int angle = scriptEng.operands[i] + 0x200;
                        if (scriptEng.operands[i] >= 0)
                            angle = scriptEng.operands[i];
                        stageLayouts[arrayVal].angle = angle & 0x1FF;
                        break;
                    }
                    case VAR_TILELAYERXPOS: stageLayouts[arrayVal].xpos = scriptEng.operands[i]; break;
                    case VAR_TILELAYERYPOS: stageLayouts[arrayVal].ypos = scriptEng.operands[i]; break;
                    case VAR_TILELAYERZPOS: stageLayouts[arrayVal].zpos = scriptEng.operands[i]; break;
                    case VAR_TILELAYERPARALLAXFACTOR: stageLayouts[arrayVal].parallaxFactor = scriptEng.operands[i]; break;
                    case VAR_TILELAYERSCROLLSPEED: stageLayouts[arrayVal].scrollSpeed = scriptEng.operands[i]; break;
                    case VAR_TILELAYERSCROLLPOS: stageLayouts[arrayVal].scrollPos = scriptEng.operands[i]; break;
                    case VAR_TILELAYERDEFORMATIONOFFSET: stageLayouts[arrayVal].deformationOffset = scriptEng.operands[i]; break;
                    case VAR_TILELAYERDEFORMATIONOFFSETW: stageLayouts[arrayVal].deformationOffsetW = scriptEng.operands[i]; break;
                    case VAR_HPARALLAXPARALLAXFACTOR: hParallax.parallaxFactor[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_HPARALLAXSCROLLSPEED: hParallax.scrollSpeed[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_HPARALLAXSCROLLPOS: hParallax.scrollPos[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_VPARALLAXPARALLAXFACTOR: vParallax.parallaxFactor[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_VPARALLAXSCROLLSPEED: vParallax.scrollSpeed[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_VPARALLAXSCROLLPOS: vParallax.scrollPos[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_SCENE3DVERTEXCOUNT: vertexCount = scriptEng.operands[i]; break;
                    case VAR_SCENE3DFACECOUNT: faceCount = scriptEng.operands[i]; break;
                    case VAR_SCENE3DPROJECTIONX: projectionX = scriptEng.operands[i]; break;
                    case VAR_SCENE3DPROJECTIONY: projectionY = scriptEng.operands[i]; break;
#if !RETRO_REV00
                    case VAR_SCENE3DFOGCOLOR: fogColor = scriptEng.operands[i]; break;
                    case VAR_SCENE3DFOGSTRENGTH: fogStrength = scriptEng.operands[i]; break;
#endif
                    case VAR_VERTEXBUFFERX: vertexBuffer[arrayVal].x = scriptEng.operands[i]; break;
                    case VAR_VERTEXBUFFERY: vertexBuffer[arrayVal].y = scriptEng.operands[i]; break;
                    case VAR_VERTEXBUFFERZ: vertexBuffer[arrayVal].z = scriptEng.operands[i]; break;
                    case VAR_VERTEXBUFFERU: vertexBuffer[arrayVal].u = scriptEng.operands[i]; break;
                    case VAR_VERTEXBUFFERV: vertexBuffer[arrayVal].v = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERA: faceBuffer[arrayVal].a = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERB: faceBuffer[arrayVal].b = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERC: faceBuffer[arrayVal].c = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERD: faceBuffer[arrayVal].d = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERFLAG: faceBuffer[arrayVal].flag = scriptEng.operands[i]; break;
                    case VAR_FACEBUFFERCOLOR: faceBuffer[arrayVal].color = scriptEng.operands[i]; break;
                    case VAR_SAVERAM: saveRAM[arrayVal] = scriptEng.operands[i]; break;
                    case VAR_ENGINESTATE: Engine.gameMode = scriptEng.operands[i]; break;
#if RETRO_REV00
                    case VAR_ENGINEMESSAGE: break;
#endif
                    case VAR_ENGINELANGUAGE: Engine.language = scriptEng.operands[i]; break;
                    case VAR_ENGINEONLINEACTIVE: Engine.onlineActive = scriptEng.operands[i]; break;
                    case VAR_ENGINESFXVOLUME:
                        sfxVolume = scriptEng.operands[i];
                        SetGameVolumes(bgmVolume, sfxVolume);
                        break;
                    case VAR_ENGINEBGMVOLUME:
                        bgmVolume = scriptEng.operands[i];
                        SetGameVolumes(bgmVolume, sfxVolume);
                        break;
                    case VAR_ENGINEPLATFORMID: break;
                    case VAR_ENGINETRIALMODE: Engine.trialMode = scriptEng.operands[i]; break;
#if !RETRO_REV00
                    case VAR_ENGINEDEVICETYPE: break;
#endif
#if RETRO_USE_HAPTICS
                    case VAR_HAPTICSENABLED: Engine.hapticsEnabled = scriptEng.operands[i]; break;
#endif
                    case VAR_ENGINETIMER: break;
                    case VAR_ERM_WHAT_THE_SIGMA: {
                        objectEntityList[arrayVal].values[100] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE48: {
                        objectEntityList[arrayVal].values[48] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE49: {
                        objectEntityList[arrayVal].values[49] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE50: {
                        objectEntityList[arrayVal].values[50] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE51: {
                        objectEntityList[arrayVal].values[51] = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTWIDTH: {
                        objectEntityList[arrayVal].width = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTHEIGHT: {
                        objectEntityList[arrayVal].height = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINKRED: {
                        objectEntityList[arrayVal].inkred = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINKGREEN: {
                        objectEntityList[arrayVal].inkgreen = scriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINKBLUE: {
                        objectEntityList[arrayVal].inkblue = scriptEng.operands[i];
                        break;
                    }
                    case VAR_CURRENTUNIX: { 
						time_t currentTime = time(NULL);
                        scriptEng.operands[i] = (int)currentTime;
						break;
                    }
                    case VAR_CURRENTLOCALUNIX: { 
						time_t currentTime = time(NULL);
						struct tm *localTime = localtime(&currentTime);
						struct tm *utcTime = gmtime(&currentTime);
						time_t localEpoch = mktime(localTime);
						time_t utcEpoch = mktime(utcTime);
						time_t localOffset = localEpoch - utcEpoch;
						time_t localUnixTime = currentTime + localOffset;
						scriptEng.operands[i] = (int)localUnixTime; 
						break;
                    }
                }
            }
            else if (opcodeType == SCRIPTVAR_INTCONST) { // int constant
                scriptDataPtr++;
            }
            else if (opcodeType == SCRIPTVAR_STRCONST) { // string constant
                int strLen = scriptData[scriptDataPtr++];
                for (int c = 0; c < strLen; ++c) {
                    switch (c % 4) {
                        case 0: break;
                        case 1: break;
                        case 2: break;
                        case 3: ++scriptDataPtr; break;
                        default: break;
                    }
                }
                scriptDataPtr++;
            }
        }
    }
}
