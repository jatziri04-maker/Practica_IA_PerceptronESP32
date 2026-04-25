#include "timing.h"


#if TIMING_H_VERSION == 0 // Most portable option: Uses clock() with ms precision.
#include <time.h>
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		timer->is_timer_active = true;
	
	timer->t1 = clock();
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	timer->t2 = clock();
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1 <= timer->t2){
		return ((double)timer->t2 - (double)timer->t1) / (double)CLOCKS_PER_SEC;
	}
	else{
		clock_t t2 = clock();
		return ((double)t2 - (double)timer->t1) / (double)CLOCKS_PER_SEC;
	}	
}

#endif




#if TIMING_H_VERSION == 1 // Windows option: Uses QueryPerformanceCounter() with us precision.
#include <windows.h>
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active){
		QueryPerformanceFrequency(&timer->frequency);
		timer->is_timer_active = true;
	}
	
	QueryPerformanceCounter(&timer->t1);
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	QueryPerformanceCounter(&timer->t2);
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1.QuadPart <= timer->t2.QuadPart){
		return ((double)timer->t2.QuadPart - (double)timer->t1.QuadPart) / (double)timer->frequency.QuadPart;
	}
	else{
		LARGE_INTEGER t2;
		QueryPerformanceCounter(&t2);
		return ((double)t2.QuadPart - (double)timer->t1.QuadPart) / (double)timer->frequency.QuadPart;	
	}	
}

#endif




#if TIMING_H_VERSION == 2 // Linux option: Uses clock_gettime() with ns precision.
#include <time.h>
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		timer->is_timer_active = true;
	
	clock_gettime(CLOCK_MONOTONIC, &timer->t1);
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	clock_gettime(CLOCK_MONOTONIC, &timer->t2);
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1 <= timer->t2){
		return ((double)timer->t2.tv_sec - (double)timer->t1.tv_sec) + ((double)timer->t2.tv_nsec - (double)timer->t1.tv_nsec) / 1000000000.0;
	}
	else{
		struct timespec t2;
		clock_gettime(CLOCK_MONOTONIC, &t2);
		return ((double)t2.tv_sec - (double)timer->t1.tv_sec) + ((double)t2.tv_nsec - (double)timer->t1.tv_nsec) / 1000000000.0;
	}	
}

#endif




#if TIMING_H_VERSION == 3 // Arduino option: micros()
#include <Arduino.h>
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		timer->is_timer_active = true;
	
	timer->t1 = micros();
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	timer->t2 = micros();
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1 <= timer->t2){
		return ((double)timer->t2 - (double)timer->t1) / 1000000.0;
	}
	else{
        // Note: Wrapped around after ~70 minutes
		unsigned long t2 = micros();
		return ((double)t2 - (double)timer->t1) / 1000000.0;
	}	
}

#endif




#if TIMING_H_VERSION == 4 // Arduino option: millis()
#include <Arduino.h>
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		timer->is_timer_active = true;
	
	timer->t1 = millis();
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	timer->t2 = millis();
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1 <= timer->t2){
		return ((double)timer->t2 - (double)timer->t1) / 1000.0;
	}
	else{
		unsigned long t2 = millis();
		return ((double)t2 - (double)timer->t1) / 1000.0;
	}	
}

#endif




#if TIMING_H_VERSION == 5 // Espressif option: esp_timer_get_time()
#include "esp_timer.h"
#include <stdbool.h>

void start_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		timer->is_timer_active = true;
	
	timer->t1 = esp_timer_get_time();
	return;
}

void stop_timing(timing_timer_t* timer){
	if(!timer) return;
	
	if(!timer->is_timer_active)
		start_timing(timer);
	
	timer->t2 = esp_timer_get_time();
	return;	
}

double get_elapsed_time_in_seconds(timing_timer_t* timer){
	if(!timer) return 0.0;
	
	if(timer->t1 <= timer->t2){
		return ((double)timer->t2 - (double)timer->t1) / 1000000.0;
	}
	else{
		int64_t t2 = esp_timer_get_time();
		return ((double)t2 - (double)timer->t1) / 1000000.0;
	}	
}

#endif
