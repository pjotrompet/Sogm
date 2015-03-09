//-----------------------------------------------------
//
// Code generated with Faust 0.9.71 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
/************************************************************************

	IMPORTANT NOTE : this file contains two clearly delimited sections : 
	the ARCHITECTURE section (in two parts) and the USER section. Each section 
	is governed by its own copyright and license. Please check individually 
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either version 3 of 
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License 
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work 
	that contains this FAUST architecture section and distribute  
	that work under terms of your choice, so long as this FAUST 
	architecture section is not modified. 


 ************************************************************************
 ************************************************************************/

#include <libgen.h>
#include <stdlib.h>
#include <iostream>
#include <list>

#include "faust/gui/FUI.h"
#include "faust/misc.h"
#include "faust/gui/GUI.h"
#include "faust/gui/console.h"
#include "faust/audio/alsa-dsp.h"

#ifdef OSCCTRL
#include "faust/gui/OSCUI.h"
#endif

#ifdef HTTPCTRL
#include "faust/gui/httpdUI.h"
#endif


/**************************BEGIN USER SECTION **************************/
/******************************************************************************
*******************************************************************************

							       VECTOR INTRINSICS

*******************************************************************************
*******************************************************************************/


#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

using namespace std;

// Globals

#define THREAD_SIZE 64
#define QUEUE_SIZE 4096

#define WORK_STEALING_INDEX 0
#define LAST_TASK_INDEX 1


#ifdef __ICC
#define INLINE __forceinline
#else
#define INLINE inline
#endif


// On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
// flags to avoid costly denormals
#ifdef __SSE__
#include <xmmintrin.h>
#ifdef __SSE2__
#define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8040)
#else
#define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8000)
#endif
#else
#define AVOIDDENORMALS 
#endif

#ifdef __linux__

// handle 32/64 bits int size issues
#ifdef __x86_64__
#define UInt32	unsigned int
#define UInt64	unsigned long int
#else
#define UInt32	unsigned int
#define UInt64	unsigned long long int
#endif

#endif

#ifdef __APPLE__
#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacTypes.h>
#endif

class TaskQueue;
struct DSPThreadPool;

extern TaskQueue* gTaskQueueList[THREAD_SIZE];
extern DSPThreadPool* gThreadPool;
extern int gClientCount;
extern UInt64 gMaxStealing;
    
void Yield();

/**
 * Returns the number of clock cycles elapsed since the last reset
 * of the processor
 */
static INLINE UInt64 DSP_rdtsc(void)
{
	union {
		UInt32 i32[2];
		UInt64 i64;
	} count;
	
	__asm__ __volatile__("rdtsc" : "=a" (count.i32[0]), "=d" (count.i32[1]));
     return count.i64;
}

#if defined(__i386__) || defined(__x86_64__)

#define LOCK "lock ; "

static INLINE void NOP(void)
{
	__asm__ __volatile__("nop \n\t");
}

static INLINE char CAS1(volatile void* addr, volatile int value, int newvalue)
{
    register char ret;
    __asm__ __volatile__ (
						  "# CAS \n\t"
						  LOCK "cmpxchg %2, (%1) \n\t"
						  "sete %0               \n\t"
						  : "=a" (ret)
						  : "c" (addr), "d" (newvalue), "a" (value)
                          : "memory"
						  );
    return ret;
}

static INLINE int atomic_xadd(volatile int* atomic, int val) 
{ 
    register int result;
    __asm__ __volatile__ ("# atomic_xadd \n\t"
                          LOCK "xaddl %0,%1 \n\t"
                          : "=r" (result), "=m" (*atomic) 
                          : "0" (val), "m" (*atomic));
    return result;
} 

#endif


/*
static INLINE int INC_ATOMIC(volatile int* val)
{
    int actual;
    do {
        actual = *val;
    } while (!CAS1(val, actual, actual + 1));
    return actual;
}

static INLINE int DEC_ATOMIC(volatile int* val)
{
    int actual;
    do {
        actual = *val;
    } while (!CAS1(val, actual, actual - 1));
    return actual;
}
*/

static INLINE int INC_ATOMIC(volatile int* val)
{
    return atomic_xadd(val, 1);
}
 
static INLINE int DEC_ATOMIC(volatile int* val)
{
    return atomic_xadd(val, -1);
}
 
// To be used in lock-free queue
struct AtomicCounter
{
    union {
        struct {
            short fHead;	
            short fTail;	
        }
        scounter;
        int fValue;
    }info;
    
	INLINE AtomicCounter()
	{
        info.fValue = 0;
    }
     
 	INLINE  AtomicCounter& operator=(AtomicCounter& obj)
    {
        info.fValue = obj.info.fValue;
        return *this;
    }
    
	INLINE  AtomicCounter& operator=(volatile AtomicCounter& obj)
	{
        info.fValue = obj.info.fValue;
        return *this;
    }
    
};

int get_max_cpu()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

static int GetPID()
{
#ifdef WIN32
    return  _getpid();
#else
    return getpid();
#endif
}

#define Value(e) (e).info.fValue

#define Head(e) (e).info.scounter.fHead
#define IncHead(e) (e).info.scounter.fHead++
#define DecHead(e) (e).info.scounter.fHead--

#define Tail(e) (e).info.scounter.fTail
#define IncTail(e) (e).info.scounter.fTail++
#define DecTail(e) (e).info.scounter.fTail--

#define MASTER_THREAD 0

#define MAX_STEAL_DUR 50                    // in usec
#define DEFAULT_CLOCKSPERSEC 2500000000     // in cycles (2,5 Ghz)

class TaskQueue 
{
    private:
    
        int fTaskList[QUEUE_SIZE];
        volatile AtomicCounter fCounter;
        UInt64 fStealingStart;
     
    public:
  
        INLINE TaskQueue(int cur_thread)
        {
            for (int i = 0; i < QUEUE_SIZE; i++) {
                fTaskList[i] = -1;
            }
            gTaskQueueList[cur_thread] = this;	
            fStealingStart = 0;
        }
         
        INLINE void PushHead(int item)
        {
            fTaskList[Head(fCounter)] = item;
            IncHead(fCounter);
        }
        
        INLINE int PopHead()
        {
            AtomicCounter old_val;
            AtomicCounter new_val;
            
            do {
                old_val = fCounter;
                new_val = old_val;
                if (Head(old_val) == Tail(old_val)) {
                    return WORK_STEALING_INDEX;
                } else {
                    DecHead(new_val);
                }
            } while (!CAS1(&fCounter, Value(old_val), Value(new_val)));
            
            return fTaskList[Head(old_val) - 1];
        }
        
        INLINE int PopTail()
        {
            AtomicCounter old_val;
            AtomicCounter new_val;
            
            do {
                old_val = fCounter;
                new_val = old_val;
                if (Head(old_val) == Tail(old_val)) {
                   return WORK_STEALING_INDEX;
                } else {
                    IncTail(new_val);
                }
            } while (!CAS1(&fCounter, Value(old_val), Value(new_val)));
            
            return fTaskList[Tail(old_val)];
        }

		INLINE void MeasureStealingDur()
		{
            // Takes first timetamp
            if (fStealingStart == 0) {
                fStealingStart = DSP_rdtsc();
            } else if ((DSP_rdtsc() - fStealingStart) > gMaxStealing) {
                Yield();
            }
		}

		INLINE void ResetStealingDur()
		{
            fStealingStart = 0;
		}
        
        static INLINE int GetNextTask(int thread, int num_threads)
        {
            int tasknum;
            for (int i = 0; i < num_threads; i++) {
                if ((i != thread) && gTaskQueueList[i] && (tasknum = gTaskQueueList[i]->PopTail()) != WORK_STEALING_INDEX) {
                #ifdef __linux__
					//if (thread != MASTER_THREAD)
						gTaskQueueList[thread]->ResetStealingDur();
                #endif
                    return tasknum;    // Task is found
                }
            }
            NOP();
          #ifdef __linux__
			//if (thread != MASTER_THREAD)
				gTaskQueueList[thread]->MeasureStealingDur();
        #endif
            return WORK_STEALING_INDEX;    // Otherwise will try "workstealing" again next cycle...
        }
        
        INLINE void InitTaskList(int task_list_size, int* task_list, int thread_num, int cur_thread, int& tasknum)
        {
            int task_slice = task_list_size / thread_num;
            int task_slice_rest = task_list_size % thread_num;

            if (task_slice == 0) {
                // Each thread directly executes one task
                tasknum = task_list[cur_thread];
                // Thread 0 takes remaining ready tasks 
                if (cur_thread == 0) { 
                    for (int index = 0; index < task_slice_rest - thread_num; index++) {
                        PushHead(task_list[task_slice_rest + index]);
                    }
                }
            } else {
                // Each thread takes a part of ready tasks
                int index;
                for (index = 0; index < task_slice - 1; index++) {
                    PushHead(task_list[cur_thread * task_slice + index]);
                }
                // Each thread directly executes one task 
                tasknum = task_list[cur_thread * task_slice + index];
                // Thread 0 takes remaining ready tasks 
                if (cur_thread == 0) {
                    for (index = 0; index < task_slice_rest; index++) {
                        PushHead(task_list[thread_num * task_slice + index]);
                    }
                }
            }
        }
        
        static INLINE void Init()
        {
            for (int i = 0; i < THREAD_SIZE; i++) {
                gTaskQueueList[i] = 0;
            }
        }
     
};

struct TaskGraph 
{
    volatile int gTaskList[QUEUE_SIZE];
    
    TaskGraph()
    {
        for (int i = 0; i < QUEUE_SIZE; i++) {
            gTaskList[i] = 0;
        } 
    }

    INLINE void InitTask(int task, int val)
    {
        gTaskList[task] = val;
    }
    
    void Display()
    {
        for (int i = 0; i < QUEUE_SIZE; i++) {
            printf("Task = %d activation = %d\n", i, gTaskList[i]);
        } 
    }
      
    INLINE void ActivateOutputTask(TaskQueue& queue, int task, int& tasknum)
    {
        if (DEC_ATOMIC(&gTaskList[task]) == 1) {
            if (tasknum == WORK_STEALING_INDEX) {
                tasknum = task;
            } else {
                queue.PushHead(task);
            }
        }    
    }
      
    INLINE void ActivateOutputTask(TaskQueue& queue, int task)
    {
        if (DEC_ATOMIC(&gTaskList[task]) == 1) {
            queue.PushHead(task);
        }
    }
    
    INLINE void ActivateOneOutputTask(TaskQueue& queue, int task, int& tasknum)
    {
        if (DEC_ATOMIC(&gTaskList[task]) == 1) {
            tasknum = task;
        } else {
            tasknum = queue.PopHead(); 
        }
    }
    
    INLINE void GetReadyTask(TaskQueue& queue, int& tasknum)
    {
        if (tasknum == WORK_STEALING_INDEX) {
            tasknum = queue.PopHead();
        }
    }
 
};


#define THREAD_POOL_SIZE 16
#define JACK_SCHED_POLICY SCHED_FIFO

/* use 512KB stack per thread - the default is way too high to be feasible
 * with mlockall() on many systems */
#define THREAD_STACK 524288


#ifdef __APPLE__

#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacTypes.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#define THREAD_SET_PRIORITY         0
#define THREAD_SCHEDULED_PRIORITY   1

static UInt32 GetThreadPriority(pthread_t thread, int inWhichPriority);

// returns the thread's priority as it was last set by the API
static UInt32 GetThreadSetPriority(pthread_t thread)
{
    return GetThreadPriority(thread, THREAD_SET_PRIORITY);
}

// returns the thread's priority as it was last scheduled by the Kernel
static UInt32 GetThreadScheduledPriority(pthread_t thread)
{
    return GetThreadPriority(thread, THREAD_SCHEDULED_PRIORITY);
}

static int SetThreadToPriority(pthread_t thread, UInt32 inPriority, Boolean inIsFixed, UInt64 period, UInt64 computation, UInt64 constraint)
{
    if (inPriority == 96) {
        // REAL-TIME / TIME-CONSTRAINT THREAD
        thread_time_constraint_policy_data_t theTCPolicy;
        theTCPolicy.period = period;
        theTCPolicy.computation = computation;
        theTCPolicy.constraint = constraint;
        theTCPolicy.preemptible = true;
        kern_return_t res = thread_policy_set(pthread_mach_thread_np(thread), THREAD_TIME_CONSTRAINT_POLICY, (thread_policy_t)&theTCPolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT);
        return (res == KERN_SUCCESS) ? 0 : -1;
    } else {
        // OTHER THREADS
        thread_extended_policy_data_t theFixedPolicy;
        thread_precedence_policy_data_t thePrecedencePolicy;
        SInt32 relativePriority;
        
        // [1] SET FIXED / NOT FIXED
        theFixedPolicy.timeshare = !inIsFixed;
        thread_policy_set(pthread_mach_thread_np(thread), THREAD_EXTENDED_POLICY, (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);
        
        // [2] SET PRECEDENCE
        // N.B.: We expect that if thread A created thread B, and the program wishes to change
        // the priority of thread B, then the call to change the priority of thread B must be
        // made by thread A.
        // This assumption allows us to use pthread_self() to correctly calculate the priority
        // of the feeder thread (since precedency policy's importance is relative to the
        // spawning thread's priority.)
        relativePriority = inPriority - GetThreadSetPriority(pthread_self());
        
        thePrecedencePolicy.importance = relativePriority;
        kern_return_t res = thread_policy_set(pthread_mach_thread_np(thread), THREAD_PRECEDENCE_POLICY, (thread_policy_t)&thePrecedencePolicy, THREAD_PRECEDENCE_POLICY_COUNT);
        return (res == KERN_SUCCESS) ? 0 : -1;
    }
}

static UInt32 GetThreadPriority(pthread_t thread, int inWhichPriority)
{
    thread_basic_info_data_t threadInfo;
    policy_info_data_t thePolicyInfo;
    unsigned int count;
    
    // get basic info
    count = THREAD_BASIC_INFO_COUNT;
    thread_info(pthread_mach_thread_np(thread), THREAD_BASIC_INFO, (thread_info_t)&threadInfo, &count);
    
    switch (threadInfo.policy) {
        case POLICY_TIMESHARE:
            count = POLICY_TIMESHARE_INFO_COUNT;
            thread_info(pthread_mach_thread_np(thread), THREAD_SCHED_TIMESHARE_INFO, (thread_info_t)&(thePolicyInfo.ts), &count);
            if (inWhichPriority == THREAD_SCHEDULED_PRIORITY) {
                return thePolicyInfo.ts.cur_priority;
            } else {
                return thePolicyInfo.ts.base_priority;
            }
            break;
            
        case POLICY_FIFO:
            count = POLICY_FIFO_INFO_COUNT;
            thread_info(pthread_mach_thread_np(thread), THREAD_SCHED_FIFO_INFO, (thread_info_t)&(thePolicyInfo.fifo), &count);
            if ((thePolicyInfo.fifo.depressed) && (inWhichPriority == THREAD_SCHEDULED_PRIORITY)) {
                return thePolicyInfo.fifo.depress_priority;
            }
            return thePolicyInfo.fifo.base_priority;
            break;
            
        case POLICY_RR:
            count = POLICY_RR_INFO_COUNT;
            thread_info(pthread_mach_thread_np(thread), THREAD_SCHED_RR_INFO, (thread_info_t)&(thePolicyInfo.rr), &count);
            if ((thePolicyInfo.rr.depressed) && (inWhichPriority == THREAD_SCHEDULED_PRIORITY)) {
                return thePolicyInfo.rr.depress_priority;
            }
            return thePolicyInfo.rr.base_priority;
            break;
    }
    
    return 0;
}

static int GetParams(pthread_t thread, UInt64* period, UInt64* computation, UInt64* constraint)
{
    thread_time_constraint_policy_data_t theTCPolicy;
    mach_msg_type_number_t count = THREAD_TIME_CONSTRAINT_POLICY_COUNT;
    boolean_t get_default = false;
    
    kern_return_t res = thread_policy_get(pthread_mach_thread_np(thread),
                                          THREAD_TIME_CONSTRAINT_POLICY,
                                          (thread_policy_t)&theTCPolicy,
                                          &count,
                                          &get_default);
    if (res == KERN_SUCCESS) {
        *period = theTCPolicy.period;
        *computation = theTCPolicy.computation;
        *constraint = theTCPolicy.constraint;
        return 0;
    } else {
        return -1;
    }
}

static UInt64 period = 0;
static UInt64 computation = 0;
static UInt64 constraint = 0;

INLINE void GetRealTime()
{
    if (period == 0) {
        GetParams(pthread_self(), &period, &computation, &constraint);
    }
}

INLINE void SetRealTime()
{
    SetThreadToPriority(pthread_self(), 96, true, period, computation, constraint);
}

void CancelThread(pthread_t fThread)
{
    mach_port_t machThread = pthread_mach_thread_np(fThread);
    thread_terminate(machThread);
}

INLINE void Yield()
{
    //sched_yield();
}

#endif

#ifdef __linux__

static int faust_sched_policy = -1;
static struct sched_param faust_rt_param; 

INLINE void GetRealTime()
{
    if (faust_sched_policy == -1) {
        memset(&faust_rt_param, 0, sizeof(faust_rt_param));
    	pthread_getschedparam(pthread_self(), &faust_sched_policy, &faust_rt_param);
    }
}

INLINE void SetRealTime()
{
	faust_rt_param.sched_priority--;
    pthread_setschedparam(pthread_self(), faust_sched_policy, &faust_rt_param);
}

void CancelThread(pthread_t fThread)
{
    pthread_cancel(fThread);
    pthread_join(fThread, NULL);
}

INLINE void Yield()
{
    pthread_yield();
}


#endif

#define KDSPMESURE 50

static INLINE int Range(int min, int max, int val)
{
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

struct Runnable {
    
    UInt64 fTiming[KDSPMESURE];
    UInt64 fStart;
    UInt64 fStop;
    int fCounter;
    float fOldMean;
    int fOldfDynamicNumThreads;
    bool fDynAdapt;
    
    virtual void computeThread(int cur_thread) = 0;
    
    Runnable():fCounter(0), fOldMean(1000000000.f), fOldfDynamicNumThreads(1)
    {
    	memset(fTiming, 0, sizeof(long long int ) * KDSPMESURE);
        fDynAdapt = getenv("OMP_DYN_THREAD") ? strtol(getenv("OMP_DYN_THREAD"), NULL, 10) : false;
    }
    
    INLINE float ComputeMean()
    {
        float mean = 0;
        for (int i = 0; i < KDSPMESURE; i++) {
            mean += float(fTiming[i]);
        }
        mean /= float(KDSPMESURE);
        return mean;
    }
    
    INLINE void StartMeasure()
    {
        if (!fDynAdapt)
            return;
        
        fStart = DSP_rdtsc();
    }
     
    INLINE void StopMeasure(int staticthreadnum, int& dynthreadnum)
    {
        if (!fDynAdapt)
            return;
        
        fStop = DSP_rdtsc();
        fCounter = (fCounter + 1) % KDSPMESURE;
        if (fCounter == 0) {
            float mean = ComputeMean();
            if (fabs(mean - fOldMean) > 5000) {
                if (mean > fOldMean) { // Worse...
                    //printf("Worse %f %f\n", mean, fOldMean);
                    if (fOldfDynamicNumThreads > dynthreadnum) {
                        fOldfDynamicNumThreads = dynthreadnum;
                        dynthreadnum += 1;
                    } else {
                        fOldfDynamicNumThreads = dynthreadnum;
                        dynthreadnum -= 1;
                    }
                 } else { // Better...
                    //printf("Better %f %f\n", mean, fOldMean);
                    if (fOldfDynamicNumThreads > dynthreadnum) {
                        fOldfDynamicNumThreads = dynthreadnum;
                        dynthreadnum -= 1;
                    } else {
                        fOldfDynamicNumThreads = dynthreadnum;
                        dynthreadnum += 1;
                    }
                }
                fOldMean = mean;
                dynthreadnum = Range(1, staticthreadnum, dynthreadnum);
                //printf("dynthreadnum %d\n", dynthreadnum);
            }
        }
        fTiming[fCounter] = fStop - fStart; 
    }
};

struct DSPThread;

struct DSPThreadPool {
    
    DSPThread* fThreadPool[THREAD_POOL_SIZE];
    int fThreadCount; 
    volatile int fCurThreadCount;
      
    DSPThreadPool();
    ~DSPThreadPool();
    
    void StartAll(int num, bool realtime);
    void StopAll();
    void SignalAll(int num, Runnable* runnable);
    
    void SignalOne();
    bool IsFinished();
    
    static DSPThreadPool* Init();
    static void Destroy();
    
};

struct DSPThread {

    pthread_t fThread;
    DSPThreadPool* fThreadPool;
    Runnable* fRunnable;
    sem_t* fSemaphore;
    char fName[128];
    bool fRealTime;
    int fNum;
    
    DSPThread(int num, DSPThreadPool* pool)
    {
        fNum = num;
        fThreadPool = pool;
        fRunnable = NULL;
        fRealTime = false;
        
        sprintf(fName, "faust_sem_%d_%p", GetPID(), this);
        
        if ((fSemaphore = sem_open(fName, O_CREAT, 0777, 0)) == (sem_t*)SEM_FAILED) {
            printf("Allocate: can't check in named semaphore name = %s err = %s", fName, strerror(errno));
        }
    }

    virtual ~DSPThread()
    {
        sem_unlink(fName);
        sem_close(fSemaphore);
    }
    
    void Run()
    {
        while (sem_wait(fSemaphore) != 0) {}
        fRunnable->computeThread(fNum + 1);
        fThreadPool->SignalOne();
    }
    
    static void* ThreadHandler(void* arg)
    {
        DSPThread* thread = static_cast<DSPThread*>(arg);
        
        AVOIDDENORMALS;
        
        // One "dummy" cycle to setup thread
        if (thread->fRealTime) {
            thread->Run();
            SetRealTime();
        }
                  
        while (true) {
            thread->Run();
        }
        
        return NULL;
    }
    
    int Start(bool realtime)
    {
        pthread_attr_t attributes;
        struct sched_param rt_param;
        pthread_attr_init(&attributes);
        
        int priority = 60; // TODO
        int res;
        
        if (realtime) {
            fRealTime = true;
        }else {
            fRealTime = getenv("OMP_REALTIME") ? strtol(getenv("OMP_REALTIME"), NULL, 10) : true;
        }
                               
        if ((res = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE))) {
            printf("Cannot request joinable thread creation for real-time thread res = %d err = %s\n", res, strerror(errno));
            return -1;
        }

        if ((res = pthread_attr_setscope(&attributes, PTHREAD_SCOPE_SYSTEM))) {
            printf("Cannot set scheduling scope for real-time thread res = %d err = %s\n", res, strerror(errno));
            return -1;
        }

        if (realtime) {
            
            if ((res = pthread_attr_setinheritsched(&attributes, PTHREAD_EXPLICIT_SCHED))) {
                printf("Cannot request explicit scheduling for RT thread res = %d err = %s\n", res, strerror(errno));
                return -1;
            }
        
            if ((res = pthread_attr_setschedpolicy(&attributes, JACK_SCHED_POLICY))) {
                printf("Cannot set RR scheduling class for RT thread res = %d err = %s\n", res, strerror(errno));
                return -1;
            }
            
            memset(&rt_param, 0, sizeof(rt_param));
            rt_param.sched_priority = priority;

            if ((res = pthread_attr_setschedparam(&attributes, &rt_param))) {
                printf("Cannot set scheduling priority for RT thread res = %d err = %s\n", res, strerror(errno));
                return -1;
            }

        } else {
            
            if ((res = pthread_attr_setinheritsched(&attributes, PTHREAD_INHERIT_SCHED))) {
                printf("Cannot request explicit scheduling for RT thread res = %d err = %s\n", res, strerror(errno));
                return -1;
            }
        }
     
        if ((res = pthread_attr_setstacksize(&attributes, THREAD_STACK))) {
            printf("Cannot set thread stack size res = %d err = %s\n", res, strerror(errno));
            return -1;
        }
        
        if ((res = pthread_create(&fThread, &attributes, ThreadHandler, this))) {
            printf("Cannot create thread res = %d err = %s\n", res, strerror(errno));
            return -1;
        }

        pthread_attr_destroy(&attributes);
        return 0;
    }
    
    void Signal(bool stop, Runnable* runnable)
    {
        fRunnable = runnable;
        sem_post(fSemaphore);
    }
    
    void Stop()
    {
        CancelThread(fThread);
    }

};

DSPThreadPool::DSPThreadPool()
{
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        fThreadPool[i] = NULL;
    }
    fThreadCount = 0;
    fCurThreadCount = 0;
}

DSPThreadPool::~DSPThreadPool()
{
    StopAll();
    
    for (int i = 0; i < fThreadCount; i++) {
        delete(fThreadPool[i]);
        fThreadPool[i] = NULL;
    }
    
    fThreadCount = 0;
 }

void DSPThreadPool::StartAll(int num, bool realtime)
{
    if (fThreadCount == 0) {  // Protection for multiple call...  (like LADSPA plug-ins in Ardour)
        for (int i = 0; i < num; i++) {
            fThreadPool[i] = new DSPThread(i, this);
            fThreadPool[i]->Start(realtime);
            fThreadCount++;
        }
    }
}

void DSPThreadPool::StopAll()
{
    for (int i = 0; i < fThreadCount; i++) {
        fThreadPool[i]->Stop();
    }
}

void DSPThreadPool::SignalAll(int num, Runnable* runnable)
{
    fCurThreadCount = num;
        
    for (int i = 0; i < num; i++) {  // Important : use local num here...
        fThreadPool[i]->Signal(false, runnable);
    }
}

void DSPThreadPool::SignalOne()
{
    DEC_ATOMIC(&fCurThreadCount);
}

bool DSPThreadPool::IsFinished()
{
    return (fCurThreadCount == 0);
}

DSPThreadPool* DSPThreadPool::Init()
{
    if (gClientCount++ == 0 && !gThreadPool) {
        gThreadPool = new DSPThreadPool();
    }
    return gThreadPool;
}

void DSPThreadPool::Destroy()
{
    if (--gClientCount == 0 && gThreadPool) {
        delete gThreadPool;
        gThreadPool = NULL;
    }
}

#ifndef PLUG_IN

// Globals
TaskQueue* gTaskQueueList[THREAD_SIZE] = {0};

DSPThreadPool* gThreadPool = 0;
int gClientCount = 0;

int clock_per_microsec = (getenv("CLOCKSPERSEC") 
                ? strtoll(getenv("CLOCKSPERSEC"), NULL, 10) 
                : DEFAULT_CLOCKSPERSEC) / 1000000;
                
UInt64  gMaxStealing = getenv("OMP_STEALING_DUR") 
                ? strtoll(getenv("OMP_STEALING_DUR"), NULL, 10) * clock_per_microsec 
                : MAX_STEAL_DUR * clock_per_microsec;

#endif



#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

class mydsp : public dsp, public Runnable {
  private:
	quad 	fZec0[32];
	FAUSTFLOAT 	fslider0;
	quad 	fRec0[512];
	int 	fRec0_idx;
	int 	fRec0_idx_save;
	quad 	fRec1[2048];
	int 	fRec1_idx;
	int 	fRec1_idx_save;
	quad 	fRec2[2048];
	int 	fRec2_idx;
	int 	fRec2_idx_save;
	quad 	fRec3[4096];
	int 	fRec3_idx;
	int 	fRec3_idx_save;
	quad 	fRec4[4096];
	int 	fRec4_idx;
	int 	fRec4_idx_save;
	quad 	fZec1[32];
	quad 	fRec5[512];
	int 	fRec5_idx;
	int 	fRec5_idx_save;
	quad 	fRec6[2048];
	int 	fRec6_idx;
	int 	fRec6_idx_save;
	quad 	fRec7[4096];
	int 	fRec7_idx;
	int 	fRec7_idx_save;
	quad 	fRec8[1024];
	int 	fRec8_idx;
	int 	fRec8_idx_save;
	quad 	fRec9[4096];
	int 	fRec9_idx;
	int 	fRec9_idx_save;
	TaskGraph fGraph;
	FAUSTFLOAT** input;
	FAUSTFLOAT** output;
	volatile bool fIsFinished;
	int fCount;
	int fIndex;
	DSPThreadPool* fThreadPool;
	int fStaticNumThreads;
	int fDynamicNumThreads;
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL with exception");
	}

	virtual ~mydsp() 	{ DSPThreadPool::Destroy(); }
	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.81L;
		for (int i=0; i<512; i++) fRec0[i]=0;
		fRec0_idx = 0;
		fRec0_idx_save = 0;
		for (int i=0; i<2048; i++) fRec1[i]=0;
		fRec1_idx = 0;
		fRec1_idx_save = 0;
		for (int i=0; i<2048; i++) fRec2[i]=0;
		fRec2_idx = 0;
		fRec2_idx_save = 0;
		for (int i=0; i<4096; i++) fRec3[i]=0;
		fRec3_idx = 0;
		fRec3_idx_save = 0;
		for (int i=0; i<4096; i++) fRec4[i]=0;
		fRec4_idx = 0;
		fRec4_idx_save = 0;
		for (int i=0; i<512; i++) fRec5[i]=0;
		fRec5_idx = 0;
		fRec5_idx_save = 0;
		for (int i=0; i<2048; i++) fRec6[i]=0;
		fRec6_idx = 0;
		fRec6_idx_save = 0;
		for (int i=0; i<4096; i++) fRec7[i]=0;
		fRec7_idx = 0;
		fRec7_idx_save = 0;
		for (int i=0; i<1024; i++) fRec8[i]=0;
		fRec8_idx = 0;
		fRec8_idx_save = 0;
		for (int i=0; i<4096; i++) fRec9[i]=0;
		fRec9_idx = 0;
		fRec9_idx_save = 0;
		fStaticNumThreads = get_max_cpu();
		fDynamicNumThreads = getenv("OMP_NUM_THREADS") ? atoi(getenv("OMP_NUM_THREADS")) : fStaticNumThreads;
		fThreadPool = DSPThreadPool::Init();
		fThreadPool->StartAll(fStaticNumThreads - 1, false);
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("0x00");
		interface->addHorizontalSlider("Roomsize", &fslider0, 0.81L, 0.0L, 1.2L, 0.01L);
		interface->closeBox();
	}
	void display() {
		fGraph.Display();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		GetRealTime();
		this->input = input;
		this->output = output;
		StartMeasure();
		int fullcount = count;
		for (fIndex = 0; fIndex < fullcount; fIndex += 32) {
			fCount = min (32, fullcount-fIndex);
			TaskQueue::Init();
			// Initialize end task, if more than one input
			fGraph.InitTask(1,2);
			// Only initialize taks with more than one input
			fGraph.InitTask(14,5);
			fGraph.InitTask(15,5);
			fIsFinished = false;
			fThreadPool->SignalAll(fDynamicNumThreads - 1, this);
			computeThread(0);
			while (!fThreadPool->IsFinished()) {}
		}
		StopMeasure(fStaticNumThreads, fDynamicNumThreads);
	}
	void computeThread(int cur_thread) {
		int count = fCount;
		quad 	fSlow0 = min((quad)2, max((quad)0, quad(fslider0)));
		// Init graph state
		{
			TaskQueue taskqueue(cur_thread);
			int tasknum = -1;
			// Init input and output
			FAUSTFLOAT* input0 = &input[0][fIndex];
			FAUSTFLOAT* input1 = &input[1][fIndex];
			FAUSTFLOAT* output0 = &output[0][fIndex];
			FAUSTFLOAT* output1 = &output[1][fIndex];
			int task_list_size = 2;
			int task_list[2] = {2,3};
			taskqueue.InitTaskList(task_list_size, task_list, fDynamicNumThreads, cur_thread, tasknum);
			while (!fIsFinished) {
				switch (tasknum) {
					case WORK_STEALING_INDEX: { 
						tasknum = TaskQueue::GetNextTask(cur_thread, fDynamicNumThreads);
						break;
					} 
					case LAST_TASK_INDEX: { 
						fIsFinished = true;
						break;
					} 
					// SECTION : 1
					case 2: { 
						// LOOP 0x3174ca0
						// exec code
						for (int i=0; i<count; i++) {
							fZec0[i] = (1.2L * (quad)input0[i]);
						}
						
						taskqueue.PushHead(5);
						taskqueue.PushHead(6);
						taskqueue.PushHead(7);
						taskqueue.PushHead(8);
						tasknum = 4;
						break;
					} 
					case 3: { 
						// LOOP 0x317b620
						// exec code
						for (int i=0; i<count; i++) {
							fZec1[i] = (1.2L * (quad)input1[i]);
						}
						
						taskqueue.PushHead(10);
						taskqueue.PushHead(11);
						taskqueue.PushHead(12);
						taskqueue.PushHead(13);
						tasknum = 9;
						break;
					} 
					// SECTION : 2
					case 4: { 
						// LOOP 0x3174860
						// pre processing
						fRec0_idx = (fRec0_idx+fRec0_idx_save)&511;
						// exec code
						for (int i=0; i<count; i++) {
							fRec0[(fRec0_idx+i)&511] = ((fSlow0 * fRec0[(fRec0_idx+i-302)&511]) + fZec0[i]);
						}
						// post processing
						fRec0_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 14, tasknum);
						break;
					} 
					case 5: { 
						// LOOP 0x3176b10
						// pre processing
						fRec1_idx = (fRec1_idx+fRec1_idx_save)&2047;
						// exec code
						for (int i=0; i<count; i++) {
							fRec1[(fRec1_idx+i)&2047] = (fZec0[i] + (fSlow0 * fRec1[(fRec1_idx+i-1026)&2047]));
						}
						// post processing
						fRec1_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 14, tasknum);
						break;
					} 
					case 6: { 
						// LOOP 0x3177870
						// pre processing
						fRec2_idx = (fRec2_idx+fRec2_idx_save)&2047;
						// exec code
						for (int i=0; i<count; i++) {
							fRec2[(fRec2_idx+i)&2047] = (fZec0[i] + (fSlow0 * fRec2[(fRec2_idx+i-1724)&2047]));
						}
						// post processing
						fRec2_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 14, tasknum);
						break;
					} 
					case 7: { 
						// LOOP 0x3178690
						// pre processing
						fRec3_idx = (fRec3_idx+fRec3_idx_save)&4095;
						// exec code
						for (int i=0; i<count; i++) {
							fRec3[(fRec3_idx+i)&4095] = (fZec0[i] + (fSlow0 * fRec3[(fRec3_idx+i-2914)&4095]));
						}
						// post processing
						fRec3_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 14, tasknum);
						break;
					} 
					case 8: { 
						// LOOP 0x31794b0
						// pre processing
						fRec4_idx = (fRec4_idx+fRec4_idx_save)&4095;
						// exec code
						for (int i=0; i<count; i++) {
							fRec4[(fRec4_idx+i)&4095] = (fZec0[i] + (fSlow0 * fRec4[(fRec4_idx+i-2601)&4095]));
						}
						// post processing
						fRec4_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 14, tasknum);
						break;
					} 
					case 9: { 
						// LOOP 0x317b360
						// pre processing
						fRec5_idx = (fRec5_idx+fRec5_idx_save)&511;
						// exec code
						for (int i=0; i<count; i++) {
							fRec5[(fRec5_idx+i)&511] = ((fSlow0 * fRec5[(fRec5_idx+i-462)&511]) + fZec1[i]);
						}
						// post processing
						fRec5_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 15, tasknum);
						break;
					} 
					case 10: { 
						// LOOP 0x317c7e0
						// pre processing
						fRec6_idx = (fRec6_idx+fRec6_idx_save)&2047;
						// exec code
						for (int i=0; i<count; i++) {
							fRec6[(fRec6_idx+i)&2047] = (fZec1[i] + (fSlow0 * fRec6[(fRec6_idx+i-1318)&2047]));
						}
						// post processing
						fRec6_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 15, tasknum);
						break;
					} 
					case 11: { 
						// LOOP 0x317d620
						// pre processing
						fRec7_idx = (fRec7_idx+fRec7_idx_save)&4095;
						// exec code
						for (int i=0; i<count; i++) {
							fRec7[(fRec7_idx+i)&4095] = (fZec1[i] + (fSlow0 * fRec7[(fRec7_idx+i-2318)&4095]));
						}
						// post processing
						fRec7_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 15, tasknum);
						break;
					} 
					case 12: { 
						// LOOP 0x317e440
						// pre processing
						fRec8_idx = (fRec8_idx+fRec8_idx_save)&1023;
						// exec code
						for (int i=0; i<count; i++) {
							fRec8[(fRec8_idx+i)&1023] = (fZec1[i] + (fSlow0 * fRec8[(fRec8_idx+i-507)&1023]));
						}
						// post processing
						fRec8_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 15, tasknum);
						break;
					} 
					case 13: { 
						// LOOP 0x317f250
						// pre processing
						fRec9_idx = (fRec9_idx+fRec9_idx_save)&4095;
						// exec code
						for (int i=0; i<count; i++) {
							fRec9[(fRec9_idx+i)&4095] = (fZec1[i] + (fSlow0 * fRec9[(fRec9_idx+i-3132)&4095]));
						}
						// post processing
						fRec9_idx_save = count;
						
						fGraph.ActivateOneOutputTask(taskqueue, 15, tasknum);
						break;
					} 
					case 14: { 
						// LOOP 0x3174620
						// exec code
						for (int i=0; i<count; i++) {
							output0[i] = (FAUSTFLOAT)((((fRec0[(fRec0_idx+i)&511] + fRec1[(fRec1_idx+i)&2047]) + fRec2[(fRec2_idx+i)&2047]) + fRec3[(fRec3_idx+i)&4095]) + fRec4[(fRec4_idx+i)&4095]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 15: { 
						// LOOP 0x317b120
						// exec code
						for (int i=0; i<count; i++) {
							output1[i] = (FAUSTFLOAT)((((fRec5[(fRec5_idx+i)&511] + fRec6[(fRec6_idx+i)&2047]) + fRec7[(fRec7_idx+i)&4095]) + fRec8[(fRec8_idx+i)&1023]) + fRec9[(fRec9_idx+i)&4095]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
				}
			}
		}
	}
};



/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/
					
mydsp*	DSP;

std::list<GUI*>               GUI::fGuiList;

//-------------------------------------------------------------------------
// 									MAIN
//-------------------------------------------------------------------------
int main(int argc, char *argv[] )
{
	char* appname = basename (argv [0]);
    char  rcfilename[256];
	char* home = getenv("HOME");
	snprintf(rcfilename, 255, "%s/.%src", home, appname);
	
	DSP = new mydsp();
	if (DSP==0) {
        std::cerr << "Unable to allocate Faust DSP object" << std::endl;
		exit(1);
	}

	CMDUI* interface = new CMDUI(argc, argv);
	FUI* finterface	= new FUI();
	DSP->buildUserInterface(interface);
	DSP->buildUserInterface(finterface);

#ifdef HTTPCTRL
	httpdUI* httpdinterface = new httpdUI(appname, DSP->getNumInputs(), DSP->getNumOutputs(), argc, argv);
	DSP->buildUserInterface(httpdinterface);
    std::cout << "HTTPD is on" << std::endl;
#endif

#ifdef OSCCTRL
	GUI* oscinterface = new OSCUI(appname, argc, argv);
	DSP->buildUserInterface(oscinterface);
#endif

	alsaaudio audio (argc, argv, DSP);
	audio.init(appname, DSP);
	finterface->recallState(rcfilename);	
	audio.start();
	
#ifdef HTTPCTRL
	httpdinterface->run();
#endif
	
#ifdef OSCCTRL
	oscinterface->run();
#endif
	interface->run();
	
	audio.stop();
	finterface->saveState(rcfilename);
    
    // desallocation
    delete interface;
    delete finterface;
#ifdef HTTPCTRL
	 delete httpdinterface;
#endif
#ifdef OSCCTRL
	 delete oscinterface;
#endif

  	return 0;
}
/********************END ARCHITECTURE SECTION (part 2/2)****************/

