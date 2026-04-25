#include <stdio.h>	
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>	
#include <limits.h>		

#include "perceptron.h"
#include "timing.h"

// Helper macro for cleaner access to 2D array's elements:
#define X2(sample, dim) 	x[(sample) * dimensions + (dim)]
#define X1(sample) 		x[(sample) * dimensions]

#if PERCEPTRON_H_VERSION == 1 // Arduino IDE:
	#include <Arduino.h>
    #define PERCEPTRON_PRINT(...) Serial.printf(__VA_ARGS__)

#else
	#include <time.h>
    #define PERCEPTRON_PRINT(...) printf(__VA_ARGS__)	
	
#endif

// **** Function to generate truth table and output values for "AND" or "OR" logic functions:
double** perceptron_get_truth_table(uint32_t dimensions, uint8_t training_selector){
	
	// **** Checks if there is a valid number of dimensions:
	if(dimensions < 1){
		PERCEPTRON_PRINT("Error: The number of dimensions must be at least 1.");
		
		#if PERCEPTRON_H_VERSION == 0 	// Windows:
			exit(EXIT_FAILURE);			// - Terminate program with failure status.
		#else 							// Arduino IDE and ESP-IDF:
			while(true);				// - Infinite loop.
		#endif
	}
	
	
	
	
	// **** Creates truth table:
	uint32_t number_of_rows = (uint32_t) pow(2, (double) dimensions);	// No. of rows for truth table.
	
	//double truth_table[number_of_rows][dimensions + 1] = {0};
	double** truth_table = (double**) malloc((size_t)number_of_rows * sizeof(double*));
	
	for(uint32_t i = 0; i < number_of_rows; i++)
		truth_table[i] = (double*) malloc ((size_t) (dimensions + 1) * sizeof(double));
	
	
	// **** Fills input columns of truth table:
	for(uint32_t col = 0; col < dimensions; col++){
		// Period to alternate input values:
		uint32_t period = (uint32_t) pow(2, (double) (dimensions - (col + 1)));
		
		// Fills every cell (row) in column:
		bool input_value = 0;
		for(uint32_t row = 0; row < number_of_rows; row++){
			truth_table[row][col] = (double) input_value;	// Assigns value to table cell.
			
			if((row + 1) % period == 0)		// Switches value if period is completed.
				input_value = !input_value;
		}
	}
	
	
	
	
	// **** Fills output column of truth table:
	if(training_selector >= 1 && training_selector <= 6){
		
		switch(training_selector){
			// AND:
			case 1:
				for(uint32_t row = 0; row < number_of_rows; row++){
					bool output = (bool) truth_table[row][0];
					
					for(uint32_t col = 1; col < dimensions; col++)
						output &= (bool) truth_table[row][col];
					
					truth_table[row][dimensions] = (double) output;	
				}
				break;
			
			
			// OR:
			case 2:
				for(uint32_t row = 0; row < number_of_rows; row++){
					bool output = (bool) truth_table[row][0];
					
					for(uint32_t col = 1; col < dimensions; col++)
						output |= (bool) truth_table[row][col];
					
					truth_table[row][dimensions] = (double) output;	
				}
				break;
			
			// NAND:
			case 3:
				for(uint32_t row = 0; row < number_of_rows; row++){
					bool output = (bool) truth_table[row][0];
					
					for(uint32_t col = 1; col < dimensions; col++)
						output &= (bool) truth_table[row][col];
					
					truth_table[row][dimensions] = (double) !output;	
				}
				break;
				
			// NOR:
			case 4:
				for(uint32_t row = 0; row < number_of_rows; row++){
					bool output = (bool) truth_table[row][0];
					
					for(uint32_t col = 1; col < dimensions; col++)
						output |= (bool) truth_table[row][col];
					
					truth_table[row][dimensions] = (double) !output;	
				}
				break;
				
			// XOR:
			case 5:
				for(uint32_t row = 0; row < number_of_rows; row++){
					uint32_t ones_counter = 0;
					
					for(uint32_t col = 0; col < dimensions; col++){
						if(truth_table[row][col] == 1)
							ones_counter++;
					}
					
					// Determine output's value:
					if(ones_counter == 1)
						truth_table[row][dimensions] = 1;
					else
						truth_table[row][dimensions] = 0;
				}
				break;
				
			// XNOR:
			case 6:
				for(uint32_t row = 0; row < number_of_rows; row++){
					uint32_t ones_counter = 0;
					
					for(uint32_t col = 0; col < dimensions; col++){
						if(truth_table[row][col] == 1)
							ones_counter++;
					}
					
					// Determine output's value:
					if(ones_counter == 1)
						truth_table[row][dimensions] = 0;
					else
						truth_table[row][dimensions] = 1;
				}
				break;
			
			default:
				PERCEPTRON_PRINT("Error: The training selector must have a value between 1 and 7");
				
				#if PERCEPTRON_H_VERSION == 0 	// Windows:
					exit(EXIT_FAILURE);			// - Terminate program with failure status.
				#else 							// Arduino IDE and ESP-IDF:
					while(true);				// - Infinite loop.
				#endif
				
				break;
		}
	}
	
	return truth_table;
}




// **** Function to free dinamically allocated memory to store data table:
void perceptron_free_data_table(double** data_table, uint32_t samples_number){
	for(uint32_t i = 0; i < samples_number; i++)
		free(data_table[i]);
	
	free(data_table);
	
	return;
}




// **** Function to print truth table on console:
void perceptron_print_data_table(double** data_table, uint32_t dimensions, uint32_t samples_number){
	// Table's header (variables' names):
	for(uint32_t col = 0; col < dimensions; col++){
		PERCEPTRON_PRINT("| \tX%u \t", (unsigned int)col);
	}
	PERCEPTRON_PRINT("| \tY \t|\n");
	
	// Table's body (values):
	for(uint32_t row = 0; row < samples_number; row++){
		for(uint32_t col = 0; col <= dimensions; col++){
			PERCEPTRON_PRINT("| \t%g \t", data_table[row][col]);
		}
		PERCEPTRON_PRINT("|\n");
	}
	return;
}


// Function to generate a uniformly distributed float in [min, max)
double random_double(double min, double max){
	static bool is_first_function_call = true;
	
	#if PERCEPTRON_H_VERSION == 1 // Arduino IDE:
		if(is_first_function_call){
			randomSeed(analogRead(A0) + millis()); // Seeds the random number generator.
			is_first_function_call = false;
		}
		
	    if(min > max){ // Swaps if range is reversed:
	        double temp = min;
	        min = max;
	        max = temp;
	    }
	    
	    // Note: "random(0, 10001) / 10000.0" gets a random value between 0 and 1, which gets scaled to the range:
	    return min + (random(0, 10001) / 10000.0) * (max - min);
	    
	#else // Windows and ESP-IDF:
		if(is_first_function_call){
			srand((unsigned int)time(NULL)); // Seeds the random number generator with current time.
			is_first_function_call = false;
		}
		
	    if(min > max){ // Swaps if range is reversed:
	        double temp = min;
	        min = max;
	        max = temp;
	    }
	    
	    // Note: rand() / RAND_MAX gets a random value between 0 and 1, which gets scaled to the range:
	    return min + ((double)rand() / ((double)RAND_MAX)) * (max - min);
	    
    #endif
}


// **** Function that defines the activation function of neuron:
bool perceptron_activation_function(double r){
	if(r >= 0) 	return 1;
	else 		return 0;
}




void perceptron_init(perceptron_model_t* model, uint32_t dimensions, uint8_t transformation_pow, activation_function_e activation_function){
	model->dimensions = dimensions;
	model->non_linear_transformation_power = transformation_pow;
	model->activation_function = activation_function;
	
	model->w = (double*) malloc((size_t)(dimensions + (transformation_pow != 0) + 1) * sizeof(double));
	// Note: 	- w's first elements correspond to w1, w2, ..., wp, where p = no. dimensions.
	//			- w's last element always corresponds to w0 (bias).
	//			- w's second to last value might correspond to either wp or wp+1 (extra 
	//			  dimension, in case non_linear_transformation_power > 0).
	
	model->flags = IS_INITIALIZED;
	
	return;
}

void perceptron_free(perceptron_model_t* model){
	model->dimensions = 0;
	model->non_linear_transformation_power = 0;
	model->activation_function = UNSPECIFIED;
	
	free(model->w);
	
	model->flags = IS_EMPTY;
	
	return;
}


double perceptron_get_higher_dimension(double* x, uint32_t dimensions, uint8_t transformation_sel){
	double x_extra_dimension = 0;
	
	switch(transformation_sel){
		case 2:
			for(uint32_t i = 0; i < dimensions; i++)
				x_extra_dimension += x[i];
				
			x_extra_dimension = pow(x_extra_dimension, transformation_sel);
			break;
		
		default:
			return 0;
	}
	return x_extra_dimension;
}


double perceptron_predict(perceptron_model_t* model, double* x, uint32_t dimensions){
	if(dimensions != model->dimensions){
		PERCEPTRON_PRINT("\nError: Input vector's dimensions don't match perceptron's dimensions.\n");
		
		#if PERCEPTRON_H_VERSION == 0 	// Windows:
			exit(EXIT_FAILURE);			// - Terminate program with failure status.
		#else 							// Arduino IDE and ESP-IDF:
			while(true);				// - Infinite loop.
		#endif	
	}
	
	double r = 0;
	
	for(uint32_t i = 0; i < dimensions; i++) // Calculates prediction value of sample.
		r += model->w[i] * x[i]; 	// r = w1*x1 + w2*x2 + ... wp*xp + ...
	
	switch(model->non_linear_transformation_power){
		case 2:
			r += model->w[dimensions] * perceptron_get_higher_dimension(x, dimensions, model->non_linear_transformation_power); // ... + wp+1 * xp+1 + ...
			break;
			
		default:
			break;
	}
	
	r += model->w[dimensions + (model->non_linear_transformation_power != 0)];		// ... + w0.
	
	return (double) perceptron_activation_function(r);
}


void perceptron_train(perceptron_model_t* model, uint32_t dimensions, uint32_t samples_number, double* x, double* y, training_algorithm_e training, double min_error, uint32_t max_epochs, double n, bool print_process){
	uint32_t w_length = dimensions + 1 + (model->non_linear_transformation_power != 0);
	timing_timer_t timer_1 = {0}; // Creates timer, in case user wants to time training.
	
	// Ckecks if input vectors' dimensions match perceptron's dimensions:
	if(dimensions != model->dimensions){
		if(print_process)
			PERCEPTRON_PRINT("\nError: Input vectors' dimensions don't match perceptron's dimensions.\n");
		
		#if PERCEPTRON_H_VERSION == 0 	// Windows:
			exit(EXIT_FAILURE);			// - Terminate program with failure status.
		#else 							// Arduino IDE and ESP-IDF:
			while(true);				// - Infinite loop.
		#endif	
	}
	
	switch(training){
		case LMS: { // Least Mean Squares training method.
			double error[samples_number];	// Array of errors in each training sample.
			
			if(print_process){				
				PERCEPTRON_PRINT("\n________________________________________________________\n\n");
				PERCEPTRON_PRINT("Training LDA with LMS...\n\n");
				PERCEPTRON_PRINT("Initializing weights with random values:\n\n");
				
				start_timing(&timer_1); // Starts timing training execution.
			}
			
			// Initializes weights with random values:
			for(uint32_t i = 0; i < w_length; i++){
				model->w[i] = random_double(0, 1);
				if(print_process) PERCEPTRON_PRINT("%.3f \t", model->w[i]);
			}
			if(print_process) PERCEPTRON_PRINT("\n\n");
			
			if(print_process && model->non_linear_transformation_power != 0){
				PERCEPTRON_PRINT("Note: w[-2] = %.3f corresponds to a new dimension,\n", model->w[w_length - 2]);
				PERCEPTRON_PRINT("which was added to apply a non-linear transformation\n");
				PERCEPTRON_PRINT("to input data in order to solve special cases (like XOR).\n\n");
			}
			
			// Trains LDA:
			for(uint32_t epoch = 0; epoch < max_epochs; epoch++){
				// Epoch:
				
				uint32_t errors_greater_than_min_error = 0;	// Number of errors above minimum error.
				
				// Executes as many iterations as the number of training values:
				for(uint32_t sample = 0; sample < samples_number; sample++){
					// Gets error between prediction and real output:
					error[sample] = y[sample] - perceptron_predict(model, &X1(sample), dimensions);
					
					// Adjusts new weights accordingly:
					
					for(uint32_t i = 0; i < dimensions; i++)
						model->w[i] += n * error[sample] * X2(sample, i);
					
					switch(model->non_linear_transformation_power){
						case 2: {
							double x_extra_dimension = 0;
							x_extra_dimension = perceptron_get_higher_dimension(&X1(sample), dimensions, model->non_linear_transformation_power);
							model->w[w_length - 2] += n * error[sample] * x_extra_dimension; // ... + wp+1 * xp+1 + ...
							break;
						}							
						default:
							break;
					}
					
					model->w[w_length - 1] += n * error[sample];
					errors_greater_than_min_error += (fabs(error[sample]) >= min_error) ? 1 : 0;
				}
				
				if(print_process) PERCEPTRON_PRINT("Epoch: %u \t| Number of errors above threshold: %u\n", (unsigned int)epoch, (unsigned int)errors_greater_than_min_error);
				
				if(errors_greater_than_min_error == 0)	// If the number of errors equals 0. Stops training.
					break;
			}
			break;
		}
		default:
			if(print_process){
				PERCEPTRON_PRINT("\nError: Invalid training algorithm selected.\n");
				//PERCEPTRON_PRINT("Please, make sure to have already initialized the model with the following function:\n");
				//PERCEPTRON_PRINT("perceptron_init(perceptron_model_t* model, uint32_t dimensions, uint8_t transformation_pow, activation_function_e activation_function)");
			}
			
			#if PERCEPTRON_H_VERSION == 0 	// Windows:
				exit(EXIT_FAILURE);			// - Terminate program with failure status.
			#else 							// Arduino IDE and ESP-IDF:
				while(true);				// - Infinite loop.
			#endif
			break;
	}
	
	// Training finished. 
	
	// Prints weights:
	if(print_process){
		PERCEPTRON_PRINT("\nTraining finished. These are the resulting weights:\n\n");
	
		for(uint32_t i = 0; i < w_length; i++)
			PERCEPTRON_PRINT("%.3f \t", model->w[i]);
			
		if(model->non_linear_transformation_power != 0){
			PERCEPTRON_PRINT("\n\nNote: w[-2] = %.3f corresponds to a new dimension.\n", model->w[w_length - 2]);
			PERCEPTRON_PRINT("In most cases w[-2]*(xp+1) = w[-2]*(sum(x)^d) (generally d = 2).\n");
		}
		
		// Stops timing and prints how much time training took.
		stop_timing(&timer_1);
		PERCEPTRON_PRINT("\n\nTraining took %.6lf seconds.\n\n", get_elapsed_time_in_seconds(&timer_1));
	}
	
	model->flags = IS_TRAINED;
			
	return;
}

uint32_t perceptron_test(perceptron_model_t* model, uint32_t dimensions, uint32_t samples_number, double* x, double* y, bool print_process){
	
	// Ckecks if input vectors' dimensions match perceptron's dimensions:
	if(dimensions != model->dimensions){
		if(print_process)
			PERCEPTRON_PRINT("\nError: Input vectors' dimensions don't match perceptron's dimensions.\n");
		
		#if PERCEPTRON_H_VERSION == 0 	// Windows:
			exit(EXIT_FAILURE);			// - Terminate program with failure status.
		#else 							// Arduino IDE and ESP-IDF:
			while(true);				// - Infinite loop.
		#endif	
	}
	
	if(print_process){
		PERCEPTRON_PRINT("\n________________________________________________________\n\n");
		PERCEPTRON_PRINT("Running tests...\n\n");
		PERCEPTRON_PRINT("Results: \n");
	}
	
	uint32_t number_of_errors = 0;
	
	for(uint32_t sample = 0; sample < samples_number; sample++){
		bool out = (bool) perceptron_predict(model, &X1(sample), dimensions);
		
		if(out != y[sample])
			number_of_errors++;
			
		if(print_process) PERCEPTRON_PRINT("%d ", out); // Prints results of all samples. 
	}
	
	if(print_process){
		PERCEPTRON_PRINT("\n\nSuccessful predictions: %u/%u\n\n", (unsigned int)(samples_number - number_of_errors), (unsigned int)samples_number);
		PERCEPTRON_PRINT("Testing finished.\n");
	}
	
	return number_of_errors;
}
