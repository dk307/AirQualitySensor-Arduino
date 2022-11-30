#include "operations.h"
#include "wifi_manager.h"
#include "config_manager.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <StreamString.h>
#include <nvs_flash.h>
#include <Update.h>

#define USE_LITTLEFS false
#define ESP_MRD_USE_LITTLEFS false
#define ESP_MRD_USE_SPIFFS false
#define ESP_MRD_USE_EEPROM true

#define MRD_TIMES 5
#define MRD_TIMEOUT 10
#define MRD_ADDRESS 0
#define MULTIRESETDETECTOR_DEBUG false
#include <ESP_MultiResetDetector.h>

operations operations::instance;

void operations::factoryReset()
{
	log_w("Doing Factory Reset");
	nvs_flash_erase();
	config::erase();
	reset();
}

void operations::begin()
{
	mrd = new MultiResetDetector(MRD_TIMEOUT, MRD_ADDRESS);

	if (mrd->detectMultiReset())
	{
		log_w("Detected Multi Reset Event!!!!");
		factoryReset();
	}
	else
	{
		log_i("Not detected Multi Reset Event");
	}
}

void operations::reboot()
{
	reboot_pending = true;
}

bool operations::start_update(size_t length, const String &md5, String &error)
{
	log_i("Update call start with length:%d bytes", length);
	log_i("Current Sketch size:%d bytes", ESP.getSketchSize());
	log_i("Free sketch space:%d bytes", ESP.getFreeSketchSpace());

	const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
	if (!Update.setMD5(md5.c_str()))
	{
		log_e("Md5 Invalid:%s", error.c_str());
		return false;
	}

	if (Update.begin(maxSketchSpace))
	{
		log_i("Update begin successful");
		return true;
	}
	else
	{
		get_update_error(error);
		log_e("Update begin failed with %s", error.c_str());
		return false;
	}
}

bool operations::write_update(const uint8_t *data, size_t length, String &error)
{
	log_d("Update write with length:%d", length);
	log_d("Update stats Size: %d progress:%d remaining:%d ", Update.size(), Update.progress(), Update.remaining());
	const auto written = Update.write(const_cast<uint8_t *>(data), length);
	if (written == length)
	{
		log_d("Update write successful");
		return true;
	}
	else
	{
		get_update_error(error);
		log_e("Update write failed with %s", error.c_str());
		return false;
	}
}

bool operations::end_update(String &error)
{
	log_e("Update end called");

	if (Update.end(true))
	{
		log_i("Update end successful");
		return true;
	}
	else
	{
		get_update_error(error);
		log_e("Update end failed with %s", error.c_str());
		return false;
	}
}

void operations::abort_update()
{
	log_d("Update end called");
	if (Update.isRunning())
	{
		if (Update.end(true))
		{
			log_i("Aborted update");
		}
		else
		{
			log_e("Aborted update failed");
		}
	}
}

bool operations::is_update_in_progress()
{
	return Update.isRunning();
}

void operations::get_update_error(String &error)
{
	StreamString streamString;
	Update.printError(streamString);
	error = std::move(streamString);
}

void operations::loop()
{
	if (mrd)
	{
		mrd->loop();
		if (!mrd->waitingForMRD())
		{
			delete mrd;
			mrd = nullptr;
		}
	}

	if (reboot_pending)
	{
		reboot_pending = false;
		reset();
	}
}

[[noreturn]] void operations::reset()
{
	log_i("Restarting...");
	delay(2000); // for http response, etc to finish
	ESP.restart();
	for (;;)
	{
		delay(100);
	}
}