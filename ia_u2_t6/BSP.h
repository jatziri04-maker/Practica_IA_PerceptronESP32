/* ********************** TAREA 1: SISTEMAS EXPERTOS *********************** 
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA
* CARRERA:      ING. ELECTRÓNICA - INTELIGENCIA ARTIFICIAL EN SISTEMAS EMBEBIDOS
* MATERIA:      INTELIGENCIA ARTIFICIAL
* PROFESOR:	    DR. JUAN ALBERTO RAMÍREZ QUINTANA
* ALUMNOS:		JATZIRI DENNISE ROMERO BUSTILLOS		22061040
				ABDIEL ALEJANDRO RODRÍGUEZ CORONADO		22061055
* PERIODO:		ENE-JUN 2026
* ARCHIVO:		"BSP.h"
* FECHA:        20-FEB-2026
***************************************************************************/


/***************************************************************************
  ********************* DESCRIPCIÓN GENERAL DEL ARCHIVO ******************** 
  
  ...
  
***************************************************************************/




/* ************************************************************************* */
/* ********************** DIRECTIVAS DE PREPROCESADOR ********************** */

#ifndef BSP_H
#define BSP_H

// **** Librerías utilizadas:

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"

#include "driver/gpio.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "hal/adc_types.h"
#include "hal/gpio_types.h"

#include <sys/unistd.h>
#include <math.h>
#include "esp_log.h"




// Mapeo de pines a canales ADC1:
#define GPIO36_ADC_CHANNEL	ADC_CHANNEL_0
#define GPIO37_ADC_CHANNEL	ADC_CHANNEL_1
#define GPIO38_ADC_CHANNEL	ADC_CHANNEL_2
#define GPIO39_ADC_CHANNEL	ADC_CHANNEL_3
#define GPIO32_ADC_CHANNEL	ADC_CHANNEL_4
#define GPIO33_ADC_CHANNEL	ADC_CHANNEL_5
#define GPIO34_ADC_CHANNEL	ADC_CHANNEL_6
#define GPIO35_ADC_CHANNEL	ADC_CHANNEL_7

// Mapeo de pines a unidades ADC (por lo pronto, no se usa el ADC2):
#define ADC_UNIT_GPIO_32_TO_39	ADC_UNIT_1	// Unidad ADC conectada a GPIOs 32 a 39.
#define ADC_UNIT_GPIO_0_TO_26	ADC_UNIT_2	// Unidad ADC conectada a GPIOs 0, 2, 4, 12-15, 25-27.
// NOTA: 	La unidad ADC2 es compartida con el driver Wi-Fi. Si el driver está
//			habilitado, no se puede usar de manera adecuada el ADC2.




// **** CONFIGURACIONES DEL PROGRAMA:

// Umbrales de luz (oscuro <= U1 <= medio <= U2 <= brillante):
#define LIGHT_U1	750		// Umbral inferior (oscuro/medio).
#define LIGHT_U2	970	// Umbral superior (medio/brillante).
// NOTA 1:	Umbrales para mediciones con ADC de 10 bits (1024 niveles).
// NOTA 2:	Estos umbrales consideran el valor de una variable directamente proporcional
//			a la intensidad de la luz incidente. Comprobar HW o medición por SW para
//			que los umbrales tengan sentido con el valor de la variable.

#define DEBOUNCE_TIME_MS	200	// Periodo de anti-rebote del botón en milisegundos.




/* ************************************************************************* */
/* ***************** DECLARACIONES Y DEFINICIONES GLOBALES ***************** */

// **** TIPOS DE DATOS DEFINIDOS POR EL USUARIO:

// Enumeración para definir modo de GPIO:
typedef enum{
	OUTPUT_MODE,
	OUTPUT_PWM_MODE,
	INPUT_MODE,
	INPUT_PULLUP_MODE,
	INPUT_PULLDOWN_MODE,
	INPUT_ANALOG_MODE
} gpio_mode_e;


// Estructura para manejar puertos GPIO básicos:
typedef struct{
	uint8_t 	pin_num;
	uint16_t 	state;
	uint16_t 	last_state;
	bool 		inverted_logic;
	gpio_mode_e mode;
} gpio_setup_t;


// Estructura para manejar un puerto GPIO como botón:
typedef struct{
	gpio_setup_t 	gpio;
	bool			intr_enable;
	uint64_t 		last_press_time_ms;
	gpio_int_type_t	intr_type;
} button_setup_t;


// Estructura para manejar puertos GPIO con ADC:
typedef struct{
	gpio_setup_t	gpio;			// Estructura de configuración básica para GPIO.
	uint8_t 		resolution;		// Resolución usada por el ADC para dicho pin.
	adc_unit_t 		unit;			// Define la unidad de ADC asociada al pin.
	adc_channel_t 	channel;		// Define el canal ADC asociado al pin.
} adc_setup_t;





// **** DECLARACIÓN DE VARIABLES GLOBALES:
extern bool button_pressed_flag;

// **** Handlers globales del ADC:
extern adc_oneshot_unit_handle_t 	adc_handle;
extern adc_cali_handle_t 			cali_handle;




/* ************************************************************************* */
/* ****************** PROTOTIPOS DE FUNCIONES PRINCIPALES ****************** */


// **** [GPIO.c] PROTOTIPOS DE FUNCIONES PARA MANEJAR PUERTOS GPIO DIGITALES:
void gpio_init(gpio_setup_t* gpio, uint8_t pin_num, uint16_t state, bool inverted_logic, gpio_mode_e mode);

void button_init(button_setup_t* button, uint8_t pin_num, bool inverted_logic, gpio_mode_e mode, bool intr_enable, gpio_int_type_t intr_type);
bool button_was_pressed(button_setup_t* button);
//void led_init(gpio_setup_t*, uint8_t, uint16_t, bool, gpio_mode_e);


bool gpio_digital_read(gpio_setup_t* gpio);
void gpio_digital_write(gpio_setup_t* gpio, bool state);

bool gpio_get_state(gpio_setup_t* gpio);
bool gpio_get_last_state(gpio_setup_t* gpio);
void gpio_set_last_state(gpio_setup_t* gpio);


// **** [ADC.c] PROTOTIPOS DE FUNCIONES PARA MANEJAR PUERTOS GPIO CON ADC:
void adc_unit_init(adc_unit_t adc_unit);
bool init_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);

void adc_channel_init(adc_setup_t* adc);
uint16_t adc_get_raw(adc_setup_t* adc);


// **** [PRINT.c] PROTOTIPOS DE FUNCIONES PARA IMPRESIÓN POR SERIAL:
void print_welcome_message(void);

/* ************************************************************************* */




#endif
