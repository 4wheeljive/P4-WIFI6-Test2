#pragma once

#include "FastLED.h"
#include <ArduinoJson.h>

/* If you use more than ~4 characteristics, you need to increase numHandles in this file:
C:\Users\...\.platformio\packages\framework-arduinoespressif32\libraries\BLE\src\BLEServer.h
Setting numHandles = 60 has worked for 7 characteristics.  
*/

#ifndef DISABLE_BLE
   //#include <BLEDevice.h>
   //#include <BLEServer.h>
   //#include <BLEUtils.h>
   //#include <BLE2902.h>
#endif

#include <NimBLEDevice.h>

#include <string>

#include <FS.h>
#include "LittleFS.h"
#define FORMAT_LITTLEFS_IF_FAILED true 

bool displayOn = true;
extern bool debug;
//bool pauseAnimation = false;

uint8_t dummy = 1;

extern uint8_t PROGRAM;
extern uint8_t MODE;

using namespace fl;

 // PROGRAM/MODE FRAMEWORK ****************************************

  enum Program : uint8_t {
      RAINBOW = 0,
      WAVES = 1,
      BUBBLE = 2,
      DOTS = 3,
      FXWAVE2D = 4,
      RADII = 5,
      ANIMARTRIX = 6,
      TEST = 7,
      SYNAPTIDE = 8,
      CUBE = 9,
//      AUDIOREACTIVE = 10,
      PROGRAM_COUNT
  };

  // Program names in PROGMEM
  const char rainbow_str[] PROGMEM = "rainbow";
  const char waves_str[] PROGMEM = "waves";
  const char bubble_str[] PROGMEM = "bubble";
  const char dots_str[] PROGMEM = "dots";
  const char fxwave2d_str[] PROGMEM = "fxwave2d";
  const char radii_str[] PROGMEM = "radii";
  const char animartrix_str[] PROGMEM = "animartrix";
  const char test_str[] PROGMEM = "test";
  const char synaptide_str[] PROGMEM = "synaptide";
  const char cube_str[] PROGMEM = "cube";
//  const char audioreactive_str[] PROGMEM = "audioreactive";

  const char* const PROGRAM_NAMES[] PROGMEM = {
      rainbow_str, waves_str, bubble_str, dots_str,
      fxwave2d_str, radii_str, animartrix_str, test_str,
      synaptide_str, cube_str //audioreactive_str
  };

  // Mode names in PROGMEM
  const char palette_str[] PROGMEM = "palette";
  const char pride_str[] PROGMEM = "pride";
  const char octopus_str[] PROGMEM = "octopus";
  const char flower_str[] PROGMEM = "flower";
  const char lotus_str[] PROGMEM = "lotus";
  const char radial_str[] PROGMEM = "radial";
  const char lollipop_str[] PROGMEM = "lollipop";
  const char polarwaves_str[] PROGMEM = "polarwaves";
  const char spiralus_str[] PROGMEM = "spiralus";
  const char caleido1_str[] PROGMEM = "caleido1";
  const char coolwaves_str[] PROGMEM = "coolwaves";
  const char chasingspirals_str[] PROGMEM = "chasingspirals";
  const char complexkaleido6_str[] PROGMEM = "complexkaleido6";
  const char water_str[] PROGMEM = "water";
  const char experiment1_str[] PROGMEM = "experiment1";
  const char experiment2_str[] PROGMEM = "experiment2";
  const char fluffyblobs_str[] PROGMEM = "fluffyblobs";
   /*
   const char spectrumbars_str[] PROGMEM = "spectrumbars";
   const char radialspectrum_str[] PROGMEM = "radialspectrum";
   const char waveform_str[] PROGMEM = "waveform";
   const char vumeter_str[] PROGMEM = "vumeter";
   const char matrixrain_str[] PROGMEM = "matrixrain";
   const char fireeffect_str[] PROGMEM = "fireeffect";
   const char plasmawave_str[] PROGMEM = "plasmawave";
   */

  const char* const WAVES_MODES[] PROGMEM = {
      palette_str, pride_str
   };
  const char* const RADII_MODES[] PROGMEM = {
      octopus_str, flower_str, lotus_str, radial_str, lollipop_str
  };
  const char* const ANIMARTRIX_MODES[] PROGMEM = {
      polarwaves_str, spiralus_str, caleido1_str, coolwaves_str, chasingspirals_str,
      complexkaleido6_str, water_str, experiment1_str, experiment2_str, 
      fluffyblobs_str 
   };

  /*const char* const AUDIOREACTIVE_MODES[] PROGMEM = {
      spectrumbars_str, radialspectrum_str, waveform_str, vumeter_str, matrixrain_str, 
      fireeffect_str, plasmawave_str, 
  };*/  

  const uint8_t MODE_COUNTS[] = {0, 2, 0, 0, 0, 5, 10, 0, 0, 0};

   // Visualizer parameter mappings - PROGMEM arrays for memory efficiency
   // Individual parameter arrays for each visualizer
   const char* const RAINBOW_PARAMS[] PROGMEM = {};
   const char* const WAVES_PALETTE_PARAMS[] PROGMEM = {"speed", "hueIncMax", "blendFract", "brightTheta"};
   const char* const WAVES_PRIDE_PARAMS[] PROGMEM = {"speed", "hueIncMax", "blendFract", "brightTheta"};
   const char* const BUBBLE_PARAMS[] PROGMEM = {"speed", "scale", "movement"};
   const char* const DOTS_PARAMS[] PROGMEM = {"tail"};
   const char* const FXWAVE2D_PARAMS[] PROGMEM = {"speed", "speedLower", "dampLower", "speedUpper", "dampUpper", "blurGlobFact"};
   const char* const RADII_OCTOPUS_PARAMS[] PROGMEM = {"zoom", "angle", "speedInt"};
   const char* const RADII_FLOWER_PARAMS[] PROGMEM = {"zoom", "angle", "speedInt"};
   const char* const RADII_LOTUS_PARAMS[] PROGMEM = {"zoom", "angle", "speedInt"};
   const char* const RADII_RADIAL_PARAMS[] PROGMEM = {"zoom", "angle", "speedInt"};
   const char* const RADII_LOLLIPOP_PARAMS[] PROGMEM = {"zoom", "angle", "speedInt", "radius", "edge"};
   const char* const ANIMARTRIX_POLARWAVES_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "twist", "radius", "edge", "z", "ratBase", "ratDiff"};
   const char* const ANIMARTRIX_SPIRALUS_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff", "offBase", "offDiff"};
   const char* const ANIMARTRIX_CALEIDO1_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff", "offBase", "offDiff"};
   const char* const ANIMARTRIX_COOLWAVES_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff", "offBase", "offDiff"};
   const char* const ANIMARTRIX_CHASINGSPIRALS_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "twist", "radius", "edge", "ratBase", "ratDiff", "offBase", "offDiff"};
   const char* const ANIMARTRIX_COMPLEXKALEIDO6_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "twist", "radius", "edge", "z", "ratBase", "ratDiff"};
   const char* const ANIMARTRIX_WATER_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff"};
   const char* const ANIMARTRIX_EXPERIMENT1_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff"};
   const char* const ANIMARTRIX_EXPERIMENT2_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "ratBase", "ratDiff", "offBase", "offDiff"};
   const char* const ANIMARTRIX_FLUFFYBLOBS_PARAMS[] PROGMEM = {"speed", "zoom", "scale", "angle", "z", "radialSpeed", "linearSpeed", "z", "ratBase", "ratDiff" };
   const char* const SYNAPTIDE_PARAMS[] PROGMEM = {"bloomEdge", "decayBase", "decayChaos", "ignitionBase", "ignitionChaos", "neighborBase", "neighborChaos", "spatialDecay", "decayZones", "timeDrift", "pulse", "influenceBase", "influenceChaos", "entropyRate", "entropyBase", "entropyChaos"};
   const char* const CUBE_PARAMS[] PROGMEM = {"scale", "angleRateX", "angleRateY", "angleRateZ"};

   // Struct to hold visualizer name and parameter array reference
   struct VisualizerParamEntry {
      const char* visualizerName;
      const char* const* params;
      uint8_t count;
   };

   // String-based lookup table - mirrors JavaScript VISUALIZER_PARAMS
   // Can number values be replace by an array element count?
   const VisualizerParamEntry VISUALIZER_PARAM_LOOKUP[] PROGMEM = {
      {"rainbow", RAINBOW_PARAMS, 4},
      {"waves-palette", WAVES_PALETTE_PARAMS, 4},
      {"waves-pride", WAVES_PRIDE_PARAMS, 4},
      {"bubble", BUBBLE_PARAMS, 3},
      {"dots", DOTS_PARAMS, 1},
      {"fxwave2d", FXWAVE2D_PARAMS, 6},
      {"radii-octopus", RADII_OCTOPUS_PARAMS, 3},
      {"radii-flower", RADII_FLOWER_PARAMS, 3},
      {"radii-lotus", RADII_LOTUS_PARAMS, 3},
      {"radii-radial", RADII_RADIAL_PARAMS, 3},
      {"radii-lollipop", RADII_LOLLIPOP_PARAMS, 5},
      {"animartrix-polarwaves", ANIMARTRIX_POLARWAVES_PARAMS, 10},
      {"animartrix-spiralus", ANIMARTRIX_SPIRALUS_PARAMS, 9},
      {"animartrix-caleido1", ANIMARTRIX_CALEIDO1_PARAMS, 9},
      {"animartrix-coolwaves", ANIMARTRIX_COOLWAVES_PARAMS, 9},
      {"animartrix-chasingspirals", ANIMARTRIX_CHASINGSPIRALS_PARAMS, 11},
      {"animartrix-complexkaleido6", ANIMARTRIX_COMPLEXKALEIDO6_PARAMS, 10},
      {"animartrix-water", ANIMARTRIX_WATER_PARAMS, 7},
      {"animartrix-experiment1", ANIMARTRIX_EXPERIMENT1_PARAMS, 7},
      {"animartrix-experiment2", ANIMARTRIX_EXPERIMENT2_PARAMS, 9},
      {"animartrix-fluffyblobs", ANIMARTRIX_FLUFFYBLOBS_PARAMS, 10},
      {"synaptide", SYNAPTIDE_PARAMS, 16},
      {"cube", CUBE_PARAMS, 4}

   };

  class VisualizerManager {
  public:
      static String getVisualizerName(int programNum, int mode = -1) {
          if (programNum < 0 || programNum > PROGRAM_COUNT-1) return "";

          // Get program name from flash memory
          char progName[16];
          strcpy_P(progName,(char*)pgm_read_ptr(&PROGRAM_NAMES[programNum]));

          if (mode < 0 || MODE_COUNTS[programNum] == 0) {
              return String(progName);
          }

          // Get mode name
          const char* const* modeArray = nullptr;
          switch (programNum) {
              case WAVES: modeArray = WAVES_MODES; break;
              case RADII: modeArray = RADII_MODES; break;
              case ANIMARTRIX: modeArray = ANIMARTRIX_MODES; break;
              //case AUDIOREACTIVE: modeArray = AUDIOREACTIVE_MODES; break;
              default: return String(progName);
          }

          if (mode >= MODE_COUNTS[programNum]) return String(progName);

          char modeName[20];
          strcpy_P(modeName,(char*)pgm_read_ptr(&modeArray[mode]));

         //return String(progName) + "-" + String(modeName);
         String result = "";
         result += String(progName);
         result += "-";
         result += String(modeName);
         return result;
      }
      
      // Get parameter list based on visualizer name
      static const VisualizerParamEntry* getVisualizerParams(const String& visualizerName) {
          const int LOOKUP_SIZE = sizeof(VISUALIZER_PARAM_LOOKUP) / sizeof(VisualizerParamEntry);
          
          for (int i = 0; i < LOOKUP_SIZE; i++) {
              char entryName[32];
              strcpy_P(entryName, (char*)pgm_read_ptr(&VISUALIZER_PARAM_LOOKUP[i].visualizerName));
              
              if (visualizerName.equals(entryName)) {
                  return &VISUALIZER_PARAM_LOOKUP[i];
              }
          }
          return nullptr;
      }
  };  // class VisualizerManager


// Parameter control *************************************************************************************

uint8_t cBright = 75;
uint8_t cMapping = 0;
uint8_t cOverrideMapping = 0;

EaseType getEaseType(uint8_t value) {
    switch (value) {
        case 0: return EASE_NONE;
        case 1: return EASE_IN_QUAD;
        case 2: return EASE_OUT_QUAD;
        case 3: return EASE_IN_OUT_QUAD;
        case 4: return EASE_IN_CUBIC;
        case 5: return EASE_OUT_CUBIC;
        case 6: return EASE_IN_OUT_CUBIC;
        case 7: return EASE_IN_SINE;
        case 8: return EASE_OUT_SINE;
        case 9: return EASE_IN_OUT_SINE;
    }
    FL_ASSERT(false, "Invalid ease type");
    return EASE_NONE;
}

uint8_t cEaseSat = 0;
uint8_t cEaseLum = 0;

// PARAMETER CONTROLS ==================================================================

// Waves
bool rotateWaves = true; 
float cHueIncMax = 2500;
uint8_t cBlendFract = 128;
float cBrightTheta = 1;

// animARTrix/common
float cSpeed = 1.f;
float cZoom = 1.f;
float cScale = 1.f; 
float cAngle = 1.f; 
float cTwist = 1.f;
float cRadius = 1.0f;
float cRadialSpeed = 1;
float cLinearSpeed = 5;
float cEdge = 1.0f;
float cZ = 1.f; 
uint8_t cSpeedInt = 1;

// animARTrix
float cRatBase = 0.0f; 
float cRatDiff= 1.f; 
float cOffBase = 1.f; 
float cOffDiff = 1.f; 
uint8_t cFxIndex = 0;
uint8_t cColOrd = 0;

float cRed = 1.f; 
float cGreen = 1.f; 
float cBlue = 1.f;

//fxWave2d
float cSpeedLower = .16f;
float cDampLower = 8.f;
float cSpeedUpper = .24f;
float cDampUpper = 8.f;
float cBlurGlobFact = 1.f;
bool fancyTrigger = false;

//Bubble
float cMovement = 1.f;

//Dots
float cTail = 1.f;

//Synaptide
float cBloomEdge = 1.0f;
double cDecayBase = .95;
double cDecayChaos = .05;
double cIgnitionBase = .16;
double cIgnitionChaos = .05;
double cNeighborBase = .48;
double cNeighborChaos = .08;
float cSpatialDecay = 0.002f;
float cDecayZones = 1.0f;
float cTimeDrift = 1.0f;
float cPulse = 1.0f;
double cInfluenceBase = 0.7;
double cInfluenceChaos = 0.35;
uint16_t cEntropyRate = 120;
float cEntropyBase = 0.05f;
float cEntropyChaos = 0.15f;

//Cube
float cAngleRateX = 0.02f;
float cAngleRateY = 0.03f;
float cAngleRateZ = 0.01f;
bool cAngleFreezeX = false;
bool cAngleFreezeY = false;
bool cAngleFreezeZ = false;
/*
bool cRotateLinearX = true;
bool cRotateLinearY = true;
bool cRotateLinearZ = true;
bool cRotateRadialX = false;
bool cRotateRadialY = false;
bool cRotateRadialZ = false;
*/

bool Layer1 = true;
bool Layer2 = false;
bool Layer3 = true;
bool Layer4 = true;
bool Layer5 = false;
bool Layer6 = true;
bool Layer7 = true;
bool Layer8 = false;
bool Layer9 = true;

ArduinoJson::JsonDocument sendDoc;
ArduinoJson::JsonDocument receivedJSON;

//*******************************************************************************
//BLE CONFIGURATION *************************************************************

#ifndef DISABLE_BLE
   NimBLEServer* pServer = NULL;
   NimBLECharacteristic* pButtonCharacteristic = NULL;
   NimBLECharacteristic* pCheckboxCharacteristic = NULL;
   NimBLECharacteristic* pNumberCharacteristic = NULL;
   NimBLECharacteristic* pStringCharacteristic = NULL;
   NimBLEAdvertising* pAdvertising = NULL;

   bool deviceConnected = false;
   bool wasConnected = false;

   #define SERVICE_UUID                  	"19b10000-e8f2-537e-4f6c-d104768a1214"
   #define BUTTON_CHARACTERISTIC_UUID     "19b10001-e8f2-537e-4f6c-d104768a1214"
   #define CHECKBOX_CHARACTERISTIC_UUID   "19b10002-e8f2-537e-4f6c-d104768a1214"
   #define NUMBER_CHARACTERISTIC_UUID     "19b10003-e8f2-537e-4f6c-d104768a1214"
   #define STRING_CHARACTERISTIC_UUID     "19b10004-e8f2-537e-4f6c-d104768a1214"

#endif

//*******************************************************************************
// CONTROL FUNCTIONS ************************************************************

void startingPalette() {
   gCurrentPaletteNumber = random(0,gGradientPaletteCount-1);
   CRGBPalette16 gCurrentPalette( gGradientPalettes[gCurrentPaletteNumber] );
   gTargetPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
   gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
}

// UI update functions ***********************************************

void sendReceiptButton(uint8_t receivedValue) {
   #ifndef DISABLE_BLE
      pButtonCharacteristic->setValue(String(receivedValue).c_str());
      pButtonCharacteristic->notify();
      if (debug) {
         Serial.print("Button value received: ");
         Serial.println(receivedValue);
      }
   #endif
}

void sendReceiptCheckbox(String receivedID, bool receivedValue) {
  
   #ifndef DISABLE_BLE

      sendDoc.clear();
      sendDoc["id"] = receivedID;
      sendDoc["val"] = receivedValue;

      String jsonString;
      serializeJson(sendDoc, jsonString);

      pCheckboxCharacteristic->setValue(jsonString);
      
      pCheckboxCharacteristic->notify();
      
      if (debug) {
         Serial.print("Sent receipt for ");
         Serial.print(receivedID);
         Serial.print(": ");
         Serial.println(receivedValue);
      }

   #endif
}

void sendReceiptNumber(String receivedID, float receivedValue) {

    #ifndef DISABLE_BLE
      sendDoc.clear();
      sendDoc["id"] = receivedID;
      sendDoc["val"] = receivedValue;

      String jsonString;
      serializeJson(sendDoc, jsonString);

      pNumberCharacteristic->setValue(jsonString);
      
      pNumberCharacteristic->notify();
      
      if (debug) {
         Serial.print("Sent receipt for ");
         Serial.print(receivedID);
         Serial.print(": ");
         Serial.println(receivedValue);
      }
   #endif
}

void sendReceiptString(String receivedID, String receivedValue) {

    #ifndef DISABLE_BLE
   sendDoc.clear();
   sendDoc["id"] = receivedID;
   sendDoc["val"] = receivedValue;

   String jsonString;
   serializeJson(sendDoc, jsonString);

   pStringCharacteristic->setValue(jsonString);

   pStringCharacteristic->notify();
   
   if (debug) {
      Serial.print("Sent receipt for ");
      Serial.print(receivedID);
      Serial.print(": ");
      Serial.println(receivedValue);
   }
   #endif
}

//***********************************************************************
// PARAMETER/PRESET MANAGEMENT SYSTEM ("PPMS")
// X-Macro table 
#define PARAMETER_TABLE \
   X(uint8_t, OverrideMapping, 0) \
   X(uint8_t, ColOrd, 1.0f) \
   X(float, Speed, 1.0f) \
   X(float, Zoom, 1.0f) \
   X(float, Scale, 1.0f) \
   X(float, Angle, 1.0f) \
   X(float, Twist, 1.0f) \
   X(float, LinearSpeed, 5.0f) \
   X(float, RadialSpeed, 1.0f) \
   X(float, Radius, 1.0f) \
   X(float, Edge, 1.0f) \
   X(float, Z, 1.0f) \
   X(float, RatBase, 0.0f) \
   X(float, RatDiff, 1.0f) \
   X(float, OffBase, 1.0f) \
   X(float, OffDiff, 1.0f) \
   X(float, Red, 1.0f) \
   X(float, Green, 0.8f) \
   X(float, Blue, 0.6f) \
   X(uint8_t, SpeedInt, 1) \
   X(float, HueIncMax, 2500.0f) \
   X(uint8_t, BlendFract, 128) \
   X(float, BrightTheta, 1.0f) \
   X(float, SpeedLower, .16f) \
   X(float, DampLower, 8.0f) \
   X(float, SpeedUpper, .24f) \
   X(float, DampUpper, 6.0f) \
   X(float, BlurGlobFact, 1.0f) \
   X(float, Movement, 1.0f) \
   X(float, Tail, 1.0f) \
   X(uint8_t, EaseSat, 0) \
   X(uint8_t, EaseLum, 0) \
   X(float, BloomEdge, 1.0f) \
   X(double, DecayBase, .95) \
   X(double, DecayChaos, .04) \
   X(double, IgnitionBase, .16) \
   X(double, IgnitionChaos, .05) \
   X(double, NeighborBase, .48) \
   X(double, NeighborChaos, .06) \
   X(float, SpatialDecay, 0.002f) \
   X(float, DecayZones, 1.0f) \
   X(float, TimeDrift, 1.0f) \
   X(float, Pulse, 1.0f) \
   X(double, InfluenceBase, 0.7) \
   X(double, InfluenceChaos, 0.35) \
   X(uint16_t, EntropyRate, 180) \
   X(float, EntropyBase, 0.05f) \
   X(float, EntropyChaos, 0.15f) \
   X(float, AngleRateX, 0.02f) \
   X(float, AngleRateY, 0.03f) \
   X(float, AngleRateZ, 0.01f) \
   X(bool, AngleFreezeX, false) \
   X(bool, AngleFreezeY, false) \
   X(bool, AngleFreezeZ, false)

/*
   X(bool, RotateLinearX, false) \
   X(bool, RotateLinearY, false) \
   X(bool, RotateLinearZ, false) \
   X(bool, RotateRadialX, false) \
   X(bool, RotateRadialY, false) \
   X(bool, RotateRadialZ, false)
*/

// Auto-generated helper functions using X-macros
void captureCurrentParameters(ArduinoJson::JsonObject& params) {
    #define X(type, parameter, def) params[#parameter] = c##parameter;
    PARAMETER_TABLE
    #undef X
}

void applyCurrentParameters(const ArduinoJson::JsonObjectConst& params) {
    #define X(type, parameter, def) \
        if (!params[#parameter].isNull()) { \
            auto newValue = params[#parameter].as<type>(); \
            if (c##parameter != newValue) { \
                c##parameter = newValue; \
                sendReceiptNumber("in" #parameter, c##parameter); \
            } \
        }
    PARAMETER_TABLE
    #undef X
}


// Preset file persistence functions with JSON structure
bool savePreset(int presetNumber) {
    String filename = "/preset_";
    filename += presetNumber;
    filename += ".json";
    
    ArduinoJson::JsonDocument preset;
    preset["programNum"] = PROGRAM;
    if (MODE_COUNTS[PROGRAM] > 0) { 
      preset["modeNum"] = MODE;
    }    
    ArduinoJson::JsonObject params = preset["parameters"].to<ArduinoJson::JsonObject>();
    captureCurrentParameters(params);
    
    File file = LittleFS.open(filename, "w");
    if (!file) {
        Serial.print("Failed to save preset: ");
        Serial.println(filename);
        return false;
    }
    
    serializeJson(preset, file);
    file.close();
    
    Serial.print("Preset saved: ");
    Serial.println(filename);
    return true;
}

bool loadPreset(int presetNumber) {
    String filename = "/preset_";
    filename += presetNumber;
    filename += ".json";
    
    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.print("Failed to load preset: ");
        Serial.println(filename);
        return false;
    }
    
    ArduinoJson::JsonDocument preset;
    ArduinoJson::DeserializationError error = deserializeJson(preset, file);
    file.close();
    
    if (preset["programNum"].isNull() || preset["parameters"].isNull()) {
        Serial.print("Invalid preset format: ");
        Serial.println(filename);
        return false;
    }

    PROGRAM = (uint8_t)preset["programNum"];
    if (!preset["modeNum"].isNull()) {
      MODE = (uint8_t)preset["modeNum"];
    }
    //pauseAnimation = true;
    applyCurrentParameters(preset["parameters"]);
    //pauseAnimation = false;
    
    Serial.print("Preset loaded: ");
    Serial.println(filename);
    return true;
}

//***********************************************************************

void sendDeviceState() { 
   if (debug) {
      Serial.println("Sending device state...");
   }
   
   ArduinoJson::JsonDocument stateDoc;
   stateDoc["program"] = PROGRAM;
   stateDoc["mode"] = MODE;
   
   String currentVisualizer = VisualizerManager::getVisualizerName(PROGRAM, MODE); 
   
   // Get parameter list for current visualizer
   const VisualizerParamEntry* visualizerParams = VisualizerManager::getVisualizerParams(currentVisualizer);

   ArduinoJson::JsonObject params = stateDoc["parameters"].to<ArduinoJson::JsonObject>();

   if (debug) {
       String currentVisualizer = VisualizerManager::getVisualizerName(PROGRAM, MODE);
       Serial.print("Current visualizer: ");
       Serial.println(currentVisualizer);
       Serial.print("Found params: ");
       Serial.println(visualizerParams != nullptr ? "YES" : "NO");
       if (visualizerParams != nullptr) {
           Serial.print("Param count: ");
           Serial.println(visualizerParams->count);
       }
   }
   
   if (visualizerParams != nullptr) {
       // Loop through parameters for current visualizer
       for (uint8_t i = 0; i < visualizerParams->count; i++) {
           char paramName[32];
           strcpy_P(paramName, (char*)pgm_read_ptr(&visualizerParams->params[i]));
           
           if (debug) {
               Serial.print("Processing parameter: ");
               Serial.println(paramName);
           }
       }
   }

   // Add parameter values to JSON based on visualizer params
   for (uint8_t i = 0; i < visualizerParams->count; i++) {
       char paramName[32];
       strcpy_P(paramName, (char*)pgm_read_ptr(&visualizerParams->params[i]));
       
       bool paramFound = false;
       // Use X-macro to match parameter names and add values
       // Handle case-insensitive comparison for parameter names
       #define X(type, parameter, def) \
           if (strcasecmp(paramName, #parameter) == 0) { \
               params[paramName] = c##parameter; \
               if (debug) { \
                   Serial.print("Added parameter "); \
                   Serial.print(paramName); \
                   Serial.print(": "); \
                   Serial.println(c##parameter); \
               } \
               paramFound = true; \
           }
       PARAMETER_TABLE
       #undef X
       
       if (!paramFound) {
           Serial.print("Warning: Parameter not found in X-macro table: ");
           Serial.println(paramName);
       }
   }

   
   String stateJson;
   serializeJson(stateDoc, stateJson);
   sendReceiptString("deviceState", stateJson);
}


// Handle UI request functions ***********************************************

std::string convertToStdString(const String& flStr) {
   return std::string(flStr.c_str());
}

void processButton(uint8_t receivedValue) {

   sendReceiptButton(receivedValue);
      
   if (receivedValue < 20) { // Program selection
      PROGRAM = receivedValue;
      MODE = 0;
      displayOn = true;
   }
   
   if (receivedValue >= 20 && receivedValue < 40) { // Mode selection
      MODE = receivedValue - 20;
      cFxIndex = MODE;
      displayOn = true;
   }

   if (debug) {
      Serial.print("Current visualizer: ");
      Serial.println(VisualizerManager::getVisualizerName(PROGRAM, MODE));
   }

   //if (receivedValue == 91) { updateUI(); }
   if (receivedValue == 92) { sendDeviceState(); }
   if (receivedValue == 94) { fancyTrigger = true; }
   //if (receivedValue == 95) { resetAll(); }
   
   if (receivedValue == 98) { displayOn = true; }
   if (receivedValue == 99) { displayOn = false; }

   if (receivedValue >= 101 && receivedValue <= 150) { 
      uint8_t savedPreset = receivedValue - 100;  
      savePreset(savedPreset); 
   }

   if (receivedValue >= 151 && receivedValue <= 200) { 
       uint8_t presetToLoad = receivedValue - 150;
       if (loadPreset(presetToLoad)) {
           loadPreset(presetToLoad);
           Serial.print("Loaded preset: ");
           Serial.println(presetToLoad);
       }
   }
}

//*****************************************************************************

void processNumber(String receivedID, float receivedValue ) {

   sendReceiptNumber(receivedID, receivedValue);
  
   if (receivedID == "inBright") {
      cBright = receivedValue;
      BRIGHTNESS = cBright;
      FastLED.setBrightness(BRIGHTNESS);
   };


   if (receivedID == "inPalNum") {
      uint8_t newPalNum = receivedValue;
      gTargetPalette = gGradientPalettes[ newPalNum ];
      if(debug) {
         Serial.print("newPalNum: ");
         Serial.println(newPalNum);
      }
   };
  
   // Auto-generated custom parameter handling using X-macros
   #define X(type, parameter, def) \
       if (receivedID == "in" #parameter) { c##parameter = receivedValue; return; }
   PARAMETER_TABLE
   #undef X

}

void processCheckbox(String receivedID, bool receivedValue ) {
   
   sendReceiptCheckbox(receivedID, receivedValue);
   
   if (receivedID == "cx10") {rotateWaves = receivedValue;};
   if (receivedID == "cxLayer1") {Layer1 = receivedValue;};
   if (receivedID == "cxLayer2") {Layer2 = receivedValue;};
   if (receivedID == "cxLayer3") {Layer3 = receivedValue;};
   if (receivedID == "cxLayer4") {Layer4 = receivedValue;};
   if (receivedID == "cxLayer5") {Layer5 = receivedValue;};
   if (receivedID == "cxLayer6") {Layer6 = receivedValue;};
   if (receivedID == "cxLayer7") {Layer7 = receivedValue;};
   if (receivedID == "cxLayer8") {Layer8 = receivedValue;};
   if (receivedID == "cxLayer9") {Layer9 = receivedValue;};

   if (receivedID == "cx11") {mappingOverride = receivedValue;};
   /*
   if (receivedID == "cx12") {cEnableAudio = receivedValue;};
   if (receivedID == "cx13") {cAutoGain  = receivedValue;};
   if (receivedID == "cx14") {cBeatFlash = receivedValue;};
   if (receivedID == "cx15") {cMirrorMode = receivedValue;};
   if (receivedID == "cx16") {cBeatDetect = receivedValue;};
   */

   if (receivedID == "cx21") {cAngleFreezeX = receivedValue;};
   if (receivedID == "cx22") {cAngleFreezeY = receivedValue;};
   if (receivedID == "cx23") {cAngleFreezeZ = receivedValue;};
   /*
   if (receivedID == "cx24") {cRotateLinearX = receivedValue;};
   if (receivedID == "cx25") {cRotateLinearY = receivedValue;};
   if (receivedID == "cx26") {cRotateLinearZ = receivedValue;};
   if (receivedID == "cx27") {cRotateRadialX = receivedValue;};
   if (receivedID == "cx28") {cRotateRadialY = receivedValue;};
   if (receivedID == "cx29") {cRotateRadialZ = receivedValue;};
   */
  
}

void processString(String receivedID, String receivedValue ) {
   sendReceiptString(receivedID, receivedValue);
}

//*******************************************************************************
// CALLBACKS ********************************************************************

#ifndef DISABLE_BLE

   class MyServerCallbacks: public NimBLEServerCallbacks {
   void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
      deviceConnected = true;
      wasConnected = true;
      if (debug) {Serial.println("Device Connected");}
   };

   void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
      deviceConnected = false;
      wasConnected = true;
   }
   };

   class ButtonCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
      void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {

         String value = String(pCharacteristic->getValue().c_str());
         if (value.length() > 0) {
            
            uint8_t receivedValue = value[0];
            
            if (debug) {
               Serial.print("Button value received: ");
               Serial.println(receivedValue);
            }
            
            processButton(receivedValue);
         
         }
      }
};

      class CheckboxCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
         void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
      
            String receivedBuffer = String(pCharacteristic->getValue().c_str());
      
            if (receivedBuffer.length() > 0) {
                        
               if (debug) {
                  Serial.print("Received buffer: ");
                  Serial.println(receivedBuffer);
               }
            
               ArduinoJson::deserializeJson(receivedJSON, receivedBuffer);
               String receivedID = receivedJSON["id"] ;
               bool receivedValue = receivedJSON["val"];
            
               if (debug) {
                  Serial.print(receivedID);
                  Serial.print(": ");
                  Serial.println(receivedValue);
               }
            
               processCheckbox(receivedID, receivedValue);
            
            }
         }
      };

      class NumberCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
         void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
            
            String receivedBuffer = String(pCharacteristic->getValue().c_str());
            
            if (receivedBuffer.length() > 0) {
            
               if (debug) {
                  Serial.print("Received buffer: ");
                  Serial.println(receivedBuffer);
               }
            
               ArduinoJson::deserializeJson(receivedJSON, receivedBuffer);
               String receivedID = receivedJSON["id"] ;
               float receivedValue = receivedJSON["val"];
            
               if (debug) {
                  Serial.print(receivedID);
                  Serial.print(": ");
                  Serial.println(receivedValue);
               }
            
               processNumber(receivedID, receivedValue);
            }
         }
      };

      class StringCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
         void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
            
            String receivedBuffer = String(pCharacteristic->getValue().c_str());
            
            if (receivedBuffer.length() > 0) {
            
               if (debug) {
                  Serial.print("Received buffer: ");
                  Serial.println(receivedBuffer);
               }
            
               ArduinoJson::deserializeJson(receivedJSON, receivedBuffer);
               String receivedID = receivedJSON["id"] ;
               String receivedValue = receivedJSON["val"];
            
               if (debug) {
                  Serial.print(receivedID);
                  Serial.print(": ");
                  Serial.println(receivedValue);
               }
            
               processString(receivedID, receivedValue);
            }
         }
      };

#endif

//*******************************************************************************
// BLE SETUP FUNCTION ***********************************************************

#ifndef DISABLE_BLE

   void bleSetup() {

      NimBLEDevice::init("Aurora Portal");

      pServer = NimBLEDevice::createServer();
      pServer->setCallbacks(new MyServerCallbacks());

      NimBLEService *pService = pServer->createService(SERVICE_UUID);

      pButtonCharacteristic = pService->createCharacteristic(
                        BUTTON_CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::READ |
                        NIMBLE_PROPERTY::NOTIFY
                     );
      pButtonCharacteristic->setCallbacks(new ButtonCharacteristicCallbacks());
      pButtonCharacteristic->setValue(String(dummy).c_str());
      
      pCheckboxCharacteristic = pService->createCharacteristic(
                        CHECKBOX_CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::READ |
                        NIMBLE_PROPERTY::NOTIFY
                     );
      pCheckboxCharacteristic->setCallbacks(new CheckboxCharacteristicCallbacks());
      pCheckboxCharacteristic->setValue(String(dummy).c_str());
      
      pNumberCharacteristic = pService->createCharacteristic(
                        NUMBER_CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::READ |
                        NIMBLE_PROPERTY::NOTIFY
                     );
      pNumberCharacteristic->setCallbacks(new NumberCharacteristicCallbacks());
      pNumberCharacteristic->setValue(String(dummy).c_str());
      
      pStringCharacteristic = pService->createCharacteristic(
                        STRING_CHARACTERISTIC_UUID,
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::READ |
                        NIMBLE_PROPERTY::NOTIFY
                     );
      pStringCharacteristic->setCallbacks(new StringCharacteristicCallbacks());
      pStringCharacteristic->setValue(String(dummy).c_str());
      

      //**********************************************************

      pService->start();

      pAdvertising = NimBLEDevice::getAdvertising();
      pAdvertising->addServiceUUID(SERVICE_UUID);

      // Set up advertisement data with device name for Web Bluetooth compatibility
      NimBLEAdvertisementData advertisementData;
      advertisementData.setName("Aurora Portal");
      advertisementData.setCompleteServices(NimBLEUUID(SERVICE_UUID));
      pAdvertising->setAdvertisementData(advertisementData);

      // Set up scan response data
      NimBLEAdvertisementData scanResponseData;
      scanResponseData.setName("Aurora Portal");
      pAdvertising->setScanResponseData(scanResponseData);

      //pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
      pAdvertising->start();
      if (debug) {Serial.println("Waiting a client connection to notify...");}
}
#endif
