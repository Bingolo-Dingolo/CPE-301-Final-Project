#include <LiquidCrystal.h> // lcd display
#include <Stepper.h> // stepper motor
#include <RTClib.h> // real time clock
#include <Wire.h> // real time clock
#include <DHT.h> // temp and humidity module

#define something thing

volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 
volatile unsigned char* 

// LCD pins to pins in the Arduino
const int RS = 43, EN = 45 D4 = 50, D5 = 51, D6 = 52, D7 = 53;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

steps = 100;
Stepper stepper(steps, 27, 29, 31, 33);



void setup(){
  U0Init(9600);

  
}

void loop(){
  
  
}

void set_interrupt_regs(){
    *DDR_D &= ~(1 << 3); // start button, INPUT (PD3)
    *PORT_D |= (1 << 3);; // pull up resistor for start button
    *my_EICRA |= (1 << 6); // ISC30
    *my_EICRA |= (1 << 7); // ISC31
    *my_TCCR1A = 0;
    *my_TCCR1B = 0;
    *my_TCNT1 = 49910;
    *my_TCCR1B |= (1 << 0); // CS10
    *my_TCCR1B |= (1 << 2); // CS12
}

void adc_init(){
    // setup the A register
    *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
    *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
    *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
    *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
    // setup the MUX Register
    *my_ADMUX &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
    *my_ADMUX |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
    *my_ADMUX &= 0b11011111; // clear bit 5 to 0 for right adjust result
    *my_ADMUX &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

void U0init(int U0baud){
    unsigned long FCPU = 16000000;
    unsigned int tbaud;
    tbaud = (FCPU / 16 / U0baud - 1);
    // Same as (FCPU / (16 * U0baud)) - 1;
    *my_UCSR0A = 0x20;
    *my_UCSR0B = 0x18;
    *my_UCSR0C = 0x06;
    *my_UBRR0 = tbaud;
}

void U0putchar(unsigned char U0pdata){
    while ((*my_UCSR0A & TBE) == 0);
    *my_UDR0 = U0pdata;
}

unsigned int adc_read(unsigned char adc_channel_num){
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40; //0b 0100 0000
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}

unsigned char kbhit(){
  return *myUCSR0A & RDA;
}

unsigned char getChar(){
  return *myUDR0;
}

void putChar(unsigned char U0pdata){
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}
