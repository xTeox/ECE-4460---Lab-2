// LAB 2 part C. This program counts Timer B interrupts and during the lab session will be modified to display the interrupt overhead

// Variable declarations
int int_count;          // This variable will count Timer B interrupts
int display_int_count;  // This variable will increment for every 1000 Timer B interrupts
int Timer_B_value;      // This variable will hold the current contents of the Timer B count registers

// Function prototype
void Tmr_B_isr();                     // Timer B interrupt service routine
void DispStr(int x, int y, char *s);  // Display strings on computer monitor

main()
{
    // Initializations
    auto char display[128];
    auto char key;
    brdInit();
    int_count = 0;
    display_int_count = 0;
    Timer_B_value = 0;
    // Set up Timer B B1 interrupt
    SetVectIntern(0x0B, Tmr_B_isr);      // Set up Timer B interrupt vector
    WrPortI(TBCR, &TBCRShadow, 0x0A);    // Clock Timer B with (perclk/16), priority 2
    WrPortI(TBM1R, NULL, 0x40);          // Set up match register(s) to 10-bit value 0x15E
    WrPortI(TBL1R, NULL, 0x5E);          // Set up match register(s) to 10-bit value 0x15E
    WrPortI(TBCSR, &TBCSRShadow, 0x03);  // Enable Timer B and B1 match interrupt
    // This is the background process. It repeatedly updates the computer monitor with the values of the interrupt count, and the overhead
    while (1) {
        // Update computer monitor
          display[0] = '\0';
           sprintf(display, "Number of Timer B interrupts = %d (x1000)", display_int_count);
        DispStr(5, 2, display);
        display[0] = '\0';
        sprintf(display, "Timer B overhead = %d  x(PCLK/16 cycles)", Timer_B_value-350);  //Time at start of ISR minus time of ISR call (0x15E)
        DispStr(5, 4, display);
        if (kbhit()) {                       // See if key has been pressed
            key = getchar();
            if (key == 0x71 || key == 0x51)  // Yes, see if it's the q or Q key
                exit(0);
        }
    }
}

// Timer B interrupt service routine
nodebug root interrupt void Tmr_B_isr()
{
    int TBCMR_status, TBCLR_status;
    char TMRB_status;
    TMRB_status = RdPortI(TBCSR);  // This clears the interrupt flag.
    WrPortI(TBM1R, NULL, 0x40);    // Set up match register(s) to 10-bit value 0x15E (350)
    WrPortI(TBL1R, NULL, 0x5E);    // Set up match register(s) to 10-bit value 0x15E (350)
    int_count++;                   // Increment interrupt counter
    if (int_count == 1000) {       // Divide increment counter by 1000
        display_int_count++;
        int_count = 0;
    }
    /* During your lab session, you will modify the ISR to calculate the interrupt overhead. To do this you must
    1. Read the current time value in the TBCMR-TBCLR register pair. Scale and add properly to give an accurate value.
    2. Subtract the time at which the interrupt was generated: 0x15E = 350 (or TBM1R=0x40, TBL1R=0x5E)
    3. Interrupt overhead will be displayed by the background process
    */
    TBCLR_status = RdPortI(TBCLR);
    TBCMR_status = RdPortI(TBCMR);
    if (!(TBCLR_status & 0x80) && ((TBCLR_status ^ RdPortI(TBCLR)) & 0x80))
        TBCMR_status--;
}

// Set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}
