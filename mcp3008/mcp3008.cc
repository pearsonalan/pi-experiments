#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <mcp3004.h>
#include <wiringPiSPI.h>

#include <iostream>
#include <string>
#include <vector>

#define BASE 100
#define SPI_CHAN 0
#define ADC_RANGE 1024.0
#define AREF 5.0
#define R_A 10.0  // Resistor in series with Thermistor in voltage divider. (in K Ohms)

#define SAMPLES 100 
#define CHANNELS 2

struct ChannelSamples {
public:
	float value[SAMPLES] = { 0.0 };
	float average = 0.0;
}; 

class Samples {
public:
	Samples() = default;
	~Samples() = default;

	void sample();
	int sample_count() const { return sample_count_; }
	void calculateAverages();
	float getAverageForChannel(int c) const {
		return channels_[c].average;
	}

protected:
	ChannelSamples channels_[CHANNELS];
	int sample_count_ = 0;
	int sample_pos_ = 0;
};

void Samples::sample() {
	for (int c = 0; c < CHANNELS; ++c) {
		channels_[c].value[sample_pos_] = analogRead(BASE + c);
	}
	sample_pos_++;
	if (sample_pos_ == SAMPLES)
		sample_pos_ = 0;
	if (sample_count_ < SAMPLES)
		sample_count_++;
	calculateAverages();
}

void Samples::calculateAverages() {
	for (int c = 0; c < CHANNELS; ++c) {
		channels_[c].average = 0.0;
		for (int s = 0; s < sample_count_; s++) {
			channels_[c].average += channels_[c].value[s];
		}
		channels_[c].average /= sample_count_;
	}
}

struct TemperatureResult {
	// Voltage drop across the resistor
	float voltage;

	// Computed resistance from voltage divider formula
	float resistance;

	// Temperature in Celsius
	float temp_c;

	// Temperature in Farenheit
	float temp_f;
};

TemperatureResult convertTemperature(float resistor_a, float adc_value) {
	TemperatureResult result;
	float temp_k;

	// Convert the reading from the ADC into a voltage.
	result.voltage = adc_value / ADC_RANGE * AREF;

	// Calculate resistance value of thermistor (in K Ohms)
	result.resistance = resistor_a * result.voltage / (AREF - result.voltage);

	// Calculate temperature (Kelvin)
	temp_k = 1 / (1 / (273.15 + 25) + log(result.resistance / 10) / 3950.0);

	// Calculate temperature (Celsius)
	result.temp_c = temp_k - 273.15;

	// Calculate temperature in F
	result.temp_f = (result.temp_c * 9.0 / 5.0) + 32.0;

	return result;
}

void printResult(int channel, float reading, TemperatureResult& temp) {
	printf("\tChannel %d: value=%0.2f, volts=%02f, Rt=%0.2f, Temp=%0.1fC %0.1fF\n",
		channel, reading, temp.voltage, temp.resistance, temp.temp_c, temp.temp_f);
}

void showResult(Samples& samples) {
	for (int c = 0; c < CHANNELS; c++) {
		std::cout << "Channel " << c << ": value=" << samples.getAverageForChannel(c) << "; ";
	}
	std::cout << std::endl;

	TemperatureResult temp;
	float reading = samples.getAverageForChannel(0);
	temp = convertTemperature(9.95, reading);
	printResult(0, reading, temp);

	reading = samples.getAverageForChannel(1);
	temp = convertTemperature(9.95, reading);
	printResult(1, reading, temp);

	printf("\n");
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	int retval = wiringPiSPISetup(0, 500000);
	std::cout << "wiringPiSPISetup RC = " << retval << std::endl;
	mcp3004Setup(BASE, SPI_CHAN);
	
	// Loop indefinitely, waiting for 500ms between each set of data
	int iteration = 0;
	Samples samples;
	while (1) {
		std::cout << "Sampling..." << std::endl;
		samples.sample();

		if (++iteration == 5) {
			showResult(samples);
			iteration = 0;
		}

		usleep(500000);
	}

	return 0;
}
