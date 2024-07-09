// include the Pi Pico SDK without annoying warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wvolatile"
#pragma GCC diagnostic ignored "-Wconversion"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/time.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#pragma GCC diagnostic pop

#include "version.h"

#include "utils.h"
#include "Tasks.h"

// LedBlinkTask: Task to blink an LED on & off
class LedBlinkTask : public Tasks::Task
{
public:
    // Task execution interval in microseconds
    unsigned intervalMicros() const override { return 500'000; }

    // Task initialization, called once at program start
    void init() override
    {
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }

    // Main task function, executed at (approximately) the specified interval
    void execute() override
    {
        fLed = !fLed;
        gpio_put(PICO_DEFAULT_LED_PIN, fLed);
    }

private:
    bool fLed = false;
};

// LedColourTask: Task to colour-cycle an RGB LED
class LedColourTask : public Tasks::Task
{
public:
    // Task execution interval in microseconds
    unsigned intervalMicros() const override { return 10'000; }

    // Task initialization, called once at program start
    void init() override
    {
        sm = pio_claim_unused_sm(pio, true);
        uint offset = pio_add_program(pio, &ws2812_program);
        ws2812_program_init(pio, sm, offset, PICO_DEFAULT_WS2812_PIN, 800000, true);
        put_pixel(urgb_u32(0, 0, 0));
    }

    // Main task function, executed at (approximately) the specified interval
    void execute() override
    {
        uint8_t n = ++index;
        uint32_t colour;
        if (n < 85) {
            colour = urgb_u32(n * 3, (uint8_t)(255u - n * 3), 0);
        } else if (n < 170) {
            n -= 85;
            colour = urgb_u32((uint8_t)(255u - n * 3), 0, n * 3);
        } else {
            n -= 170;
            colour = urgb_u32(0, n * 3, (uint8_t)(255u - n * 3));
        }
        put_pixel(colour);
    }

private:
    const PIO pio = pio0;
    uint sm = 0;
    uint8_t index = 0;

    static constexpr uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
    {
        constexpr unsigned DIM_FACTOR = 5;
        return ((uint32_t) (r / DIM_FACTOR) << 8) |
               ((uint32_t) (g / DIM_FACTOR) << 16) |
                (uint32_t) (b / DIM_FACTOR);
    }

    void put_pixel(uint32_t pixel_grb)
    {
        pio_sm_put(pio, sm, pixel_grb << 8u);
    }
};

// Task List
static constexpr auto taskList = Tasks::makeTaskList<
    //DebugTask,
    LedBlinkTask,
    LedColourTask
>();

int main()
{
    // Initialize all the Tasks and run them forever
    taskList.initAll();
    while (true) {
        taskList.runAll();
    }
}
