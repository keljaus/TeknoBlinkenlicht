#pragma once

// C++ includes
#include <iostream>
#include <cstdlib>
#include <ctime>

#if defined(WIN32)
#include <stdint.h>
#endif

// 3rd party library includes
#include "RtMidi.h"
#include "IMIDIListener.h"

class MidiController {
private:
  // MIDI I/O
  RtMidiIn* midiin;
  int8_t numPortInID;

  RtMidiOut* midiout;
  int8_t numPortOutID;

  // MIDI input event listener
  IMIDIListener *listener;

public:
  // Constructor
  MidiController(
    std::string strMIDIIn = "",
    std::string strMIDIOut = "",
    IMIDIListener *listener = NULL);

  // Destructor
  virtual ~MidiController();

  // Static device query functions
  static std::vector<std::string> getInPortNames();
  static std::vector<std::string> getOutPortNames();

  // MIDI I/O
  void reset();
  void clearBuffer();
  void processInput();
  void sendOutput(uint8_t numLED, uint8_t color);
};
