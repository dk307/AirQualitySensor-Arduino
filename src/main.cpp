#include <Arduino.h>
#include "hardware.h"
#include "hardware\display.h"

void setup(void)
{
	Serial.begin(115200);
	if (!hardware::instance.pre_begin()) {
		log_e("Boot Failure");
	}

	hardware::instance.begin();

	display::instance.update_boot_message("Start Done");
	display::instance.set_main_screen();
}

void loop(void)
{
	hardware::instance.loop();
}
