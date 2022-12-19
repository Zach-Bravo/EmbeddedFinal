#include <msp430.h> 

/* Digital GPIO
 *      LED Visual Response
 *       - Pin 3.0 Output Pulse From PWM System (Timer)
 *       - May Be Unable To Use Timer Due To Physical Timer Output
 *       - If Internal Timer Can Be Used Internally Then We Can
 *       - Function Based On Light PIR Sensor
 *      PIR Sensor
 *       - Pin 2.2 Input
 *       - When "On", Interrupt Or Store Value Via Polling Process.
 *
 *
 * I2C
 *      Thermal Sensor
 *       - Pin 1.3 & Pin 1.4
 *
 * ANALOG ADC
 *      Microphone
 *       - Pin 5.0
 *       - Polling Process (Constantly Sensing)
 */


void Configure_GPIO();
void LED_CHECK();
void PIR_CHECK();
void ConfigureI2C_Temp();
void ConfigureAdc_MK();
void ReadADC();

volatile long MKout = 0.0;
volatile int temperature = 0;
volatile float temperature_c = 0.0;

unsigned int a;
int i = 0;
#define BUTTON BIT2 //push button P4.1

#define BME280_I2C_ADDRESS 0x76

int main(void)
{
    unsigned char data[3]; // Buffer to hold temperature data
    PM5CTL0 &= ~LOCKLPM5;
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	Configure_GPIO();
	ConfigureAdc_MK();
	while(1)
	{
	    LED_CHECK();
	    ReadADC();

	    while (!(UCB0IFG & UCTXIFG0)); // Wait for TX buffer to be ready
        UCB0TXBUF = 0x05; // Register address for temperature data (LSB)
        while (!(UCB0IFG & UCTXIFG0)); // Wait for TX buffer to be ready
        UCB0CTLW0 &= ~UCTR; // Set to receive mode
        UCB0CTLW0 |= UCTXSTT; // Send repeated start condition
        while (!(UCB0IFG & UCRXIFG0)); // Wait for RX buffer to be ready
        data[0] = UCB0RXBUF; // Read temperature data (MSB)
        UCB0CTLW0 |= UCTXSTP; // Send stop condition
        while (!(UCB0IFG & UCRXIFG0)); // Wait for RX buffer to be ready
        data[1] = UCB0RXBUF; // Read temperature data (LSB)
        while (!(UCB0IFG & UCTXIFG0)); // Wait for TX buffer to be ready
        data[2] = UCB0RXBUF; // Read temperature data (XLSB)


	    temperature = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4); // Combine temperature data
	    temperature_c = temperature / 16.0; // Convert temperature to degrees Celsius
	}
	//return 0;
}

void Configure_GPIO()
{
    P3OUT &= ~BIT0; //Clearing LED
    P3DIR |= BIT0;  //3.0 Output
    //P2OUT &= ~BIT0; //2.0 Input Check That This Puts Power Into//////Check A Led Blink Example With BUTTONE
    P3OUT = 0x0;
    //P2IN = 0x0;
    //P6OUT &= ~BIT6;
    //P6DIR |= BIT6;
    P2DIR &= ~BIT2; // P2.0 becomes input
    P2REN |= BIT2;
    P2OUT |= BIT2;

    P1SEL1 |= BIT2; // Configure P1.2 as I2C slave data input (UCB0SDA)
    P1SEL0 |= BIT2;
    P1SEL1 |= BIT3; // Configure P1.3 as I2C slave clock input (UCB0SCL)
    P1SEL0 |= BIT3;
}
void LED_CHECK()
{
    /*
    if(P2IN == 0x04)
    {
        //__delay_cycles(500);
        //P6OUT |= BIT6;
        //P3OUT &= ~BIT0;

    }
    else
    {
        //P6OUT &= ~BIT6;
        //P3OUT |= BIT0;

    }
    */
    if((P2IN & BUTTON) == 0x04)
    {
    _delay_cycles(2000);
    if((P2IN & BUTTON) == 0x04)
    {
        i=1;
        if(i == 1)
        {
            a=1;
            P3OUT |= BIT0;

        }
        else
        {
        a=0;
        }
    }
    }
    else
    {
        i=0;
        a=0;
        P3OUT &= ~BIT0;
    }
    //while((P2IN & BUTTON) == 0x04);
}

void ConfigureI2C_Temp()
{
    UCB0CTLW0 |= UCSWRST; // Put I2C state machine in reset
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK; // I2C master mode, SMCLK clock source
    UCB0BRW = 0x0008; // baud rate = SMCLK / 8
    UCB0I2CSA = BME280_I2C_ADDRESS; // Set slave address
    UCB0CTLW0 &= ~UCSWRST; // Release I2C state machine from reset
    //unsigned char data[3]; // Buffer to hold temperature data
    UCB0CTLW0 |= UCTR | UCTXSTT; // Start I2C transmission and send start condition
    UCB0TXBUF = 0xFA; // Register address for temperature data (MSB)
}
void ConfigureAdc_MK()
{
    P5SEL0 |=  BIT0;
    P5SEL1 |=  BIT0; //Check This!!!

    /*ADCCTL0 |= ADCSHT_8 | ADCON;                                  // ADC_ON, Change ADCSHT
    ADCCTL1 |= ADCSHS_0 | ADCCONSEQ_0 | ADCSHP;                   // s/w trig, single ch/conv, MODOSC CHECK
    ADCCTL2 &= ~ADCRES;                                           // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                          // 12-bit conversion results
    ADCMCTL0 |= ADCSREF_1 | ADCINCH_8;                            // ADC input ch A8
    ADCIE |= ADCIE0;                                              // Interrupt Enable For Flags
    ADCCTL0 |= ADCENC | ADCSC;                                    // Enable Sampling, Check Other Thing*/
    //Setting ADC Controls
    ADCCTL0 |= ADCSHT_2 | ADCON; // ADC ON, Sample&HoldTime=16 ADC clks
    ADCCTL1 |= ADCSHP;           // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;          // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;         // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_8;       //A1 ADC input select; Vref=AVCC

    __delay_cycles(400);
}

void ReadADC()
{
    ADCCTL0 |= ADCENC | ADCSC;     // Sampling and conversion start
    while (!(ADCIFG & ADCIFG0));   // Wait for sample to be sampled and converted
    MKout = ADCMEM0;

    //ADCCTL0 &= ~ADCIFG;
    //ADCIE &= ~ADCIE0;
}
