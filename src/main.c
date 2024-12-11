#include <stdbool.h>
#include <stm8s.h>
#include "main.h"
#include "milis.h"
#include "delay.h"
          

#define CLK_GPIO            GPIOB				
#define CLK_PIN             GPIO_PIN_0		
#define DATA_GPIO           GPIOB				
#define DATA_PIN            GPIO_PIN_4	
#define CS_GPIO             GPIOB				
#define CS_PIN              GPIO_PIN_2		

#define CLK_HIGH 			GPIO_WriteHigh(CLK_GPIO, CLK_PIN)
#define CLK_LOW 			GPIO_WriteLow(CLK_GPIO, CLK_PIN)
#define DATA_HIGH 		    GPIO_WriteHigh(DATA_GPIO, DATA_PIN)
#define DATA_LOW 			GPIO_WriteLow(DATA_GPIO, DATA_PIN)
#define CS_HIGH 			GPIO_WriteHigh(CS_GPIO, CS_PIN)
#define CS_LOW 				GPIO_WriteLow(CS_GPIO, CS_PIN)
#define STAV_BTN            GPIO_ReadInputPin(BTN_PORT, BTN_PIN)

#define NOOP 				0  	    // No operation
#define DIGIT0 				1		// zápis hodnoty na 1. cifru
#define DIGIT1 				2		// zápis hodnoty na 2. cifru
#define DIGIT2 				3		// zápis hodnoty na 3. cifru
#define DIGIT3 				4		// zápis hodnoty na 4. cifru
#define DIGIT4 				5		// zápis hodnoty na 5. cifru
#define DIGIT5 				6		// zápis hodnoty na 6. cifru
#define DIGIT6 				7		// zápis hodnoty na 7. cifru
#define DIGIT7 				8		// zápis hodnoty na 8. cifru
#define DECODE_MODE 	    9		// Aktivace/Deaktivace znakové sady (my volíme vždy hodnotu DECODE_ALL)
#define INTENSITY 		    10	    // Nastavení jasu - argument je číslo 0 až 15 (větší číslo větší jas)
#define SCAN_LIMIT 		    11	    // Volba počtu cifer (velikosti displeje) - argument je číslo 0 až 7 (my dáváme vždy 7)
#define SHUTDOWN 			12	    // Aktivace/Deaktivace displeje (ON / OFF)
#define DISPLAY_TEST 	    15	    // Aktivace/Deaktivace "testu" (rozsvítí všechny segmenty)

#define DISPLAY_ON		    1		// zapne displej
#define DISPLAY_OFF		    0		// vypne displej

#define DISPLAY_TEST_ON 	1	    // zapne test displeje
#define DISPLAY_TEST_OFF 	0	    // vypne test displeje

#define DECODE_ALL			0b11111111 // (lepší zápis 0xff) zapíná znakovou sadu pro všechny cifry
#define DECODE_NONE			0       // vypíná znakovou sadu pro všechny cifry

#define segunda 1000 

uint32_t cas_tim=0;         //TIM2

const uint32_t period=50;   //s pro milis()
void max7219_init(void);
void max7219_posli(uint8_t adresa, uint8_t data);

void init (void){GPIO_Init(BTN_PORT, BTN_PIN, GPIO_MODE_IN_PU_NO_IT);}


/*
void main(void){
    max7219_init();
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovat MCU na 16MHz
    init_milis(); // spustit časovač millis
    
    uint8_t jednotky = 0;

    while (1) {
        static uint32_t last_time = 0;
        max7219_posli(DIGIT0,jednotky);
        max7219_posli(DIGIT1,jednotky);
        max7219_posli(DIGIT2,jednotky);
        max7219_posli(DIGIT3,jednotky);
        max7219_posli(DIGIT4,jednotky);
        max7219_posli(DIGIT5,jednotky);
        max7219_posli(DIGIT6,jednotky);
        max7219_posli(DIGIT7,jednotky);

        if (milis() - last_time >= period) {
            last_time += period;
            jednotky += 1;       
            if (jednotky > 9)   {jednotky = 0;} 
        }
    }
}        
*/


void main(void){
    max7219_init();
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);  // taktovat MCU na 16MHz
    //init_milis();                                 // spustit časovač millis
    TIM2_init();                                    //spustit timer 2
    enableInterrupts();                             //přerušení
    max7219_posli(DIGIT3,0b00001111);
    max7219_posli(DIGIT4,0b00001111);
    max7219_posli(DIGIT5,0b00001111);
    max7219_posli(DIGIT6,0b00001111);
    max7219_posli(DIGIT7,0b00001111);
    
    uint8_t jednotky = 0;
    uint8_t desitky = 0;
    uint8_t stovky = 0;

    uint32_t cas=0;

    while (1) {
    static uint32_t last_time = 0;

    //if (milis() - last_time >= period) {
        //last_time += period;
    if ((cas_tim - cas) > 1000) {
        cas = cas_tim;
        jednotky += 1;       
        if (jednotky > 9) {
            jednotky = 0;
            desitky += 1;
        }
        if (desitky > 9) {
            desitky = 0;
            stovky += 1;
        }
        if (stovky > 9) {
            jednotky = 0;
            desitky = 0;
            stovky = 0;
        }
        if (stovky > 0) {
            max7219_posli(DIGIT2, stovky);
            max7219_posli(DIGIT1, desitky);
            max7219_posli(DIGIT0, jednotky);
        } 
        else if (desitky > 0) {
            max7219_posli(DIGIT2, 0b00001111); 
            max7219_posli(DIGIT1, desitky);
            max7219_posli(DIGIT0, jednotky);
        } 
        else {
            max7219_posli(DIGIT2, 0b00001111); 
            max7219_posli(DIGIT1, 0b00001111); 
            max7219_posli(DIGIT0, jednotky);
        }
    }
    }
}


void TIM2_init(void)
{
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);   // Povolení hodin pro TIM2
    TIM2_TimeBaseInit(TIM2_PRESCALER_16, 999);                  // Předdělička 16, autoreload 999
    TIM2_ClearFlag(TIM2_FLAG_UPDATE);                           // Vymazání příznaku přerušení
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);                      // Povolení přerušení pro update
    TIM2_Cmd(ENABLE);                                           // Spuštění časovače
}


void max7219_init(void){
    GPIO_Init(CS_GPIO,CS_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(CLK_GPIO,CLK_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(DATA_GPIO,DATA_PIN,GPIO_MODE_OUT_PP_LOW_SLOW);

    max7219_posli(DECODE_MODE, DECODE_ALL);     // zapnout znakovou sadu na všech cifrách
    max7219_posli(SCAN_LIMIT, 7);               // velikost displeje 8 cifer (počítáno od nuly, proto je argument číslo 7)
    max7219_posli(INTENSITY, 2);                // volíme ze začátku nízký jas (vysoký jas může mít velkou spotřebu - až 0.25A !)
    max7219_posli(DISPLAY_TEST, DISPLAY_TEST_OFF); 
    max7219_posli(SHUTDOWN, DISPLAY_ON);        // zapneme displej
}

void max7219_posli(uint8_t adresa, uint8_t data){
    uint8_t maska; 
    CS_LOW; 
    maska = 0b10000000; 
    CLK_LOW; 
    while(maska){ 
        if(maska & adresa){ 
            DATA_HIGH; 
        }
        else{ 
            DATA_LOW;	
        }
        CLK_HIGH; 
        maska = maska>>1; 
        CLK_LOW; 
    }

    maska = 0b10000000;
    while(maska){ 
        if(maska & data){ 
            DATA_HIGH; 
        }
        else{ 
            DATA_LOW;	
        }
        CLK_HIGH; 
        maska = maska>>1; 
        CLK_LOW; 
    }
    CS_HIGH; 
}





