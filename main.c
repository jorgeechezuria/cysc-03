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

/* Queue para comunicar los dos nucleos */
queue_t queue;

/* Main para el core 1 */
void core1_main() {
  
    while(1) {
        /* Variable para recuperar el dato de la queue */
        temperature_data_t data;
        /* Espera a que esten los datos para recibir */
        queue_remove_blocking(&queue, &data);
        printf("Temperatura actual: %.2f\n", data.lm35);
        printf("Temperatura deseada: %2f\n", data.pote);
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
        adc_select_input(0);
        uint16_t Lectura = adc_read();
        float Volt = (Lectura*3.3)/4095;
        float Temp = (Volt*1)/0.01;
        adc_select_input(1);
        uint16_t LPote = adc_read();
        float Potenciometro = (LPote*35.0/4095.0);
        /* Cuando los datos estan listos, enviar por la queue */
        data.lm35 = Temp;
        data.pote = Potenciometro;
        queue_add_blocking(&queue, &data);
        sleep_ms(500);
    }
}
