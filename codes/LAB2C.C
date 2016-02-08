// LAB 2 part C. This program counts Timer B interrupts and during the lab session will be modified to display the interrupt overhead

// uC/OS-II configuration constants
#define TASK_STK_SIZE    512  // Size of each task's stacks (# of bytes)
#define OS_MAX_EVENTS      1  // Maximum number of events (semaphores, queues, mailboxes)
#define OS_MAX_TASKS       4  // Maximum number of tasks system can create (less stat and idle tasks)
#define OS_TASK_STAT_EN    1  // Enable statistics task creation
#define OS_TICKS_PER_SEC 128  // Number of ticks per second
#use "ucos2.lib"

// Task and function prototypes
void TaskStart(void *data);   // Function prototype of startup task
void Task(void *data);        // Function prototype of tasks
void InitTimerInt();          // Setup Timer B interrupts
void Tmr_B_isr();             // Timer B interrupt service routine
void UpdateStat();            // Display RTOS statistics
void DispStr(int x, int y, char *s);

// Variable declarations
OS_EVENT *TimerSem;           // Timer semaphore
int int_count;                // This variable will count Timer B interrupts
int display_int_count;        // This variable will increment for every 1000 Timer B interrupts
int Timer_B_value;            // This variable will hold the current contents of the Timer B count registers

void main (void)
{
    brdInit();                  // Initialize MCU baord
    OSInit();                   // Initialize uC/OS-II
    OSTaskCreate(TaskStart, (void *)0, TASK_STK_SIZE, 10);
    TimerSem = OSSemCreate(0);  // Semaphore for the timer
    OSStart();                  // Start multitasking
}

void TaskStart (void *data)
{
    OSTaskCreate(Task, (void *)0, TASK_STK_SIZE, 11);
    InitTimerInt();  // Setup Timer B internal interrupts
    OSStatInit();    // Initialize statistics task
    DispStr(0, 12, "#Tasks          : xxxxx  CPU Usage: xxxxx %");
    DispStr(0, 13, "#Task switch/sec: xxxxx");
    DispStr(0, 14, "<-PRESS 'Q' TO QUIT->");
    for (;;) {
        UpdateStat();
        OSTimeDly(OS_TICKS_PER_SEC);  // Wait one second
    }
}
// Tasks
nodebug void Task (void *data)
{
    auto UBYTE err;
    auto char display[128];
    auto char key;    
    // Prepare the STDIO screen
    display[0] = '\0';
    sprintf(display, "Number of Timer B interrupts: xxxxx (x1000)");
    DispStr(5, 2, display);
    display[0] = '\0';
    sprintf(display, "Timer B overhead: xxxxx  x(PCLK/16 cycles)");  //Time at start of ISR minus time of ISR call (0x15E)
    DispStr(5, 4, display);
    for (;;) {
        OSSemPend(TimerSem, 0, &err);  // Wait for Semaphore to be posted
        display[0] = '\0';
        sprintf(display, "%5d", display_int_count);
        DispStr(35, 2, display);
        display[0] = '\0';
        sprintf(display, "%5d", Timer_B_value-350);
        DispStr(23, 4, display);
    }
}

// Setup Timer B interrupts
void InitTimerInt()
{
    int_count = 0;
    display_int_count = 0;
    Timer_B_value = 0;
    // Set up Timer B B1 interrupt
    SetVectIntern(0x0B, Tmr_B_isr);      // Set up Timer B interrupt vector
    WrPortI(TBCR, &TBCRShadow, 0x09);    // Clock Timer B with (perclk/16), priority 1
    WrPortI(TBM1R, NULL, 0x40);          // Set up match register(s) to 10-bit value 0x15E
    WrPortI(TBL1R, NULL, 0x5E);          // Set up match register(s) to 10-bit value 0x15E
    WrPortI(TBCSR, &TBCSRShadow, 0x03);  // Enable Timer B and B1 match interrupt
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
    OSIntExit();
}

// Helper functions
void UpdateStat()
{
    char key, s[50];
    sprintf(s, "%5d", OSTaskCtr);        // Display #tasks running
    DispStr(18, 12, s);
    #if OS_TASK_STAT_EN
        sprintf(s, "%5d", OSCPUUsage);   // Display CPU usage in %
        DispStr(36, 12, s);
    #endif
    sprintf(s, "%5d", OSCtxSwCtr);       // Display avg #context switches per 5 seconds
    DispStr(18, 13, s);
    OSCtxSwCtr = 0;
    if (kbhit()) {                       // See if key has been pressed
        key = getchar();
        if (key == 0x71 || key == 0x51)  // Yes, see if it's the q or Q key
            exit(0);
    }
}

// Set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}
