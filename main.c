#include <msp430.h> 

unsigned int value, l_i = 0;
 int light = 0, light_room = 0, dim_led = 50;
 int temp = 0, temp_room = 0;
 int touch = 0, touch_room = 0;
unsigned int flag = 0;
 int adc_reading[3];

void config_ADC(void);
void config_GPIO(void);

void fade_LED(int);

void update_analog_v(void);



/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	config_GPIO();
	config_ADC();


	update_analog_v();
	light_room = light; temp_room = temp, touch_room = touch;


	for (;;) {
	    update_analog_v();

	    dim_led = light;
	    dim_led = ((dim_led - 50) * 100) / (900 - 50);

	    if (dim_led <= 5) {
	        dim_led = 0;
	    } else if (dim_led >= 95) {
	        dim_led = 100;
	    }

	    fade_LED(dim_led);

	    if (light < light_room * 1.8 && light > light_room * 1.1) { }
	    else {
	        if (light >= light_room * 1.8) {
	            P1OUT |= BIT4;
	            __delay_cycles(200);
	        }
	        if (light <= light_room * 1.1) {
	            P1OUT &= ~BIT4;
	            __delay_cycles(200);
	        }
	    }

	    if (temp > temp_room * 1.03) {
	        P1OUT |= BIT5;
	        __delay_cycles(200);
	    } else {
	        P1OUT &= ~BIT5;
	        __delay_cycles(200);

	    }

	    if (touch > touch_room * 0.7 && touch < touch_room * 0.9) {

	    } else {
	        if (touch >= touch_room * 0.9) { P2OUT &= ~BIT0; __delay_cycles(200); }
	        if (touch <= touch_room * 0.7) { P2OUT |= BIT0; __delay_cycles(200); }
	    }
	}
}

void config_GPIO(void) {
    P1OUT = 0x00;
    P2OUT = 0x00;
    P1DIR = 0x00;
    P1REN = 0x00;
    P2REN = 0x00;
    P2DIR = 0x00;
    P1DIR |= (BIT4 + BIT5 + BIT6);
    P2DIR |= BIT0;
}

void config_ADC(void) {
    ADC10CTL1 = INCH_2 | CONSEQ_1;
    ADC10CTL0 = ADC10SHT_2 | MSC | ADC10ON;

    while (ADC10CTL1 & BUSY); // poll until ADC setup

    ADC10DTC1 = 0x03;
    ADC10AE0 |= (BIT0 + BIT1 + BIT2); // op. sel.
}

void fade_LED(int v_PWM) {
    P1SEL |= BIT6;
    CCR0 = 100 - 0;
    CCTL1 = OUTMOD_3;
    CCR1 = v_PWM;
    TACTL = TASSEL_2 + MC_1;
}

void update_analog_v(void) {
    l_i = 0;
    temp = 0; light = 0; touch = 0; // zero out sensor values

    for (l_i = 1; l_i <= 5; ++l_i) {
        ADC10CTL0 &= ~ENC;

        while (ADC10CTL1 & BUSY); // poll until ADC ready

        ADC10SA = (unsigned)&adc_reading[0];
        ADC10CTL0 |= (ENC | ADC10SC);

        while (ADC10CTL1 & BUSY); // poll until ADC ready

        light += adc_reading[0];
        touch += adc_reading[1];
        temp += adc_reading[2];
    }

    light /= 5;
    touch /= 5;
    temp /= 5;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    __bic_SR_register_on_exit(CPUOFF);
}
