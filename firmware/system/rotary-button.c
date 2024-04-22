/**
 * @file rotary-button.c
 * @author Anthony Rabine (anthony@rabine.fr)
 * @brief
 * @version 0.1
 * @date 2023-03-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libutil.h"
#include "ost_hal.h"
#include "debug.h"

#include "rotary-button.h"

static int counter = 0;
static int currentStateCLK;
static int lastStateCLK;
static ost_rotary_dir_t currentDirection = OST_ROTARY_UNKNOWN;

unsigned long lastButtonPress = 0;

void ost_rotary_initialize()
{
    // Read the initial state of CLK
    lastStateCLK = ost_hal_gpio_get(OST_GPIO_ROTARY_A);
}

void ost_rotary_tick()
{

    // Read the current state of CLK
    currentStateCLK = ost_hal_gpio_get(OST_GPIO_ROTARY_A);

    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
    {

        // If the DT state is different than the CLK state then
        // the encoder is rotating CCW so decrement
        if (ost_hal_gpio_get(OST_GPIO_ROTARY_B) != currentStateCLK)
        {
            counter--;
            currentDirection = OST_ROTARY_LEFT;
        }
        else
        {
            // Encoder is rotating CW so increment
            counter++;
            currentDirection = OST_ROTARY_RIGHT;
        }
    }

    // Remember last CLK state
    lastStateCLK = currentStateCLK;
}
