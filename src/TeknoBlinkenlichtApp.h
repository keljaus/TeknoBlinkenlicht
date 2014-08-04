#pragma once

// C++ includes
#include <list>
#include <vector>
#include <map>
#if defined(WIN32)
  #include <stdint.h>
  #include <array>
#endif

// Boost includes
#include <boost/foreach.hpp>

// Cinder includes
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Display.h"
#include "cinder/Json.h"
#include "cinder/Easing.h"
#include "cinder/Timeline.h"
#include "cinder/Serial.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

// Project includes
#include "MidiController.h"
#include "timers.h"

// Debug variable
#define DEBUG_OUTPUT 1

// Possible game colorz
#define COLOR_NONE 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_BLUE 3
#define COLOR_YELLOW 4
#define COLOR_LILAC 5
#define COLOR_ORANGE 6

// Arduino config
#define ARDUINO_PORT "cu.usbmodemfa131"
#define ARDUINO_BAUD_RATE 9600

// MIDI controller name
#define MIDI_CONTROLLER_IN "Launch Control"
#define MIDI_CONTROLLER_OUT "Launch Control"

// MIDI Controller LED addresses
#define MIDI_LED_1 0x09
#define MIDI_LED_2 0x0A
#define MIDI_LED_3 0x0B
#define MIDI_LED_4 0x0C
#define MIDI_LED_5 0x19
#define MIDI_LED_6 0x1A
#define MIDI_LED_7 0x1B
#define MIDI_LED_8 0x1C

// MIDI Controller LED light IDs
#define MIDI_LED_OFF 0x0C
#define MIDI_LED_GREEN 0x3C
#define MIDI_LED_AMBER 0x3F
#define MIDI_LED_RED 0x0F

// Helper konb states
#define KNOB_UNLOADED 0
#define KNOB_LOADED 1
#define KNOB_USED 2

// Time constants
#define TIME_DECREMENT 0.1f
#define MAX_COLOR_TIME 5.0f
#define MIN_COLOR_TIME 0.5f

// Helpers
#define HELPER_TIME 5.0f
#define HELPER_GHOST 1.0f

// Spede increase
#define SPEDE_INCREASE 0.05f

// using namespace ci;
// using namespace ci::app;

class TeknoBlinkenlichtApp : public ci::app::AppBasic, public IMIDIListener {
private:
  microTime mStartTime;

  // Background
  ci::gl::Texture tBackground;

  // Spede
  ci::gl::Texture tSpede;

  // Arduino
  ci::Serial::Device devArduino;
  ci::Serial *serArduino;

  // MIDI controller
  MidiController *controller;

  // MIDI LEDs
  std::map<std::string, ci::gl::Texture> mapLEDs;
  std::map<uint8_t, uint8_t> colorPairs;
  uint8_t arrColors[6];
  int8_t numButtonsPressed;

  // MIDI knob stuff
  ci::gl::Texture tKnob;
  uint8_t rotations[12];

  // Status variables
  microTime mColorTime;
  float fTimeLimit;
  int8_t clrActive;

  microTime mTrollTime;
  float fSpedeEnergy;

  uint8_t helpers[12];
  int8_t numHelpersUsed;

  bool bReadyToChange;

  // Color functions
  void initColors();
  void clearColor();
  void changeColor();

  // Game functions
  void resetGame();
  int setupDone = 0;
  bool bGameOver;

public:
  #if defined(DEBUG_OUTPUT)
    void mouseDown(ci::app::MouseEvent event);
  #endif

  void onMIDIEvent(std::vector<uint8_t> &message);

  void prepareSettings(ci::app::AppBasic::Settings *settings);
	void setup();
	void update();
	void draw();
  void shutdown();
};
