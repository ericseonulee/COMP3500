
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 6/5/2017 to distribute to students to redo Lab 1                    *
* Updated 5/9/2017 for COMP 3500 labs                                         *
* Date  : February 20, 2009                                                   *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common2.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

//typedef enum {TAT,RT,CBT,THGT,WT} Metric;
typedef enum {TAT,RT,CBT,THGT,WT,AWTJQ} Metric;
typedef enum {INFINITE,OMAP,PAGING,BESTFIT,WORSTFIT} MemoryPolicy;

/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 10 
#define FCFS            1 
#define RR              3 


#define MAXMETRICS      6



/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/
typedef struct MemoryBlock {
	struct MemoryBlock *previous;
	struct MemoryBlock *next;
  Memory size;
  Memory top;
	ProcessControlBlock *process;
} MemoryBlock;
typedef struct MemoryBlockList {
	MemoryBlock *Head;
	MemoryBlock *Tail;
} MemoryBlockList;

/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

Quantity NumberofJobs[MAXMETRICS]; // Number of Jobs for which metric was collected
Average  SumMetrics[MAXMETRICS]; // Sum for each Metrics
MemoryBlockList MemoryBlocks;
const MemoryPolicy policy = WORSTFIT; // Policy selection
const int PageSize = 8192;
int NumberOfAvailablePages;//AvailableMemory expressed as pages
int NumberOfRequestedPages; //MemoryRequested expressed as pages 
/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void                 ManageProcesses(void);
void                 NewJobIn(ProcessControlBlock whichProcess);
void                 BookKeeping(void);
Flag                 ManagementInitialization(void);
void                 LongtermScheduler(void);
void                 IO();
void                 CPUScheduler(Identifier whichPolicy);
ProcessControlBlock *SRTF();
void                 Dispatcher();
Flag                 omap (ProcessControlBlock *currentProcess);
Flag                 paging (ProcessControlBlock *currentProcess);
Flag                 bestfit (ProcessControlBlock *currentProcess);
Flag                 worstfit (ProcessControlBlock *currentProcess);
Flag				         removeBlock(MemoryBlock *memoryBlock);
void                 push(MemoryBlock *MemoryBlock);
void                 compactMemory();
void                 PrintMemoryBlocks();


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment operating systems. The parent    *
*           process is the "Operating Systems" managing the processes using   *
*           the resources (CPU and Memory) of the system                      *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
   if (Initialization(argc,argv)){
     ManageProcesses();
   }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Sources and process events (written by students)    *
\***********************************************************************/

void ManageProcesses(void){
  ManagementInitialization();
  while (1) {
    IO();
    CPUScheduler(PolicyNumber);
    Dispatcher();
  }
}

/***********************************************************************\
* Input : none                                                          *          
* Output: None                                                          *        
* Function:                                                             *
*    1) if CPU Burst done, then move process on CPU to Waiting Queue    *
*         otherwise (RR) return to rReady Queue                         *                           
*    2) scan Waiting Queue to find processes with complete I/O          *
*           and move them to Ready Queue                                *         
\***********************************************************************/
void IO() {
  ProcessControlBlock *currentProcess = DequeueProcess(RUNNINGQUEUE); 
  if (currentProcess){
    if (currentProcess->RemainingCpuBurstTime <= 0) { // Finished current CPU Burst
      currentProcess->TimeEnterWaiting = Now(); // Record when entered the waiting queue
      EnqueueProcess(WAITINGQUEUE, currentProcess); // Move to Waiting Queue
      currentProcess->TimeIOBurstDone = Now() + currentProcess->IOBurstTime; // Record when IO completes
      currentProcess->state = WAITING;
    } else { // Must return to Ready Queue                
      currentProcess->JobStartTime = Now();                                               
      EnqueueProcess(READYQUEUE, currentProcess); // Mobe back to Ready Queue
      currentProcess->state = READY; // Update PCB state 
    }
  }

  /* Scan Waiting Queue to find processes that got IOs  complete*/
  ProcessControlBlock *ProcessToMove;
  /* Scan Waiting List to find processes that got complete IOs */
  ProcessToMove = DequeueProcess(WAITINGQUEUE);
  if (ProcessToMove){
    Identifier IDFirstProcess =ProcessToMove->ProcessID;
    EnqueueProcess(WAITINGQUEUE,ProcessToMove);
    ProcessToMove = DequeueProcess(WAITINGQUEUE);
    while (ProcessToMove){
      if (Now()>=ProcessToMove->TimeIOBurstDone){
	ProcessToMove->RemainingCpuBurstTime = ProcessToMove->CpuBurstTime;
	ProcessToMove->JobStartTime = Now();
	EnqueueProcess(READYQUEUE,ProcessToMove);
      } else {
	EnqueueProcess(WAITINGQUEUE,ProcessToMove);
      }
      if (ProcessToMove->ProcessID == IDFirstProcess){
	break;
      }
      ProcessToMove =DequeueProcess(WAITINGQUEUE);
    } // while (ProcessToMove)
  } // if (ProcessToMove)
}

/***********************************************************************\    
 * Input : whichPolicy (1:FCFS, 2: SRTF, and 3:RR)                      *        
 * Output: None                                                         * 
 * Function: Selects Process from Ready Queue and Puts it on Running Q. *
\***********************************************************************/
void CPUScheduler(Identifier whichPolicy) {
  ProcessControlBlock *selectedProcess;
  if ((whichPolicy == FCFS) || (whichPolicy == RR)) {
    selectedProcess = DequeueProcess(READYQUEUE);
  } else{ // Shortest Remaining Time First 
    selectedProcess = SRTF();
  }
  if (selectedProcess) {
    selectedProcess->state = RUNNING; // Process state becomes Running                                     
    EnqueueProcess(RUNNINGQUEUE, selectedProcess); // Put process in Running Queue                         
  }
}

/***********************************************************************\                         
 * Input : None                                                         *                                     
 * Output: Pointer to the process with shortest remaining time (SRTF)   *                                     
 * Function: Returns process control block with SRTF                    *                                     
\***********************************************************************/
ProcessControlBlock *SRTF() {
  /* Select Process with Shortest Remaining Time*/
  ProcessControlBlock *selectedProcess, *currentProcess = DequeueProcess(READYQUEUE);
  selectedProcess = (ProcessControlBlock *) NULL;
  if (currentProcess){
    TimePeriod shortestRemainingTime = currentProcess->TotalJobDuration - currentProcess->TimeInCpu;
    Identifier IDFirstProcess =currentProcess->ProcessID;
    EnqueueProcess(READYQUEUE,currentProcess);
    currentProcess = DequeueProcess(READYQUEUE);
    while (currentProcess){
      if (shortestRemainingTime >= (currentProcess->TotalJobDuration - currentProcess->TimeInCpu)){
        EnqueueProcess(READYQUEUE,selectedProcess);
        selectedProcess = currentProcess;
        shortestRemainingTime = currentProcess->TotalJobDuration - currentProcess->TimeInCpu;
      }
      else {
	      EnqueueProcess(READYQUEUE,currentProcess);
      }
      if (currentProcess->ProcessID == IDFirstProcess){
	      break;
      }
      currentProcess =DequeueProcess(READYQUEUE);
    } // while (ProcessToMove)
  } // if (currentProcess)
  return(selectedProcess);
}

/***********************************************************************\  
 * Input : None                                                         *   
 * Output: None                                                         *   
 * Function:                                                            *
 *  1)If process in Running Queue needs computation, put it on CPU      *
 *              else move process from running queue to Exit Queue      *     
\***********************************************************************/
void Dispatcher() {
  double start;
  ProcessControlBlock *processOnCPU = Queues[RUNNINGQUEUE].Tail;
  if (!processOnCPU) {
    return;
  }
  if(processOnCPU->TimeInCpu == 0.0) {
    SumMetrics[RT] += Now()- processOnCPU->JobArrivalTime;
    NumberofJobs[RT]++;
    processOnCPU->StartCpuTime = Now();
  }
  
  if (processOnCPU->TimeInCpu >= processOnCPU-> TotalJobDuration) {
    printf(" >>>>>Process # %d complete, %d Processes Completed So Far <<<<<<\n",
	  processOnCPU->ProcessID,NumberofJobs[THGT]);   
    processOnCPU=DequeueProcess(RUNNINGQUEUE);
    EnqueueProcess(EXITQUEUE,processOnCPU);
    if (policy == OMAP) {
      AvailableMemory += processOnCPU->MemoryAllocated;
      printf(" >> deallocated %u from %d, %u AvailableMemory\n", processOnCPU->MemoryAllocated, processOnCPU->ProcessID, AvailableMemory);
      processOnCPU->MemoryAllocated = 0;
    }
    else if (policy == BESTFIT || policy == WORSTFIT) {
      struct MemoryBlock *memoryBlock = MemoryBlocks.Head;
      while(memoryBlock) {
        if (memoryBlock->process && memoryBlock->process->ProcessID == processOnCPU->ProcessID) {
          memoryBlock->process = NULL;
          printf(" >> deallocated memory block from process %d\n", processOnCPU->ProcessID);
          break;
        }
        memoryBlock = memoryBlock->next;
      }
    }
    else if (policy == PAGING) {
        NumberOfRequestedPages = ceil((float)processOnCPU->MemoryRequested/PageSize);
        printf(" >> deallocated %d pages from %d, %d frames available\n", NumberOfRequestedPages, processOnCPU->ProcessID, NumberOfAvailablePages);
        NumberOfAvailablePages += NumberOfRequestedPages;
    }

    NumberofJobs[THGT]++;
    NumberofJobs[TAT]++;
    NumberofJobs[WT]++;
    NumberofJobs[CBT]++;
    SumMetrics[TAT]     += Now() - processOnCPU->JobArrivalTime;
    SumMetrics[WT]      += processOnCPU->TimeInReadyQueue;

    LongtermScheduler();
  }
  else {
    TimePeriod CpuBurstTime = processOnCPU->CpuBurstTime;
    processOnCPU->TimeInReadyQueue += Now() - processOnCPU->JobStartTime;
    if (PolicyNumber == RR) {
      CpuBurstTime = Quantum;
      if (processOnCPU->RemainingCpuBurstTime < Quantum)
	      CpuBurstTime = processOnCPU->RemainingCpuBurstTime;
    }
    processOnCPU->RemainingCpuBurstTime -= CpuBurstTime;

    TimePeriod StartExecution = Now();
    OnCPU(processOnCPU, CpuBurstTime);
    processOnCPU->TimeInCpu += CpuBurstTime;
    SumMetrics[CBT] += CpuBurstTime;
  }
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run when a job is added to the Job Queue    *
\***********************************************************************/
void NewJobIn(ProcessControlBlock whichProcess){
  ProcessControlBlock *NewProcess;
  /* Add Job to the Job Queue */
  NewProcess = (ProcessControlBlock *) malloc(sizeof(ProcessControlBlock));
  memcpy(NewProcess,&whichProcess,sizeof(whichProcess));
  NewProcess->TimeInCpu = 0; // Fixes TUX error
  NewProcess->RemainingCpuBurstTime = NewProcess->CpuBurstTime; // SB_ 6/4 Fixes RR
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}


/***********************************************************************\                                                   
* Input : None                                                         *                                                    
* Output: None                                                         *                                                    
* Function:                                                            *
* 1) BookKeeping is called automatically when 250 arrived              *
* 2) Computes and display metrics: average turnaround  time, throughput*
*     average response time, average waiting time in ready queue,      *
*     and CPU Utilization                                              *                                                     
\***********************************************************************/
void BookKeeping(void){
  double end = Now(); // Total time for all processes to arrive
  Metric m;

  // Compute averages and final results
  if (NumberofJobs[TAT] > 0){
    SumMetrics[TAT] = SumMetrics[TAT]/ (Average) NumberofJobs[TAT];
  }
  if (NumberofJobs[RT] > 0){
    SumMetrics[RT] = SumMetrics[RT]/ (Average) NumberofJobs[RT];
  }
  SumMetrics[CBT] = SumMetrics[CBT]/ Now();

  if (NumberofJobs[WT] > 0){
    SumMetrics[WT] = SumMetrics[WT]/ (Average) NumberofJobs[WT];
  }
  
  if (NumberofJobs[AWTJQ] > 0){
    SumMetrics[AWTJQ] = SumMetrics[AWTJQ] / (Average) NumberofJobs[AWTJQ];
  }

  printf("\n********* Processes Managemenent Numbers ******************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", NumberofJobs[THGT]);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f  AWTJQ=%f\n", 
	 SumMetrics[TAT], SumMetrics[RT], SumMetrics[CBT], 
	 NumberofJobs[THGT]/Now(), SumMetrics[WT], SumMetrics[AWTJQ]);

  exit(0);
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: Decides which processes should be admitted in Ready Queue   *
*           If enough memory and within multiprogramming limit,         *
*           then move Process from Job Queue to Ready Queue             *
\***********************************************************************/
void LongtermScheduler(void) {
  ProcessControlBlock *currentProcess = DequeueProcess(JOBQUEUE);
  Flag hasSucceeded;
  while (currentProcess) {
   switch(policy) {
    case OMAP: hasSucceeded = omap(currentProcess);
      break;
    case PAGING: hasSucceeded = paging(currentProcess);
      break;
    case BESTFIT: hasSucceeded = bestfit(currentProcess);
      break;
    case WORSTFIT: hasSucceeded = worstfit(currentProcess);
      break;
    case INFINITE: hasSucceeded = TRUE;
      break;
   }
    if (hasSucceeded) {
      currentProcess->TimeInJobQueue = Now() - currentProcess->JobArrivalTime;
      currentProcess->MemoryAllocated = currentProcess->MemoryRequested;
      SumMetrics[AWTJQ] += currentProcess->TimeInJobQueue;
      NumberofJobs[AWTJQ]++;
      currentProcess->JobStartTime = Now();
      EnqueueProcess(READYQUEUE, currentProcess);
      currentProcess->state = READY;
      currentProcess = DequeueProcess(JOBQUEUE);
    }
    else {
      printf(">> not enough memory for process %d\n", currentProcess->ProcessID);
      EnqueueProcess(JOBQUEUE, currentProcess);
      break;
    }
  }
}

Flag omap (ProcessControlBlock *currentProcess) {
  if (AvailableMemory >= currentProcess->MemoryRequested ) {
    AvailableMemory -= currentProcess->MemoryRequested;
    currentProcess->MemoryAllocated = currentProcess->MemoryRequested;
    printf(" >> allocated %u to %d, %u AvailableMemory\n", currentProcess->MemoryAllocated, currentProcess->ProcessID, AvailableMemory);
    return TRUE;
  }
  else {
    return FALSE;
  }
}

Flag paging(ProcessControlBlock *currentProcess)  {
  NumberOfRequestedPages = ceil((float) currentProcess->MemoryRequested/PageSize);
  if (NumberOfAvailablePages >= NumberOfRequestedPages) {
    NumberOfAvailablePages -= NumberOfRequestedPages;
    return TRUE;
  }
  else {
    return FALSE;
  }
}

Flag bestfit (ProcessControlBlock *currentProcess) {
   struct MemoryBlock* currentMemoryBlock = MemoryBlocks.Head;
   struct MemoryBlock* selectedMemoryBlock;
   Memory sizeOfSmallestBlock = UINT_MAX; // -1 is max unsigned int
   while (currentMemoryBlock) {
      if (!currentMemoryBlock->process && currentMemoryBlock->size >= currentProcess->MemoryRequested && currentMemoryBlock->size <= sizeOfSmallestBlock) {
        selectedMemoryBlock = currentMemoryBlock;
        sizeOfSmallestBlock = currentMemoryBlock->size;
      }
      currentMemoryBlock = currentMemoryBlock->next;
    }
    if (selectedMemoryBlock && selectedMemoryBlock->top + currentProcess->MemoryRequested < AvailableMemory) { // assign that process to the selected blocks
      struct MemoryBlock *newBlock = malloc(sizeof(MemoryBlock));
      newBlock->process = currentProcess;
      newBlock->top = selectedMemoryBlock->top;
      newBlock->size = currentProcess->MemoryRequested;
      printf(" >> Allocating block at %u to process %d\n", newBlock->top, newBlock->process->ProcessID);
      push(newBlock);

      currentProcess->TopOfMemory = selectedMemoryBlock->top;

      selectedMemoryBlock->top += currentProcess->MemoryRequested;

      selectedMemoryBlock->size -= currentProcess->MemoryRequested;
      return TRUE;
    }
    else {
      compactMemory();
      if (!MemoryBlocks.Tail->process && MemoryBlocks.Tail->size >= currentProcess->MemoryRequested) {
        struct MemoryBlock *newBlock = malloc(sizeof(MemoryBlock));
        newBlock->process = currentProcess;
        newBlock->top = selectedMemoryBlock->top;
        newBlock->size = currentProcess->MemoryRequested;
        printf(" >> COMPACTION SUCCESS! Allocating block at %u to process %d\n", newBlock->top, newBlock->process->ProcessID);
        push(newBlock);

        currentProcess->TopOfMemory = selectedMemoryBlock->top;
        selectedMemoryBlock->top += currentProcess->MemoryRequested;
        selectedMemoryBlock->size -= currentProcess->MemoryRequested;
        return TRUE;
      }
    }
  return FALSE;
}

Flag worstfit(ProcessControlBlock *currentProcess) {
  struct MemoryBlock* currentMemoryBlock = MemoryBlocks.Head;
   struct MemoryBlock* selectedMemoryBlock;
   Memory sizeOfBiggestBlock = 0;
   while (currentMemoryBlock) {
      if (!currentMemoryBlock->process && currentMemoryBlock->size >= currentProcess->MemoryRequested && currentMemoryBlock->size >= sizeOfBiggestBlock) {
        selectedMemoryBlock = currentMemoryBlock;
        sizeOfBiggestBlock = selectedMemoryBlock->size;
      }
      currentMemoryBlock = currentMemoryBlock->next;
    }
    if (selectedMemoryBlock && selectedMemoryBlock->top + currentProcess->MemoryRequested <= AvailableMemory) { // assign that process to the selected blocks
      struct MemoryBlock *newBlock = malloc(sizeof(MemoryBlock));
      newBlock->process = currentProcess;
      newBlock->top = selectedMemoryBlock->top;
      newBlock->size = currentProcess->MemoryRequested;
      printf(" >> Allocating block at %u to process %d\n", newBlock->top, newBlock->process->ProcessID);
      push(newBlock);

      currentProcess->TopOfMemory = selectedMemoryBlock->top;
      selectedMemoryBlock->top += currentProcess->MemoryRequested;
      selectedMemoryBlock->size -= currentProcess->MemoryRequested;
      return TRUE;
    }
    else {
      compactMemory();
      if (!MemoryBlocks.Tail->process && MemoryBlocks.Tail->size >= currentProcess->MemoryRequested) {
        struct MemoryBlock *newBlock = malloc(sizeof(MemoryBlock));
        newBlock->process = currentProcess;
        newBlock->top = selectedMemoryBlock->top;
        newBlock->size = currentProcess->MemoryRequested;
        printf(" >> COMPACTION SUCCESS! Allocating block at %u to process %d\n", newBlock->top, newBlock->process->ProcessID);
        push(newBlock);
        currentProcess->TopOfMemory = selectedMemoryBlock->top;
        selectedMemoryBlock->top += currentProcess->MemoryRequested;
        selectedMemoryBlock->size -= currentProcess->MemoryRequested;
        return TRUE;
      } 
    }
  return FALSE;
}

void compactMemory() {
  printf("!! MEMORY COMPACTION... !!\n");
  struct MemoryBlock *memoryBlock = MemoryBlocks.Head;
  Memory sizeOfNewBlock = 0;
  Memory addressOfNewBlock = 0;
  int count = 0;
  while (memoryBlock) {
    if (memoryBlock->process == NULL) {
      sizeOfNewBlock += memoryBlock->size;
      removeBlock(memoryBlock);
      count++;
    }
    else {
      memoryBlock->process->TopOfMemory = addressOfNewBlock;
      addressOfNewBlock += memoryBlock->process->MemoryAllocated;
    }
    memoryBlock = memoryBlock->next;
  }
  if (sizeOfNewBlock > 0) {
    struct MemoryBlock *newFreeBlock = malloc(sizeof(MemoryBlock));
    newFreeBlock->size = sizeOfNewBlock;
    newFreeBlock->top = addressOfNewBlock;
    push(newFreeBlock);
    printf("!! COMPACTED %d MEMORY BLOCKS FOR %d BYTES !!\n", count, sizeOfNewBlock);
    PrintMemoryBlocks();
  }
}

/***********************************************************************\
* Input : pointer to the block                                          *
* Output: if successfully deleted                                       *
\***********************************************************************/
Flag removeBlock(MemoryBlock *memoryBlock) {
	if (memoryBlock) {
		if (memoryBlock->previous) {
			if (memoryBlock->next) {
				memoryBlock->previous->next = memoryBlock->next;
				memoryBlock->next->previous = memoryBlock->previous;
				return TRUE;
			}
      else {
				MemoryBlocks.Tail = memoryBlock->previous;
				memoryBlock->previous->next = NULL;
				return TRUE;
			}
		}
    else if (memoryBlock->next) {
			MemoryBlocks.Head = memoryBlock->next;
			memoryBlock->next->previous = NULL;
			return TRUE;
		}
    else {
			return FALSE;
		}
	}	
}

void push(MemoryBlock *memoryBlock) {
  memoryBlock->previous = MemoryBlocks.Tail;
  memoryBlock->previous->next = memoryBlock;
  memoryBlock->next = NULL;
  MemoryBlocks.Tail = memoryBlock;
}

void PrintMemoryBlocks() {
  printf("---------- MEMORY BLOCKS ----------\n");
  struct MemoryBlock* temp = MemoryBlocks.Head;
  while (temp) {
    if (temp->process) {
      printf("Block occupied by process %d at address %u\n",temp->process->ProcessID, temp->process->TopOfMemory);
    } else {
      printf("Unoccupied block location: %u Block size: %u\n",temp->top, temp->size);
    }
    temp = temp->next;
  }

  printf("---------- END MEMORY BLOCKS ----------\n");
}

/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Intialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  NumberOfAvailablePages = floor(AvailableMemory / PageSize);
  Metric m;

  for (m = TAT; m < MAXMETRICS; m++) {
     NumberofJobs[m] = 0;
     SumMetrics[m]   = 0.0;
  }

  struct MemoryBlock* memory = malloc(sizeof(MemoryBlock));
  memory->top = 0;
  memory->size = AvailableMemory;
  MemoryBlocks.Head = memory;
  MemoryBlocks.Tail = memory;
  PrintMemoryBlocks();
  //return TRUE;
}