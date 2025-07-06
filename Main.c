/************************************************************************************************************************
 * Project Title : 8051-Based Smart Automotive Instrument Cluster Simulation
 * 
 * Description   : This project simulates a smart automotive dashboard using
 *                 an 8051 microcontroller in Proteus. It displays:
 *                   - Vehicle speed (using Timer1 as external pulse counter)
 *                   - Engine temperature (using LM35 + ADC0804)
 *                   - Fuel level simulation (using Timer0 as software timer)
 * 
 *                 A 16x2 LCD displays real-time data. The system can be toggled 
 *                 ON/OFF via an external interrupt (INT0). Additional features 
 *                 include a temperature warning LED and low-fuel alert.
 * 
 * Author        : Tekchand Ramesh Lanjewar
 * Date          : July 2025
 * Tools Used    : Keil uVision, Proteus, ADC0804, LM35, AT89C51, LCD
 * 
 * Peripherals   : 
 *   - Timer1 (Counter mode)  : Speed pulse counting (via T1 pin)
 *   - Timer0 (Timer mode)    : Simulates time-based fuel reduction
 *   - External Interrupt INT0: Toggle system ON/OFF
 *   - ADC0804                : Reads analog voltage from LM35 sensor
 *   - LCD 16x2               : Displays speed, fuel, and temperature
 *   - LED                    : Indicates high temperature
 * 
 * Crystal Frequency : 12 MHz
 * Target MCU        : AT89C51 (8051 family)
 ************************************************************************************************************************/

#include <reg51.h>
#include <lcd.c>

// LED connected to P3.0 to indicate high temperature
sbit led = P3^0;

// ADC data port (ADC0804 output connected to P1)
#define adc_port P1

// Constants used in speed calculation
#define PULSE_COUNT 50
#define WHEEL_CIRCUMFERENCE 1.884  // in meters
#define PULSES_PER_REVOLUTION 20


// ADC control signals
sbit rd = P2^1;    // Read pin of ADC
sbit wr = P3^6;    // Write pin of ADC
sbit intr = P3^7;  // Interrupt pin from ADC (goes LOW when conversion is done)

// Global variables
volatile int system = 0;              // Toggle system ON/OFF using interrupt
unsigned char adc_val;       // ADC digital value
unsigned int mv;             // Millivolt value from ADC
unsigned int temp;           // Temperature in °C
unsigned int fuel = 100;     // Fuel level percentage
unsigned int count;          // Pulse count for speed
unsigned long speed;         // Calculated speed (km/h)

// Function declarations
void conv();     // Start ADC conversion
void read();     // Read ADC result
void timer();    // Start Timer0 (used to simulate fuel consumption)
void counter();  // Configure Timer1 as counter for speed pulses

int main()
{
    led = 0;
    intr = 1;
    lcd_init();  // Initialize LCD

    // Enable External Interrupt 0 (for system ON/OFF toggle)
    EA = 1;      // Enable global interrupt
    EX0 = 1;     // Enable INT0
    IT0 = 1;     // INT0 triggered on falling edge

    // Initial dummy speed value (calculated based on static pulse count)
    speed = ((PULSE_COUNT * WHEEL_CIRCUMFERENCE * 3600) / (1000 * PULSES_PER_REVOLUTION));

    counter();  // Set up Timer1 as external counter for speed pulses

    while (system)  // Loop runs only when system is ON
    {
        conv();     // Trigger ADC conversion (LM35)
        read();     // Read ADC value and convert to temperature

        // Read the pulse count from Timer1 (for speed)
        count = (TH1 << 8) | TL1;

        speed += 5;  // Dummy increment for demonstration (can be replaced with real speed calculation)

        // Start Timer0 to simulate fuel usage only if fuel is sufficient
        if (fuel >= 10)
        {
            timer();  // Start Timer0 (simulates time delay before fuel drop)
        }

        // If Timer0 overflows and fuel is still above threshold
        if (TF0 == 1 && fuel >= 10)
        {
            fuel -= 10;   // Decrease fuel level
            TF0 = 0;      // Clear Timer0 overflow flag
            TR0 = 0;      // Stop Timer0
        }

        // Convert ADC value to millivolts and then to temperature
        mv = adc_val * 10;
        temp = mv / 10;

        // Display "LowFuel" warning if fuel is at or below 20%
        if (fuel <= 20 && fuel >= 10)
        {
            lcd_out(1, 10, "LowFuel");
        }
        else if (fuel < 10)
        {
            lcd_out(1, 10, "LowFuel");
            speed = 0;    // Stop the vehicle
            TR1 = 0;      // Stop Timer1 (pulse counter)
        }

        // If temperature exceeds 40°C, turn on LED
        if (temp > 40)
        {
            led = 1;
        }
        else if (temp <= 40)
        {
            led = 0;
        }

        // Display system status on LCD
        lcd_out(1, 1, "TERMINAL");

        lcd_out(2, 1, "s");      // Speed label
        lcd_out(2, 2, ":");
        lcd_print(2, 3, speed, 2);

        lcd_out(2, 6, "F");      // Fuel label
        lcd_out(2, 7, ":");
        lcd_print(2, 8, fuel, 2);
        lcd_out(2, 10, "%");

        lcd_out(2, 12, "T");     // Temperature label
        lcd_out(2, 13, ":");
        lcd_print(2, 14, temp, 2);
        lcd_out(2, 16, "c");

        delay_ms(100);  // Small delay for display refresh
    }
		return 0;
}


/************************************************************
 * Function: ISR_ex0
 * -----------------
 * External Interrupt 0 Service Routine (INT0 - P3.2)
 * Toggles the system ON/OFF state using a hardware switch.
 * 
 * Triggered on falling edge (IT0 = 1)
 ************************************************************/

// External interrupt service routine (INT0)
// Toggles system ON/OFF when interrupt occurs

void ISR_ex0(void) interrupt 0
{
    system ^= 1;  // Toggle system state
}

// Function to trigger ADC conversion

/************************************************************
 * Function: conv
 * --------------
 * Starts the ADC conversion by toggling the WR pin of ADC0804.
 * Waits until the INTR pin goes low, indicating conversion complete.
 ************************************************************/

void conv()
{
    wr = 0;
    wr = 1;
    while (intr == 1);  // Wait for conversion to complete (INTR goes low)
}

// Function to read digital output from ADC

/************************************************************
 * Function: read
 * --------------
 * Reads the digital value from ADC0804 via port P1.
 * Reads on RD low, then stores value in adc_val.
 * Introduces a short delay for ADC recovery.
 ************************************************************/

void read()
{
    rd = 1;
    rd = 0;
    adc_val = P1;     // Read digital value from ADC0804
    delay_ms(250);
    rd = 1;
}

// Timer0 configuration to simulate time delay for fuel usage

/************************************************************
 * Function: timer
 * ---------------
 * Configures and starts Timer0 in Mode 1 (16-bit timer mode)
 * Used to simulate periodic fuel consumption delay.
 * Loaded with preset value to overflow approximately every 1 sec.
 ************************************************************/

void timer()
{
    TMOD = 0x01;      // Timer0 Mode 1 (16-bit timer)
    TH0 = 0xFE;       // Load high byte (for ~1s delay based on 11.0592 MHz)
    TL0 = 0x17;       // Load low byte
    TR0 = 1;          // Start Timer0
}

// Configure Timer1 as Counter to count external pulses (e.g., from speed sensor)

/************************************************************
 * Function: counter
 * -----------------
 * Configures Timer1 in Mode 1 as a 16-bit external counter.
 * C/T1 = 1 ? Timer1 counts pulses from external sensor at T1 pin (P3.5)
 * Used to measure speed pulses for speed calculation.
 ************************************************************/

void counter()
{
    TMOD = 0x05;      // Timer0 = Mode 1 (Timer), Timer1 = Mode 1 (Counter)
                      // C/T1 = 1 ? Timer1 works as external counter on P3.5 (T1 pin)
    TR1 = 1;          // Start Timer1 (pulse counter)
}
