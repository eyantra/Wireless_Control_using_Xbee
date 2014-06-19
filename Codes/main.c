/**************************************************************************************************
		Platform: Fire Bird V P89V51RD2
		Wireless Communication
		Written by: ERTS Lab,IITB Mumbai
		Last Modification: 2010
		Compiled with: uVision3 V3.90; C Compiler: C51.Exe, V8.18
**************************************************************************************************/

#include <intrins.h>
#include "p89v51rx2.H"

sbit buzzer = P2^7;			//Buzzer pin
sbit LB=P1^0;	    		//LEFT BACK Pin
sbit LF=P1^1;				//LEFT FRONT Pin
sbit RF=P1^2;				//RIGHT FRONT Pin
sbit RB=P3^4;				//RIGHT BACK Pin
sbit left_velocity=P1^3;	//Left velocity control pin. 
sbit right_velocity=P1^4;	//Right velocity control pin.

sbit start_conv = P3^6;		 //P3^6 is connected to Start and ALE pin of ADC0808
sbit output_enable = P3^7;		 //P3^7 is connected to Output Enable Pin of ADC0808

unsigned char front_sharp_sensor = 0; //used in stop n go ACC


// function for giving a delay of ms milliseconds
void delay_ms(unsigned int ms)
{
unsigned int i,j;

for(i=0;i<ms;i++)
for(j=0;j<53;j++);
}//end of delay_ms


unsigned char ADC_conversion(unsigned char ADC_ch_no)
{
unsigned char ADC_data;

P0 = ADC_ch_no;		  // to select channel no. send address on P0

start_conv = 1; // ADC0808 will latch the address on L-H transition on the ALE pin
start_conv = 0;	// ADC0808 will start conversion on H-L transition on the start pin
delay_ms(2);	// conversion time for ADC0808

P0 =0xFF;	        // output enable and read
output_enable = 1;	//Enabling o/p of ADC0808 for to read the converted data
ADC_data = P0;		//storing adc_data
output_enable = 0;	//disabling ADC 0808 o/p
return ADC_data;	//returning adc_data
}

/**********************************************************************************
RF =  RIGHT MOTOR FORWARD
RB =  RIGHT MOTOR BACKWARD
LF =  LEFT MOTOR FORWARD
LB =  LEFT MOTOR BACKWARD
**********************************************************************************/
//direction control subroutines
void forward(void)
{
RF=1;
LF=1;
RB=0;
LB=0;
}//end of forward

void backward(void)
{
RF=0;
LF=0;
RB=1;
LB=1;
}//end of backward


void left(void)
{
RF=1;
LF=0;
RB=0;
LB=0;
}//end of Left

void right(void)  
{
RF=0;
LF=1;
RB=0;
LB=0;
}//end of Right

void stop(void)
{

RF=0;
LF=0;
RB=0;
LB=0;
}//end of Stop

/**************************************************************************
	Function: uart_setup()
		
	Description: This function configures Timer1 for generating the Baudrate.
			     The TH1 register should be written with following hex numbers
		         for desired baud rate. After setting the baudrate, SCON register
			     for enabling Rx and setting up the frame format.
 	
	       		 9600 ----->  0xFD
		 	     4800 ----->  0xFA
			     2400 ----->  0xF4
			     1200 ----->  0xE8

**************************************************************************/
void uart_setup()
{
    TMOD = 0x20;		// configure timer1 for Mode 2 operation for the correct baud rate 
     TH1 = 0xFD;     	// 9600 bps for 11.0592 MHz clock 
    TCON = 0x40;    	// Start timer 1 by setting TR1 = 1 
    SCON = 0x50;  		// Set Serial IO to receive and normal mode 
	
	RI=0;               //Receive Interrupt Flag is cleared
	TI=0;  				//Tx interrupt flag is cleared
}//end of uart_setup


/***************************************************************************
	Function: void char RxData()
	
	Description: This function copies any data that is present in the SBUF 
			     register and returns it to the calling function.
***************************************************************************/

 unsigned char RxData()
{
    unsigned char rcv_data;																 
	rcv_data = SBUF;              //if any data is available copy it from SBUF
	return rcv_data;              //return the received data
}//end of RxDAta





//main function
void main(void)
{
 unsigned char rx_data;
  buzzer=1;
 left_velocity=1;  //setting this pin to one sets the motor to run at maximum velocity. 
				   //Thus enable pin of Motor driver is always on unlike in PWM mode.
 right_velocity=1; //setting this pin to one sets the motor to run at maximum velocity.
				   //Thus enable pin of Motor driver is always on unlike in PWM mode
 uart_setup();     // calling the UART setup function

 
 
 
 while(1)					//loop continuously
 {
  front_sharp_sensor = ADC_conversion(2);

  if (RI==1)                // if any data is present store it and transmit it back 
  {
   rx_data = RxData();      //get the received data	
      
   if(rx_data == 'F'){		//move robot fprward 
   forward(); 
   RI = 0;
   TI = 0;  
   }
   if(rx_data == 'B'){ //move robot backward
    backward();
	RI = 0;
	TI = 0;
   }
   if(rx_data == 'L'){  //move robot to left
    stop();
	left();
	delay_ms(500);
	stop();
	RI = 0;
	TI = 0;
   }
   if(rx_data == 'R'){  //move robot to right
    stop();
	right();
	delay_ms(500);
	stop();
	RI = 0;
	TI = 0;
   }
   if(rx_data == 'S'){  //move robot to right
    stop();
	RI = 0;
	TI = 0;
   }								 

   if(rx_data == 'O'){  //switchon buzzer 
    buzzer=0;
	RI = 0;
	TI = 0;
   }
   if(rx_data == 'N'){  //switch off buzzer
    buzzer=1;
	RI = 0;
	TI = 0;
   }    
    RI = 0;
	TI = 0;                  //Clear receive interrupt. Must be cleared by the user.  
  }
  
  if(front_sharp_sensor >= 110) // obstacle is near the robot, stop and turn on the buzzer
	{
	   stop(); 
	   backward();
	   left();
	   delay_ms(500);
	   stop();
	   	   //buzzer=0; 
	}
  
 }
}//end of main					 

