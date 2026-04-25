/* ********************** TAREA 1: SISTEMAS EXPERTOS *********************** 
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA
* CARRERA:      ING. ELECTRÓNICA - INTELIGENCIA ARTIFICIAL EN SISTEMAS EMBEBIDOS
* MATERIA:      INTELIGENCIA ARTIFICIAL
* PROFESOR:	    DR. JUAN ALBERTO RAMÍREZ QUINTANA
* ALUMNOS:		JATZIRI DENNISE ROMERO BUSTILLOS		22061040
				ABDIEL ALEJANDRO RODRÍGUEZ CORONADO		22061055
* PERIODO:		ENE-JUN 2026
* ARCHIVO:		"GPIO.c"
* FECHA:        20-FEB-2026
***************************************************************************/


/***************************************************************************
  ********************* DESCRIPCIÓN GENERAL DEL ARCHIVO ******************** 
  
  ...
  
***************************************************************************/




/* ************************************************************************* */
/* ********************** DIRECTIVAS DE PREPROCESADOR ********************** */

// **** Librerías utilizadas:
#include "BSP.h"




/* ************************************************************************* */
/* ************************ DEFINICIÓN DE FUNCIONES ************************ */


/* ************* FUNCIONES PARA MANEJAR PUERTOS GPIO GENERALES ************* */

void gpio_init(gpio_setup_t* gpio, uint8_t pin_num, uint16_t state, bool inverted_logic, gpio_mode_e mode){
	
	gpio->pin_num 		= pin_num;
	gpio->state 		= (inverted_logic) ? !state : state;
	gpio->last_state 	= gpio->state;
	gpio->inverted_logic = inverted_logic;
	gpio->mode 			= mode;
	
	gpio_reset_pin((gpio_num_t)pin_num);
	
	switch(mode){
		case OUTPUT_MODE:	
			gpio_set_direction((gpio_num_t)pin_num, GPIO_MODE_OUTPUT);
			gpio_set_level((gpio_num_t)pin_num, gpio->state);
			break;
			
		case OUTPUT_PWM_MODE:
			// No implementado para esta actividad.
			break;
			
		case INPUT_MODE:
			gpio_set_direction((gpio_num_t)pin_num, GPIO_MODE_INPUT);
			gpio_set_pull_mode((gpio_num_t)pin_num, GPIO_FLOATING);
			break;
			
		case INPUT_PULLUP_MODE:
			gpio_set_direction((gpio_num_t)pin_num, GPIO_MODE_INPUT);
			gpio_set_pull_mode((gpio_num_t)pin_num, GPIO_PULLUP_ONLY);
			break;
			
		case INPUT_PULLDOWN_MODE:
			gpio_set_direction((gpio_num_t)pin_num, GPIO_MODE_INPUT);
			gpio_set_pull_mode((gpio_num_t)pin_num, GPIO_PULLDOWN_ONLY);
			break;
		
		case INPUT_ANALOG_MODE:
			// No se implementa, dado que lo maneja la función adc_init().
			break;
			
		default:
			break;
	}
	
	return;
}


void gpio_digital_write(gpio_setup_t* gpio, bool state){
	
	switch(gpio->mode){
		case OUTPUT_MODE:
			gpio->state = (gpio->inverted_logic) ? !state : state;
			gpio_set_level((gpio_num_t)gpio->pin_num, gpio->state);
			break;
			
		case OUTPUT_PWM_MODE:
			// No se implementa en este programa.
			break;
			
		default:
			printf("\nError: Intento de escritura en pin no configurado como salida.\n");
			break;
	}
	
	return;
}


bool gpio_digital_read(gpio_setup_t* gpio){
	
	switch(gpio->mode){
		case OUTPUT_MODE:	
		case OUTPUT_PWM_MODE:
			break;
		
		case INPUT_MODE:
		case INPUT_PULLDOWN_MODE:
		case INPUT_PULLUP_MODE:
			gpio->state = gpio_get_level((gpio_num_t)gpio->pin_num);
			break;
			
		case INPUT_ANALOG_MODE:
			// No se implementa, dado que lo maneja la función adc_init().
			break;
		
		default:
			printf("\nError: Pin no configurado ");
			printf(" %d\n", gpio->pin_num);
			return -1;
	}
	
	return (gpio->inverted_logic) ? !gpio->state : gpio->state;
}


bool gpio_get_state(gpio_setup_t* gpio){
	return (gpio->inverted_logic) ? !gpio->state : gpio->state;
}

bool gpio_get_last_state(gpio_setup_t* gpio){
	return (gpio->inverted_logic) ? !gpio->state : gpio->state;
}

void gpio_set_last_state(gpio_setup_t* gpio){
	gpio->last_state = gpio->state;
	return;
}




/* ********************* FUNCIONES PARA MANEJAR BOTONES ******************** */

void IRAM_ATTR push_isr_handler(void* arg){
	button_pressed_flag = true;
	return;
}


void button_init(button_setup_t* button, uint8_t pin_num, bool inverted_logic, gpio_mode_e mode, bool intr_enable, gpio_int_type_t intr_type){
	button->last_press_time_ms 	= 0;
	button->intr_enable 		= intr_enable;
	button->intr_type 			= intr_type;
	
	if(intr_enable){
		button->gpio.pin_num		= pin_num;
		button->gpio.inverted_logic	= inverted_logic;
		button->gpio.last_state		= inverted_logic;
		button->gpio.state			= inverted_logic;
		button->gpio.mode			= mode;
		
		gpio_config_t io_config = {
			.intr_type 	= intr_type,
			.mode 		= GPIO_MODE_INPUT,
			.pin_bit_mask	= (1ULL << pin_num),
			.pull_up_en		= (mode == INPUT_PULLUP_MODE) ? 1 : 0,
			.pull_down_en	= (mode == INPUT_PULLDOWN_MODE) ? 1 : 0			 
		};
		gpio_config(&io_config);
		
		gpio_install_isr_service(0);
		gpio_isr_handler_add(pin_num, push_isr_handler, NULL);
	}
	else{
		button->intr_type = GPIO_INTR_DISABLE;
		gpio_init(&button->gpio, pin_num, inverted_logic, inverted_logic, mode);
	}
	return;
}


bool button_was_pressed(button_setup_t* button){
	bool was_pressed = false;
	
	if(button->intr_enable){
		// Con interrupciones y anti-rebote:
		// - Menor tiempo de respuesta.
		// - NO es necesario que siga presionado cuando se llama a esta función.
		if(button_pressed_flag){
			uint64_t now = esp_timer_get_time() / 1000;
			
			if((now - button->last_press_time_ms) > DEBOUNCE_TIME_MS){
				button->last_press_time_ms = now;
				was_pressed = true;
			}
			button_pressed_flag = false;
		}
	}
	else{
		// Sin interrupciones, pero con anti-rebote:
		// - Mayor tiempo de respuesta.
		// - Tiene que seguir presionado cuando se llama a esta función.
		
		gpio_digital_read(&button->gpio);
		
		if(button->gpio.state != button->gpio.inverted_logic && button->gpio.last_state == button->gpio.inverted_logic){
			uint16_t now = esp_timer_get_time() / 1000;
			
			if((now - button->last_press_time_ms) > DEBOUNCE_TIME_MS){
				button->last_press_time_ms = now;
				was_pressed = true;
			}
		}
		button->gpio.last_state = button->gpio.state;
	}
	// Retardo para evitar conflictos con el Wathdog timer:
	vTaskDelay(pdMS_TO_TICKS(10));
	
	return was_pressed;
}


