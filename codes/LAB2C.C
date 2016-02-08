/* LAB 2 part C. This program counts Timer B interrupts and during the lab session will be modified to display the interrupt overhead */

/* Variable declarations */
int int_count;										// This variable will count timer B interrupts
int display_int_count;			  				// This variable will increment for every 1000 timer B interrupts
int Timer_B_value;								// This variable will hold the current contents of the Timer B count registers


/* function prototype */
void Tmr_B_isr(); 										// Timer B interrupt service routine
void DispStr(int x, int y, char *s);				// display strings on computer monitor

main()
{
	/* Initializations */
	auto char display[128];
   auto char key;

	brdInit();

	int_count = 0;
	display_int_count = 0;
	Timer_B_value = 0;

	/******************************************** FILL IN THE '---' ********************************************/
	/* set up timer B B1 interrupt */
	SetVectIntern(0x0B, Tmr_B_isr); 					// set up timer B interrupt vector
	WrPortI(TBCR, &TBCRShadow, ----); 				// clock timer B with (perclk/16), priority 2
	WrPortI(TBM1R, NULL,----); 						// set up match register(s) to 10-bit value 0x15E
	WrPortI(TBL1R, NULL,----);							// set up match register(s) to 10-bit value 0x15E
	WrPortI(TBCSR, &TBCSRShadow, ----); 			// enable Timer B and B1 match interrupt
	/***********************************************************************************************************/

	/* This is the background process. It repeatedly updates the computer monitor with the values of the interrupt count, and the overhead */
	while(1)	{
   	/* Update computer monitor */
      display[0] = '\0';
   	sprintf(display, "Number of Timer B interrupts = %d (X1000)", display_int_count);
		DispStr(5, 2, display);

		display[0] = '\0';
		sprintf(display, "Timer B overhead = %d  X(PCLK/16 cycles)", (Timer_B_value-350));			//Time at start of ISR minus time of ISR call (0x15E)
		DispStr(5, 4, display);

		if (kbhit()) {										// See if key has been pressed
			key = getchar();
			if (key == 0x71 || key == 0x51)			// Yes, see if it's the q or Q key
				exit(0);
   	}

 	}
}

/* Timer B interrupt service routine */
nodebug root interrupt void Tmr_B_isr()
{
	char TMRB_status;

	TMRB_status = RdPortI(TBCSR);						// This clears the interrupt flag.
	WrPortI(TBM1R, NULL, 0x40); 						// set up match register(s) to 10-bit value 0x15E
	WrPortI(TBL1R, NULL, 0x5E);  						// set up match register(s) to 10-bit value 0x15E

   int_count++;											// increment interrupt counter
	if (int_count == 1000){								// divide increment counter by 1000
		display_int_count++;
		int_count = 0;
	}

	/* During your lab session, you will modify the ISR to calculate the interrupt overhead. To do this you must
	1. Read the current time value in the TBCMR-TBCLR register pair. Scale and add properly to give an accurate value.
	2. Subtract the time at which the interrupt was generated: 0x15E = 350 (or TBM1R=0x40, TBL1R=0x5E)
	3. Interrupt overhead will be displayed by the background process
	*/
}


/* set the STDIO cursor location and display a string  */
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}