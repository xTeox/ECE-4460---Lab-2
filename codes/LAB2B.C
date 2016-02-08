// Definitions
#define TASK_STK_SIZE          512  // Size of each task's stacks (# of bytes)
#define N_TASKS                  3  // Number of identical tasks
#define MAX_COUNT              100  // Maximum count value
#define TMA5_MASK             0x20  // Mask for Timer A5 in TACSR
#define TMA6_MASK             0x40  // Mask for Timer A6 in TACSR
#define TMA7_MASK             0x80  // Mask for Timer A7 in TACSR

// Redefine uC/OS-II configuration constants as necessary
#define OS_MAX_EVENTS      N_TASKS  // Maximum number of events (semaphores, queues, mailboxes)
#define OS_MAX_TASKS     3+N_TASKS  // Maximum number of tasks system can create (less stat and idle tasks)
#define OS_TASK_STAT_EN          1  // Enable statistics task creation
#define OS_TICKS_PER_SEC       128  // Number of ticks per second
#use "ucos2.lib"

// Task and function prototypes
void InitTimerInt();                // Setup Timer A interrupts
void Tmr_A_isr();                   // Timer A interrupt service routine
void UpdateStat();                  // Display RTOS statistics
void Task(void *data);              // Function prototype of tasks
void TaskStart(void *data);         // Function prototype of startup task
void DispStr(int x, int y, char *s);
char count_TMRA5;
char count_TMRA6;
char count_TMRA7;
int display_count[N_TASKS];
UBYTE     TaskData[N_TASKS];        // Parameters to pass to each task
OS_EVENT *TimerSem[N_TASKS];

void main (void)
{
    auto UBYTE i;
    brdInit();  // Initialize MCU baord
    OSInit();   // Initialize uC/OS-II
    OSTaskCreate(TaskStart, (void *)0, TASK_STK_SIZE, 10);
    for (i = 0 ; i < N_TASKS ; i++)
        TimerSem[i] = OSSemCreate(0);  // Semaphores for timer 5,6,7
    OSStart();                         // Start multitasking
}

void TaskStart (void *data)
{
    auto UBYTE  i;
    // Create N_TASKS identical tasks
    for (i = 0; i < N_TASKS; i++) {
        TaskData[i] = i;          // Each task has its own number
        OSTaskCreate(Task, (void *)&TaskData[i], TASK_STK_SIZE, 11+i);
    }
    InitTimerInt();  // Setup Timer A internal interrupts
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
    auto UBYTE num;
    auto char display[128];
    // Prepare the STDIO screen
    num = *((int *)data) + 5;
    sprintf(display, "Number of TMRA%d interrupts: xxxxx  (x100)", num);
    DispStr(20, num, display);
    for (;;) {
        num = *((int *)data);
        display[0] = '\0';
        OSSemPend(TimerSem[num], 0, &err);  // Wait for Semaphore to be posted
        sprintf(display, "%5d", display_count[num]);
        DispStr(48, num+5, display);
    }
}
// Interrupt routine
nodebug root interrupt void Tmr_A_isr()
{
    char TMRA_status;                    // This will hold the interrupt flags
    TMRA_status = RdPortI(TACSR);        // Read interrupt flags and store to TMRA_status
    // Timer A demultiplexing
    if (TMRA_status & TMA7_MASK) {       // if Timer A7 has trigered
        count_TMRA7++;
        if (count_TMRA7 == MAX_COUNT) {  // if Timer A7 has interrupted the CPU 100 times
            display_count[2]++;          // increment corresponding display counter
            count_TMRA7 = 0;             // clear Timer A7 counter
            OSSemPost(TimerSem[2]);      // Post a semaphore for Timer A7 counter
        }
    }
    if (TMRA_status & TMA6_MASK) {       // if Timer A6 has trigered
        count_TMRA6++;                   // increment Timer A6 counter
        if (count_TMRA6 == MAX_COUNT) {  // if Timer A6 has interrupted the CPU 100 times
            display_count[1]++;          // increment coressponding display counter
            count_TMRA6 = 0;             // clear Timer A6 counter
            OSSemPost(TimerSem[1]);      // Post a semaphore for Timer A6 Counter
        }
    }
    if (TMRA_status & TMA5_MASK) {       // if Timer A5 has trigered
        count_TMRA5++;                   // increment Timer A5 counter
        if (count_TMRA5 == MAX_COUNT) {  // if Timer A5 has interrupted the CPU 100 times
            display_count[0]++;          // increment corresponding display counter
            count_TMRA5 = 0;             // clear the Timer A5 counter
            OSSemPost(TimerSem[0]);      // Post a semaphore for Timer A5 Counter
        }
    }
    OSIntExit();
}

// Helper functions
void InitTimerInt()
{
    auto UBYTE i;
    count_TMRA5 = 0;
    count_TMRA6 = 0;
    count_TMRA7 = 0;
    for (i = 0; i < N_TASKS; i++)
        display_count[i] = 0;
    // Setup internal interrupt and set up timer A 
    SetVectIntern(0x0A, Tmr_A_isr);      // Set up timer A interrupt vector
    WrPortI(TAT7R, &TAT7RShadow, 0x3F);  // Set TMRA7 to count down from 63
    WrPortI(TAT6R, &TAT6RShadow, 0x7F);  // Set TMRA6 to count down from 127
    WrPortI(TAT5R, &TAT5RShadow, 0xFF);  // Set TMRA5 to count down from 255
    WrPortI(TAT1R, &TAT1RShadow, 0xFF);  // Set TMRA1 to count down from 255
    WrPortI(TACR,  &TACRShadow,  0xE1);  // Clock TMRA5-7 by TMRA1 and set interrupt priority to 1
    WrPortI(TACSR, &TACSRShadow, 0xE1);  // Enable TMRA5-7 and main clock for Timer A
}

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

void DispStr (int x, int y, char *s)
{
    x += 0x20;
    y += 0x20;
    printf ("\x1B=%c%c%s", x, y, s);
}
