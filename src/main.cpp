#include <Arduino.h>
#include "hardware.h"

void setup(void)
{
	log_d("Total heap: %d", ESP.getHeapSize());
	log_d("Free heap: %d", ESP.getFreeHeap());
	log_d("Total PSRAM: %d", ESP.getPsramSize());
	log_d("Free PSRAM: %d", ESP.getFreePsram());

	Serial.begin(115200);
	hardware::instance.pre_begin();

	hardware::instance.begin();
}

void loop(void)
{
	hardware::instance.loop();
}
