#include "mcc_generated_files/mcc.h"

// Variables
uint16_t convertion_A0 = 0;
uint8_t secondes;
uint16_t total;
uint8_t etat;

enum etats {
    enable, disable
};

void intertimer(void) {
    secondes++;
}

/*
                         Main application
 */
void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    TMR0_SetInterruptHandler(intertimer);
    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    uCAN_MSG rxCan;
    rxCan.frame.id = 0;
    rxCan.frame.dlc = 0;
    rxCan.frame.data0 = 0;

    uCAN_MSG txCan;
    txCan.frame.id = 0x1C0;
    txCan.frame.dlc = 2;
    txCan.frame.data0 = 0;
    txCan.frame.data1 = 0;
    
    etat = disable;

    while (1) {
        if(CAN_receive(&rxCan) >= 1){
            if ((rxCan.frame.id == 0x100) || (rxCan.frame.id == 0x120)) {
                if (rxCan.frame.data0 != 0x00) {                  
                    secondes = 0;
                    etat = disable;
                } else {
                    etat = enable;
                }
            }
        }
        if (secondes >= 20) {
            secondes = 0;
            //ADC
            if (etat == enable) {
                convertion_A0 = ADC_GetConversion(channel_AN0);
                convertion_A0 = ((float) convertion_A0 / 4095)*300;
                if (convertion_A0 > 300) {
                    convertion_A0 = 0;
                }
                txCan.frame.data0 = (convertion_A0 >> 8) & 0xFF;
                txCan.frame.data1 = convertion_A0 & 0xFF;
                total = (txCan.frame.data0 << 8) + txCan.frame.data1;
                // Add your application code
                CAN_transmit(&txCan);
            }
        }

    }
}

/**
 End of File
 */
