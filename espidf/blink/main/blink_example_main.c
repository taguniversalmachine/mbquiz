/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


static const char *TAG = "blink";

/* BEGIN ADC */
#define ADC1_EXAMPLE_CHAN0          ADC1_CHANNEL_2
#define ADC2_EXAMPLE_CHAN0          ADC2_CHANNEL_0
static const char *TAG_CH[2][10] = {{"ADC1_CH2"}, {"ADC2_CH0"}};

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11

#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP_FIT
static int adc_raw[2][10];

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
        esp_adc_cal_characterize(ADC_UNIT_2, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc2_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

/* END ADC */


int g_ButtonState = 0;
int g_DistanceSensorState = 0;
float g_potentiometer_range = 0;

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
//#define BLINK_GPIO CONFIG_BLINK_GPIO
#define GREEN_LED_PIN 1
#define BUTTON_PIN 2
#define BLUE_LED_PIN 13
#define DISTANCE_SENSOR_PIN 4
#define UPDATE_INTERVAL_MS 100

//static uint8_t s_led_state = 0;


static void set_green_led(int state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(GREEN_LED_PIN, state);
}

static void set_blue_led(int state){
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLUE_LED_PIN, state);
}

static void configure_green_led(void)
{
    ESP_LOGI(TAG, "Configuring Green LED");
    gpio_reset_pin(GREEN_LED_PIN);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(GREEN_LED_PIN, GPIO_MODE_OUTPUT);
}

static void configure_blue_led(void)
{
    ESP_LOGI(TAG, "Configuring Blue LED");
    gpio_reset_pin(BLUE_LED_PIN);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLUE_LED_PIN, GPIO_MODE_OUTPUT);
}

static void configure_button(void)
{
    ESP_LOGI(TAG, "Configuring button");
    gpio_reset_pin(BUTTON_PIN);
    /* Set the GPIO as input */
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
}

static void configure_distance_sensor(void) {
    ESP_LOGI(TAG, "Configuring distance sensor");
    gpio_reset_pin(DISTANCE_SENSOR_PIN);
    gpio_set_direction(DISTANCE_SENSOR_PIN, GPIO_MODE_INPUT);
}

void button_task(void *pvParameters) {
    while (1) {
        g_ButtonState = gpio_get_level(BUTTON_PIN);
     //   ESP_LOGI(TAG, "Polling button %d", g_ButtonState);
        vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL_MS)); // Delay for the specified interval
    }
}

void distance_sensor_task(void *pvParameters){
    int distance_sensor_state = 0;
    while (1) {
      distance_sensor_state = gpio_get_level(DISTANCE_SENSOR_PIN);
     // ESP_LOGI(TAG, "xDistance Sensor: %d   g_DistanceSensorState: %d\n   ", distance_sensor_state, g_DistanceSensorState);
      if (g_DistanceSensorState != distance_sensor_state) {
        ESP_LOGI(TAG, "Distance Sensor: %d\n", distance_sensor_state);
      }

      g_DistanceSensorState = distance_sensor_state;
      vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL_MS)); // Delay for the specified interval
    }
}

void adc_poll_task(void* pvParameters){
  //  esp_err_t ret = ESP_OK;
    uint32_t voltage = 0;
    bool cali_enable = adc_calibration_init();

    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));
    //ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

   while (1) {
        adc_raw[0][0] = adc1_get_raw(ADC1_EXAMPLE_CHAN0);
        ESP_LOGI(TAG_CH[0][0], "raw  data: %d", adc_raw[0][0]);
        if (cali_enable) {
            voltage = esp_adc_cal_raw_to_voltage(adc_raw[0][0], &adc1_chars);
            ESP_LOGI(TAG_CH[0][0], "cali data: %d mV", voltage);
            g_potentiometer_range = (100 * voltage)/4000;
            ESP_LOGI(TAG, "potentiometer range: %f\n", g_potentiometer_range);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
/*
        do {
            ret = adc2_get_raw(ADC2_EXAMPLE_CHAN0, ADC_WIDTH_BIT_DEFAULT, &adc_raw[1][0]);
        } while (ret == ESP_ERR_INVALID_STATE);
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(TAG_CH[1][0], "raw  data: %d", adc_raw[1][0]);
        if (cali_enable) {
            voltage = esp_adc_cal_raw_to_voltage(adc_raw[1][0], &adc2_chars);
            ESP_LOGI(TAG_CH[1][0], "cali data: %d mV", voltage);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  */
    }
}

void blue_blinker_task(void* pvParameters){
    int blink_state = 0;
    int delay = 0;
    while (1) {
       if (blink_state) {
          blink_state = 0;
       } else {
          blink_state = 1;
       }
       set_blue_led(blink_state);
       // Potentiometer range is 0-4000
       delay = (int) g_potentiometer_range /2;
       vTaskDelay(delay);
    }
 }

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_green_led();
    ESP_LOGI(TAG, "Configured Green LED");
    configure_blue_led();
    ESP_LOGI(TAG, "Configured Blue LED");
    configure_button();
    ESP_LOGI(TAG, "Configured Button");
    configure_distance_sensor();
    ESP_LOGI(TAG, "Configured Distance Sensor");

    // Create the input task
   //  BaseType_t xTaskCreate(    TaskFunction_t pvTaskCode,
   //                         const char * const pcName,
   //                         configSTACK_DEPTH_TYPE usStackDepth,
   //                         void *pvParameters,
   //                         UBaseType_t uxPriority,
    //                        TaskHandle_t *pxCreatedTask
    //                      );
    ESP_LOGI(TAG, "Creating button task");
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
    ESP_LOGD(TAG, "Created  button task");

    ESP_LOGI(TAG, "Creating ADC task");
    xTaskCreate(adc_poll_task, "ADC poll task", 2048, NULL, 10, NULL);
    ESP_LOGD(TAG, "Created  ADC task");

    ESP_LOGI(TAG, "Creating Distance Sensor task");
    xTaskCreate(distance_sensor_task, "Distance Sensor Task", 2048, NULL, 10, NULL);
    ESP_LOGD(TAG, "Created distance sensor task");

    ESP_LOGI(TAG, "Creating blink task");
    xTaskCreate(blue_blinker_task, "Blinker task", 2048, NULL, 10, NULL);
    ESP_LOGD(TAG, "Created blink task");

    while (1) {
     //   ESP_LOGI(TAG, "Turning the LED %s!", g_inputState == true ? "ON" : "OFF");
        set_green_led(g_DistanceSensorState);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
