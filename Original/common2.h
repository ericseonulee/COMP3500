/*****************************************************************************\
 * Laboratory Exercises COMP 3500                                              *
 * Author: Saad Biaz                                                           *
 * Updated for COMP 3500 to generate Processes                                 *
 * Updated 5/5/2017 included <unistd.h>                                        *
 * Date  : February 20, 2009 (Improved compared to the version for lab2)       *
 *         January 18, 2012  (Improved it)                                     *
 * File  : common.h                                                            *
 \*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#include <unistd.h>   /* for getpid, read, write, fork                       */
/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

typedef int             Identifier;
typedef int             Priority;
typedef short           Flag;
typedef unsigned        Status;
typedef enum {NEW, READY, RUNNING, WAITING, DONE} State;
typedef enum {JOBQUEUE,READYQUEUE,RUNNINGQUEUE,WAITINGQUEUE,EXITQUEUE} Queue;
typedef double          Timestamp;
typedef double          TimePeriod;
typedef int             Quantity;
typedef double          Average;
typedef unsigned        Memory;

/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/

// Constants for Process Management
#define  MAXQUEUES           5     /* Max queues Job, Ready, Run.., W., Ex   */
#define  MAX_PRIORITIES      4     /* Number of Priority Levels              */
#define  FALSE               0     /* False condition                        */
#define  TRUE                1     /* True condition                         */

#define  RATIO_TOTAL_CPUBURST 4.0  /* Ratio TotalJobDuration to CpuBurstTime */
//For second chance lab 1, changed RATIO_IOBURST_CPUBURST from 3.0 to 0.25
#define  RATIO_IOBURST_CPUBURST 0.25 /* Ratio IOBurst to CPU Burst           */
#define  CONTEXTSWITCHOVERHEAD 0.0001 /* Computing Overhead due to Context Switch */

// Constant for Memory Management
#define  MAXMEMORYSIZE 0x100000 // 1 MB                   
#define  MAXPROCMEMORY  0x10000 // 64KB Max Process Size to get 32KB average
#define  MINPROCMEMORY    0x100 // 256 Bytes Min Process Size                    


/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/

typedef struct ProcessControlBlockTag{
  Identifier ProcessID;
  State      state;
  Priority   priority;
  Timestamp  JobArrivalTime;   /* Time when job first entered job queue    */
  TimePeriod TotalJobDuration; /* Total CPU time job requires              */
  TimePeriod TimeInCpu;        /* Total time process spent so far on CPU   */
  TimePeriod CpuBurstTime;     /* Length of typical CPU burst of job       */
  TimePeriod RemainingCpuBurstTime; /* Remaing time of current CPU burst   */
  TimePeriod IOBurstTime;      /* Length of typical I/O burst of job       */
  TimePeriod TimeIOBurstDone;  /* Time when current I/O will be done       */
  Timestamp  JobStartTime;     /* Time when job first entered ready queue  */
  Timestamp  StartCpuTime;     /* Time when job was first placed on CPU    */
  Timestamp  TimeEnterWaiting; /* Last time Job Entered the Waiting Queue  */
  Timestamp  JobExitTime;      /* Time when job first entered exit queue   */
  TimePeriod TimeInReadyQueue; /* Total time process spent in ready queue  */
  TimePeriod TimeInWaitQueue;  /* Total time process spent in wait queue   */
  TimePeriod TimeInJobQueue;   /* Total time process spent in job queue    */
  Memory     TopOfMemory;      /* Address of top of allocated memory block */
  Memory     MemoryAllocated;  /* Amount of Allocated memory in bytes      */
  Memory     MemoryRequested;  /* Amount of requested memory in bytes      */
  struct ProcessControlBlockTag *previous; /* previous element in linked list */
  struct ProcessControlBlockTag *next;     /* next element in linked list */
} ProcessControlBlock;

typedef struct QueueParmsTag{
  ProcessControlBlock *Head;
  ProcessControlBlock *Tail;
} QueueParms;

/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
extern Flag                  Show;
extern QueueParms            Queues[MAXQUEUES];   /* Head and Tails for Job, Ready...queues  */
extern Identifier            PolicyNumber; /* (1:FCFS) , (2:SRTF), (3:RR) */
extern TimePeriod            Quantum;        

extern Memory                AvailableMemory; // Total Available Memory

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

/***********************************************************************\                    
* Input    : None                                                       *                    
* Output   : Returns the current system time                            *                        
\***********************************************************************/
extern Timestamp            Now(void);

/***********************************************************************\                         
 * Input : Queue where to enqueue and Element to enqueue                 *                        
 * Output: Updates Head and Tail as needed                               *                        
 * Function: Enqueues FIFO element in queue and updates tail and head    *                        
\***********************************************************************/
extern void                 EnqueueProcess(Queue whichQueue,
					   ProcessControlBlock *whichProcess);

/***********************************************************************\                         
 * Input : Queue from which to dequeue                                  *                         
 * Output: Tail of queue                                                *                         
 * Function: Removes tail element and updates tail and head accordingly *                        
\***********************************************************************/
extern ProcessControlBlock *DequeueProcess(Queue whichQueue);

/***********************************************************************\                         
 * Input : none                                                          *                        
 * Output: None                                                          *                        
 * Function: CPU executes whichProcess for CPUBurst                      *                        
\***********************************************************************/
extern void                 OnCPU(ProcessControlBlock *whichProcess, TimePeriod CPUBurst);


extern void                 NewJobIn(ProcessControlBlock whichProcess);
extern void                 BookKeeping(void);
extern Flag                 Initialization(int argc, char **argv);
extern void                 DisplayProcess(char c,ProcessControlBlock *whichProcess);
extern void                 DisplayQueue(char *str, Queue whichQueue);
















