/*Smart Semaphore MSP430
 * using six LEDs
 * and two HALL sensors
 * LED G1: P1.2
 * LED Y1: P1.5
 * LED R1: P1.4
 * LED G2: P2.3
 * LED Y2: P2.0
 * LED R2: P2.2
 * HALL1 : P1.1
 * HALL2 : P2.1
 *
*                MSP430x5xx
*            -----------------
*         /|\|              XIN|-
*          | |                 |
*          --|RST          XOUT|-
*            |                 |
*            |             P1.1|-->HALL1
*   LED Y2<--|P2.0             |
*            |             P1.2|-->LED G1
*    HALL2-->|P2.1             |
*            |             P1.4|-->LED R1
*   LED R2<--|P2.2             |
*            |             P1.5|-->LED Y1
*   LED G2<--|P2.3             |
*            */
#include <msp430.h> 
#include "clock.h"
#include "pmm.h"

//LEDS
#define LEDG1DIR P1DIR
#define LEDY1DIR P1DIR
#define LEDR1DIR P1DIR
#define LEDG2DIR P1DIR
#define LEDY2DIR P2DIR
#define LEDR2DIR P2DIR
#define LEDG1OUT P1OUT
#define LEDY1OUT P1OUT
#define LEDR1OUT P1OUT
#define LEDG2OUT P1OUT
#define LEDY2OUT P2OUT
#define LEDR2OUT P2OUT
#define LEDG1 BIT2
#define LEDY1 BIT5
#define LEDR1 BIT4
#define LEDG2 BIT3
#define LEDY2 BIT0
#define LEDR2 BIT2

//Hall Sensor
#define HALL1DIR P2DIR
#define HALL2DIR P1DIR
#define HALL1OUT P2OUT
#define HALL2OUT P1OUT
#define HALL1REN P2REN
#define HALL2REN P1REN
#define HALL1IN P2IN
#define HALL2IN P1IN
#define HALL1 BIT1
#define HALL2 BIT1

//State Machine
volatile int estado;
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define S5 5
#define ty 4000000
#define tg 12000000

void tx (char tx_B);
volatile char buff[2];
volatile int count=0;
volatile int rx_done=0;

volatile unsigned int i;

void main()
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Increase core voltage so CPU can run at 16Mhz
    // Step 1 allows clocks up to 12MHz, step 2 allows rising MCLK to 16MHz
    pmmVCore(1);
    pmmVCore(2);

    // Configure clock
    // This should make MCLK @16MHz, SMCLK @1MHz and ACLK @32768Hz
    clockInit();

    // uart init

    P3SEL = BIT3 + BIT4;


//    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
//    UCA0CTL1 |= UCSSEL_2;
//    UCA0BR0 = 0x41;                              // 1MHz 1200 (see User's Guide)
//    UCA0BR1 = 0x03;
//    UCA0MCTL = 0x09;
//    UCA0CTL1 &= ~UCSWRST;                       // exit reset serial

    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset** USCI entra em reset
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 52;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_0 + UCBRF_1 + UCOS16;
    UCA0CTL1 &= ~UCSWRST;
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
    _enable_interrupts();


    tx(10);

}

void tx (char tx_B){
    int cnt;

                  // Outputs: LEDs
                   LEDG1DIR |=LEDG1;
                   LEDY1DIR |=LEDY1;
                   LEDR1DIR |=LEDR1;
                   LEDG2DIR |=LEDG2;
                   LEDY2DIR |=LEDY2;
                   LEDR2DIR |=LEDR2;

                   // Inputs: HALL
                   HALL1DIR &= ~HALL1;
                   HALL2DIR &= ~HALL2;
                   HALL1REN |= HALL1;
                   HALL2REN |= HALL2;
                   HALL1OUT |= HALL1;
                   HALL2OUT |= HALL2;

                   // Adjustments
                   //LEDS G1 and R2 on
                   LEDG1OUT |= LEDG1;
                   LEDY1OUT &=~ LEDY1;
                   LEDR1OUT &=~ LEDR1;
                   LEDG2OUT &=~ LEDG2;
                   LEDY2OUT &=~ LEDY2;
                   LEDR2OUT |= LEDR2;
                   //initial state
                   estado=S0;
                   int p_state1=0;                 // previous state of LED1
                   int p_state2=0;                 // previous state of LED2
                   int anterior = 0;
                   int anterior2 = 0;

    for (;;)
    {

        if(((HALL1IN&HALL1) && (HALL2IN&HALL2))==1){ //no Hall
          for(i=0;i<13000;i++); //debouncing
          UCA0TXBUF = 4;
        }
        else if(((HALL1IN&HALL1) && !(HALL2IN&HALL2))==1){//HALL 2
          for(i=0;i<13000;i++); //debouncing
          UCA0TXBUF = 3;
        }
        else if((!(HALL1IN&HALL1) && (HALL2IN&HALL2))==1){//HALL 1
          for(i=0;i<13000;i++); //debouncing
          UCA0TXBUF = 2;
        }
        else if((!(HALL1IN&HALL1) && !(HALL2IN&HALL2))==1){//HALL 1 and HALL 2
          for(i=0;i<13000;i++); //debouncing
          UCA0TXBUF = 1;
        }


        while ((UCA0STAT & UCBUSY));
        //

        if(rx_done == 1){                     //flag
        //
            if(buff[0] == 1)             // HALL1 e HALL2
            {
                    switch(estado)
                    {
                      case S0:
                        //LEDS G1 and R2 on
                      LEDG1OUT |= LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(tg);
                      estado=S1;
                      break;
                      case S1:
                        //LEDS Y1 and R2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT |= LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(ty);
                      estado=S2;
                      break;
                      case S2:
                        //LEDS R1 and G2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT |= LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(tg);
                      estado=S3;
                      break;
                      case S3:
                        //LEDS R1 and Y2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT |= LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(ty);
                      estado=S0;
                      break;
                    }
                    }
            if(buff[0] == 2){        //HALL1 on
                    switch(estado)
                    {
                      case S0:
                        //LEDS G1 and R2 on
                      LEDG1OUT |= LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(tg);
                      estado=S0;
                      break;
                      case S1:
                        //LEDS Y1 and R2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT |= LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(ty);
                      estado=S2;
                      break;
                      case S2:
                        //LEDS R1 and G2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT |= LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(tg);
                      estado=S3;
                      break;
                      case S3:
                        //LEDS R1 and Y2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT |= LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(ty);
                      estado=S0;
                      break;
                    }

                    }


            if(buff[0] == 3)             // HALL2
            {
                   switch(estado)
                    {
                      case S0:
                        //LEDS G1 and R2 on
                      LEDG1OUT |= LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(tg);
                      estado=S1;
                      break;
                      case S1:
                        //LEDS Y1 and R2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT |= LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(ty);
                      estado=S2;
                      break;
                      case S2:
                        //LEDS R1 and G2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT |= LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(tg);
                      estado=S3;
                      break;
                      case S3:
                        //LEDS R1 and Y2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT |= LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(ty);
                      estado=S0;
                      break;
                    }
                    }
            if(buff[0] == 4){                                       // no sensor
                    switch(estado)
                    {
                      case S0:
                        //LEDS G1 and R2 on
                      LEDG1OUT |= LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(tg);
                      estado=S0;
                      break;
                      case S1:
                        //LEDS Y1 and R2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT |= LEDY1;
                      LEDR1OUT &=~ LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT |= LEDR2;
                      __delay_cycles(ty);
                      estado=S2;
                      break;
                      case S2:
                        //LEDS R1 and G2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT |= LEDG2;
                      LEDY2OUT &=~ LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(tg);
                      estado=S2;
                      break;
                      case S3:
                        //LEDS R1 and Y2 on
                      LEDG1OUT &=~ LEDG1;
                      LEDY1OUT &=~ LEDY1;
                      LEDR1OUT |= LEDR1;
                      LEDG2OUT &=~ LEDG2;
                      LEDY2OUT |= LEDY2;
                      LEDR2OUT &=~ LEDR2;
                      __delay_cycles(ty);
                      estado=S0;
                      break;
                    }
                    }
            }
       //
        rx_done = 0;
        }
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

    buff[0]=UCA0RXBUF;
    rx_done = 1;

}
