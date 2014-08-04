#pragma once

#include <vector>

class IMIDIListener {
public:
  virtual ~IMIDIListener() {};
  virtual void onMIDIEvent(std::vector<uint8_t> &message) = 0;
};
