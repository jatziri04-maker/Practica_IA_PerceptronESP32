#ifndef TIMING_H
#define TIMING_H

#define TIMING_H_VERSION 1
// VERSION == 0 -> Most portable option: 	Uses clock() with ms precision.
// VERSION == 1 -> Windows option: 			Uses QueryPerformanceCounter() with us precision.
// VERSION == 2 -> Linux option: 			Uses clock_gettime() with ns precision.
// VERSION == 3 -> Arduino option: 			Uses micros() with 4 us precision. Interrupts must be enabled with "interrupts()".
// VERSION == 4 -> Arduino option: 			Uses millis() with ms precision.
// VERSION == 5 -> Espressif option:		Uses esp_timer_get_time() with us precision.


// Automatic platform detection:
	#if defined(_WIN32) || defined(_WIN64)
		#undef TIMING_H_VERSION
	    #define TIMING_H_VERSION 1
	    
	#elif defined(__linux__) || defined(__linux) || defined(linux)
		#undef TIMING_H_VERSION
	    #define TIMING_H_VERSION 2
	    
	#elif defined(__APPLE__) && defined(__MACH__)
		#undef TIMING_H_VERSION
	    #define TIMING_H_VERSION 2
	    
	#elif defined(ARDUINO)
		#undef TIMING_H_VERSION
	    #define TIMING_H_VERSION 3
	    
	#elif defined(ESP_PLATFORM) && !defined(ARDUINO)
		#undef TIMING_H_VERSION
	    #define TIMING_H_VERSION 5
	    
	#endif


	#if TIMING_H_VERSION == 0 // Most portable option: Uses clock() with ms precision.
		#include <time.h>
		#include <stdbool.h>
		
		typedef struct{
			clock_t t1, t2;
			bool is_timer_active;
		} timing_timer_t;
	
	
	#elif TIMING_H_VERSION == 1 // Windows option: Uses QueryPerformanceCounter() with us precision.
		#include <windows.h>
		#include <stdbool.h>
		
		typedef struct{
			LARGE_INTEGER frequency, t1, t2;
			bool is_timer_active;
		} timing_timer_t;
		
	
	#elif TIMING_H_VERSION == 2 // Linux option: Uses clock_gettime() with ns precision.
		#include <time.h>
		#include <stdbool.h>
		
		typedef struct{
			struct timespec t1, t2;
			bool is_timer_active;
		} timing_timer_t;
		
	
	#elif TIMING_H_VERSION == 3 || TIMING_H_VERSION == 4 // Arduino option.
	    // Arduino environment (includes both Uno and ESP32 Arduino core)
	    #include <Arduino.h>
		#include <stdbool.h>
		
		typedef struct{
			unsigned long t1, t2;
			bool is_timer_active;
		} timing_timer_t;
		
	
	#elif TIMING_H_VERSION == 5 // Espressif option: Uses esp_timer_get_time() with us precision.
    	// ESP-IDF environment (without Arduino)
    	#include "esp_timer.h"
		#include <stdbool.h>
		
		typedef struct{
			int64_t t1, t2;
			bool is_timer_active;
		} timing_timer_t;
	
	
	#else
		#error "Unsupported platform"
		
	#endif
	
	
#ifdef __cplusplus
extern "C" {
#endif

void start_timing(timing_timer_t* timer);
void stop_timing(timing_timer_t* timer);
double get_elapsed_time_in_seconds(timing_timer_t* timer);

#ifdef __cplusplus
}
#endif


#endif