#include <Arduino.h>
#include "hardware.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "web_server.h"
#include "ntp_time.h"
#include "operations.h"
#include "hardware\display.h"

void setup(void)
{
	Serial.begin(115200);
	log_i("Start");
	if (!hardware::instance.pre_begin())
	{
		log_e("Boot Failure");
		delay(5000);
		ESP.restart();
	}

	config::instance.pre_begin();

	log_i("Pre Begin Done");

	operations::instance.begin();
	hardware::instance.begin();
	display::instance.update_boot_message("Wifi setup");
	wifi_manager::instance.begin();
	display::instance.update_boot_message("Webserver setup");
	web_server::instance.begin();
	ntp_time::instance.begin();
	display::instance.update_boot_message("Done");
	display::instance.set_main_screen();

	log_i("Done on core:%d", xPortGetCoreID());
}

void loop(void)
{
	hardware::instance.loop();
	wifi_manager::instance.loop();
	config::instance.loop();
	ntp_time::instance.loop();
	operations::instance.loop();

	delay(2);
}
