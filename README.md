# Smart-Semaphore-MSP430

Components:
 * LED G1: P1.2
 * LED Y1: P1.5
 * LED R1: P1.4
 * LED G2: P2.3
 * LED Y2: P2.0
 * LED R2: P2.2
 * HALL1 : P1.1
 * HALL2 : P2.1

                MSP430x5xx
            -----------------
         /|\|              XIN|-
          | |                 |
          --|RST          XOUT|-
            |                 |
            |             P1.1|-->HALL1
   LED Y2<--|P2.0             |
            |             P1.2|-->LED G1
    HALL2-->|P2.1             |
            |             P1.4|-->LED R1
   LED R2<--|P2.2             |
            |             P1.5|-->LED Y1
   LED G2<--|P2.3             |
            */
