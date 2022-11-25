#include <Arduino.h>
#include "hardware.h"
#include "wifi_manager.h"
#include "operations.h"
#include "hardware\display.h"

void setup(void)
{
	Serial.begin(115200);
	if (!hardware::instance.pre_begin()) {
		log_e("Boot Failure");
	}
	
	log_i("Pre Begin Done");

	operations::instance.begin();
	hardware::instance.begin();
	// display::instance.update_boot_message("Checking Wifi");
	wifi_manager::instance.begin();

	display::instance.update_boot_message("Done");
	display::instance.set_main_screen();
}

void loop(void)
{
	hardware::instance.loop();
	wifi_manager::instance.loop();
	operations::instance.loop();
}
