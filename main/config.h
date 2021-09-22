//
// Created by alex2772 on 08.12.17.
//

#pragma once

#include <driver/gpio.h>

#define TIME_OFFSET 3


// VERSION

#define CCOS_VERSION "0.4.0 beta"


// PINS

#define ADC_BATTERY ADC1_CHANNEL_5
#define PIN_MOTOR GPIO_NUM_23
#define PIN_POWER_BUTTON GPIO_NUM_0
#define PIN_LED_R (FirmwareConfig::read().mMajorModel < 5 ? GPIO_NUM_5 : GPIO_NUM_12)
#define PIN_LED_G (FirmwareConfig::read().mMajorModel < 5 ? GPIO_NUM_17 : GPIO_NUM_5)
#define PIN_LED_B GPIO_NUM_22
#define PIN_INPUT (FirmwareConfig::read().mMajorModel < 5 ? GPIO_NUM_16 : GPIO_NUM_27)
#define PIN_PROGRAMMABLE (FirmwareConfig::read().mMajorModel < 5 ? GPIO_NUM_34 : GPIO_NUM_25)
#define PIN_PERIPHERAL (FirmwareConfig::read().mMajorModel < 5 ? GPIO_NUM_21 : GPIO_NUM_2)

#define BATT_UP 1290
#define BATT_DOWN 900