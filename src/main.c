#include <stdbool.h>
#include <stm8s.h>
#include <stdio.h>
#include "main.h"
#include "milis.h"
//#include "delay.h"
#include "uart1.h"
#include "daughterboard.h"
#include "adc_helper.h"

void init(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktování ma 16 MHz
    init_milis();
    init_uart1();

    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL14, DISABLE); // disablem se vypne vstupní buffer, kompenzace zákmitu
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL15, DISABLE);
    // nastavíme clock pro ADC 2 (16 MHz / 4 = 4 MHz)
    ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);
    // na jaké maximální frekvenci může A/D převodník fungovat, chceme frekvenci co největší, 
    // musíme ji ale stáhnout pod určitou mez, proto zde zavádíme děličku
    ADC2_AlignConfig(ADC2_ALIGN_RIGHT); // ve většině případů chceme doprava - typicky doprava
    // budeme potřebovat 2 osmibitové registry
    // nastavíme multiplexer na **jakýkoli** kanál
    ADC2_Select_Channel(ADC2_CHANNEL_14);
    // rozběhneme AD převodník
    ADC2_Cmd(ENABLE);
    // počkáme, až se rozběhne ADC (necelých 7 us)
    ADC2_Startup_Wait();
}

int main(void)
{
    uint32_t time = 0;
    uint16_t vref, vtemp, temp;
    init();

    while (1) {
        if(milis() - time > 1000) {
            time = milis();

            vref = ADC_get(CHANNEL_VREF) * (5000L + 512) / 1023; // konstanta typu long int, 
            // jeden z operandů chceme, aby bylo 32 bitové číslo 
            // (1023 * 5000 se do 16 bitů nevleze, poté se podělí a do 16 b výsledku se už vleze)
            // dělení jako poslední - dělení je ztrátová operace
            ADC_get(CHANNEL_VTEMP);
            ADC_get(CHANNEL_VTEMP);
            ADC_get(CHANNEL_VTEMP);
            vtemp = (uint32_t)ADC_get(CHANNEL_VTEMP) * (5000L + 512) / 1023; // napájecí napětí je Ucc = 5 V
            // + polovina jmenovatele (zajištění správného zaokrouhlování), v našem případě + 512
            temp = (vtemp - 400) / 19.5;
            // menší přesnost
            vtemp = ((vtemp * 2495L) + (vref/2)) / vref;
            // korekci chyby pomocí katologové ref. hodnoty - počítáme se stejnou chybou
            temp = (100L*vtemp - 40000L + (195/2)) / 195;
            // větší přesnost
            printf("%u mV, %u mV %u,%u ˚C\n", vref, vtemp, temp/10, temp%10);
            // celočíselnou část a desetinnou část rozdělíme
        }
    }
}


/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
