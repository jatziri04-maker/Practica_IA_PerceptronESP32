#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#define PERCEPTRON_H_VERSION 0
// PERCEPTRON_H_VERSION == 0 -> Windows system.
// PERCEPTRON_H_VERSION == 1 -> Arduino IDE.
// PERCEPTRON_H_VERSION == 2 -> ESP-IDF in Espressif IDE.

// Automatic platform detection:
	#if defined(_WIN32) || defined(_WIN64)
		#undef PERCEPTRON_H_VERSION
	    #define PERCEPTRON_H_VERSION 0
	        
	#elif defined(ARDUINO)
		#undef PERCEPTRON_H_VERSION
	    #define PERCEPTRON_H_VERSION 1
	    
	#elif defined(ESP_PLATFORM) && !defined(ARDUINO)
		#undef PERCEPTRON_H_VERSION
	    #define PERCEPTRON_H_VERSION 2
	    
	#endif


#include <stdbool.h>
#include <stdint.h>

typedef enum{
	IS_EMPTY = 0,
	IS_INITIALIZED,
	IS_TRAINED
} perceptron_flags_e;

typedef enum{
	UNSPECIFIED = 0,
	STEP,
	SIGMOID,
	HYPERBOLIC_TANGENT,
	RELU
} activation_function_e;

typedef enum{
	LMS = 0
} training_algorithm_e;

typedef struct{
	double* 	w;
	uint32_t 	dimensions;
	uint8_t 	non_linear_transformation_power;
	perceptron_flags_e 		flags;
	activation_function_e 	activation_function;
} perceptron_model_t;


	#ifdef __cplusplus
	extern "C" {
	#endif

double** perceptron_get_truth_table(uint32_t dimensions, uint8_t training_selector);
void perceptron_free_data_table(double** data_table, uint32_t samples_number);
void perceptron_print_data_table(double** data_table, uint32_t dimensions, uint32_t samples_number);

double random_double(double min, double max);

bool perceptron_activation_function(double r);

void perceptron_init(perceptron_model_t* model, uint32_t dimensions, uint8_t transformation_pow, activation_function_e activation_function);
void perceptron_free(perceptron_model_t* model);

double perceptron_get_higher_dimension(double* x, uint32_t dimensions, uint8_t transformation_sel);
double perceptron_predict(perceptron_model_t* model, double* x, uint32_t dimensions);
void perceptron_train(perceptron_model_t* model, uint32_t dimensions, uint32_t samples_number, double* x, double* y, training_algorithm_e training, double min_error, uint32_t max_epochs, double n, bool print_process);
uint32_t perceptron_test(perceptron_model_t* model, uint32_t dimensions, uint32_t samples_number, double* x, double* y,bool print_process);

	#ifdef __cplusplus
	}
	#endif


#endif