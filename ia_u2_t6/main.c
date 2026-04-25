#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_task_wdt.h"    // Task watchdog

#include "BSP.h"
#include "perceptron.h"
#include "timing.h"


// **** CONFIGURACIONES DE PINES:

// Botón para cambiar el modo del sistema:
#define BUTTON_PIN		18	// Modo del sistema.

// LED que indica el modo del sistema (OFF / RUN).
#define LED_MODE_PIN	2	// Modo del sistema.

// LED que indica la salida del sistema (0 / 1).
#define LED_OUTPUT_PIN	4	// Salida del sistema.


#define NUMBER_OF_INPUT_POTS 5
#define POTS_BINARY_THRESHOLD 512

// Pines de los sensores del sistema:
#define POT_1_PIN 36
#define POT_2_PIN 39
#define POT_3_PIN 34
#define POT_4_PIN 35
#define POT_5_PIN 32


bool button_pressed_flag = false;
adc_oneshot_unit_handle_t 	adc_handle;
adc_cali_handle_t 			cali_handle = NULL;

void app_main(void){
	
	// **** Hardware setup:
	
	if(NUMBER_OF_INPUT_POTS < 2 || NUMBER_OF_INPUT_POTS > 5){
		printf("\nError: NUMBER_OF_INPUT_POTS must be between 2 and 5.\n\n"); 
		while(true);
	}
	
	uint8_t system_mode = 0;
	// system_mode == 0 	-> OFF
	// system_mode == 1 	-> ON (RUN)
	
	button_setup_t button;
	gpio_setup_t led_system_mode;
	gpio_setup_t led_system_output;
	
	// Initializes button as button_setup_t object (with interruption):
	button_init(&button, BUTTON_PIN, true, INPUT_PULLUP_MODE, true, GPIO_INTR_NEGEDGE);
	
	// Initializes LEDs as basic gpio_setup_t objects:
	gpio_init(&led_system_mode, LED_MODE_PIN, false, false, OUTPUT_MODE);
	gpio_init(&led_system_output, LED_OUTPUT_PIN, false, false, OUTPUT_MODE);
	
	// Creates and initializes potentiometers pins as adc_setup_t objects:
	
	adc_setup_t input_pots[NUMBER_OF_INPUT_POTS];
	
	uint8_t input_pots_pins[5] = {
		POT_1_PIN, POT_2_PIN, POT_3_PIN, 
		POT_4_PIN, POT_5_PIN
	};
	
	uint8_t input_pots_adc_channels[5] = {
		GPIO36_ADC_CHANNEL, GPIO39_ADC_CHANNEL,
		GPIO34_ADC_CHANNEL, GPIO35_ADC_CHANNEL,
		GPIO32_ADC_CHANNEL
	};
	
	uint8_t input_pots_adc_unit = ADC_UNIT_GPIO_32_TO_39;
	
	adc_unit_init(input_pots_adc_unit);

	
	for(uint8_t i = 0; i < NUMBER_OF_INPUT_POTS; i++){
		input_pots[i].gpio.pin_num 			= input_pots_pins[i];
		input_pots[i].gpio.state			= 0;
		input_pots[i].gpio.last_state 		= 0;
		input_pots[i].gpio.inverted_logic 	= false;
		input_pots[i].gpio.mode 			= INPUT_ANALOG_MODE;
		input_pots[i].resolution 			= 10;
		input_pots[i].unit					= input_pots_adc_unit;
		input_pots[i].channel				= input_pots_adc_channels[i];
		
		adc_channel_init(&input_pots[i]);
	}
	
	init_calibration(ADC_UNIT_1, ADC_ATTEN_DB_12, &cali_handle);
	
	
	// **** Training data configuration:
	
	uint32_t dimensions 		= NUMBER_OF_INPUT_POTS;	// No. of dimensions.
	uint8_t training_selector	= 3;	// 1 == AND; 2 == OR; 3 == NAND; 4 == NOR.
										// 5 == XOR; 6 == XNOR; 7 == Custom.
	
	uint8_t higher_dim_mapping_op = 0;	// 0 == No operation; (x1 + x2 + ...)^d. 
	uint8_t custom_training_data_selector = 1;	// 0 == User selected; n == Program selected n.
	
	// Custom training datasets (program selected):
	// Note:	custom_training_data_selector = Number of training dataset selected.
	// Note:	First columns of training datasets' arrays: xi (input vector).
	//			Last column of training datasets' arrays:	yi (desired output / label).
	
	double custom_training_data_1[4][3] = { // Custom training dataset 1.
		{0, 0, 0},
		{0, 1, 1},
		{1, 0, 1},
		{1, 1, 0},
	};
	
	
	
	
	perceptron_model_t model = {0};
	double input_pots_values[NUMBER_OF_INPUT_POTS] = {0};
	double output = 0;
	
	
	// **** Ciclo principal del sistema:
    while(true){
		
		// **** Arranque/paro del sistema con botón:
		if(button_was_pressed(&button)){
			++system_mode;
			
			switch(system_mode){
				case 0:	// Modo: OFF.
					gpio_digital_write(&led_system_mode, false); print_welcome_message(); break;
				case 1:	// Modo: RUN.
					gpio_digital_write(&led_system_mode, true); break;
				default:
					// **** 7th Step [VERY IMPORTANT]: Free allocated memory used by perceptron model.
					perceptron_free(&model);
					gpio_digital_write(&led_system_mode, false);
					gpio_digital_write(&led_system_output, false);
					system_mode = 0;
					break;
			}
		}
		
		// **** Con el sistema en modo RUN:
		if(system_mode == 1){
			
			// **** 1st Step: Create truth table. This data will be used for training:
			
			uint32_t samples_number;	// No. of rows for training data.
			double** training_samples;
			
			// Selection of training data:
			
			if(training_selector < 7){ 	// Logic gates' truth tables as training data.
				samples_number = (uint32_t) pow(2, (double) dimensions); // Num. rows for truth table.
				training_samples = perceptron_get_truth_table(dimensions, training_selector);
			}
			else if(custom_training_data_selector == 0){	// User selected training data (user input).
				printf("Unfinished code in this option.");
				while(true) vTaskDelay(pdMS_TO_TICKS(10));
				// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MISSING CODE
			}
			else if(custom_training_data_selector == 1){ // Custom training dataset 1.
				samples_number 	= sizeof(custom_training_data_1) / sizeof(custom_training_data_1[0]);
				dimensions		= sizeof(custom_training_data_1[0]) / sizeof(custom_training_data_1[0][0]) - 1;
				
				// Creates dinamically allocated array and assigns its pointers to training_samples:
				training_samples = (double**) malloc((size_t)samples_number * sizeof(double*));
				
				for(uint32_t i = 0; i < samples_number; i++)
					training_samples[i] = (double*) malloc ((size_t) (dimensions + 1) * sizeof(double));
				
				// Assigns values from custom_training_data_1 to training_samples:
				for(uint32_t row = 0; row < samples_number; row++)	// All samples.
					for(uint32_t col = 0; col <= dimensions; col++)	// xi and yi (<= instead of <).
						training_samples[row][col] = custom_training_data_1[row][col];
			}
			else{
				samples_number = 0;
				training_samples = NULL;
				while(true) vTaskDelay(pdMS_TO_TICKS(10));
			}
			
			puts("________________________________________________________\n");
			puts("Training data (truth table):\n");
			
			perceptron_print_data_table(training_samples, dimensions, samples_number);
			
			
			
			
			// **** 2nd Step: Create training data arrays from truth table:
			
			double x[samples_number][dimensions];	// Creates the array of input vectors.
			double y[samples_number];				// Creates the array of labels (desired outputs).
			
			// Assigns the values of the truth table to the training data arrays:
			for(uint32_t row = 0; row < samples_number; row++){
				for(uint32_t col = 0; col < dimensions; col++)
					x[row][col] = training_samples[row][col];	// Assings values to array of input vectors.
				
				// Assigns training_samples's last column of values (2nd index = dimensions) to array of labels:
				y[row] = training_samples[row][dimensions];
			}
			
			perceptron_free_data_table(training_samples, samples_number);	// Frees allocated memory for truth table.
			
			
			
			
			// **** 3rd Step: Creation and initialization of perceptron model:
			
			perceptron_init(&model, dimensions, higher_dim_mapping_op, STEP);
			
			
			// **** 4th Step: Training (Least Mean Squares method):
			
			// Cycle control variables:
			double min_error 	= 0.01;	// Minimum acceptable error in each training sample.
			uint32_t max_epochs	= 500;	// Maximum number of iterations/epochs.
			
			// Parameters:
			double n = 0.5;					// Training rate.
			
			// Trains model:
			perceptron_train(&model, dimensions, samples_number, &x[0][0], y, LMS, min_error, max_epochs, n, true);
			
			
			// **** 5th Step: Testing:
			perceptron_test(&model, dimensions, samples_number, &x[0][0], y, true);
			
			
			// **** 6th step: Inference:
			printf("\nStarting inference...\n\n");
			
			system_mode = 2;
		}
		
		if(system_mode == 2){
			for(uint8_t i = 0; i < NUMBER_OF_INPUT_POTS; i++){
				if(adc_get_raw(&input_pots[i]) >= POTS_BINARY_THRESHOLD)
					input_pots_values[i] = 1;
				else
					input_pots_values[i] = 0;
			}
			
			output = perceptron_predict(&model, &input_pots_values[0], dimensions);
			
			gpio_digital_write(&led_system_output, (bool)output);
		
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		
        vTaskDelay(pdMS_TO_TICKS(20));
    }
	
}
