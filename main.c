#include "mcc_generated_files/mcc.h"

// Variables
uint16_t convertion_A0 = 0;
uint8_t seconde;
uint8_t verif_connex;
uint16_t total;
uint8_t etat;

enum etats {
    enable, disable
};

void intertimer(void) {
    seconde++;
    verif_connex++;
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

    // Channel 1
    RXM0SIDH = 0xFB; // Masque 0xFDE0
    RXM0SIDL = 0xE0;
    RXF0SIDH = 0x20; // Filtre juste lire 0x120 et 0x100
    RXF0SIDL = 0x00;
    // Channel 2
    RXM1SIDH = 0xFB; // Masque 0xFDE0
    RXM1SIDL = 0xE0;
    RXF1SIDH = 0x20; // Filtre juste lire 0x120 et 0x100
    RXF1SIDL = 0x00;

    uCAN_MSG rxCan;
    rxCan.frame.id = 0;
    rxCan.frame.dlc = 0;
    rxCan.frame.data0 = 0;

    uCAN_MSG txCan;
    txCan.frame.id = 0x1C0; //Assignation ID Panneau Solaire
    txCan.frame.dlc = 2;
    txCan.frame.data0 = 0;
    txCan.frame.data1 = 0;

    etat = disable;

    while (1) {
        if (CAN_receive(&rxCan) >= 1) { //Si réception sur bus CAN
            verif_connex = 0; //Remettre à zero detecteur de perte de connexion
            //Si message provient de ID Interface ou Capteur Vent
            if ((rxCan.frame.id == 0x100) || (rxCan.frame.id == 0x120)) {
                if (rxCan.frame.data0 != 0x00) {//Si donnée est diff. de zéro
                    seconde = 0; //compteur seconde remis à zéro
                    etat = disable; //on passe à l'état désactiver le panneau
                    IO_RC0_SetHigh(); //Allumer la LED Rouge
                    IO_RC1_SetLow(); //Éteindre la LED Verte
                } else { //Si donnée est égale à zéro
                    etat = enable; //Passer à l'état Panneau Solaire actif
                    IO_RC1_SetHigh(); //Allumer la LED Verte
                    IO_RC0_SetLow(); //Éteindre la LED Rouge
                }
            }
        } 
        if (verif_connex >= 40){ //Si détecte perte de connexion plus que 2 secs
            IO_RC0_SetHigh(); //Allumer la LED Rouge
        }
        if (seconde >= 20) { //Si une seconde s'écoule
            seconde = 0; //Remettre compteur seconde à zéro
            //ADC
            if (etat == enable) { //Si Panneau Solaire est actif
                //Convertion ADC de la photorésistance
                convertion_A0 = ADC_GetConversion(channel_AN0);
                convertion_A0 = ((float) convertion_A0 / 4095)*300;
                if (convertion_A0 > 300) {
                    convertion_A0 = 0;
                }
                //Bitshift pour transférer données lues sur BUS CAN
                txCan.frame.data0 = (convertion_A0 >> 8) & 0xFF;
                txCan.frame.data1 = convertion_A0 & 0xFF;
                total = (txCan.frame.data0 << 8) + txCan.frame.data1;
                CAN_transmit(&txCan); //Transmission données sur BUS CAN
            }
        }

    }
}

/**
 End of File
 */
