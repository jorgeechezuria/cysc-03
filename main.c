#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

/* Struct para los datos de temperatura */
typedef struct {
  float lm35;
  float pote;
} temperature_data_t;
#define PICO_DEFAULT_LED_PIN    25
#define PICO_DEFAULT_LED_PIN1 6
#define PICO_DEFAULT_LED_PIN2 7;
/* Queue para comunicar los dos nucleos */
queue_t queue;

/* Main para el core 1 */
void core1_main() {6
// asigno un pin al pwm
gpio_set_function(PICO_DEFAULT_LED_PIN1, GPIO_FUNC_PWM);
gpio_set_function(PICO_DEFAULT_LED_PIN2, GPIO_FUNC_PWM);
uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
// configuro la velocidad de lectura 
pwm_config_set_clkdiv(&config, 4.f);
// inicio el pwm
pwm_init(slice_num, &config, true);
    while(1) {
        /* Variable para recuperar el dato de la queue */
        temperature_data_t data;
        /* Espera a que esten los datos para recibir */
        queue_remove_blocking(&queue, &data);
        printf("Temperatura actual: %.2f\n", data.lm35);
        printf("Temperatura deseada: %2f\n", data.pote);
        float TempX = data.lm35-data.pote;

        if(TempX<-10){
            TempX = -10
        }
        if(TempX>10){
            TempX = 10
        }

        float PWM = (TempX*65536)/10

        if(TempX<0){
            // Enciendo El Caloventor y Apago el aire acc
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN6, 0); 
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN7, PWM);
        }else
        if(TempX>0){
            // Enciendo Aire acc y Apago el Caloventor
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN6, PWM);
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN7, 0);
        }else
        if(TempX==0){
            //Apago Todo
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN6, 0);
            pwm_set_gpio_level(PICO_DEFAULT_LED_PIN7, 0);
        }
    }
}    

        
/* Main para el core 0 */
int main() {
    stdio_init_all();
    /* Inicializa la cola para enviar un unico dato */
    queue_init(&queue, sizeof(temperature_data_t), 1);
    /* Inicializa el core 1 */
    multicore_launch_core1(core1_main);
    adc_init();
     adc_gpio_init(26);
     adc_gpio_init(27);

    while(1) {
        /* Variable para enviar los datos */
        temperature_data_t data;
        //Elijo canal del adc
        adc_select_input(0);
        //Saco los Valores del adc
        uint16_t Lectura = adc_read();
        float Volt = (Lectura*3.3)/4095;
        float Temp = (Volt*1)/0.01;
        adc_select_input(1);
        uint16_t LPote = adc_read();
        float Potenciometro = (LPote*35.0/4095.0);
        /* Cuando los datos estan listos, enviar por la queue */
        //preparo los valores para mandarlos al otro nucleo
        data.lm35 = Temp;
        data.pote = Potenciometro;
        queue_add_blocking(&queue, &data);
        sleep_ms(500);
    }
}
