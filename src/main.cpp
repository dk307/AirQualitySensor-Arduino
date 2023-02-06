#include <Arduino.h>
#include "hardware.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "web_server.h"
#include "ntp_time.h"
#include "operations.h"
#include "logging.h"
#include "hardware\display.h"

sd_card card;

void boot_failure()
{
	log_e("Boot Failure");
	delay(5000);
	ESP.restart();
	delay(2000);
}

void setup(void)
{
	Serial.begin(115200);

	log_i("Start on Core %d", xPortGetCoreID());

	if (!card.pre_begin())
	{
		boot_failure();
		return;
	}

	if (!hardware::instance.pre_begin())
	{
		boot_failure();
		return;
	}

	config::instance.pre_begin();

	logger::instance.enable_sd_logging();

	log_i("Pre Begin Done");

	operations::instance.begin();
	hardware::instance.begin();
	hardware::instance.update_boot_message("Wifi setup");
	wifi_manager::instance.begin();
	hardware::instance.update_boot_message("Webserver setup");
	web_server::instance.begin();
	ntp_time::instance.begin();
	hardware::instance.update_boot_message("Done");
	hardware::instance.set_main_screen();

	log_i("Done on core:%d", xPortGetCoreID());
}

void loop(void)
{
	wifi_manager::instance.loop();
	config::instance.loop();
	ntp_time::instance.loop();
	operations::instance.loop();

	delay(3);
}
