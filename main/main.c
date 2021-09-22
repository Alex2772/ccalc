#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <string.h>
/*
#define LOAD_ICON_X 54
#define LOAD_ICON_Y 42
#define LOAD_ICON_SIZE 20

#define CIRCLE_COUNT_ICON_X 100
#define CIRCLE_COUNT_ICON_Y 52

#define I2C_CONNECTION

#ifdef I2C_CONNECTION
    #include <driver/i2c.h>
#endif
#include "fonts.h"
#include "ssd1306.h"

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

#ifdef I2C_CONNECTION
    #define PROTOCOL SSD1306_PROTO_I2C
    #define ADDR     SSD1306_I2C_ADDR_0
#else
    #define PROTOCOL SSD1306_PROTO_SPI4
    #define CS_PIN   5
    #define DC_PIN   4
#endif

#define DEFAULT_FONT FONT_FACE_TERMINUS_6X12_ISO8859_1

static const ssd1306_t dev = {
    .protocol = PROTOCOL,
#ifdef I2C_CONNECTION
    .addr = ADDR,
#else
    .cs_pin   = CS_PIN,
    .dc_pin   = DC_PIN,
#endif
    .width    = DISPLAY_WIDTH,
    .height   = DISPLAY_HEIGHT,
        .screen = SSD1306_SCREEN
};

#include "driver/uart.h"

static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

TimerHandle_t fps_timer_handle = NULL; // Timer handler
TimerHandle_t font_timer_handle = NULL;

uint8_t frame_done = 0; // number of frame send.
uint8_t fps = 0; // image per second.

const font_info_t *font = NULL; // current font
font_face_t font_face = 0;

#define SECOND (1000 / portTICK_PERIOD_MS)


void fps_timer(TimerHandle_t h)
{
    fps = frame_done; // Save number of frame already send to screen
    frame_done = 0;
}

void font_timer(TimerHandle_t h)
{
    do {
        if (++font_face >= font_builtin_fonts_count)
            font_face = 0;
        font = font_builtin_fonts[font_face];
    } while (!font);

    printf("Selected builtin font %d\n", font_face);
}
#define BLINK_GPIO 22

void blink_task(void *pvParameter)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

uint8_t lst = 0;
void app_main(void)
{
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    // Set UART pins using UART0 default pins i.e. no changes
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 256, 256, 10, 0, 0);
    while (ssd1306_init(&dev) != 0) {
        printf("%s: failed to init SSD1306 lcd\n", __func__);
        vTaskDelay(SECOND);
        esp_restart();
    }
    ssd1306_display_on(&dev, true);
    ssd1306_set_whole_display_lighting(&dev, true);
    vTaskDelay(SECOND);
    ssd1306_stop_scroll(&dev);
    ssd1306_set_segment_remapping_enabled(&dev, true);
    ssd1306_set_scan_direction_fwd(&dev, false);
    ssd1306_set_whole_display_lighting(&dev, false);
    font = font_builtin_fonts[font_face];
    vTaskDelay(SECOND);


    char text[20];
    uint8_t x0 = LOAD_ICON_X;
    uint8_t y0 = LOAD_ICON_Y;
    uint8_t x1 = LOAD_ICON_X + LOAD_ICON_SIZE;
    uint8_t y1 = LOAD_ICON_Y + LOAD_ICON_SIZE;
    uint16_t count = 0;



    fps_timer_handle = xTimerCreate("fps_timer", SECOND, pdTRUE, NULL, fps_timer);
    xTimerStart(fps_timer_handle, 0);
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(text, 0, sizeof(text));

        uart_read_bytes(UART_NUM_0, &lst, 1, 0);
        sprintf(text, "%d %d", lst & 0xf, lst >> 4);

        ssd1306_draw_string(&dev, buffer, font, 0, 0, text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        sprintf(text, "FPS: %u", fps);
        ssd1306_draw_string(&dev, buffer, font_builtin_fonts[DEFAULT_FONT], 0, 20, text, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

        // generate loading icon
        ssd1306_draw_line(&dev, buffer, x0, y0, x1, y1, OLED_COLOR_BLACK);
        if (x0 < (LOAD_ICON_X + LOAD_ICON_SIZE)) {
            x0++;
            x1--;
        }
        else if (y0 < (LOAD_ICON_Y + LOAD_ICON_SIZE)) {
            y0++;
            y1--;
        }
        else {
            x0 = LOAD_ICON_X;
            y0 = LOAD_ICON_Y;
            x1 = LOAD_ICON_X + LOAD_ICON_SIZE;
            y1 = LOAD_ICON_Y + LOAD_ICON_SIZE;
        }
        ssd1306_draw_line(&dev, buffer, x0, y0, x1, y1, OLED_COLOR_WHITE);
        ssd1306_draw_rectangle(&dev, buffer, LOAD_ICON_X, LOAD_ICON_Y,
                               LOAD_ICON_SIZE + 1, LOAD_ICON_SIZE + 1, OLED_COLOR_WHITE);

        // generate circle counting
        for (uint8_t i = 0; i < 10; i++) {
            if ((count >> i) & 0x01)
                ssd1306_draw_circle(&dev, buffer, CIRCLE_COUNT_ICON_X, CIRCLE_COUNT_ICON_Y, i, OLED_COLOR_BLACK);
        }

        count = count == 0x03FF ? 0 : count + 1;

        for (uint8_t i = 0; i < 10; i++) {
            if ((count>>i) & 0x01)
                ssd1306_draw_circle(&dev,buffer, CIRCLE_COUNT_ICON_X, CIRCLE_COUNT_ICON_Y, i, OLED_COLOR_WHITE);
        }

        ssd1306_load_frame_buffer(&dev, buffer);

        frame_done++;
    }
}
*/
extern void _init();
void app_main() {
    _init();
}