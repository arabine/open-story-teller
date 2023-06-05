
#include "ost_hal.h"
#include "debug.h"
#include "filesystem.h"
#include "picture.h"
#include "ost_tasker.h"
#include "rotary-button.h"

#define RUN_TESTS 1

#ifndef RUN_TESTS
int main(void)
{
    // Low level initialization, mainly platform stuff
    // After this call, debug_printf *MUST* be available
    ost_system_initialize();
    debug_printf("\r\n>>>>> Starting OpenStoryTeller: V%d.%d <<<<<\n", 1, 0);

    // File system access
    filesystem_mount();

    // Display
    ost_display_initialize();
    decompress();

    // Audio

    // Tasker
    ost_tasker_init();

    for (;;)
    {
    }

    return 0;
}
#else

#include "sdcard.h"

int main()
{
    ost_system_initialize();

    // 1. Test the printf output
    debug_printf("\r\n [OST] Starting OpenStoryTeller tests: V%d.%d\r\n", 1, 0);

    filesystem_mount();

    // 2. Unit test the SD Card
    // sdcard_init();

    for (;;)
    {
        ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 1);
        ost_system_delay_ms(1000);
        ost_hal_gpio_set(OST_GPIO_DEBUG_LED, 0);
        ost_system_delay_ms(1000);
    }
    return 0;
}
#endif
