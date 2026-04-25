/* ********************** TAREA 1: SISTEMAS EXPERTOS *********************** 
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA
* CARRERA:      ING. ELECTRÓNICA - INTELIGENCIA ARTIFICIAL EN SISTEMAS EMBEBIDOS
* MATERIA:      INTELIGENCIA ARTIFICIAL
* PROFESOR:	    DR. JUAN ALBERTO RAMÍREZ QUINTANA
* ALUMNOS:		JATZIRI DENNISE ROMERO BUSTILLOS		22061040
				ABDIEL ALEJANDRO RODRÍGUEZ CORONADO		22061055
* PERIODO:		ENE-JUN 2026
* ARCHIVO:		"ADC.c"
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
#include "esp_adc/adc_oneshot.h"




/* ************************************************************************* */
/* ************************ DEFINICIÓN DE FUNCIONES ************************ */


/* ************** FUNCIONES BÁSICAS PARA MANEJAR PUERTOS ADC *************** */

// **** Función para inicializar la unidad ADC a utilizar:
void adc_unit_init(adc_unit_t adc_unit){
	
	adc_oneshot_unit_init_cfg_t unit_cfg = {
		.unit_id	= adc_unit,
		.ulp_mode	= ADC_ULP_MODE_DISABLE
	};
	
    ESP_ERROR_CHECK(adc_oneshot_new_unit(
		&unit_cfg, 
		&adc_handle
	));
	
	return;
}


// **** Función para calibrar unidad del ADC:
bool init_calibration(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle){
	adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_10,
        .default_vref = 1100,  // Default reference voltage in mV (will use eFuse if available)
    };
    
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, out_handle);
    if (ret == ESP_OK) {
        ESP_LOGI("LM35", "Calibration scheme created successfully");
    } else {
        ESP_LOGE("LM35", "Failed to create calibration scheme: %s", esp_err_to_name(ret));
        if (ret == ESP_ERR_NOT_SUPPORTED) {
            ESP_LOGW("LM35", "Calibration eFuse not burnt - using uncalibrated readings");
        }
    }
    return ret == ESP_OK;
	
}


// **** Función para configurar el canal ADC a utilizar:
void adc_channel_init(adc_setup_t* adc){
	
	adc_oneshot_chan_cfg_t channel_cfg = {
		.atten		= ADC_ATTEN_DB_12,	// Rango de entrada: 0-3.3V
		.bitwidth	= adc->resolution
	};
	
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
		adc_handle, 
		adc->channel, 
		&channel_cfg
	));
	
	return;
}


// **** Función para leer dato en canal ADC:
uint16_t adc_get_raw(adc_setup_t* adc){
	int reading;
	
	ESP_ERROR_CHECK(adc_oneshot_read(
		adc_handle, 
		adc->channel, 
		&reading
	));
	adc->gpio.state = (uint16_t)reading;
	
	if(adc->gpio.inverted_logic)
		return (pow(2, adc->resolution) - 1) - adc->gpio.state;
	else
 		return adc->gpio.state;
}
