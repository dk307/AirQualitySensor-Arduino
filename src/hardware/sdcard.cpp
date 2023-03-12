#include "sdcard.h"
#include "main.h"

#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include "logging/logging_tags.h"

bool sd_card::pre_begin()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SDMMC_CLK, SDMMC_D0, SDMMC_CMD);

    return mount();
}

bool sd_card::mount()
{
    if (!SD.begin(SD_CS, SPI, 4000000, "/sd", 15, false))
    {
        ESP_LOGE(HARDWARE_TAG, "Failed to initialize SD Card");
        return false;
    }

    const auto card_type = SD.cardType();
    if (card_type == CARD_NONE)
    {
        ESP_LOGE(HARDWARE_TAG, "No SD card");
        return false;
    }
    else
    {
        ESP_LOGI(HARDWARE_TAG, "SD card type :%d", card_type);
        ESP_LOGI(HARDWARE_TAG, "SD Card Size: %llu MB", SD.cardSize() / (1024 * 1024));
    }

    return true;
}
