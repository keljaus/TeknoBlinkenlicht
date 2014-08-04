#include "MidiController.h"
#if defined(WIN32)
#include <stdint.h>
#endif
// Get the port names of all MIDI input devices
std::vector<std::string> MidiController::getInPortNames() {
  std::vector<std::string> ports = std::vector<std::string>();

  // RtMidiIn constructor
  RtMidiIn* midi_input;
  try {
    midi_input = new RtMidiIn();
  }
  catch (RtMidiError &error) {
    // error.printMessage();
    // exit(EXIT_FAILURE);
    std::cerr << "Could not initialize RtMidi for Input" << std::endl;
    return ports;
  }

  // Check inputs
  uint8_t nPorts = midi_input->getPortCount();
  if (nPorts == 0) {
    std::cout << "No MIDI input sources available!" << std::endl;
    // exit(EXIT_FAILURE);
    return ports;
  }

  // std::cout << "There are " << nPorts << " MIDI input sources available.";
  // std::cout << std::endl << std::endl;
  for (uint8_t iPort = 0; iPort < nPorts; iPort++) {
    std::string portName;

    try {
      portName = midi_input->getPortName(iPort);
    }
    catch (RtMidiError &error) {
      // error.printMessage();
      // exit(EXIT_FAILURE);
      std::cerr << error.getType() << " - " << error.getMessage() << std::endl;
      return std::vector<std::string>();
    }

    // std::cout << "  Input Port #" << i+1 << ": " << portName << std::endl;
    ports.push_back(portName);
  }

  return ports;
}

// Get the port names of all MIDI output devices
std::vector<std::string> MidiController::getOutPortNames() {
  std::vector<std::string> ports = std::vector<std::string>();

  // RtMidiOut constructor
  RtMidiOut* midi_output;
  try {
    midi_output = new RtMidiOut();
  }
  catch (RtMidiError &error) {
    std::cerr << "Could not initialize RtMidi for Output" << std::endl;
    // error.printMessage();
    // exit(EXIT_FAILURE);
    return ports;
  }

  // Check outputs
  uint8_t nPorts = midi_output->getPortCount();
  if (nPorts == 0) {
    std::cout << "No MIDI output ports available!" << std::endl;
    // exit(EXIT_FAILURE);
    return ports;
  }

  // std::cout << "There are " << nPorts << " MIDI output ports available." << std::endl;
  for (uint8_t iPort = 0; iPort < nPorts; iPort++) {
    std::string portName;

    try {
      portName = midi_output->getPortName(iPort);
    }
    catch (RtMidiError &error) {
      // error.printMessage();
      // exit(EXIT_FAILURE);
      std::cerr << error.getType() << " - " << error.getMessage() << std::endl;
      return std::vector<std::string>();
    }

    // std::cout << "  Output Port #" << i+1 << ": " << portName << std::endl;
    ports.push_back(portName);
  }

  return ports;
}

// Constructor
MidiController::MidiController(
  std::string strMIDIIn,
  std::string strMIDIOut,
  IMIDIListener *listener) {

  // Store listener pointer
  this->listener = listener;

  // Open MIDI input
  this->numPortInID = -1;
  this->midiin = NULL;

  if (strMIDIIn.compare("") != 0) {
    this->midiin = new RtMidiIn();

    uint8_t nPorts = this->midiin->getPortCount();
    std::string strPortName;
    for (uint8_t iPort = 0; iPort < nPorts; iPort++) {
      // Skip unmatching ports
      strPortName = this->midiin->getPortName(iPort);
      if (strPortName.compare(strMIDIIn) != 0) {
        continue;
      }

      // Listen MIDI
      this->numPortInID = iPort;
      this->midiin->openPort(iPort);

      // Don't ignore sysex, timing, or active sensing messages.
      this->midiin->ignoreTypes(false, false, false);

      // Clear input buffer
      this->clearBuffer();

      // Print some nice output
      // std::cout << "Reading MIDI from port: " << iPort << std::endl;

      // Break once a matching MIDI device has been found/opened
      break;
    }
  }

  // Open MIDI output
  this->numPortOutID = -1;
  this->midiout = NULL;

  if (strMIDIOut.compare("") != 0) {
    this->midiout = new RtMidiOut();

    uint8_t nPorts = this->midiout->getPortCount();
    for (uint8_t iPort = 0; iPort < nPorts; iPort++) {
      // Skip non-matching
      if (this->midiout->getPortName(iPort).compare(strMIDIOut) != 0) {
        continue;
      }

      this->numPortOutID = iPort;
      this->midiout->openPort(iPort);

      // Break once found
      break;
    }
  }
}

// Destructor
MidiController::~MidiController() {
  this->midiin->closePort();
  this->midiout->closePort();
}

// Reset MIDI I/O
void MidiController::reset() {
  // Reset input port
  if (this->numPortInID >= 0) {
    this->midiin->closePort();
    this->midiin->openPort(this->numPortInID);

    // Clear buffer
    this->clearBuffer();
  }

  // Reset output port
  if (this->numPortOutID >= 0) {
    this->midiout->closePort();
    this->midiout->openPort(this->numPortOutID);
  }
}

// Clear input queue
void MidiController::clearBuffer() {
  if (!this->midiin) { return; }

  uint8_t nBytes;
  double stamp;
  std::vector<uint8_t> message;
  do {
    stamp = this->midiin->getMessage(&message);
    nBytes = message.size();
  } while (nBytes > 0);
}

void MidiController::processInput() {
  // Don't try to process input if MIDI in is not active
  if (!this->midiin) { return; }

  uint8_t nBytes;
  double stamp;
  std::vector<uint8_t> message;

  do {
    stamp = this->midiin->getMessage(&message);
    nBytes = message.size();
    if (nBytes < 1) { continue; }

    // Print output
    /*
    for (int i = 0; i < nBytes; i++) {
      std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
    }
    std::cout << "stamp = " << stamp << std::endl;
    */
    
    // Pass on data to listener
    if (this->listener) {
      this->listener->onMIDIEvent(message);
    }
  } while (nBytes > 0);
}

// Blinkenlicht!
void MidiController::sendOutput(uint8_t numLED, uint8_t color) {
  if (!this->midiout) { return; }

  std::vector<uint8_t> message;
  message.push_back(0x90);
  message.push_back(numLED);
  message.push_back(color);
  this->midiout->sendMessage(&message);
}
