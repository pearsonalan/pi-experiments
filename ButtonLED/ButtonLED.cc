/**********************************************************************
* Filename    : ButtonLED.c
* Description : Control led by button.
* Author      : www.freenove.com
* modification: 2019/12/26
**********************************************************************/
#include <time.h>
#include <stdio.h>

#include <wiringPi.h>

class Timer {
 public:
  Timer() {
    clock_gettime(CLOCK_REALTIME, &start_time_);
  }

  timespec GetCurrentTime() const {
    timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    current_time.tv_sec -= start_time_.tv_sec;
    current_time.tv_nsec -= start_time_.tv_nsec;
    if (current_time.tv_nsec < 0) {
      current_time.tv_sec--;
      current_time.tv_nsec += 1000000000;
    }
    return current_time;
  }
  
 private:
  timespec start_time_;
};

void usleep(long microseconds) {
  const timespec sleep_time = { microseconds / 1000000, (microseconds % 1000000) * 1000 };
  nanosleep(&sleep_time, NULL);
}

int main(int argc, char** argv) {
  const int led_pin = 0;
  const int button_pin = 1;
  int button_state = HIGH;

  printf("Program is starting... \n");
  wiringPiSetup();	//Initialize wiringPi.	

  Timer timer;

  pinMode(led_pin, OUTPUT); //Set led_pin to output
  pinMode(button_pin, INPUT); //Set button_pin to input

  pullUpDnControl(button_pin, PUD_UP);  //pull up to HIGH level

  for (;;) {
    usleep(1000);
    int current_state = digitalRead(button_pin);
    if (current_state != button_state) {
      timespec current_time = timer.GetCurrentTime();
      printf("%04d.%06d: Button transition from %d -> %d.\n",
             current_time.tv_sec,
             current_time.tv_nsec / 1000,
             button_state, current_state);
      button_state = current_state;
      if (button_state == LOW) {
        // Button is pressed.  Make GPIO output HIGH level
        digitalWrite(led_pin, HIGH);
      } else {
        // Button is released. Make GPIO output LOW level
        digitalWrite(led_pin, LOW);
      }
    }
  }

  return 0;
}

