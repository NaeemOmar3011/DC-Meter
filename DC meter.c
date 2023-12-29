#include "generic_macros_header.h"

unsigned char voltage_counter, current_counter;
unsigned int voltage, current;
volatile unsigned long voltage_buffer, current_buffer;
// get more info

ISR(ADC_vect)
{
	if(READBIT(ADMUX,MUX1)) // If channel ADC2 was Selected
	{
		voltage_buffer += ADC; // Accumulating Voltage values into a buffer
		CLRBIT(ADMUX,MUX1); // Select Channel ADC0
		voltage_counter++; // Increasing the Measurement Counter
	}
	else
	{
		current_buffer += ADC; // we Accumukate the Current buffer values in a buffer
		SETBIT(ADMUX,MUX1); // Select Channel ADC2
		current_counter++; // Increasing the Measurement Counter
	}
}
// Functions for working with LCD
#define RS PD0
#define EN PD2
// Command passing function
void lcd_com(unsigned char p)
{
	PORTD &= ~(1 << RS); // RS = 0 (Command entry)
	PORTD |= (1 << EN); // EN = 1 (Start writing Command on LCD)
	PORTD &= 0x0F; PORTD |= (p & 0xF0); // Senior Nible
	_delay_ms(1);
	PORTD &= ~(1 << EN); // EN = 0 (End of writing Command on LCD)
	_delay_ms(1);
	PORTD |= (1 << EN); // EN = 1 (Start writing Command on LCD)
	PORTD &= 0x0F; PORTD |= (p << 4); // Junior Nible
	_delay_ms(1);
	PORTD &= ~(1 << EN); // EN = 0 (End of writing Command on LCD)
	_delay_ms(1);
}
// Data transfer Function
void lcd_data(unsigned char p)
{
	PORTD |= (1 << RS)|(1 << EN); // RS = 1 (Data Recording), EN - 1 (Start writing Command on LCD)
	PORTD &= 0x0F; PORTD |= (p & 0xF0); // Senior Nible
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (End of writing Command on LCD)
	_delay_us(100);
	PORTD |= (1 << EN); // EN = 1 (Start writing Command on LCD)
	PORTD &= 0x0F; PORTD |= (p << 4); // Junior Nible
	_delay_us(100);
	PORTD &= ~(1 << EN); // EN = 0 (End of writing Command on LCD))
	_delay_us(100);
}
// Function to Display a Line into LCD
void lcd_string(unsigned char command, char *string)
{
	lcd_com(0x0C); // Turn on the display, the Cursor is not visible
	lcd_com(command); // Address of the familiar place
	// Print Characters until the end of the Line
	while(*string != '\0')
	{
		lcd_data(*string); // Output the Symbol
		string++; // Next Character of the Line
	}
}
// LCD Initialization Function
void lcd_init(void)
{
	DDRD = 0xFF;
	PORTD = 0x00;
	_delay_ms(50); // Wait for The LCD to be ready
	// Configure 4 Bit mode
	PORTD |= (1 << PD5);
	PORTD &= ~(1 << PD4);
	// Activated 4 Digit mode
	PORTD |= (1 << EN);
	PORTD &= ~(1 << EN);
	_delay_ms(5);
	lcd_com(0x28); // Bus 4bit LCD 2 Lines
	lcd_com(0x08); // Turn off the Display completely
	lcd_com(0x01); // Clear the Display
	_delay_us(100);
	lcd_com(0x06); // Shift the Cursor to the Right
	lcd_com(0x0C); // Turn on the display the cursor is visable
}
int main(void)
{
	// Setting up the ADC
	ADMUX |= (1 << REFS1)|(1 << REFS0); // Internal ION 2,56V
	ADCSRA |= (1 << ADEN) // ADC Resolution
	|(1 << ADSC) // Start transformation
	|(1 << ADFR) // Continuous ADS Operation mode
	|(1 << ADPS2)|(1 << ADPS1) |(1 << ADPS0) // Prescaler by 128 (ADC Frequency 125kHz)
	|(1 << ADIE); // Enable ADC interrupt
	sei(); // Globally Enable Interrupt
	CLRBIT(DDRC,PC0);
	CLRBIT(DDRC,PC2);
	
	lcd_init(); // LCD Initialization
	_delay_ms(25);
	lcd_string(0x80 ,"VOLTS * AMPERES");
	lcd_string(0xC0 ,"  .   *   . ");
	while(1)
	{
		// Calculate the Average Voltage value
		if(voltage_counter == 250)
		{
			voltage = ((voltage_buffer*256*150)/1024)/voltage_counter;
			voltage_counter = 0; // Reset the Measurement Counter to zero
			voltage_buffer = 0; // Resetting the Voltage Buffer
			lcd_com(0xC0);
			lcd_data((voltage/10000%10)+'0');
			lcd_data((voltage/1000%10)+'0');
			lcd_com(0xC3);
			lcd_data((voltage/100%10)+'0');
		}
		// Calculate the average Current volt
		if(current_counter == 250)
		{
			current = ((current_buffer*256*10)/1024)/current_counter;
			current_counter = 0; // Reset the Measurement Counter to zero
			current_buffer = 0; // Resetting the Voltage Buffer
			lcd_com(0xC9);
			lcd_data((current/1000%10)+'0');
			lcd_com(0xCB);
			lcd_data((current/100%10)+'0');
			lcd_data((current/10%10)+'0');
		}
	}
}