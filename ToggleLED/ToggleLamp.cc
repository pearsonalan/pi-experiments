// ToggleLamp.cc

#include <functional>
#include <iostream>

#include <wiringPi.h>

class LED {
 public:
  LED(int pin) : pin_(pin) {
    // Set pin to output
    pinMode(pin_, OUTPUT);
    WriteState();
  }

  void SetState(int state) {
    state_ = state;
    WriteState();
  }

  void ToggleState() {
    state_ = !state_;
    WriteState();
  }

  int state() const { return state_; }

 private:
  void WriteState() const {
    digitalWrite(pin_, state_);
  }

  int pin_;
  int state_ = LOW;
};

class Button {
 public:

  Button(int pin) : pin_(pin) {
    pinMode(pin_, INPUT);           // Set button pin to input
    pullUpDnControl(pin_, PUD_UP);  // Pull up to high level
  }

  int Poll() const {
    return digitalRead(pin_);
  } 

 private:
  int pin_;
};

class DebounceButton {
 public:
  using Callback = std::function<void()>;

  DebounceButton(int pin, const Callback& pressed_cb, const Callback& released_cb) :
    button_(pin),
    pressed_cb_(pressed_cb),
    released_cb_(released_cb)
  {
  }

  void Poll() {
    int reading = button_.Poll();
    int now = millis();

    if (reading != last_reading_) {
      // If the button state has changed, record the time point
      last_change_time_ = now;
      last_reading_ = reading;
    }

    // If changing-state of the button last beyond the time
    // we set, we consider that the current button state is
    // an effective change rather than a buffeting.
    if (now - last_change_time_ > CAPTURE_TIME) {
      if (reading != state_) {
        // If button state is changed, update the data.
        state_ = reading;
        if (state_ == LOW) {
          pressed_cb_();
        } else {
          released_cb_();
        }
      }
    }
  }

 private:
  Button button_;
  Callback pressed_cb_;
  Callback released_cb_;

  const long CAPTURE_TIME = 50; // Set the stable time for button state 

  long last_change_time_;         // Store the change time of button state
  int state_ = HIGH;      // Store the State of button
  int last_reading_ = HIGH;  // Store the lastState of button
};


int main(void) {
  const int LED_PIN = 0; 	     // Define the LED pin
  const int BUTTON_PIN = 1;	   // Define the Button pin

  std::cout << "Program is starting..." << std::endl;

  // Initialize wiringPi.	
  wiringPiSetup();

  LED led(LED_PIN);

  auto pressed_cb = [&led]() {
    std::cout << "Button is pressed!" << std::endl;
    led.ToggleState();
    if (led.state() == HIGH) {
      std::cout << "Turn on LED" << std::endl;
    } else {
      std::cout << "Turn off LED" << std::endl;
    }
  };

  auto released_cb = []() {
    std::cout << "Button is released!" << std::endl;
  };

  DebounceButton button(BUTTON_PIN, pressed_cb, released_cb);

  for (;;) {
    button.Poll();
  }

  return 0;
}

