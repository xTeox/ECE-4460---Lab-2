// LAB 2 Part A 
// This program displays the number of times each of the Timer A interrupts have occured on the computer monitor

// Definitions
#define MAX_COUNT   100  // Maximum count value
#define TMA5_MASK  0x20  // Mask for Timer A5 in TACSR
#define TMA6_MASK  0x40  // Mask for Timer A6 in TACSR
#define TMA7_MASK  0x80  // MAsk for Timer A7 in TACSR

// Variable declarations
char count_TMRA5;        // This variable will count the number of times Timer A5 has triggered an interrupted
char count_TMRA6;        // This variable will count the number of times Timer A6 has triggered an interrupted
char count_TMRA7;        // This variable will count the number of times Timer A7 has triggered an interrupted
int  display_count[3];   // This variable will hold the count values divided by 100
char i;                  // Iterator

// Function prototype
void Tmr_A_isr();                     // Timer A interrupt service routine
void DispStr(int x, int y, char *s);  // Display strings on computer monitor

main()
{
    // Initializations
    auto char display[128];
    auto char key;
    brdInit();
    count_TMRA5 = 0;                      // Initialize counters to zero
    count_TMRA6 = 0;                      // Initialize counters to zero
    count_TMRA7 = 0;                      // Initialize counters to zero
    for (i = 0; i < 3; i++)               // Initialize display counters to zero
        display_count[i] = 0;
    // Set up Timer A interrupts
    SetVectIntern(0x0A, Tmr_A_isr);       // Set up timer A interrupt vector
    WrPortI(TAT7R, &TAT7RShadow, 0x3F);   // Set TMRA7 to count down from 63
    WrPortI(TAT6R, &TAT6RShadow, 0x7F);   // Set TMRA6 to count down from 127
    WrPortI(TAT5R, &TAT5RShadow, 0xFF);   // Set TMRA5 to count down from 255
    WrPortI(TAT1R, &TAT1RShadow, 0xFF);   // Set TMRA1 to count down from 255
    WrPortI(TACR,  &TACRShadow,  0xE1);   // Clock TMRA5,TMRA6,TMRA7 by TMRA1. Clock TMRA4 by PCLK/2. Set Timer A to trigger priority 1 interrupts
    WrPortI(TACSR, &TACSRShadow, 0xE1);   // Enable interrupts for TMRA5,TMRA6,TMRA7 only. Enable main clock for Timer A
    DispStr(5, 10, "<-PRESS 'Q' TO QUIT->");
    // This is the background process. It repeatedly updates the computer monitor with the values in the display_count array
    while (1) {
        for(i = 0; i < 3; i++) {
            display[0] = '\0';
            sprintf(display, "Number of TMRA%d interrupts: %d\  (x100)", i+5, display_count[i]);
            DispStr(5, i+1, display);
        }
        if (kbhit()) {                       // Check if any key has been pressed
            key = getchar();
            if (key == 0x71 || key == 0x51)  // Check if it's the key 'q' or 'Q'
                exit(0);
        }
    }
}

// Timer A interrupt service routine
nodebug root interrupt void Tmr_A_isr()
{
    char TMRA_status;              // This will hold the interrupt flags
    TMRA_status = RdPortI(TACSR);  // Read interrupt flags and store to TMRA_status
    /* There are three possible events leading to this service routine being called
    Write code to determine which of the three     timers has triggered the interrupt.
    Note it is possible for more than one timer to expire at the same time, thus your code must service all triggers.
    By testing the interrupt flags in TMRA_status, you can determine which counters have expired.
    Use three count variables count_TMRAX (X is 5, 6 and 7) to count the number of interrupts.
    For each Timer AX interrupt, you must increment the corresponding count variable.
    i.e., if Timer A5 has expired, then increment count_TMRA5. */
    // During your lab session, you will replace the following code to perform as specified above
    if (TMRA_status & TMA5_MASK)
    {
        count_TMRA5++;
        // The following code increment the display array for every 100 interrupts for a respective timer and resets the count variable
        if (count_TMRA5 == MAX_COUNT)    // If Timer A5 has interrupted the CPU 100 times
        {
            display_count[0]++;          // Increment the display counter
            count_TMRA5 = 0;             // Clear the Timer A5 counter
        }
    }
    if (TMRA_status & TMA6_MASK)
    {
        count_TMRA6++;
        // The following code increment the display array for every 100 interrupts for a respective timer and resets the count variable
        if (count_TMRA6 == MAX_COUNT)    // If Timer A6 has interrupted the CPU 100 times
        {
            display_count[1]++;          // Increment the display counter
            count_TMRA6 = 0;             // Clear the Timer A6 counter
        }
    }
    if (TMRA_status & TMA7_MASK)
    {
        count_TMRA7++;
        // The following code increment the display array for every 100 interrupts for a respective timer and resets the count variable
        if (count_TMRA7 == MAX_COUNT)    // If Timer A6 has interrupted the CPU 100 times
        {
            display_count[2]++;          // Increment the display counter
            count_TMRA7 = 0;             // Clear the Timer A6 counter
        }
    }
}

// Set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}
