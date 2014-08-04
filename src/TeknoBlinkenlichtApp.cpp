#include "TeknoBlinkenlichtApp.h"

// Prepare application; screen size, title, etc. etc.
void TeknoBlinkenlichtApp::prepareSettings(ci::app::AppBasic::Settings *settings) {
  std::cout << "Tekno Blinkenlicht" << std::endl;
  settings->setTitle("Tekno Blinkenlicht");

  // 16:9 baby, yeah!
  settings->setWindowSize(1024, 576);
  // settings->setFullScreen();

  settings->setFrameRate(60);
  settings->enableMultiTouch(false);
}

// Setup application data
void TeknoBlinkenlichtApp::setup() {
  // Load graphics
  this->tBackground = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("background.png")));
  this->tSpede = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("spede.png")));
  this->tKnob = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("potikka.png")));

  this->mapLEDs["blue"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-blue.png")));
  this->mapLEDs["green"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-green.png")));
  this->mapLEDs["lilac"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-lilac.png")));
  this->mapLEDs["orange"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-orange.png")));
  this->mapLEDs["red"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-red.png")));
  this->mapLEDs["yellow"] = ci::gl::Texture(ci::loadImage(ci::app::loadAsset("midi-yellow.png")));

  // Try to initialize Arduino
  this->devArduino = ci::Serial::findDeviceByName(ARDUINO_PORT);
  if (this->devArduino.getName().compare(ARDUINO_PORT) != 0) {
    std::cerr << "Le game needs an Arduino. Check port: " << ARDUINO_PORT << std::endl;
    // exit(EXIT_FAILURE);
    this->serArduino = NULL;
  } else {
    #if defined(DEBUG_OUTPUT)
      std::cout << "Arduino found on port: " << ARDUINO_PORT << std::endl;
    #endif

    // Open serial connection to Arduino
    this->serArduino = new ci::Serial(this->devArduino, ARDUINO_BAUD_RATE);
  }

  // Initialize MIDI I/O
  this->controller = new MidiController(MIDI_CONTROLLER_IN, MIDI_CONTROLLER_OUT, this);

  this->bGameOver = false;
  this->mStartTime = getTimeNow();
}

void TeknoBlinkenlichtApp::resetGame() {
  // Reset game state
  this->fSpedeEnergy = 0.0f;
  this->fTimeLimit = MAX_COLOR_TIME + TIME_DECREMENT;
  this->bGameOver = false;

  // MIDI controller stuff
  this->controller->reset();

  this->initColors();

  for (int iKnob = 0; iKnob < 12; iKnob++) {
    this->helpers[iKnob] = KNOB_UNLOADED;
  }
  this->numHelpersUsed = 0;

  this->numButtonsPressed = 0;
  this->bReadyToChange = false;

  // Pick first color
  this->changeColor();
}

void TeknoBlinkenlichtApp::initColors() {
  // Screen controller LEDs
  this->arrColors[0] = COLOR_GREEN;
  this->arrColors[1] = COLOR_YELLOW;
  this->arrColors[2] = COLOR_ORANGE;
  this->arrColors[3] = COLOR_RED;
  this->arrColors[4] = COLOR_LILAC;
  this->arrColors[5] = COLOR_BLUE;

  // MIDI controller LEDs
  this->controller->sendOutput(MIDI_LED_1, MIDI_LED_AMBER);
  this->controller->sendOutput(MIDI_LED_2, MIDI_LED_AMBER);
  this->controller->sendOutput(MIDI_LED_3, MIDI_LED_AMBER);
  this->controller->sendOutput(MIDI_LED_4, MIDI_LED_AMBER);
  this->controller->sendOutput(MIDI_LED_5, MIDI_LED_AMBER);
  this->controller->sendOutput(MIDI_LED_6, MIDI_LED_AMBER);
}

// Turn off Arduino and MIDI LEDs
void TeknoBlinkenlichtApp::clearColor() {
  if (this->serArduino) {
    this->serArduino->writeByte(COLOR_NONE);
  }

  this->controller->sendOutput(MIDI_LED_1, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_2, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_3, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_4, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_5, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_6, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_7, MIDI_LED_OFF);
  this->controller->sendOutput(MIDI_LED_8, MIDI_LED_OFF);
}

// Get a new color
void TeknoBlinkenlichtApp::changeColor() {
  if (this->setupDone == 0) { return; }
  if (this->bGameOver) { return; }

  // Randomize a new LED color
  uint8_t newColor;
  do {
    newColor = (random() % 6) + 1;
  } while (newColor == this->clrActive);
  this->clrActive = newColor;

  #if defined(DEBUG_OUTPUT)
    switch (this->clrActive) {
      case COLOR_RED:
        std::cout << "New color RED, press GREEN" << std::endl;
        break;

      case COLOR_GREEN:
        std::cout << "New color GREEN, press RED" << std::endl;
        break;

      case COLOR_BLUE:
        std::cout << "New color BLUE, press ORANGE" << std::endl;
        break;

      case COLOR_YELLOW:
        std::cout << "New color YELLOW, press LILAC" << std::endl;
        break;

      case COLOR_LILAC:
        std::cout << "New color LILAC, press YELLOW" << std::endl;
        break;

      case COLOR_ORANGE:
        std::cout << "New color ORANGE, press BLUE" << std::endl;
        break;
    }
  #endif

  // Send color message to Arduino
  if (this->serArduino) {
    this->serArduino->writeByte(this->clrActive);
  }

  // Start timing color
  this->mColorTime = getTimeNow();
  if (this->fTimeLimit > MIN_COLOR_TIME) {
    this->fTimeLimit -= TIME_DECREMENT;
  }

  // Prevent double changes when player lifts finger.
  // Occurs if a timed change happens while a button is being pressed.
  this->bReadyToChange = false;
}

#if defined(DEBUG_OUTPUT)
  // Some unrelated Cinder functionality tests
  void TeknoBlinkenlichtApp::mouseDown(ci::app::MouseEvent event) {
    //JeiSON test
    ci::JsonTree jTree;
    jTree.addChild(ci::JsonTree("key", 0));
    jTree["key"].addChild(ci::JsonTree("another", "value"));
    std::cout << jTree << std::endl;

    // Query displays
    std::vector<ci::DisplayRef> displays = ci::Display::getDisplays();
    ci::DisplayRef display;
    uint8_t numDisplay = 0;
    std::cout << "Displays: " << displays.size() << std::endl;
    BOOST_FOREACH(display, displays) {
      std::cout << "- display #" << (int)numDisplay << ": " << display->getSize() << std::endl;
      numDisplay++;
    }

    // Query Serial devices
    const std::vector<ci::Serial::Device> &devices = ci::Serial::getDevices();
    if (devices.size() < 1) {
      std::cout << "No Serial devices available" << std::endl;
    } else {
      std::cout << "Serial devices: " << devices.size() << std::endl;
      ci::Serial::Device device;
      BOOST_FOREACH(device, devices) {
        // if (device.getName().compare(ARDUINO_PORT) == 0) {
          std::cout << "- " << device.getName() << std::endl;
        // }
      }
    }

    // Query MIDI in devices
    std::vector<std::string> vecMIDIIn = MidiController::getInPortNames();
    std::string strController;
    int numDevice = 0;
    BOOST_FOREACH(strController, vecMIDIIn) {
      std::cout << "Midi in device #" << numDevice << ": " << strController << std::endl;
      numDevice++;
    }

    // Query MIDI out devices
    std::vector<std::string> vecMIDIOut = MidiController::getOutPortNames();
    numDevice = 0;
    BOOST_FOREACH(strController, vecMIDIOut) {
      std::cout << "Midi out device #" << numDevice << ": " << strController << std::endl;
      numDevice++;
    }
  }
#endif

// Process incoming MIDI events
void TeknoBlinkenlichtApp::onMIDIEvent(std::vector<uint8_t> &message) {
  #if defined(DEBUG_OUTPUT)
    std::cout << "MIDI event: ";
    for (int iByte = 0; iByte < message.size(); iByte++) {
       std::cout << (int)message[iByte] << " ";
    }
    std::cout << std::endl;
  #endif

  // Do not really process events until game is properly setup
  if (this->setupDone == 0) { return; }

  // MIDI potentiometers a.k.a. le knobs
  if (message[0] == 176) {
    // Get knob number by MIDI code
    int8_t iKnob = -1;
    if ((message[1] >= 21) && (message[1] <= 26)) {
      iKnob = message[1] - 21;
    }

    if ((message[1] >= 41) && (message[1] <= 46)) {
      iKnob = message[1] - 41 + 6;
    }

    if (iKnob >= 0) {
      this->rotations[iKnob] = message[2];

      switch (this->helpers[iKnob]) {
        case KNOB_UNLOADED:
          if (message[2] == 127) {
            this->helpers[iKnob] = KNOB_LOADED;
          }
          break;

        case KNOB_LOADED:
          if (message[2] == 0) {
            this->helpers[iKnob] = KNOB_USED;

            // Every used knob gives only half of the help of the previous did
            float fHelperDivider = powf(2.0f, (float)this->numHelpersUsed);
            this->numHelpersUsed++;

            // Reduce Spede
            this->fSpedeEnergy -= HELPER_GHOST / fHelperDivider;
            if (this->fSpedeEnergy < 0.0f) {
              this->fSpedeEnergy = 0.0f;
            }

            // Increase color time
            /*
            this->fTimeLimit += HELPER_TIME / fHelperDivider;
            if (this->fTimeLimit > MAX_COLOR_TIME) {
              this->fTimeLimit = MAX_COLOR_TIME;
            }
            */
          }
          break;

        case KNOB_USED:
          break;
      }
    }
  }

  // Check if game over
  if (this->bGameOver) { return; }

  // Handle MIDI pressed
  if (message[0] == 144) {
    switch (message[1]) {
      case 9:
        // GREEN -> RED
        this->arrColors[0] = COLOR_RED;

        if (this->clrActive == COLOR_RED) {
          this->controller->sendOutput(MIDI_LED_1, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_1, MIDI_LED_RED);
        }

        break;

      case 10:
        // YELLOW -> LILAC
        this->arrColors[1] = COLOR_LILAC;

        if (this->clrActive == COLOR_LILAC) {
          this->controller->sendOutput(MIDI_LED_2, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_2, MIDI_LED_RED);
        }

        break;

      case 11:
        // ORANGE -> BLUE
        this->arrColors[2] = COLOR_BLUE;

        if (this->clrActive == COLOR_BLUE) {
          this->controller->sendOutput(MIDI_LED_3, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_3, MIDI_LED_RED);
        }

        break;

      case 12:
        // RED -> GREEN
        this->arrColors[3] = COLOR_GREEN;

        if (this->clrActive == COLOR_GREEN) {
          this->controller->sendOutput(MIDI_LED_4, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_4, MIDI_LED_RED);
        }

        break;

      case 25:
        // LILAC -> YELLOW
        this->arrColors[4] = COLOR_YELLOW;

        if (this->clrActive == COLOR_YELLOW) {
          this->controller->sendOutput(MIDI_LED_5, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_5, MIDI_LED_RED);
        }

        break;

      case 26:
        // BLUE -> ORANGE
        this->arrColors[5] = COLOR_ORANGE;

        if (this->clrActive == COLOR_ORANGE) {
          this->controller->sendOutput(MIDI_LED_6, MIDI_LED_GREEN);
        } else {
          this->fSpedeEnergy += SPEDE_INCREASE;
          this->controller->sendOutput(MIDI_LED_6, MIDI_LED_RED);
        }

        break;

      default:
        break;
    }

    // Allow color change when only one button is pressed
    this->numButtonsPressed++;
    if (this->numButtonsPressed == 1) {
      this->bReadyToChange = true;
    }
  }

  // Handle MIDI up
  if (message[0] == 128) {
    switch (message[1]) {
      case 9:
        this->arrColors[0] = COLOR_GREEN;
        this->controller->sendOutput(MIDI_LED_1, MIDI_LED_AMBER);
        break;

      case 10:
        this->arrColors[1] = COLOR_YELLOW;
        this->controller->sendOutput(MIDI_LED_2, MIDI_LED_AMBER);
        break;

      case 11:
        this->arrColors[2] = COLOR_ORANGE;
        this->controller->sendOutput(MIDI_LED_3, MIDI_LED_AMBER);
        break;

      case 12:
        this->arrColors[3] = COLOR_RED;
        this->controller->sendOutput(MIDI_LED_4, MIDI_LED_AMBER);
        break;

      case 25:
        this->arrColors[4] = COLOR_LILAC;
        this->controller->sendOutput(MIDI_LED_5, MIDI_LED_AMBER);
        break;

      case 26:
        this->arrColors[5] = COLOR_BLUE;
        this->controller->sendOutput(MIDI_LED_6, MIDI_LED_AMBER);
        break;

      default:
        break;
    }

    this->numButtonsPressed--;
    if (this->numButtonsPressed < 0) {
      this->numButtonsPressed = 0;
    }

    if (this->numButtonsPressed == 0) {
      if (this->bReadyToChange == true) {
        this->changeColor();
      }
    }
  }
}

// Screen update stuff
void TeknoBlinkenlichtApp::update() {
  // Process MIDI input
  this->controller->processInput();

  // Exec until game setup
  if (this->setupDone == 0) {
    if (getTimeDiffNow(this->mStartTime) < 2.0) {
      return;
    }

    this->setupDone = 1;
    this->resetGame();
    this->initColors();
    this->changeColor();
  }

  // Check for a timed change
  if (this->bGameOver == false) {
    if (this->fTimeLimit < getTimeDiffNow(this->mColorTime)) {
      this->fSpedeEnergy += SPEDE_INCREASE;

      if (this->fSpedeEnergy < 1.0f) {
        this->changeColor();
      }
    }
  }

  // Check if game over
  if (this->bGameOver == false) {
    if (this->fSpedeEnergy >= 1.0f) {
      this->bGameOver = true;
      this->mTrollTime = getTimeNow();
    }
  }

  if (this->bGameOver) {
    float fTimeDiff = getTimeDiffNow(this->mTrollTime);
    if (fTimeDiff >= 10.0f) {
      this->resetGame();
    } else {
      // MIDI LED trolling
      uint8_t LEDs[6] = {MIDI_LED_1, MIDI_LED_2, MIDI_LED_3, MIDI_LED_4, MIDI_LED_5, MIDI_LED_6};

      int iTimeDiff = (int)ceilf(fTimeDiff * 4.0f);
      uint8_t numLEDcolor = (iTimeDiff % 2 == 0) ? MIDI_LED_AMBER : MIDI_LED_RED;

      for (uint8_t iLED = 0; iLED < 6; iLED++) {
        this->controller->sendOutput(LEDs[iLED], numLEDcolor);
      }
    }
  }
}

// Draw screen
void TeknoBlinkenlichtApp::draw() {
  // Clear screen to black
	ci::gl::clear(ci::Color(0, 0, 0));
  ci::gl::enableAlphaBlending();

  // Draw background image
  ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));
  ci::gl::draw(this->tBackground, ci::app::getWindowBounds());

  // Do not proceed until game setup
  if (this->setupDone == 0) { return; }

  // Draw knobs
  ci::Vec2f rowOffset = ci::Vec2f(175, 134);
  ci::Vec2f vecSpacing = ci::Vec2f(116, 0);

  ci::Vec2f vecKnob = this->tKnob.getSize();
  ci::Area area(ci::Vec2f(0, 0), vecKnob);

  for (uint8_t iKnob = 0; iKnob < 12; iKnob++) {
    // Change row from seventh knob
    if (iKnob == 6) {
      rowOffset = ci::Vec2f(175, 291);
    }

    // Translate knob to its correct place
    ci::gl::pushModelView();
    if (iKnob < 6) {
      ci::gl::translate(rowOffset + vecSpacing * iKnob);
    } else {
      ci::gl::translate(rowOffset + vecSpacing * (iKnob - 6));
    }

    // Rotate knob image to match knob's value
    ci::gl::pushModelView();
    ci::gl::translate(vecKnob / 2);
    ci::gl::rotate(300.0f / 127.0f * this->rotations[iKnob] - 150.0f);
    ci::gl::translate(-vecKnob / 2);

    // Pick color overlay by knob's state
    switch (this->helpers[iKnob]) {
      case KNOB_UNLOADED:
        ci::gl::color(ci::ColorA(0.3f, 1.0f, 0.4f, 1.0f));
        break;

      case KNOB_LOADED:
        ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));
        break;

      case KNOB_USED:
      default:
        ci::gl::color(ci::ColorA(1.0f, 0.4f, 0.3f, 1.0f));
        break;
    }

    // Finally draw the knob
    ci::gl::draw(this->tKnob, area);
    ci::gl::popModelView();
    ci::gl::popModelView();
  }

  // Draw MIDI LEDs
  ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));

  for (uint8_t iLED = 0; iLED < 6; iLED++) {
    // Move on-screen MIDI controller's LED to its correct place
    ci::gl::pushModelView();
    rowOffset = ci::Vec2f(168, 406);
    vecSpacing = ci::Vec2f(116.5f, 0);
    ci::gl::translate(rowOffset + vecSpacing * iLED);

    // Pick an image matching the color choice
    // (might be its harmonic couple, if the button is being pressed)
    switch (this->arrColors[iLED]) {
      case COLOR_NONE:
        break;

      case COLOR_RED:
        ci::gl::draw(this->mapLEDs["red"]);
        break;

      case COLOR_GREEN:
        ci::gl::draw(this->mapLEDs["green"]);
        break;

      case COLOR_BLUE:
        ci::gl::draw(this->mapLEDs["blue"]);
        break;

      case COLOR_YELLOW:
        ci::gl::draw(this->mapLEDs["yellow"]);
        break;

      case COLOR_LILAC:
        ci::gl::draw(this->mapLEDs["lilac"]);
        break;

      case COLOR_ORANGE:
        ci::gl::draw(this->mapLEDs["orange"]);
        break;
    }

    ci::gl::popModelView();
  }

  // Draw spede
  ci::gl::color(ci::ColorA(1.0f, 1.0f, 1.0f, this->fSpedeEnergy));
  ci::gl::draw(this->tSpede, ci::app::getWindowBounds());
}

// App shutdown
void TeknoBlinkenlichtApp::shutdown() {
  delete(this->controller);

  if (this->serArduino) {
    delete(this->serArduino);
  }
}

CINDER_APP_BASIC(TeknoBlinkenlichtApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_32))
