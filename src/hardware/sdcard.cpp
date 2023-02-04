#include "sdcard.h"
#include "main.h"

#include <Arduino.h>
#include <SD.h>
#include <FS.h>

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
        log_e("Failed to initialize SD Card");
        return false;
    }

    const auto card_type = SD.cardType();
    if (card_type == CARD_NONE)
    {
        log_e("No SD card");
        return false;
    }
    else
    {
        log_i("SD card type :%d", card_type);
        log_i("SD Card Size: %llu MB", SD.cardSize() / (1024 * 1024));
    }

    return true;
}
