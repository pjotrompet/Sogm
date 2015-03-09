//-----------------------------------------------------
//
// Code generated with Faust 0.9.71 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <math.h>
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
#include "faust/gui/faustqt.h"
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
	quad 	fYec0[64];
	int 	fYec0_idx;
	int 	fYec0_idx_save;
	FAUSTFLOAT 	fslider0;
	quad 	fYec1[128];
	int 	fYec1_idx;
	int 	fYec1_idx_save;
	quad 	fZec0[32];
	quad 	fYec2[256];
	int 	fYec2_idx;
	int 	fYec2_idx_save;
	quad 	fZec1[32];
	quad 	fYec3[512];
	int 	fYec3_idx;
	int 	fYec3_idx_save;
	quad 	fZec2[32];
	quad 	fYec4[1024];
	int 	fYec4_idx;
	int 	fYec4_idx_save;
	quad 	fZec3[32];
	quad 	fYec5[2048];
	int 	fYec5_idx;
	int 	fYec5_idx_save;
	quad 	fZec4[32];
	quad 	fYec6[4096];
	int 	fYec6_idx;
	int 	fYec6_idx_save;
	quad 	fZec5[32];
	quad 	fYec7[8192];
	int 	fYec7_idx;
	int 	fYec7_idx_save;
	quad 	fZec6[32];
	quad 	fYec8[8192];
	int 	fYec8_idx;
	int 	fYec8_idx_save;
	quad 	fZec7[32];
	quad 	fYec9[16384];
	int 	fYec9_idx;
	int 	fYec9_idx_save;
	quad 	fZec8[32];
	quad 	fYec10[16384];
	int 	fYec10_idx;
	int 	fYec10_idx_save;
	quad 	fZec9[32];
	quad 	fYec11[32768];
	int 	fYec11_idx;
	int 	fYec11_idx_save;
	quad 	fZec10[32];
	quad 	fZec11[32];
	quad 	fZec12[32];
	int 	iConst0;
	quad 	fConst1;
	int 	iConst2;
	int 	iConst3;
	FAUSTFLOAT 	fslider1;
	quad 	fYec12[32768];
	int 	fYec12_idx;
	int 	fYec12_idx_save;
	quad 	fZec13[32];
	quad 	fYec13[16777216];
	int 	fYec13_idx;
	int 	fYec13_idx_save;
	quad 	fRec0[16777216];
	int 	fRec0_idx;
	int 	fRec0_idx_save;
	quad 	fYec14[65536];
	int 	fYec14_idx;
	int 	fYec14_idx_save;
	quad 	fZec14[32];
	int 	iConst4;
	int 	iConst5;
	quad 	fYec15[16777216];
	int 	fYec15_idx;
	int 	fYec15_idx_save;
	quad 	fRec1[16777216];
	int 	fRec1_idx;
	int 	fRec1_idx_save;
	int 	iConst6;
	int 	iConst7;
	quad 	fZec15[32];
	quad 	fYec16[16777216];
	int 	fYec16_idx;
	int 	fYec16_idx_save;
	quad 	fRec2[16777216];
	int 	fRec2_idx;
	int 	fRec2_idx_save;
	int 	iConst8;
	int 	iConst9;
	quad 	fYec17[16777216];
	int 	fYec17_idx;
	int 	fYec17_idx_save;
	quad 	fRec3[16777216];
	int 	fRec3_idx;
	int 	fRec3_idx_save;
	int 	iConst10;
	int 	iConst11;
	quad 	fYec18[16777216];
	int 	fYec18_idx;
	int 	fYec18_idx_save;
	quad 	fRec4[16777216];
	int 	fRec4_idx;
	int 	fRec4_idx_save;
	int 	iConst12;
	int 	iConst13;
	quad 	fYec19[16777216];
	int 	fYec19_idx;
	int 	fYec19_idx_save;
	quad 	fRec5[16777216];
	int 	fRec5_idx;
	int 	fRec5_idx_save;
	int 	iConst14;
	int 	iConst15;
	quad 	fYec20[16777216];
	int 	fYec20_idx;
	int 	fYec20_idx_save;
	quad 	fRec6[16777216];
	int 	fRec6_idx;
	int 	fRec6_idx_save;
	int 	iConst16;
	int 	iConst17;
	quad 	fYec21[16777216];
	int 	fYec21_idx;
	int 	fYec21_idx_save;
	quad 	fRec7[16777216];
	int 	fRec7_idx;
	int 	fRec7_idx_save;
	quad 	fConst18;
	int 	iConst19;
	int 	iConst20;
	quad 	fYec22[33554432];
	int 	fYec22_idx;
	int 	fYec22_idx_save;
	quad 	fRec8[33554432];
	int 	fRec8_idx;
	int 	fRec8_idx_save;
	int 	iConst21;
	int 	iConst22;
	quad 	fYec23[33554432];
	int 	fYec23_idx;
	int 	fYec23_idx_save;
	quad 	fRec9[33554432];
	int 	fRec9_idx;
	int 	fRec9_idx_save;
	int 	iConst23;
	int 	iConst24;
	quad 	fYec24[33554432];
	int 	fYec24_idx;
	int 	fYec24_idx_save;
	quad 	fRec10[33554432];
	int 	fRec10_idx;
	int 	fRec10_idx_save;
	int 	iConst25;
	int 	iConst26;
	quad 	fYec25[33554432];
	int 	fYec25_idx;
	int 	fYec25_idx_save;
	quad 	fRec11[33554432];
	int 	fRec11_idx;
	int 	fRec11_idx_save;
	int 	iConst27;
	int 	iConst28;
	quad 	fYec26[33554432];
	int 	fYec26_idx;
	int 	fYec26_idx_save;
	quad 	fRec12[33554432];
	int 	fRec12_idx;
	int 	fRec12_idx_save;
	int 	iConst29;
	int 	iConst30;
	quad 	fYec27[33554432];
	int 	fYec27_idx;
	int 	fYec27_idx_save;
	quad 	fRec13[33554432];
	int 	fRec13_idx;
	int 	fRec13_idx_save;
	int 	iConst31;
	int 	iConst32;
	quad 	fYec28[33554432];
	int 	fYec28_idx;
	int 	fYec28_idx_save;
	quad 	fRec14[33554432];
	int 	fRec14_idx;
	int 	fRec14_idx_save;
	int 	iConst33;
	int 	iConst34;
	quad 	fYec29[33554432];
	int 	fYec29_idx;
	int 	fYec29_idx_save;
	quad 	fRec15[33554432];
	int 	fRec15_idx;
	int 	fRec15_idx_save;
	quad 	fConst35;
	int 	iConst36;
	int 	iConst37;
	quad 	fYec30[67108864];
	int 	fYec30_idx;
	int 	fYec30_idx_save;
	quad 	fRec16[67108864];
	int 	fRec16_idx;
	int 	fRec16_idx_save;
	int 	iConst38;
	int 	iConst39;
	quad 	fYec31[67108864];
	int 	fYec31_idx;
	int 	fYec31_idx_save;
	quad 	fRec17[67108864];
	int 	fRec17_idx;
	int 	fRec17_idx_save;
	int 	iConst40;
	int 	iConst41;
	quad 	fYec32[67108864];
	int 	fYec32_idx;
	int 	fYec32_idx_save;
	quad 	fRec18[67108864];
	int 	fRec18_idx;
	int 	fRec18_idx_save;
	int 	iConst42;
	int 	iConst43;
	quad 	fYec33[67108864];
	int 	fYec33_idx;
	int 	fYec33_idx_save;
	quad 	fRec19[67108864];
	int 	fRec19_idx;
	int 	fRec19_idx_save;
	int 	iConst44;
	int 	iConst45;
	quad 	fYec34[67108864];
	int 	fYec34_idx;
	int 	fYec34_idx_save;
	quad 	fRec20[67108864];
	int 	fRec20_idx;
	int 	fRec20_idx_save;
	int 	iConst46;
	int 	iConst47;
	quad 	fYec35[67108864];
	int 	fYec35_idx;
	int 	fYec35_idx_save;
	quad 	fRec21[67108864];
	int 	fRec21_idx;
	int 	fRec21_idx_save;
	int 	iConst48;
	int 	iConst49;
	quad 	fYec36[67108864];
	int 	fYec36_idx;
	int 	fYec36_idx_save;
	quad 	fRec22[67108864];
	int 	fRec22_idx;
	int 	fRec22_idx_save;
	int 	iConst50;
	int 	iConst51;
	quad 	fYec37[67108864];
	int 	fYec37_idx;
	int 	fYec37_idx_save;
	quad 	fRec23[67108864];
	int 	fRec23_idx;
	int 	fRec23_idx_save;
	quad 	fConst52;
	int 	iConst53;
	int 	iConst54;
	quad 	fYec38[67108864];
	int 	fYec38_idx;
	int 	fYec38_idx_save;
	quad 	fRec24[67108864];
	int 	fRec24_idx;
	int 	fRec24_idx_save;
	int 	iConst55;
	int 	iConst56;
	quad 	fYec39[67108864];
	int 	fYec39_idx;
	int 	fYec39_idx_save;
	quad 	fRec25[67108864];
	int 	fRec25_idx;
	int 	fRec25_idx_save;
	int 	iConst57;
	int 	iConst58;
	quad 	fYec40[67108864];
	int 	fYec40_idx;
	int 	fYec40_idx_save;
	quad 	fRec26[67108864];
	int 	fRec26_idx;
	int 	fRec26_idx_save;
	int 	iConst59;
	int 	iConst60;
	quad 	fYec41[67108864];
	int 	fYec41_idx;
	int 	fYec41_idx_save;
	quad 	fRec27[67108864];
	int 	fRec27_idx;
	int 	fRec27_idx_save;
	int 	iConst61;
	int 	iConst62;
	quad 	fYec42[67108864];
	int 	fYec42_idx;
	int 	fYec42_idx_save;
	quad 	fRec28[67108864];
	int 	fRec28_idx;
	int 	fRec28_idx_save;
	int 	iConst63;
	int 	iConst64;
	quad 	fYec43[67108864];
	int 	fYec43_idx;
	int 	fYec43_idx_save;
	quad 	fRec29[67108864];
	int 	fRec29_idx;
	int 	fRec29_idx_save;
	int 	iConst65;
	int 	iConst66;
	quad 	fYec44[67108864];
	int 	fYec44_idx;
	int 	fYec44_idx_save;
	quad 	fRec30[67108864];
	int 	fRec30_idx;
	int 	fRec30_idx_save;
	int 	iConst67;
	int 	iConst68;
	quad 	fYec45[67108864];
	int 	fYec45_idx;
	int 	fYec45_idx_save;
	quad 	fRec31[67108864];
	int 	fRec31_idx;
	int 	fRec31_idx_save;
	quad 	fConst69;
	int 	iConst70;
	int 	iConst71;
	quad 	fYec46[134217728];
	int 	fYec46_idx;
	int 	fYec46_idx_save;
	quad 	fRec32[134217728];
	int 	fRec32_idx;
	int 	fRec32_idx_save;
	int 	iConst72;
	int 	iConst73;
	quad 	fYec47[134217728];
	int 	fYec47_idx;
	int 	fYec47_idx_save;
	quad 	fRec33[134217728];
	int 	fRec33_idx;
	int 	fRec33_idx_save;
	int 	iConst74;
	int 	iConst75;
	quad 	fYec48[134217728];
	int 	fYec48_idx;
	int 	fYec48_idx_save;
	quad 	fRec34[134217728];
	int 	fRec34_idx;
	int 	fRec34_idx_save;
	int 	iConst76;
	int 	iConst77;
	quad 	fYec49[134217728];
	int 	fYec49_idx;
	int 	fYec49_idx_save;
	quad 	fRec35[134217728];
	int 	fRec35_idx;
	int 	fRec35_idx_save;
	int 	iConst78;
	int 	iConst79;
	quad 	fYec50[134217728];
	int 	fYec50_idx;
	int 	fYec50_idx_save;
	quad 	fRec36[134217728];
	int 	fRec36_idx;
	int 	fRec36_idx_save;
	int 	iConst80;
	int 	iConst81;
	quad 	fYec51[134217728];
	int 	fYec51_idx;
	int 	fYec51_idx_save;
	quad 	fRec37[134217728];
	int 	fRec37_idx;
	int 	fRec37_idx_save;
	int 	iConst82;
	int 	iConst83;
	quad 	fYec52[134217728];
	int 	fYec52_idx;
	int 	fYec52_idx_save;
	quad 	fRec38[134217728];
	int 	fRec38_idx;
	int 	fRec38_idx_save;
	int 	iConst84;
	int 	iConst85;
	quad 	fYec53[134217728];
	int 	fYec53_idx;
	int 	fYec53_idx_save;
	quad 	fRec39[134217728];
	int 	fRec39_idx;
	int 	fRec39_idx_save;
	quad 	fConst86;
	int 	iConst87;
	int 	iConst88;
	quad 	fYec54[134217728];
	int 	fYec54_idx;
	int 	fYec54_idx_save;
	quad 	fRec40[134217728];
	int 	fRec40_idx;
	int 	fRec40_idx_save;
	int 	iConst89;
	int 	iConst90;
	quad 	fYec55[134217728];
	int 	fYec55_idx;
	int 	fYec55_idx_save;
	quad 	fRec41[134217728];
	int 	fRec41_idx;
	int 	fRec41_idx_save;
	int 	iConst91;
	int 	iConst92;
	quad 	fYec56[134217728];
	int 	fYec56_idx;
	int 	fYec56_idx_save;
	quad 	fRec42[134217728];
	int 	fRec42_idx;
	int 	fRec42_idx_save;
	int 	iConst93;
	int 	iConst94;
	quad 	fYec57[134217728];
	int 	fYec57_idx;
	int 	fYec57_idx_save;
	quad 	fRec43[134217728];
	int 	fRec43_idx;
	int 	fRec43_idx_save;
	int 	iConst95;
	int 	iConst96;
	quad 	fYec58[134217728];
	int 	fYec58_idx;
	int 	fYec58_idx_save;
	quad 	fRec44[134217728];
	int 	fRec44_idx;
	int 	fRec44_idx_save;
	int 	iConst97;
	int 	iConst98;
	quad 	fYec59[134217728];
	int 	fYec59_idx;
	int 	fYec59_idx_save;
	quad 	fRec45[134217728];
	int 	fRec45_idx;
	int 	fRec45_idx_save;
	int 	iConst99;
	int 	iConst100;
	quad 	fYec60[134217728];
	int 	fYec60_idx;
	int 	fYec60_idx_save;
	quad 	fRec46[134217728];
	int 	fRec46_idx;
	int 	fRec46_idx_save;
	int 	iConst101;
	int 	iConst102;
	quad 	fYec61[134217728];
	int 	fYec61_idx;
	int 	fYec61_idx_save;
	quad 	fRec47[134217728];
	int 	fRec47_idx;
	int 	fRec47_idx_save;
	quad 	fConst103;
	int 	iConst104;
	int 	iConst105;
	quad 	fYec62[134217728];
	int 	fYec62_idx;
	int 	fYec62_idx_save;
	quad 	fRec48[134217728];
	int 	fRec48_idx;
	int 	fRec48_idx_save;
	int 	iConst106;
	int 	iConst107;
	quad 	fYec63[134217728];
	int 	fYec63_idx;
	int 	fYec63_idx_save;
	quad 	fRec49[134217728];
	int 	fRec49_idx;
	int 	fRec49_idx_save;
	int 	iConst108;
	int 	iConst109;
	quad 	fYec64[134217728];
	int 	fYec64_idx;
	int 	fYec64_idx_save;
	quad 	fRec50[134217728];
	int 	fRec50_idx;
	int 	fRec50_idx_save;
	int 	iConst110;
	int 	iConst111;
	quad 	fYec65[134217728];
	int 	fYec65_idx;
	int 	fYec65_idx_save;
	quad 	fRec51[134217728];
	int 	fRec51_idx;
	int 	fRec51_idx_save;
	int 	iConst112;
	int 	iConst113;
	quad 	fYec66[134217728];
	int 	fYec66_idx;
	int 	fYec66_idx_save;
	quad 	fRec52[134217728];
	int 	fRec52_idx;
	int 	fRec52_idx_save;
	int 	iConst114;
	int 	iConst115;
	quad 	fYec67[134217728];
	int 	fYec67_idx;
	int 	fYec67_idx_save;
	quad 	fRec53[134217728];
	int 	fRec53_idx;
	int 	fRec53_idx_save;
	int 	iConst116;
	int 	iConst117;
	quad 	fYec68[134217728];
	int 	fYec68_idx;
	int 	fYec68_idx_save;
	quad 	fRec54[134217728];
	int 	fRec54_idx;
	int 	fRec54_idx_save;
	int 	iConst118;
	int 	iConst119;
	quad 	fYec69[134217728];
	int 	fYec69_idx;
	int 	fYec69_idx_save;
	quad 	fRec55[134217728];
	int 	fRec55_idx;
	int 	fRec55_idx_save;
	quad 	fConst120;
	int 	iConst121;
	int 	iConst122;
	quad 	fYec70[33554432];
	int 	fYec70_idx;
	int 	fYec70_idx_save;
	quad 	fRec56[33554432];
	int 	fRec56_idx;
	int 	fRec56_idx_save;
	int 	iConst123;
	int 	iConst124;
	quad 	fYec71[33554432];
	int 	fYec71_idx;
	int 	fYec71_idx_save;
	quad 	fRec57[33554432];
	int 	fRec57_idx;
	int 	fRec57_idx_save;
	int 	iConst125;
	int 	iConst126;
	quad 	fYec72[33554432];
	int 	fYec72_idx;
	int 	fYec72_idx_save;
	quad 	fRec58[33554432];
	int 	fRec58_idx;
	int 	fRec58_idx_save;
	int 	iConst127;
	int 	iConst128;
	quad 	fYec73[33554432];
	int 	fYec73_idx;
	int 	fYec73_idx_save;
	quad 	fRec59[33554432];
	int 	fRec59_idx;
	int 	fRec59_idx_save;
	int 	iConst129;
	int 	iConst130;
	quad 	fYec74[33554432];
	int 	fYec74_idx;
	int 	fYec74_idx_save;
	quad 	fRec60[33554432];
	int 	fRec60_idx;
	int 	fRec60_idx_save;
	int 	iConst131;
	int 	iConst132;
	quad 	fYec75[33554432];
	int 	fYec75_idx;
	int 	fYec75_idx_save;
	quad 	fRec61[33554432];
	int 	fRec61_idx;
	int 	fRec61_idx_save;
	int 	iConst133;
	int 	iConst134;
	quad 	fYec76[33554432];
	int 	fYec76_idx;
	int 	fYec76_idx_save;
	quad 	fRec62[33554432];
	int 	fRec62_idx;
	int 	fRec62_idx_save;
	int 	iConst135;
	int 	iConst136;
	quad 	fYec77[33554432];
	int 	fYec77_idx;
	int 	fYec77_idx_save;
	quad 	fRec63[33554432];
	int 	fRec63_idx;
	int 	fRec63_idx_save;
	quad 	fConst137;
	int 	iConst138;
	int 	iConst139;
	quad 	fYec78[134217728];
	int 	fYec78_idx;
	int 	fYec78_idx_save;
	quad 	fRec64[134217728];
	int 	fRec64_idx;
	int 	fRec64_idx_save;
	int 	iConst140;
	int 	iConst141;
	quad 	fYec79[134217728];
	int 	fYec79_idx;
	int 	fYec79_idx_save;
	quad 	fRec65[134217728];
	int 	fRec65_idx;
	int 	fRec65_idx_save;
	int 	iConst142;
	int 	iConst143;
	quad 	fYec80[134217728];
	int 	fYec80_idx;
	int 	fYec80_idx_save;
	quad 	fRec66[134217728];
	int 	fRec66_idx;
	int 	fRec66_idx_save;
	int 	iConst144;
	int 	iConst145;
	quad 	fYec81[134217728];
	int 	fYec81_idx;
	int 	fYec81_idx_save;
	quad 	fRec67[134217728];
	int 	fRec67_idx;
	int 	fRec67_idx_save;
	int 	iConst146;
	int 	iConst147;
	quad 	fYec82[134217728];
	int 	fYec82_idx;
	int 	fYec82_idx_save;
	quad 	fRec68[134217728];
	int 	fRec68_idx;
	int 	fRec68_idx_save;
	int 	iConst148;
	int 	iConst149;
	quad 	fYec83[134217728];
	int 	fYec83_idx;
	int 	fYec83_idx_save;
	quad 	fRec69[134217728];
	int 	fRec69_idx;
	int 	fRec69_idx_save;
	int 	iConst150;
	int 	iConst151;
	quad 	fYec84[134217728];
	int 	fYec84_idx;
	int 	fYec84_idx_save;
	quad 	fRec70[134217728];
	int 	fRec70_idx;
	int 	fRec70_idx_save;
	int 	iConst152;
	int 	iConst153;
	quad 	fYec85[134217728];
	int 	fYec85_idx;
	int 	fYec85_idx_save;
	quad 	fRec71[134217728];
	int 	fRec71_idx;
	int 	fRec71_idx_save;
	quad 	fConst154;
	int 	iConst155;
	int 	iConst156;
	quad 	fYec86[268435456];
	int 	fYec86_idx;
	int 	fYec86_idx_save;
	quad 	fRec72[268435456];
	int 	fRec72_idx;
	int 	fRec72_idx_save;
	int 	iConst157;
	int 	iConst158;
	quad 	fYec87[268435456];
	int 	fYec87_idx;
	int 	fYec87_idx_save;
	quad 	fRec73[268435456];
	int 	fRec73_idx;
	int 	fRec73_idx_save;
	int 	iConst159;
	int 	iConst160;
	quad 	fYec88[268435456];
	int 	fYec88_idx;
	int 	fYec88_idx_save;
	quad 	fRec74[268435456];
	int 	fRec74_idx;
	int 	fRec74_idx_save;
	int 	iConst161;
	int 	iConst162;
	quad 	fYec89[268435456];
	int 	fYec89_idx;
	int 	fYec89_idx_save;
	quad 	fRec75[268435456];
	int 	fRec75_idx;
	int 	fRec75_idx_save;
	int 	iConst163;
	int 	iConst164;
	quad 	fYec90[268435456];
	int 	fYec90_idx;
	int 	fYec90_idx_save;
	quad 	fRec76[268435456];
	int 	fRec76_idx;
	int 	fRec76_idx_save;
	int 	iConst165;
	int 	iConst166;
	quad 	fYec91[268435456];
	int 	fYec91_idx;
	int 	fYec91_idx_save;
	quad 	fRec77[268435456];
	int 	fRec77_idx;
	int 	fRec77_idx_save;
	int 	iConst167;
	int 	iConst168;
	quad 	fYec92[268435456];
	int 	fYec92_idx;
	int 	fYec92_idx_save;
	quad 	fRec78[268435456];
	int 	fRec78_idx;
	int 	fRec78_idx_save;
	int 	iConst169;
	int 	iConst170;
	quad 	fYec93[268435456];
	int 	fYec93_idx;
	int 	fYec93_idx_save;
	quad 	fRec79[268435456];
	int 	fRec79_idx;
	int 	fRec79_idx_save;
	quad 	fConst171;
	int 	iConst172;
	int 	iConst173;
	quad 	fYec94[8388608];
	int 	fYec94_idx;
	int 	fYec94_idx_save;
	quad 	fRec80[8388608];
	int 	fRec80_idx;
	int 	fRec80_idx_save;
	int 	iConst174;
	int 	iConst175;
	quad 	fYec95[8388608];
	int 	fYec95_idx;
	int 	fYec95_idx_save;
	quad 	fRec81[8388608];
	int 	fRec81_idx;
	int 	fRec81_idx_save;
	int 	iConst176;
	int 	iConst177;
	quad 	fYec96[8388608];
	int 	fYec96_idx;
	int 	fYec96_idx_save;
	quad 	fRec82[8388608];
	int 	fRec82_idx;
	int 	fRec82_idx_save;
	int 	iConst178;
	int 	iConst179;
	quad 	fYec97[8388608];
	int 	fYec97_idx;
	int 	fYec97_idx_save;
	quad 	fRec83[8388608];
	int 	fRec83_idx;
	int 	fRec83_idx_save;
	int 	iConst180;
	int 	iConst181;
	quad 	fYec98[8388608];
	int 	fYec98_idx;
	int 	fYec98_idx_save;
	quad 	fRec84[8388608];
	int 	fRec84_idx;
	int 	fRec84_idx_save;
	int 	iConst182;
	int 	iConst183;
	quad 	fYec99[8388608];
	int 	fYec99_idx;
	int 	fYec99_idx_save;
	quad 	fRec85[8388608];
	int 	fRec85_idx;
	int 	fRec85_idx_save;
	int 	iConst184;
	int 	iConst185;
	quad 	fYec100[8388608];
	int 	fYec100_idx;
	int 	fYec100_idx_save;
	quad 	fRec86[8388608];
	int 	fRec86_idx;
	int 	fRec86_idx_save;
	int 	iConst186;
	int 	iConst187;
	quad 	fYec101[8388608];
	int 	fYec101_idx;
	int 	fYec101_idx_save;
	quad 	fRec87[8388608];
	int 	fRec87_idx;
	int 	fRec87_idx_save;
	quad 	fConst188;
	int 	iConst189;
	int 	iConst190;
	quad 	fYec102[33554432];
	int 	fYec102_idx;
	int 	fYec102_idx_save;
	quad 	fRec88[33554432];
	int 	fRec88_idx;
	int 	fRec88_idx_save;
	int 	iConst191;
	int 	iConst192;
	quad 	fYec103[33554432];
	int 	fYec103_idx;
	int 	fYec103_idx_save;
	quad 	fRec89[33554432];
	int 	fRec89_idx;
	int 	fRec89_idx_save;
	int 	iConst193;
	int 	iConst194;
	quad 	fYec104[33554432];
	int 	fYec104_idx;
	int 	fYec104_idx_save;
	quad 	fRec90[33554432];
	int 	fRec90_idx;
	int 	fRec90_idx_save;
	int 	iConst195;
	int 	iConst196;
	quad 	fYec105[33554432];
	int 	fYec105_idx;
	int 	fYec105_idx_save;
	quad 	fRec91[33554432];
	int 	fRec91_idx;
	int 	fRec91_idx_save;
	int 	iConst197;
	int 	iConst198;
	quad 	fYec106[33554432];
	int 	fYec106_idx;
	int 	fYec106_idx_save;
	quad 	fRec92[33554432];
	int 	fRec92_idx;
	int 	fRec92_idx_save;
	int 	iConst199;
	int 	iConst200;
	quad 	fYec107[33554432];
	int 	fYec107_idx;
	int 	fYec107_idx_save;
	quad 	fRec93[33554432];
	int 	fRec93_idx;
	int 	fRec93_idx_save;
	int 	iConst201;
	int 	iConst202;
	quad 	fYec108[33554432];
	int 	fYec108_idx;
	int 	fYec108_idx_save;
	quad 	fRec94[33554432];
	int 	fRec94_idx;
	int 	fRec94_idx_save;
	int 	iConst203;
	int 	iConst204;
	quad 	fYec109[33554432];
	int 	fYec109_idx;
	int 	fYec109_idx_save;
	quad 	fRec95[33554432];
	int 	fRec95_idx;
	int 	fRec95_idx_save;
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
	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 8; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		for (int i=0; i<64; i++) fYec0[i]=0;
		fYec0_idx = 0;
		fYec0_idx_save = 0;
		fslider0 = 0.6L;
		for (int i=0; i<128; i++) fYec1[i]=0;
		fYec1_idx = 0;
		fYec1_idx_save = 0;
		for (int i=0; i<256; i++) fYec2[i]=0;
		fYec2_idx = 0;
		fYec2_idx_save = 0;
		for (int i=0; i<512; i++) fYec3[i]=0;
		fYec3_idx = 0;
		fYec3_idx_save = 0;
		for (int i=0; i<1024; i++) fYec4[i]=0;
		fYec4_idx = 0;
		fYec4_idx_save = 0;
		for (int i=0; i<2048; i++) fYec5[i]=0;
		fYec5_idx = 0;
		fYec5_idx_save = 0;
		for (int i=0; i<4096; i++) fYec6[i]=0;
		fYec6_idx = 0;
		fYec6_idx_save = 0;
		for (int i=0; i<8192; i++) fYec7[i]=0;
		fYec7_idx = 0;
		fYec7_idx_save = 0;
		for (int i=0; i<8192; i++) fYec8[i]=0;
		fYec8_idx = 0;
		fYec8_idx_save = 0;
		for (int i=0; i<16384; i++) fYec9[i]=0;
		fYec9_idx = 0;
		fYec9_idx_save = 0;
		for (int i=0; i<16384; i++) fYec10[i]=0;
		fYec10_idx = 0;
		fYec10_idx_save = 0;
		for (int i=0; i<32768; i++) fYec11[i]=0;
		fYec11_idx = 0;
		fYec11_idx_save = 0;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fConst1 = (quad(13274100) / quad(iConst0));
		iConst2 = int(fConst1);
		iConst3 = int((1 + iConst2));
		fslider1 = 0.81L;
		for (int i=0; i<32768; i++) fYec12[i]=0;
		fYec12_idx = 0;
		fYec12_idx_save = 0;
		for (int i=0; i<16777216; i++) fYec13[i]=0;
		fYec13_idx = 0;
		fYec13_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec0[i]=0;
		fRec0_idx = 0;
		fRec0_idx_save = 0;
		for (int i=0; i<65536; i++) fYec14[i]=0;
		fYec14_idx = 0;
		fYec14_idx_save = 0;
		iConst4 = int((10 + fConst1));
		iConst5 = int((1 + iConst4));
		for (int i=0; i<16777216; i++) fYec15[i]=0;
		fYec15_idx = 0;
		fYec15_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec1[i]=0;
		fRec1_idx = 0;
		fRec1_idx_save = 0;
		iConst6 = int((20 + fConst1));
		iConst7 = int((1 + iConst6));
		for (int i=0; i<16777216; i++) fYec16[i]=0;
		fYec16_idx = 0;
		fYec16_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec2[i]=0;
		fRec2_idx = 0;
		fRec2_idx_save = 0;
		iConst8 = int((fConst1 + 30));
		iConst9 = int((1 + iConst8));
		for (int i=0; i<16777216; i++) fYec17[i]=0;
		fYec17_idx = 0;
		fYec17_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec3[i]=0;
		fRec3_idx = 0;
		fRec3_idx_save = 0;
		iConst10 = int((40 + fConst1));
		iConst11 = int((1 + iConst10));
		for (int i=0; i<16777216; i++) fYec18[i]=0;
		fYec18_idx = 0;
		fYec18_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec4[i]=0;
		fRec4_idx = 0;
		fRec4_idx_save = 0;
		iConst12 = int((fConst1 + 50));
		iConst13 = int((1 + iConst12));
		for (int i=0; i<16777216; i++) fYec19[i]=0;
		fYec19_idx = 0;
		fYec19_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec5[i]=0;
		fRec5_idx = 0;
		fRec5_idx_save = 0;
		iConst14 = int((fConst1 + 60));
		iConst15 = int((1 + iConst14));
		for (int i=0; i<16777216; i++) fYec20[i]=0;
		fYec20_idx = 0;
		fYec20_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec6[i]=0;
		fRec6_idx = 0;
		fRec6_idx_save = 0;
		iConst16 = int((fConst1 + 70));
		iConst17 = int((1 + iConst16));
		for (int i=0; i<16777216; i++) fYec21[i]=0;
		fYec21_idx = 0;
		fYec21_idx_save = 0;
		for (int i=0; i<16777216; i++) fRec7[i]=0;
		fRec7_idx = 0;
		fRec7_idx_save = 0;
		fConst18 = (quad(20330100) / quad(iConst0));
		iConst19 = int(fConst18);
		iConst20 = int((1 + iConst19));
		for (int i=0; i<33554432; i++) fYec22[i]=0;
		fYec22_idx = 0;
		fYec22_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec8[i]=0;
		fRec8_idx = 0;
		fRec8_idx_save = 0;
		iConst21 = int((10 + fConst18));
		iConst22 = int((1 + iConst21));
		for (int i=0; i<33554432; i++) fYec23[i]=0;
		fYec23_idx = 0;
		fYec23_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec9[i]=0;
		fRec9_idx = 0;
		fRec9_idx_save = 0;
		iConst23 = int((20 + fConst18));
		iConst24 = int((1 + iConst23));
		for (int i=0; i<33554432; i++) fYec24[i]=0;
		fYec24_idx = 0;
		fYec24_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec10[i]=0;
		fRec10_idx = 0;
		fRec10_idx_save = 0;
		iConst25 = int((30 + fConst18));
		iConst26 = int((1 + iConst25));
		for (int i=0; i<33554432; i++) fYec25[i]=0;
		fYec25_idx = 0;
		fYec25_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec11[i]=0;
		fRec11_idx = 0;
		fRec11_idx_save = 0;
		iConst27 = int((40 + fConst18));
		iConst28 = int((1 + iConst27));
		for (int i=0; i<33554432; i++) fYec26[i]=0;
		fYec26_idx = 0;
		fYec26_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec12[i]=0;
		fRec12_idx = 0;
		fRec12_idx_save = 0;
		iConst29 = int((50 + fConst18));
		iConst30 = int((1 + iConst29));
		for (int i=0; i<33554432; i++) fYec27[i]=0;
		fYec27_idx = 0;
		fYec27_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec13[i]=0;
		fRec13_idx = 0;
		fRec13_idx_save = 0;
		iConst31 = int((60 + fConst18));
		iConst32 = int((1 + iConst31));
		for (int i=0; i<33554432; i++) fYec28[i]=0;
		fYec28_idx = 0;
		fYec28_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec14[i]=0;
		fRec14_idx = 0;
		fRec14_idx_save = 0;
		iConst33 = int((70 + fConst18));
		iConst34 = int((1 + iConst33));
		for (int i=0; i<33554432; i++) fYec29[i]=0;
		fYec29_idx = 0;
		fYec29_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec15[i]=0;
		fRec15_idx = 0;
		fRec15_idx_save = 0;
		fConst35 = (quad(45202500) / quad(iConst0));
		iConst36 = int(fConst35);
		iConst37 = int((1 + iConst36));
		for (int i=0; i<67108864; i++) fYec30[i]=0;
		fYec30_idx = 0;
		fYec30_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec16[i]=0;
		fRec16_idx = 0;
		fRec16_idx_save = 0;
		iConst38 = int((10 + fConst35));
		iConst39 = int((1 + iConst38));
		for (int i=0; i<67108864; i++) fYec31[i]=0;
		fYec31_idx = 0;
		fYec31_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec17[i]=0;
		fRec17_idx = 0;
		fRec17_idx_save = 0;
		iConst40 = int((20 + fConst35));
		iConst41 = int((1 + iConst40));
		for (int i=0; i<67108864; i++) fYec32[i]=0;
		fYec32_idx = 0;
		fYec32_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec18[i]=0;
		fRec18_idx = 0;
		fRec18_idx_save = 0;
		iConst42 = int((30 + fConst35));
		iConst43 = int((1 + iConst42));
		for (int i=0; i<67108864; i++) fYec33[i]=0;
		fYec33_idx = 0;
		fYec33_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec19[i]=0;
		fRec19_idx = 0;
		fRec19_idx_save = 0;
		iConst44 = int((40 + fConst35));
		iConst45 = int((1 + iConst44));
		for (int i=0; i<67108864; i++) fYec34[i]=0;
		fYec34_idx = 0;
		fYec34_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec20[i]=0;
		fRec20_idx = 0;
		fRec20_idx_save = 0;
		iConst46 = int((50 + fConst35));
		iConst47 = int((1 + iConst46));
		for (int i=0; i<67108864; i++) fYec35[i]=0;
		fYec35_idx = 0;
		fYec35_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec21[i]=0;
		fRec21_idx = 0;
		fRec21_idx_save = 0;
		iConst48 = int((60 + fConst35));
		iConst49 = int((1 + iConst48));
		for (int i=0; i<67108864; i++) fYec36[i]=0;
		fYec36_idx = 0;
		fYec36_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec22[i]=0;
		fRec22_idx = 0;
		fRec22_idx_save = 0;
		iConst50 = int((70 + fConst35));
		iConst51 = int((1 + iConst50));
		for (int i=0; i<67108864; i++) fYec37[i]=0;
		fYec37_idx = 0;
		fYec37_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec23[i]=0;
		fRec23_idx = 0;
		fRec23_idx_save = 0;
		fConst52 = (quad(58079700) / quad(iConst0));
		iConst53 = int(fConst52);
		iConst54 = int((1 + iConst53));
		for (int i=0; i<67108864; i++) fYec38[i]=0;
		fYec38_idx = 0;
		fYec38_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec24[i]=0;
		fRec24_idx = 0;
		fRec24_idx_save = 0;
		iConst55 = int((10 + fConst52));
		iConst56 = int((1 + iConst55));
		for (int i=0; i<67108864; i++) fYec39[i]=0;
		fYec39_idx = 0;
		fYec39_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec25[i]=0;
		fRec25_idx = 0;
		fRec25_idx_save = 0;
		iConst57 = int((20 + fConst52));
		iConst58 = int((1 + iConst57));
		for (int i=0; i<67108864; i++) fYec40[i]=0;
		fYec40_idx = 0;
		fYec40_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec26[i]=0;
		fRec26_idx = 0;
		fRec26_idx_save = 0;
		iConst59 = int((30 + fConst52));
		iConst60 = int((1 + iConst59));
		for (int i=0; i<67108864; i++) fYec41[i]=0;
		fYec41_idx = 0;
		fYec41_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec27[i]=0;
		fRec27_idx = 0;
		fRec27_idx_save = 0;
		iConst61 = int((40 + fConst52));
		iConst62 = int((1 + iConst61));
		for (int i=0; i<67108864; i++) fYec42[i]=0;
		fYec42_idx = 0;
		fYec42_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec28[i]=0;
		fRec28_idx = 0;
		fRec28_idx_save = 0;
		iConst63 = int((50 + fConst52));
		iConst64 = int((1 + iConst63));
		for (int i=0; i<67108864; i++) fYec43[i]=0;
		fYec43_idx = 0;
		fYec43_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec29[i]=0;
		fRec29_idx = 0;
		fRec29_idx_save = 0;
		iConst65 = int((60 + fConst52));
		iConst66 = int((1 + iConst65));
		for (int i=0; i<67108864; i++) fYec44[i]=0;
		fYec44_idx = 0;
		fYec44_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec30[i]=0;
		fRec30_idx = 0;
		fRec30_idx_save = 0;
		iConst67 = int((70 + fConst52));
		iConst68 = int((1 + iConst67));
		for (int i=0; i<67108864; i++) fYec45[i]=0;
		fYec45_idx = 0;
		fYec45_idx_save = 0;
		for (int i=0; i<67108864; i++) fRec31[i]=0;
		fRec31_idx = 0;
		fRec31_idx_save = 0;
		fConst69 = (quad(75984300) / quad(iConst0));
		iConst70 = int(fConst69);
		iConst71 = int((1 + iConst70));
		for (int i=0; i<134217728; i++) fYec46[i]=0;
		fYec46_idx = 0;
		fYec46_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec32[i]=0;
		fRec32_idx = 0;
		fRec32_idx_save = 0;
		iConst72 = int((10 + fConst69));
		iConst73 = int((1 + iConst72));
		for (int i=0; i<134217728; i++) fYec47[i]=0;
		fYec47_idx = 0;
		fYec47_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec33[i]=0;
		fRec33_idx = 0;
		fRec33_idx_save = 0;
		iConst74 = int((20 + fConst69));
		iConst75 = int((1 + iConst74));
		for (int i=0; i<134217728; i++) fYec48[i]=0;
		fYec48_idx = 0;
		fYec48_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec34[i]=0;
		fRec34_idx = 0;
		fRec34_idx_save = 0;
		iConst76 = int((30 + fConst69));
		iConst77 = int((1 + iConst76));
		for (int i=0; i<134217728; i++) fYec49[i]=0;
		fYec49_idx = 0;
		fYec49_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec35[i]=0;
		fRec35_idx = 0;
		fRec35_idx_save = 0;
		iConst78 = int((40 + fConst69));
		iConst79 = int((1 + iConst78));
		for (int i=0; i<134217728; i++) fYec50[i]=0;
		fYec50_idx = 0;
		fYec50_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec36[i]=0;
		fRec36_idx = 0;
		fRec36_idx_save = 0;
		iConst80 = int((50 + fConst69));
		iConst81 = int((1 + iConst80));
		for (int i=0; i<134217728; i++) fYec51[i]=0;
		fYec51_idx = 0;
		fYec51_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec37[i]=0;
		fRec37_idx = 0;
		fRec37_idx_save = 0;
		iConst82 = int((60 + fConst69));
		iConst83 = int((1 + iConst82));
		for (int i=0; i<134217728; i++) fYec52[i]=0;
		fYec52_idx = 0;
		fYec52_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec38[i]=0;
		fRec38_idx = 0;
		fRec38_idx_save = 0;
		iConst84 = int((70 + fConst69));
		iConst85 = int((1 + iConst84));
		for (int i=0; i<134217728; i++) fYec53[i]=0;
		fYec53_idx = 0;
		fYec53_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec39[i]=0;
		fRec39_idx = 0;
		fRec39_idx_save = 0;
		fConst86 = (quad(102179700) / quad(iConst0));
		iConst87 = int(fConst86);
		iConst88 = int((1 + iConst87));
		for (int i=0; i<134217728; i++) fYec54[i]=0;
		fYec54_idx = 0;
		fYec54_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec40[i]=0;
		fRec40_idx = 0;
		fRec40_idx_save = 0;
		iConst89 = int((10 + fConst86));
		iConst90 = int((1 + iConst89));
		for (int i=0; i<134217728; i++) fYec55[i]=0;
		fYec55_idx = 0;
		fYec55_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec41[i]=0;
		fRec41_idx = 0;
		fRec41_idx_save = 0;
		iConst91 = int((20 + fConst86));
		iConst92 = int((1 + iConst91));
		for (int i=0; i<134217728; i++) fYec56[i]=0;
		fYec56_idx = 0;
		fYec56_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec42[i]=0;
		fRec42_idx = 0;
		fRec42_idx_save = 0;
		iConst93 = int((30 + fConst86));
		iConst94 = int((1 + iConst93));
		for (int i=0; i<134217728; i++) fYec57[i]=0;
		fYec57_idx = 0;
		fYec57_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec43[i]=0;
		fRec43_idx = 0;
		fRec43_idx_save = 0;
		iConst95 = int((40 + fConst86));
		iConst96 = int((1 + iConst95));
		for (int i=0; i<134217728; i++) fYec58[i]=0;
		fYec58_idx = 0;
		fYec58_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec44[i]=0;
		fRec44_idx = 0;
		fRec44_idx_save = 0;
		iConst97 = int((50 + fConst86));
		iConst98 = int((1 + iConst97));
		for (int i=0; i<134217728; i++) fYec59[i]=0;
		fYec59_idx = 0;
		fYec59_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec45[i]=0;
		fRec45_idx = 0;
		fRec45_idx_save = 0;
		iConst99 = int((60 + fConst86));
		iConst100 = int((1 + iConst99));
		for (int i=0; i<134217728; i++) fYec60[i]=0;
		fYec60_idx = 0;
		fYec60_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec46[i]=0;
		fRec46_idx = 0;
		fRec46_idx_save = 0;
		iConst101 = int((70 + fConst86));
		iConst102 = int((1 + iConst101));
		for (int i=0; i<134217728; i++) fYec61[i]=0;
		fYec61_idx = 0;
		fYec61_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec47[i]=0;
		fRec47_idx = 0;
		fRec47_idx_save = 0;
		fConst103 = (quad(128463300) / quad(iConst0));
		iConst104 = int(fConst103);
		iConst105 = int((1 + iConst104));
		for (int i=0; i<134217728; i++) fYec62[i]=0;
		fYec62_idx = 0;
		fYec62_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec48[i]=0;
		fRec48_idx = 0;
		fRec48_idx_save = 0;
		iConst106 = int((10 + fConst103));
		iConst107 = int((1 + iConst106));
		for (int i=0; i<134217728; i++) fYec63[i]=0;
		fYec63_idx = 0;
		fYec63_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec49[i]=0;
		fRec49_idx = 0;
		fRec49_idx_save = 0;
		iConst108 = int((20 + fConst103));
		iConst109 = int((1 + iConst108));
		for (int i=0; i<134217728; i++) fYec64[i]=0;
		fYec64_idx = 0;
		fYec64_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec50[i]=0;
		fRec50_idx = 0;
		fRec50_idx_save = 0;
		iConst110 = int((30 + fConst103));
		iConst111 = int((1 + iConst110));
		for (int i=0; i<134217728; i++) fYec65[i]=0;
		fYec65_idx = 0;
		fYec65_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec51[i]=0;
		fRec51_idx = 0;
		fRec51_idx_save = 0;
		iConst112 = int((40 + fConst103));
		iConst113 = int((1 + iConst112));
		for (int i=0; i<134217728; i++) fYec66[i]=0;
		fYec66_idx = 0;
		fYec66_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec52[i]=0;
		fRec52_idx = 0;
		fRec52_idx_save = 0;
		iConst114 = int((50 + fConst103));
		iConst115 = int((1 + iConst114));
		for (int i=0; i<134217728; i++) fYec67[i]=0;
		fYec67_idx = 0;
		fYec67_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec53[i]=0;
		fRec53_idx = 0;
		fRec53_idx_save = 0;
		iConst116 = int((60 + fConst103));
		iConst117 = int((1 + iConst116));
		for (int i=0; i<134217728; i++) fYec68[i]=0;
		fYec68_idx = 0;
		fYec68_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec54[i]=0;
		fRec54_idx = 0;
		fRec54_idx_save = 0;
		iConst118 = int((70 + fConst103));
		iConst119 = int((1 + iConst118));
		for (int i=0; i<134217728; i++) fYec69[i]=0;
		fYec69_idx = 0;
		fYec69_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec55[i]=0;
		fRec55_idx = 0;
		fRec55_idx_save = 0;
		fConst120 = (quad(22314600) / quad(iConst0));
		iConst121 = int(fConst120);
		iConst122 = int((1 + iConst121));
		for (int i=0; i<33554432; i++) fYec70[i]=0;
		fYec70_idx = 0;
		fYec70_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec56[i]=0;
		fRec56_idx = 0;
		fRec56_idx_save = 0;
		iConst123 = int((10 + fConst120));
		iConst124 = int((1 + iConst123));
		for (int i=0; i<33554432; i++) fYec71[i]=0;
		fYec71_idx = 0;
		fYec71_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec57[i]=0;
		fRec57_idx = 0;
		fRec57_idx_save = 0;
		iConst125 = int((20 + fConst120));
		iConst126 = int((1 + iConst125));
		for (int i=0; i<33554432; i++) fYec72[i]=0;
		fYec72_idx = 0;
		fYec72_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec58[i]=0;
		fRec58_idx = 0;
		fRec58_idx_save = 0;
		iConst127 = int((30 + fConst120));
		iConst128 = int((1 + iConst127));
		for (int i=0; i<33554432; i++) fYec73[i]=0;
		fYec73_idx = 0;
		fYec73_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec59[i]=0;
		fRec59_idx = 0;
		fRec59_idx_save = 0;
		iConst129 = int((40 + fConst120));
		iConst130 = int((1 + iConst129));
		for (int i=0; i<33554432; i++) fYec74[i]=0;
		fYec74_idx = 0;
		fYec74_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec60[i]=0;
		fRec60_idx = 0;
		fRec60_idx_save = 0;
		iConst131 = int((50 + fConst120));
		iConst132 = int((1 + iConst131));
		for (int i=0; i<33554432; i++) fYec75[i]=0;
		fYec75_idx = 0;
		fYec75_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec61[i]=0;
		fRec61_idx = 0;
		fRec61_idx_save = 0;
		iConst133 = int((60 + fConst120));
		iConst134 = int((1 + iConst133));
		for (int i=0; i<33554432; i++) fYec76[i]=0;
		fYec76_idx = 0;
		fYec76_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec62[i]=0;
		fRec62_idx = 0;
		fRec62_idx_save = 0;
		iConst135 = int((70 + fConst120));
		iConst136 = int((1 + iConst135));
		for (int i=0; i<33554432; i++) fYec77[i]=0;
		fYec77_idx = 0;
		fYec77_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec63[i]=0;
		fRec63_idx = 0;
		fRec63_idx_save = 0;
		fConst137 = (quad(114660000) / quad(iConst0));
		iConst138 = int(fConst137);
		iConst139 = int((1 + iConst138));
		for (int i=0; i<134217728; i++) fYec78[i]=0;
		fYec78_idx = 0;
		fYec78_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec64[i]=0;
		fRec64_idx = 0;
		fRec64_idx_save = 0;
		iConst140 = int((10 + fConst137));
		iConst141 = int((1 + iConst140));
		for (int i=0; i<134217728; i++) fYec79[i]=0;
		fYec79_idx = 0;
		fYec79_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec65[i]=0;
		fRec65_idx = 0;
		fRec65_idx_save = 0;
		iConst142 = int((20 + fConst137));
		iConst143 = int((1 + iConst142));
		for (int i=0; i<134217728; i++) fYec80[i]=0;
		fYec80_idx = 0;
		fYec80_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec66[i]=0;
		fRec66_idx = 0;
		fRec66_idx_save = 0;
		iConst144 = int((30 + fConst137));
		iConst145 = int((1 + iConst144));
		for (int i=0; i<134217728; i++) fYec81[i]=0;
		fYec81_idx = 0;
		fYec81_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec67[i]=0;
		fRec67_idx = 0;
		fRec67_idx_save = 0;
		iConst146 = int((40 + fConst137));
		iConst147 = int((1 + iConst146));
		for (int i=0; i<134217728; i++) fYec82[i]=0;
		fYec82_idx = 0;
		fYec82_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec68[i]=0;
		fRec68_idx = 0;
		fRec68_idx_save = 0;
		iConst148 = int((50 + fConst137));
		iConst149 = int((1 + iConst148));
		for (int i=0; i<134217728; i++) fYec83[i]=0;
		fYec83_idx = 0;
		fYec83_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec69[i]=0;
		fRec69_idx = 0;
		fRec69_idx_save = 0;
		iConst150 = int((60 + fConst137));
		iConst151 = int((1 + iConst150));
		for (int i=0; i<134217728; i++) fYec84[i]=0;
		fYec84_idx = 0;
		fYec84_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec70[i]=0;
		fRec70_idx = 0;
		fRec70_idx_save = 0;
		iConst152 = int((70 + fConst137));
		iConst153 = int((1 + iConst152));
		for (int i=0; i<134217728; i++) fYec85[i]=0;
		fYec85_idx = 0;
		fYec85_idx_save = 0;
		for (int i=0; i<134217728; i++) fRec71[i]=0;
		fRec71_idx = 0;
		fRec71_idx_save = 0;
		fConst154 = (quad(138077100) / quad(iConst0));
		iConst155 = int(fConst154);
		iConst156 = int((1 + iConst155));
		for (int i=0; i<268435456; i++) fYec86[i]=0;
		fYec86_idx = 0;
		fYec86_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec72[i]=0;
		fRec72_idx = 0;
		fRec72_idx_save = 0;
		iConst157 = int((10 + fConst154));
		iConst158 = int((1 + iConst157));
		for (int i=0; i<268435456; i++) fYec87[i]=0;
		fYec87_idx = 0;
		fYec87_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec73[i]=0;
		fRec73_idx = 0;
		fRec73_idx_save = 0;
		iConst159 = int((20 + fConst154));
		iConst160 = int((1 + iConst159));
		for (int i=0; i<268435456; i++) fYec88[i]=0;
		fYec88_idx = 0;
		fYec88_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec74[i]=0;
		fRec74_idx = 0;
		fRec74_idx_save = 0;
		iConst161 = int((30 + fConst154));
		iConst162 = int((1 + iConst161));
		for (int i=0; i<268435456; i++) fYec89[i]=0;
		fYec89_idx = 0;
		fYec89_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec75[i]=0;
		fRec75_idx = 0;
		fRec75_idx_save = 0;
		iConst163 = int((40 + fConst154));
		iConst164 = int((1 + iConst163));
		for (int i=0; i<268435456; i++) fYec90[i]=0;
		fYec90_idx = 0;
		fYec90_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec76[i]=0;
		fRec76_idx = 0;
		fRec76_idx_save = 0;
		iConst165 = int((50 + fConst154));
		iConst166 = int((1 + iConst165));
		for (int i=0; i<268435456; i++) fYec91[i]=0;
		fYec91_idx = 0;
		fYec91_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec77[i]=0;
		fRec77_idx = 0;
		fRec77_idx_save = 0;
		iConst167 = int((60 + fConst154));
		iConst168 = int((1 + iConst167));
		for (int i=0; i<268435456; i++) fYec92[i]=0;
		fYec92_idx = 0;
		fYec92_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec78[i]=0;
		fRec78_idx = 0;
		fRec78_idx_save = 0;
		iConst169 = int((70 + fConst154));
		iConst170 = int((1 + iConst169));
		for (int i=0; i<268435456; i++) fYec93[i]=0;
		fYec93_idx = 0;
		fYec93_idx_save = 0;
		for (int i=0; i<268435456; i++) fRec79[i]=0;
		fRec79_idx = 0;
		fRec79_idx_save = 0;
		fConst171 = (quad(4410000) / quad(iConst0));
		iConst172 = int(fConst171);
		iConst173 = int((1 + iConst172));
		for (int i=0; i<8388608; i++) fYec94[i]=0;
		fYec94_idx = 0;
		fYec94_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec80[i]=0;
		fRec80_idx = 0;
		fRec80_idx_save = 0;
		iConst174 = int((10 + fConst171));
		iConst175 = int((1 + iConst174));
		for (int i=0; i<8388608; i++) fYec95[i]=0;
		fYec95_idx = 0;
		fYec95_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec81[i]=0;
		fRec81_idx = 0;
		fRec81_idx_save = 0;
		iConst176 = int((20 + fConst171));
		iConst177 = int((1 + iConst176));
		for (int i=0; i<8388608; i++) fYec96[i]=0;
		fYec96_idx = 0;
		fYec96_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec82[i]=0;
		fRec82_idx = 0;
		fRec82_idx_save = 0;
		iConst178 = int((30 + fConst171));
		iConst179 = int((1 + iConst178));
		for (int i=0; i<8388608; i++) fYec97[i]=0;
		fYec97_idx = 0;
		fYec97_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec83[i]=0;
		fRec83_idx = 0;
		fRec83_idx_save = 0;
		iConst180 = int((40 + fConst171));
		iConst181 = int((1 + iConst180));
		for (int i=0; i<8388608; i++) fYec98[i]=0;
		fYec98_idx = 0;
		fYec98_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec84[i]=0;
		fRec84_idx = 0;
		fRec84_idx_save = 0;
		iConst182 = int((50 + fConst171));
		iConst183 = int((1 + iConst182));
		for (int i=0; i<8388608; i++) fYec99[i]=0;
		fYec99_idx = 0;
		fYec99_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec85[i]=0;
		fRec85_idx = 0;
		fRec85_idx_save = 0;
		iConst184 = int((60 + fConst171));
		iConst185 = int((1 + iConst184));
		for (int i=0; i<8388608; i++) fYec100[i]=0;
		fYec100_idx = 0;
		fYec100_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec86[i]=0;
		fRec86_idx = 0;
		fRec86_idx_save = 0;
		iConst186 = int((70 + fConst171));
		iConst187 = int((1 + iConst186));
		for (int i=0; i<8388608; i++) fYec101[i]=0;
		fYec101_idx = 0;
		fYec101_idx_save = 0;
		for (int i=0; i<8388608; i++) fRec87[i]=0;
		fRec87_idx = 0;
		fRec87_idx_save = 0;
		fConst188 = (quad(33075000) / quad(iConst0));
		iConst189 = int(fConst188);
		iConst190 = int((1 + iConst189));
		for (int i=0; i<33554432; i++) fYec102[i]=0;
		fYec102_idx = 0;
		fYec102_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec88[i]=0;
		fRec88_idx = 0;
		fRec88_idx_save = 0;
		iConst191 = int((10 + fConst188));
		iConst192 = int((1 + iConst191));
		for (int i=0; i<33554432; i++) fYec103[i]=0;
		fYec103_idx = 0;
		fYec103_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec89[i]=0;
		fRec89_idx = 0;
		fRec89_idx_save = 0;
		iConst193 = int((20 + fConst188));
		iConst194 = int((1 + iConst193));
		for (int i=0; i<33554432; i++) fYec104[i]=0;
		fYec104_idx = 0;
		fYec104_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec90[i]=0;
		fRec90_idx = 0;
		fRec90_idx_save = 0;
		iConst195 = int((30 + fConst188));
		iConst196 = int((1 + iConst195));
		for (int i=0; i<33554432; i++) fYec105[i]=0;
		fYec105_idx = 0;
		fYec105_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec91[i]=0;
		fRec91_idx = 0;
		fRec91_idx_save = 0;
		iConst197 = int((40 + fConst188));
		iConst198 = int((1 + iConst197));
		for (int i=0; i<33554432; i++) fYec106[i]=0;
		fYec106_idx = 0;
		fYec106_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec92[i]=0;
		fRec92_idx = 0;
		fRec92_idx_save = 0;
		iConst199 = int((50 + fConst188));
		iConst200 = int((1 + iConst199));
		for (int i=0; i<33554432; i++) fYec107[i]=0;
		fYec107_idx = 0;
		fYec107_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec93[i]=0;
		fRec93_idx = 0;
		fRec93_idx_save = 0;
		iConst201 = int((60 + fConst188));
		iConst202 = int((1 + iConst201));
		for (int i=0; i<33554432; i++) fYec108[i]=0;
		fYec108_idx = 0;
		fYec108_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec94[i]=0;
		fRec94_idx = 0;
		fRec94_idx_save = 0;
		iConst203 = int((70 + fConst188));
		iConst204 = int((1 + iConst203));
		for (int i=0; i<33554432; i++) fYec109[i]=0;
		fYec109_idx = 0;
		fYec109_idx_save = 0;
		for (int i=0; i<33554432; i++) fRec95[i]=0;
		fRec95_idx = 0;
		fRec95_idx_save = 0;
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
		interface->addHorizontalSlider("Pre-DelTime's", &fslider0, 0.6L, 0.0L, 15.0L, 0.1L);
		interface->addHorizontalSlider("Roomsize", &fslider1, 0.81L, 0.0L, 1.0L, 0.01L);
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
			fGraph.InitTask(1,8);
			// Only initialize taks with more than one input
			fGraph.InitTask(5,2);
			fGraph.InitTask(6,2);
			fGraph.InitTask(7,2);
			fGraph.InitTask(8,2);
			fGraph.InitTask(9,2);
			fGraph.InitTask(10,2);
			fGraph.InitTask(11,2);
			fGraph.InitTask(12,2);
			fGraph.InitTask(13,2);
			fGraph.InitTask(14,2);
			fGraph.InitTask(15,2);
			fGraph.InitTask(16,2);
			fGraph.InitTask(17,2);
			fGraph.InitTask(18,2);
			fGraph.InitTask(19,2);
			fGraph.InitTask(20,2);
			fGraph.InitTask(21,2);
			fGraph.InitTask(22,2);
			fGraph.InitTask(23,2);
			fGraph.InitTask(24,2);
			fGraph.InitTask(25,2);
			fGraph.InitTask(26,2);
			fGraph.InitTask(29,2);
			fGraph.InitTask(31,2);
			fGraph.InitTask(32,4);
			fGraph.InitTask(33,2);
			fGraph.InitTask(34,2);
			fGraph.InitTask(35,2);
			fGraph.InitTask(36,2);
			fGraph.InitTask(37,2);
			fGraph.InitTask(38,2);
			fGraph.InitTask(39,2);
			fGraph.InitTask(40,2);
			fGraph.InitTask(41,2);
			fGraph.InitTask(42,2);
			fGraph.InitTask(43,2);
			fGraph.InitTask(44,12);
			fGraph.InitTask(45,12);
			fGraph.InitTask(46,12);
			fGraph.InitTask(47,12);
			fGraph.InitTask(48,12);
			fGraph.InitTask(49,12);
			fGraph.InitTask(50,12);
			fGraph.InitTask(51,12);
			fIsFinished = false;
			fThreadPool->SignalAll(fDynamicNumThreads - 1, this);
			computeThread(0);
			while (!fThreadPool->IsFinished()) {}
		}
		StopMeasure(fStaticNumThreads, fDynamicNumThreads);
	}
	void computeThread(int cur_thread) {
		int count = fCount;
		quad 	fSlow0 = min((quad)15, max((quad)0, quad(fslider0)));
		int 	iSlow1 = int((2 * fSlow0));
		int 	iSlow2 = int((5 * fSlow0));
		int 	iSlow3 = int((7 * fSlow0));
		int 	iSlow4 = int((23 * fSlow0));
		int 	iSlow5 = int((54 * fSlow0));
		int 	iSlow6 = int((79 * fSlow0));
		int 	iSlow7 = int((220 * fSlow0));
		int 	iSlow8 = int((340 * fSlow0));
		int 	iSlow9 = int((454 * fSlow0));
		int 	iSlow10 = int((623 * fSlow0));
		int 	iSlow11 = int((845 * fSlow0));
		int 	iSlow12 = int((1304 * fSlow0));
		quad 	fSlow13 = min((quad)2, max((quad)0, quad(fslider1)));
		int 	iSlow14 = int((1532 * fSlow0));
		int 	iSlow15 = int((2204 * fSlow0));
		// Init graph state
		{
			TaskQueue taskqueue(cur_thread);
			int tasknum = -1;
			// Init input and output
			FAUSTFLOAT* input0 = &input[0][fIndex];
			FAUSTFLOAT* output0 = &output[0][fIndex];
			FAUSTFLOAT* output1 = &output[1][fIndex];
			FAUSTFLOAT* output2 = &output[2][fIndex];
			FAUSTFLOAT* output3 = &output[3][fIndex];
			FAUSTFLOAT* output4 = &output[4][fIndex];
			FAUSTFLOAT* output5 = &output[5][fIndex];
			FAUSTFLOAT* output6 = &output[6][fIndex];
			FAUSTFLOAT* output7 = &output[7][fIndex];
			if (cur_thread == 0) {
			    tasknum = 2;
			} else {
			    tasknum = TaskQueue::GetNextTask(cur_thread, fDynamicNumThreads);
			}
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
						// LOOP 0x3b70570
						// pre processing
						fYec0_idx = (fYec0_idx+fYec0_idx_save)&63;
						// exec code
						for (int i=0; i<count; i++) {
							fYec0[(fYec0_idx+i)&63] = (quad)input0[i];
						}
						// post processing
						fYec0_idx_save = count;
						
						taskqueue.PushHead(4);
						tasknum = 3;
						break;
					} 
					// SECTION : 2
					case 3: { 
						// LOOP 0x3b70460
						// pre processing
						fYec1_idx = (fYec1_idx+fYec1_idx_save)&127;
						// exec code
						for (int i=0; i<count; i++) {
							fYec1[(fYec1_idx+i)&127] = ((0.015625L * (quad)input0[i]) - (0.015625L * fYec0[(fYec0_idx+i-iSlow1)&63]));
						}
						// post processing
						fYec1_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 5, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 6, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 4: { 
						// LOOP 0x3b72ef0
						// exec code
						for (int i=0; i<count; i++) {
							fZec0[i] = ((quad)input0[i] + fYec0[(fYec0_idx+i-iSlow1)&63]);
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 5, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 6, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 3
					case 5: { 
						// LOOP 0x3b70350
						// pre processing
						fYec2_idx = (fYec2_idx+fYec2_idx_save)&255;
						// exec code
						for (int i=0; i<count; i++) {
							fYec2[(fYec2_idx+i)&255] = ((0.00390625L * fZec0[i]) - (0.25L * fYec1[(fYec1_idx+i-iSlow2)&127]));
						}
						// post processing
						fYec2_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 7, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 8, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 6: { 
						// LOOP 0x3b746d0
						// exec code
						for (int i=0; i<count; i++) {
							fZec1[i] = ((0.0078125L * fZec0[i]) + (0.5L * fYec1[(fYec1_idx+i-iSlow2)&127]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 7, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 8, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 4
					case 7: { 
						// LOOP 0x3b70240
						// pre processing
						fYec3_idx = (fYec3_idx+fYec3_idx_save)&511;
						// exec code
						for (int i=0; i<count; i++) {
							fYec3[(fYec3_idx+i)&511] = ((0.125L * fZec1[i]) - (0.25L * fYec2[(fYec2_idx+i-iSlow3)&255]));
						}
						// post processing
						fYec3_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 9, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 10, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 8: { 
						// LOOP 0x3b76240
						// exec code
						for (int i=0; i<count; i++) {
							fZec2[i] = ((0.25L * fZec1[i]) + (0.5L * fYec2[(fYec2_idx+i-iSlow3)&255]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 9, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 10, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 5
					case 9: { 
						// LOOP 0x3b70130
						// pre processing
						fYec4_idx = (fYec4_idx+fYec4_idx_save)&1023;
						// exec code
						for (int i=0; i<count; i++) {
							fYec4[(fYec4_idx+i)&1023] = ((0.125L * fZec2[i]) - (0.25L * fYec3[(fYec3_idx+i-iSlow4)&511]));
						}
						// post processing
						fYec4_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 11, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 12, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 10: { 
						// LOOP 0x3b77a90
						// exec code
						for (int i=0; i<count; i++) {
							fZec3[i] = ((0.25L * fZec2[i]) + (0.5L * fYec3[(fYec3_idx+i-iSlow4)&511]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 11, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 12, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 6
					case 11: { 
						// LOOP 0x3b70020
						// pre processing
						fYec5_idx = (fYec5_idx+fYec5_idx_save)&2047;
						// exec code
						for (int i=0; i<count; i++) {
							fYec5[(fYec5_idx+i)&2047] = ((0.125L * fZec3[i]) - (0.25L * fYec4[(fYec4_idx+i-iSlow5)&1023]));
						}
						// post processing
						fYec5_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 13, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 14, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 12: { 
						// LOOP 0x3b792e0
						// exec code
						for (int i=0; i<count; i++) {
							fZec4[i] = ((0.25L * fZec3[i]) + (0.5L * fYec4[(fYec4_idx+i-iSlow5)&1023]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 13, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 14, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 7
					case 13: { 
						// LOOP 0x3b6ff10
						// pre processing
						fYec6_idx = (fYec6_idx+fYec6_idx_save)&4095;
						// exec code
						for (int i=0; i<count; i++) {
							fYec6[(fYec6_idx+i)&4095] = ((0.125L * fZec4[i]) - (0.25L * fYec5[(fYec5_idx+i-iSlow6)&2047]));
						}
						// post processing
						fYec6_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 15, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 16, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 14: { 
						// LOOP 0x3b7ab40
						// exec code
						for (int i=0; i<count; i++) {
							fZec5[i] = ((0.25L * fZec4[i]) + (0.5L * fYec5[(fYec5_idx+i-iSlow6)&2047]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 15, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 16, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 8
					case 15: { 
						// LOOP 0x3b6fe00
						// pre processing
						fYec7_idx = (fYec7_idx+fYec7_idx_save)&8191;
						// exec code
						for (int i=0; i<count; i++) {
							fYec7[(fYec7_idx+i)&8191] = ((0.125L * fZec5[i]) - (0.25L * fYec6[(fYec6_idx+i-iSlow7)&4095]));
						}
						// post processing
						fYec7_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 17, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 18, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 16: { 
						// LOOP 0x3b7c3a0
						// exec code
						for (int i=0; i<count; i++) {
							fZec6[i] = ((0.25L * fZec5[i]) + (0.5L * fYec6[(fYec6_idx+i-iSlow7)&4095]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 17, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 18, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 9
					case 17: { 
						// LOOP 0x3b6fcf0
						// pre processing
						fYec8_idx = (fYec8_idx+fYec8_idx_save)&8191;
						// exec code
						for (int i=0; i<count; i++) {
							fYec8[(fYec8_idx+i)&8191] = ((0.125L * fZec6[i]) - (0.25L * fYec7[(fYec7_idx+i-iSlow8)&8191]));
						}
						// post processing
						fYec8_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 19, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 20, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 18: { 
						// LOOP 0x3b7dc00
						// exec code
						for (int i=0; i<count; i++) {
							fZec7[i] = ((0.25L * fZec6[i]) + (0.5L * fYec7[(fYec7_idx+i-iSlow8)&8191]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 19, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 20, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 10
					case 19: { 
						// LOOP 0x3b6fbe0
						// pre processing
						fYec9_idx = (fYec9_idx+fYec9_idx_save)&16383;
						// exec code
						for (int i=0; i<count; i++) {
							fYec9[(fYec9_idx+i)&16383] = ((0.125L * fZec7[i]) - (0.25L * fYec8[(fYec8_idx+i-iSlow9)&8191]));
						}
						// post processing
						fYec9_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 21, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 22, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 20: { 
						// LOOP 0x3b7f460
						// exec code
						for (int i=0; i<count; i++) {
							fZec8[i] = ((0.25L * fZec7[i]) + (0.5L * fYec8[(fYec8_idx+i-iSlow9)&8191]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 21, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 22, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 11
					case 21: { 
						// LOOP 0x3b6fad0
						// pre processing
						fYec10_idx = (fYec10_idx+fYec10_idx_save)&16383;
						// exec code
						for (int i=0; i<count; i++) {
							fYec10[(fYec10_idx+i)&16383] = ((0.125L * fZec8[i]) - (0.25L * fYec9[(fYec9_idx+i-iSlow10)&16383]));
						}
						// post processing
						fYec10_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 23, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 24, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 22: { 
						// LOOP 0x3b80d80
						// exec code
						for (int i=0; i<count; i++) {
							fZec9[i] = ((0.25L * fZec8[i]) + (0.5L * fYec9[(fYec9_idx+i-iSlow10)&16383]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 23, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 24, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 12
					case 23: { 
						// LOOP 0x3b6f9c0
						// pre processing
						fYec11_idx = (fYec11_idx+fYec11_idx_save)&32767;
						// exec code
						for (int i=0; i<count; i++) {
							fYec11[(fYec11_idx+i)&32767] = ((0.125L * fZec9[i]) - (0.25L * fYec10[(fYec10_idx+i-iSlow11)&16383]));
						}
						// post processing
						fYec11_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 25, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 26, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 24: { 
						// LOOP 0x3b82690
						// exec code
						for (int i=0; i<count; i++) {
							fZec10[i] = ((0.25L * fZec9[i]) + (0.5L * fYec10[(fYec10_idx+i-iSlow11)&16383]));
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 25, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 26, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 13
					case 25: { 
						// LOOP 0x3b6f8e0
						// exec code
						for (int i=0; i<count; i++) {
							fZec11[i] = ((0.25L * fZec10[i]) + (0.5L * fYec11[(fYec11_idx+i-iSlow12)&32767]));
						}
						
						fGraph.ActivateOutputTask(taskqueue, 29);
						tasknum = 27;
						break;
					} 
					case 26: { 
						// LOOP 0x3b857c0
						// pre processing
						fYec12_idx = (fYec12_idx+fYec12_idx_save)&32767;
						// exec code
						for (int i=0; i<count; i++) {
							fYec12[(fYec12_idx+i)&32767] = ((0.125L * fZec10[i]) - (0.25L * fYec11[(fYec11_idx+i-iSlow12)&32767]));
						}
						// post processing
						fYec12_idx_save = count;
						
						fGraph.ActivateOutputTask(taskqueue, 29);
						tasknum = 28;
						break;
					} 
					// SECTION : 14
					case 27: { 
						// LOOP 0x3b6f800
						// exec code
						for (int i=0; i<count; i++) {
							fZec12[i] = (0.25L * fZec11[i]);
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 32, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 31, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 28: { 
						// LOOP 0x3b856e0
						// exec code
						for (int i=0; i<count; i++) {
							fZec13[i] = (0.5L * fYec12[(fYec12_idx+i-iSlow14)&32767]);
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 32, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 31, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 29: { 
						// LOOP 0x3b890c0
						// pre processing
						fYec14_idx = (fYec14_idx+fYec14_idx_save)&65535;
						// exec code
						for (int i=0; i<count; i++) {
							fYec14[(fYec14_idx+i)&65535] = ((0.125L * fZec11[i]) - (0.25L * fYec12[(fYec12_idx+i-iSlow14)&32767]));
						}
						// post processing
						fYec14_idx_save = count;
						
						tasknum = 30;
						break;
					} 
					// SECTION : 15
					case 30: { 
						// LOOP 0x3b88fe0
						// exec code
						for (int i=0; i<count; i++) {
							fZec14[i] = (2 * fYec14[(fYec14_idx+i-iSlow15)&65535]);
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 32, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 33, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 34, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 35, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 36, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 37, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 38, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 39, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 40, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 41, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 42, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 43, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 31: { 
						// LOOP 0x3b8c600
						// exec code
						for (int i=0; i<count; i++) {
							fZec15[i] = (fZec12[i] + fZec13[i]);
						}
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 32, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 33, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 34, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 35, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 36, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 37, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 38, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 39, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 40, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 41, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 42, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 43, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					// SECTION : 16
					case 32: { 
						// LOOP 0x3b6e760
						// pre processing
						fYec13_idx = (fYec13_idx+fYec13_idx_save)&16777215;
						fRec0_idx = (fRec0_idx+fRec0_idx_save)&16777215;
						fYec15_idx = (fYec15_idx+fYec15_idx_save)&16777215;
						fRec1_idx = (fRec1_idx+fRec1_idx_save)&16777215;
						fYec16_idx = (fYec16_idx+fYec16_idx_save)&16777215;
						fRec2_idx = (fRec2_idx+fRec2_idx_save)&16777215;
						fYec17_idx = (fYec17_idx+fYec17_idx_save)&16777215;
						fRec3_idx = (fRec3_idx+fRec3_idx_save)&16777215;
						fYec18_idx = (fYec18_idx+fYec18_idx_save)&16777215;
						fRec4_idx = (fRec4_idx+fRec4_idx_save)&16777215;
						fYec19_idx = (fYec19_idx+fYec19_idx_save)&16777215;
						fRec5_idx = (fRec5_idx+fRec5_idx_save)&16777215;
						fYec20_idx = (fYec20_idx+fYec20_idx_save)&16777215;
						fRec6_idx = (fRec6_idx+fRec6_idx_save)&16777215;
						fYec21_idx = (fYec21_idx+fYec21_idx_save)&16777215;
						fRec7_idx = (fRec7_idx+fRec7_idx_save)&16777215;
						// exec code
						for (int i=0; i<count; i++) {
							fYec13[(fYec13_idx+i)&16777215] = (fZec13[i] + ((fSlow13 * fRec0[(fRec0_idx+i-iConst3)&16777215]) + fZec12[i]));
							fRec0[(fRec0_idx+i)&16777215] = fYec13[(fYec13_idx+i-iConst2)&16777215];
							fYec15[(fYec15_idx+i)&16777215] = ((fSlow13 * fRec1[(fRec1_idx+i-iConst5)&16777215]) + fZec14[i]);
							fRec1[(fRec1_idx+i)&16777215] = fYec15[(fYec15_idx+i-iConst4)&16777215];
							fYec16[(fYec16_idx+i)&16777215] = (fZec15[i] + (fSlow13 * fRec2[(fRec2_idx+i-iConst7)&16777215]));
							fRec2[(fRec2_idx+i)&16777215] = fYec16[(fYec16_idx+i-iConst6)&16777215];
							fYec17[(fYec17_idx+i)&16777215] = (fZec14[i] + (fSlow13 * fRec3[(fRec3_idx+i-iConst9)&16777215]));
							fRec3[(fRec3_idx+i)&16777215] = fYec17[(fYec17_idx+i-iConst8)&16777215];
							fYec18[(fYec18_idx+i)&16777215] = (fZec15[i] + (fSlow13 * fRec4[(fRec4_idx+i-iConst11)&16777215]));
							fRec4[(fRec4_idx+i)&16777215] = fYec18[(fYec18_idx+i-iConst10)&16777215];
							fYec19[(fYec19_idx+i)&16777215] = (fZec14[i] + (fSlow13 * fRec5[(fRec5_idx+i-iConst13)&16777215]));
							fRec5[(fRec5_idx+i)&16777215] = fYec19[(fYec19_idx+i-iConst12)&16777215];
							fYec20[(fYec20_idx+i)&16777215] = (fZec15[i] + (fSlow13 * fRec6[(fRec6_idx+i-iConst15)&16777215]));
							fRec6[(fRec6_idx+i)&16777215] = fYec20[(fYec20_idx+i-iConst14)&16777215];
							fYec21[(fYec21_idx+i)&16777215] = (fZec14[i] + (fSlow13 * fRec7[(fRec7_idx+i-iConst17)&16777215]));
							fRec7[(fRec7_idx+i)&16777215] = fYec21[(fYec21_idx+i-iConst16)&16777215];
						}
						// post processing
						fRec7_idx_save = count;
						fYec21_idx_save = count;
						fRec6_idx_save = count;
						fYec20_idx_save = count;
						fRec5_idx_save = count;
						fYec19_idx_save = count;
						fRec4_idx_save = count;
						fYec18_idx_save = count;
						fRec3_idx_save = count;
						fYec17_idx_save = count;
						fRec2_idx_save = count;
						fYec16_idx_save = count;
						fRec1_idx_save = count;
						fYec15_idx_save = count;
						fRec0_idx_save = count;
						fYec13_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 33: { 
						// LOOP 0x3b95150
						// pre processing
						fYec22_idx = (fYec22_idx+fYec22_idx_save)&33554431;
						fRec8_idx = (fRec8_idx+fRec8_idx_save)&33554431;
						fYec23_idx = (fYec23_idx+fYec23_idx_save)&33554431;
						fRec9_idx = (fRec9_idx+fRec9_idx_save)&33554431;
						fYec24_idx = (fYec24_idx+fYec24_idx_save)&33554431;
						fRec10_idx = (fRec10_idx+fRec10_idx_save)&33554431;
						fYec25_idx = (fYec25_idx+fYec25_idx_save)&33554431;
						fRec11_idx = (fRec11_idx+fRec11_idx_save)&33554431;
						fYec26_idx = (fYec26_idx+fYec26_idx_save)&33554431;
						fRec12_idx = (fRec12_idx+fRec12_idx_save)&33554431;
						fYec27_idx = (fYec27_idx+fYec27_idx_save)&33554431;
						fRec13_idx = (fRec13_idx+fRec13_idx_save)&33554431;
						fYec28_idx = (fYec28_idx+fYec28_idx_save)&33554431;
						fRec14_idx = (fRec14_idx+fRec14_idx_save)&33554431;
						fYec29_idx = (fYec29_idx+fYec29_idx_save)&33554431;
						fRec15_idx = (fRec15_idx+fRec15_idx_save)&33554431;
						// exec code
						for (int i=0; i<count; i++) {
							fYec22[(fYec22_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec8[(fRec8_idx+i-iConst20)&33554431]));
							fRec8[(fRec8_idx+i)&33554431] = fYec22[(fYec22_idx+i-iConst19)&33554431];
							fYec23[(fYec23_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec9[(fRec9_idx+i-iConst22)&33554431]));
							fRec9[(fRec9_idx+i)&33554431] = fYec23[(fYec23_idx+i-iConst21)&33554431];
							fYec24[(fYec24_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec10[(fRec10_idx+i-iConst24)&33554431]));
							fRec10[(fRec10_idx+i)&33554431] = fYec24[(fYec24_idx+i-iConst23)&33554431];
							fYec25[(fYec25_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec11[(fRec11_idx+i-iConst26)&33554431]));
							fRec11[(fRec11_idx+i)&33554431] = fYec25[(fYec25_idx+i-iConst25)&33554431];
							fYec26[(fYec26_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec12[(fRec12_idx+i-iConst28)&33554431]));
							fRec12[(fRec12_idx+i)&33554431] = fYec26[(fYec26_idx+i-iConst27)&33554431];
							fYec27[(fYec27_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec13[(fRec13_idx+i-iConst30)&33554431]));
							fRec13[(fRec13_idx+i)&33554431] = fYec27[(fYec27_idx+i-iConst29)&33554431];
							fYec28[(fYec28_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec14[(fRec14_idx+i-iConst32)&33554431]));
							fRec14[(fRec14_idx+i)&33554431] = fYec28[(fYec28_idx+i-iConst31)&33554431];
							fYec29[(fYec29_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec15[(fRec15_idx+i-iConst34)&33554431]));
							fRec15[(fRec15_idx+i)&33554431] = fYec29[(fYec29_idx+i-iConst33)&33554431];
						}
						// post processing
						fRec15_idx_save = count;
						fYec29_idx_save = count;
						fRec14_idx_save = count;
						fYec28_idx_save = count;
						fRec13_idx_save = count;
						fYec27_idx_save = count;
						fRec12_idx_save = count;
						fYec26_idx_save = count;
						fRec11_idx_save = count;
						fYec25_idx_save = count;
						fRec10_idx_save = count;
						fYec24_idx_save = count;
						fRec9_idx_save = count;
						fYec23_idx_save = count;
						fRec8_idx_save = count;
						fYec22_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 34: { 
						// LOOP 0x3ba2000
						// pre processing
						fYec30_idx = (fYec30_idx+fYec30_idx_save)&67108863;
						fRec16_idx = (fRec16_idx+fRec16_idx_save)&67108863;
						fYec31_idx = (fYec31_idx+fYec31_idx_save)&67108863;
						fRec17_idx = (fRec17_idx+fRec17_idx_save)&67108863;
						fYec32_idx = (fYec32_idx+fYec32_idx_save)&67108863;
						fRec18_idx = (fRec18_idx+fRec18_idx_save)&67108863;
						fYec33_idx = (fYec33_idx+fYec33_idx_save)&67108863;
						fRec19_idx = (fRec19_idx+fRec19_idx_save)&67108863;
						fYec34_idx = (fYec34_idx+fYec34_idx_save)&67108863;
						fRec20_idx = (fRec20_idx+fRec20_idx_save)&67108863;
						fYec35_idx = (fYec35_idx+fYec35_idx_save)&67108863;
						fRec21_idx = (fRec21_idx+fRec21_idx_save)&67108863;
						fYec36_idx = (fYec36_idx+fYec36_idx_save)&67108863;
						fRec22_idx = (fRec22_idx+fRec22_idx_save)&67108863;
						fYec37_idx = (fYec37_idx+fYec37_idx_save)&67108863;
						fRec23_idx = (fRec23_idx+fRec23_idx_save)&67108863;
						// exec code
						for (int i=0; i<count; i++) {
							fYec30[(fYec30_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec16[(fRec16_idx+i-iConst37)&67108863]));
							fRec16[(fRec16_idx+i)&67108863] = fYec30[(fYec30_idx+i-iConst36)&67108863];
							fYec31[(fYec31_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec17[(fRec17_idx+i-iConst39)&67108863]));
							fRec17[(fRec17_idx+i)&67108863] = fYec31[(fYec31_idx+i-iConst38)&67108863];
							fYec32[(fYec32_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec18[(fRec18_idx+i-iConst41)&67108863]));
							fRec18[(fRec18_idx+i)&67108863] = fYec32[(fYec32_idx+i-iConst40)&67108863];
							fYec33[(fYec33_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec19[(fRec19_idx+i-iConst43)&67108863]));
							fRec19[(fRec19_idx+i)&67108863] = fYec33[(fYec33_idx+i-iConst42)&67108863];
							fYec34[(fYec34_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec20[(fRec20_idx+i-iConst45)&67108863]));
							fRec20[(fRec20_idx+i)&67108863] = fYec34[(fYec34_idx+i-iConst44)&67108863];
							fYec35[(fYec35_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec21[(fRec21_idx+i-iConst47)&67108863]));
							fRec21[(fRec21_idx+i)&67108863] = fYec35[(fYec35_idx+i-iConst46)&67108863];
							fYec36[(fYec36_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec22[(fRec22_idx+i-iConst49)&67108863]));
							fRec22[(fRec22_idx+i)&67108863] = fYec36[(fYec36_idx+i-iConst48)&67108863];
							fYec37[(fYec37_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec23[(fRec23_idx+i-iConst51)&67108863]));
							fRec23[(fRec23_idx+i)&67108863] = fYec37[(fYec37_idx+i-iConst50)&67108863];
						}
						// post processing
						fRec23_idx_save = count;
						fYec37_idx_save = count;
						fRec22_idx_save = count;
						fYec36_idx_save = count;
						fRec21_idx_save = count;
						fYec35_idx_save = count;
						fRec20_idx_save = count;
						fYec34_idx_save = count;
						fRec19_idx_save = count;
						fYec33_idx_save = count;
						fRec18_idx_save = count;
						fYec32_idx_save = count;
						fRec17_idx_save = count;
						fYec31_idx_save = count;
						fRec16_idx_save = count;
						fYec30_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 35: { 
						// LOOP 0x3baf130
						// pre processing
						fYec38_idx = (fYec38_idx+fYec38_idx_save)&67108863;
						fRec24_idx = (fRec24_idx+fRec24_idx_save)&67108863;
						fYec39_idx = (fYec39_idx+fYec39_idx_save)&67108863;
						fRec25_idx = (fRec25_idx+fRec25_idx_save)&67108863;
						fYec40_idx = (fYec40_idx+fYec40_idx_save)&67108863;
						fRec26_idx = (fRec26_idx+fRec26_idx_save)&67108863;
						fYec41_idx = (fYec41_idx+fYec41_idx_save)&67108863;
						fRec27_idx = (fRec27_idx+fRec27_idx_save)&67108863;
						fYec42_idx = (fYec42_idx+fYec42_idx_save)&67108863;
						fRec28_idx = (fRec28_idx+fRec28_idx_save)&67108863;
						fYec43_idx = (fYec43_idx+fYec43_idx_save)&67108863;
						fRec29_idx = (fRec29_idx+fRec29_idx_save)&67108863;
						fYec44_idx = (fYec44_idx+fYec44_idx_save)&67108863;
						fRec30_idx = (fRec30_idx+fRec30_idx_save)&67108863;
						fYec45_idx = (fYec45_idx+fYec45_idx_save)&67108863;
						fRec31_idx = (fRec31_idx+fRec31_idx_save)&67108863;
						// exec code
						for (int i=0; i<count; i++) {
							fYec38[(fYec38_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec24[(fRec24_idx+i-iConst54)&67108863]));
							fRec24[(fRec24_idx+i)&67108863] = fYec38[(fYec38_idx+i-iConst53)&67108863];
							fYec39[(fYec39_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec25[(fRec25_idx+i-iConst56)&67108863]));
							fRec25[(fRec25_idx+i)&67108863] = fYec39[(fYec39_idx+i-iConst55)&67108863];
							fYec40[(fYec40_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec26[(fRec26_idx+i-iConst58)&67108863]));
							fRec26[(fRec26_idx+i)&67108863] = fYec40[(fYec40_idx+i-iConst57)&67108863];
							fYec41[(fYec41_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec27[(fRec27_idx+i-iConst60)&67108863]));
							fRec27[(fRec27_idx+i)&67108863] = fYec41[(fYec41_idx+i-iConst59)&67108863];
							fYec42[(fYec42_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec28[(fRec28_idx+i-iConst62)&67108863]));
							fRec28[(fRec28_idx+i)&67108863] = fYec42[(fYec42_idx+i-iConst61)&67108863];
							fYec43[(fYec43_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec29[(fRec29_idx+i-iConst64)&67108863]));
							fRec29[(fRec29_idx+i)&67108863] = fYec43[(fYec43_idx+i-iConst63)&67108863];
							fYec44[(fYec44_idx+i)&67108863] = (fZec15[i] + (fSlow13 * fRec30[(fRec30_idx+i-iConst66)&67108863]));
							fRec30[(fRec30_idx+i)&67108863] = fYec44[(fYec44_idx+i-iConst65)&67108863];
							fYec45[(fYec45_idx+i)&67108863] = (fZec14[i] + (fSlow13 * fRec31[(fRec31_idx+i-iConst68)&67108863]));
							fRec31[(fRec31_idx+i)&67108863] = fYec45[(fYec45_idx+i-iConst67)&67108863];
						}
						// post processing
						fRec31_idx_save = count;
						fYec45_idx_save = count;
						fRec30_idx_save = count;
						fYec44_idx_save = count;
						fRec29_idx_save = count;
						fYec43_idx_save = count;
						fRec28_idx_save = count;
						fYec42_idx_save = count;
						fRec27_idx_save = count;
						fYec41_idx_save = count;
						fRec26_idx_save = count;
						fYec40_idx_save = count;
						fRec25_idx_save = count;
						fYec39_idx_save = count;
						fRec24_idx_save = count;
						fYec38_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 36: { 
						// LOOP 0x3bbbff0
						// pre processing
						fYec46_idx = (fYec46_idx+fYec46_idx_save)&134217727;
						fRec32_idx = (fRec32_idx+fRec32_idx_save)&134217727;
						fYec47_idx = (fYec47_idx+fYec47_idx_save)&134217727;
						fRec33_idx = (fRec33_idx+fRec33_idx_save)&134217727;
						fYec48_idx = (fYec48_idx+fYec48_idx_save)&134217727;
						fRec34_idx = (fRec34_idx+fRec34_idx_save)&134217727;
						fYec49_idx = (fYec49_idx+fYec49_idx_save)&134217727;
						fRec35_idx = (fRec35_idx+fRec35_idx_save)&134217727;
						fYec50_idx = (fYec50_idx+fYec50_idx_save)&134217727;
						fRec36_idx = (fRec36_idx+fRec36_idx_save)&134217727;
						fYec51_idx = (fYec51_idx+fYec51_idx_save)&134217727;
						fRec37_idx = (fRec37_idx+fRec37_idx_save)&134217727;
						fYec52_idx = (fYec52_idx+fYec52_idx_save)&134217727;
						fRec38_idx = (fRec38_idx+fRec38_idx_save)&134217727;
						fYec53_idx = (fYec53_idx+fYec53_idx_save)&134217727;
						fRec39_idx = (fRec39_idx+fRec39_idx_save)&134217727;
						// exec code
						for (int i=0; i<count; i++) {
							fYec46[(fYec46_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec32[(fRec32_idx+i-iConst71)&134217727]));
							fRec32[(fRec32_idx+i)&134217727] = fYec46[(fYec46_idx+i-iConst70)&134217727];
							fYec47[(fYec47_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec33[(fRec33_idx+i-iConst73)&134217727]));
							fRec33[(fRec33_idx+i)&134217727] = fYec47[(fYec47_idx+i-iConst72)&134217727];
							fYec48[(fYec48_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec34[(fRec34_idx+i-iConst75)&134217727]));
							fRec34[(fRec34_idx+i)&134217727] = fYec48[(fYec48_idx+i-iConst74)&134217727];
							fYec49[(fYec49_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec35[(fRec35_idx+i-iConst77)&134217727]));
							fRec35[(fRec35_idx+i)&134217727] = fYec49[(fYec49_idx+i-iConst76)&134217727];
							fYec50[(fYec50_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec36[(fRec36_idx+i-iConst79)&134217727]));
							fRec36[(fRec36_idx+i)&134217727] = fYec50[(fYec50_idx+i-iConst78)&134217727];
							fYec51[(fYec51_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec37[(fRec37_idx+i-iConst81)&134217727]));
							fRec37[(fRec37_idx+i)&134217727] = fYec51[(fYec51_idx+i-iConst80)&134217727];
							fYec52[(fYec52_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec38[(fRec38_idx+i-iConst83)&134217727]));
							fRec38[(fRec38_idx+i)&134217727] = fYec52[(fYec52_idx+i-iConst82)&134217727];
							fYec53[(fYec53_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec39[(fRec39_idx+i-iConst85)&134217727]));
							fRec39[(fRec39_idx+i)&134217727] = fYec53[(fYec53_idx+i-iConst84)&134217727];
						}
						// post processing
						fRec39_idx_save = count;
						fYec53_idx_save = count;
						fRec38_idx_save = count;
						fYec52_idx_save = count;
						fRec37_idx_save = count;
						fYec51_idx_save = count;
						fRec36_idx_save = count;
						fYec50_idx_save = count;
						fRec35_idx_save = count;
						fYec49_idx_save = count;
						fRec34_idx_save = count;
						fYec48_idx_save = count;
						fRec33_idx_save = count;
						fYec47_idx_save = count;
						fRec32_idx_save = count;
						fYec46_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 37: { 
						// LOOP 0x3bc8e40
						// pre processing
						fYec54_idx = (fYec54_idx+fYec54_idx_save)&134217727;
						fRec40_idx = (fRec40_idx+fRec40_idx_save)&134217727;
						fYec55_idx = (fYec55_idx+fYec55_idx_save)&134217727;
						fRec41_idx = (fRec41_idx+fRec41_idx_save)&134217727;
						fYec56_idx = (fYec56_idx+fYec56_idx_save)&134217727;
						fRec42_idx = (fRec42_idx+fRec42_idx_save)&134217727;
						fYec57_idx = (fYec57_idx+fYec57_idx_save)&134217727;
						fRec43_idx = (fRec43_idx+fRec43_idx_save)&134217727;
						fYec58_idx = (fYec58_idx+fYec58_idx_save)&134217727;
						fRec44_idx = (fRec44_idx+fRec44_idx_save)&134217727;
						fYec59_idx = (fYec59_idx+fYec59_idx_save)&134217727;
						fRec45_idx = (fRec45_idx+fRec45_idx_save)&134217727;
						fYec60_idx = (fYec60_idx+fYec60_idx_save)&134217727;
						fRec46_idx = (fRec46_idx+fRec46_idx_save)&134217727;
						fYec61_idx = (fYec61_idx+fYec61_idx_save)&134217727;
						fRec47_idx = (fRec47_idx+fRec47_idx_save)&134217727;
						// exec code
						for (int i=0; i<count; i++) {
							fYec54[(fYec54_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec40[(fRec40_idx+i-iConst88)&134217727]));
							fRec40[(fRec40_idx+i)&134217727] = fYec54[(fYec54_idx+i-iConst87)&134217727];
							fYec55[(fYec55_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec41[(fRec41_idx+i-iConst90)&134217727]));
							fRec41[(fRec41_idx+i)&134217727] = fYec55[(fYec55_idx+i-iConst89)&134217727];
							fYec56[(fYec56_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec42[(fRec42_idx+i-iConst92)&134217727]));
							fRec42[(fRec42_idx+i)&134217727] = fYec56[(fYec56_idx+i-iConst91)&134217727];
							fYec57[(fYec57_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec43[(fRec43_idx+i-iConst94)&134217727]));
							fRec43[(fRec43_idx+i)&134217727] = fYec57[(fYec57_idx+i-iConst93)&134217727];
							fYec58[(fYec58_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec44[(fRec44_idx+i-iConst96)&134217727]));
							fRec44[(fRec44_idx+i)&134217727] = fYec58[(fYec58_idx+i-iConst95)&134217727];
							fYec59[(fYec59_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec45[(fRec45_idx+i-iConst98)&134217727]));
							fRec45[(fRec45_idx+i)&134217727] = fYec59[(fYec59_idx+i-iConst97)&134217727];
							fYec60[(fYec60_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec46[(fRec46_idx+i-iConst100)&134217727]));
							fRec46[(fRec46_idx+i)&134217727] = fYec60[(fYec60_idx+i-iConst99)&134217727];
							fYec61[(fYec61_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec47[(fRec47_idx+i-iConst102)&134217727]));
							fRec47[(fRec47_idx+i)&134217727] = fYec61[(fYec61_idx+i-iConst101)&134217727];
						}
						// post processing
						fRec47_idx_save = count;
						fYec61_idx_save = count;
						fRec46_idx_save = count;
						fYec60_idx_save = count;
						fRec45_idx_save = count;
						fYec59_idx_save = count;
						fRec44_idx_save = count;
						fYec58_idx_save = count;
						fRec43_idx_save = count;
						fYec57_idx_save = count;
						fRec42_idx_save = count;
						fYec56_idx_save = count;
						fRec41_idx_save = count;
						fYec55_idx_save = count;
						fRec40_idx_save = count;
						fYec54_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 38: { 
						// LOOP 0x3bd5df0
						// pre processing
						fYec62_idx = (fYec62_idx+fYec62_idx_save)&134217727;
						fRec48_idx = (fRec48_idx+fRec48_idx_save)&134217727;
						fYec63_idx = (fYec63_idx+fYec63_idx_save)&134217727;
						fRec49_idx = (fRec49_idx+fRec49_idx_save)&134217727;
						fYec64_idx = (fYec64_idx+fYec64_idx_save)&134217727;
						fRec50_idx = (fRec50_idx+fRec50_idx_save)&134217727;
						fYec65_idx = (fYec65_idx+fYec65_idx_save)&134217727;
						fRec51_idx = (fRec51_idx+fRec51_idx_save)&134217727;
						fYec66_idx = (fYec66_idx+fYec66_idx_save)&134217727;
						fRec52_idx = (fRec52_idx+fRec52_idx_save)&134217727;
						fYec67_idx = (fYec67_idx+fYec67_idx_save)&134217727;
						fRec53_idx = (fRec53_idx+fRec53_idx_save)&134217727;
						fYec68_idx = (fYec68_idx+fYec68_idx_save)&134217727;
						fRec54_idx = (fRec54_idx+fRec54_idx_save)&134217727;
						fYec69_idx = (fYec69_idx+fYec69_idx_save)&134217727;
						fRec55_idx = (fRec55_idx+fRec55_idx_save)&134217727;
						// exec code
						for (int i=0; i<count; i++) {
							fYec62[(fYec62_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec48[(fRec48_idx+i-iConst105)&134217727]));
							fRec48[(fRec48_idx+i)&134217727] = fYec62[(fYec62_idx+i-iConst104)&134217727];
							fYec63[(fYec63_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec49[(fRec49_idx+i-iConst107)&134217727]));
							fRec49[(fRec49_idx+i)&134217727] = fYec63[(fYec63_idx+i-iConst106)&134217727];
							fYec64[(fYec64_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec50[(fRec50_idx+i-iConst109)&134217727]));
							fRec50[(fRec50_idx+i)&134217727] = fYec64[(fYec64_idx+i-iConst108)&134217727];
							fYec65[(fYec65_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec51[(fRec51_idx+i-iConst111)&134217727]));
							fRec51[(fRec51_idx+i)&134217727] = fYec65[(fYec65_idx+i-iConst110)&134217727];
							fYec66[(fYec66_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec52[(fRec52_idx+i-iConst113)&134217727]));
							fRec52[(fRec52_idx+i)&134217727] = fYec66[(fYec66_idx+i-iConst112)&134217727];
							fYec67[(fYec67_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec53[(fRec53_idx+i-iConst115)&134217727]));
							fRec53[(fRec53_idx+i)&134217727] = fYec67[(fYec67_idx+i-iConst114)&134217727];
							fYec68[(fYec68_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec54[(fRec54_idx+i-iConst117)&134217727]));
							fRec54[(fRec54_idx+i)&134217727] = fYec68[(fYec68_idx+i-iConst116)&134217727];
							fYec69[(fYec69_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec55[(fRec55_idx+i-iConst119)&134217727]));
							fRec55[(fRec55_idx+i)&134217727] = fYec69[(fYec69_idx+i-iConst118)&134217727];
						}
						// post processing
						fRec55_idx_save = count;
						fYec69_idx_save = count;
						fRec54_idx_save = count;
						fYec68_idx_save = count;
						fRec53_idx_save = count;
						fYec67_idx_save = count;
						fRec52_idx_save = count;
						fYec66_idx_save = count;
						fRec51_idx_save = count;
						fYec65_idx_save = count;
						fRec50_idx_save = count;
						fYec64_idx_save = count;
						fRec49_idx_save = count;
						fYec63_idx_save = count;
						fRec48_idx_save = count;
						fYec62_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 39: { 
						// LOOP 0x3be2cb0
						// pre processing
						fYec70_idx = (fYec70_idx+fYec70_idx_save)&33554431;
						fRec56_idx = (fRec56_idx+fRec56_idx_save)&33554431;
						fYec71_idx = (fYec71_idx+fYec71_idx_save)&33554431;
						fRec57_idx = (fRec57_idx+fRec57_idx_save)&33554431;
						fYec72_idx = (fYec72_idx+fYec72_idx_save)&33554431;
						fRec58_idx = (fRec58_idx+fRec58_idx_save)&33554431;
						fYec73_idx = (fYec73_idx+fYec73_idx_save)&33554431;
						fRec59_idx = (fRec59_idx+fRec59_idx_save)&33554431;
						fYec74_idx = (fYec74_idx+fYec74_idx_save)&33554431;
						fRec60_idx = (fRec60_idx+fRec60_idx_save)&33554431;
						fYec75_idx = (fYec75_idx+fYec75_idx_save)&33554431;
						fRec61_idx = (fRec61_idx+fRec61_idx_save)&33554431;
						fYec76_idx = (fYec76_idx+fYec76_idx_save)&33554431;
						fRec62_idx = (fRec62_idx+fRec62_idx_save)&33554431;
						fYec77_idx = (fYec77_idx+fYec77_idx_save)&33554431;
						fRec63_idx = (fRec63_idx+fRec63_idx_save)&33554431;
						// exec code
						for (int i=0; i<count; i++) {
							fYec70[(fYec70_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec56[(fRec56_idx+i-iConst122)&33554431]));
							fRec56[(fRec56_idx+i)&33554431] = fYec70[(fYec70_idx+i-iConst121)&33554431];
							fYec71[(fYec71_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec57[(fRec57_idx+i-iConst124)&33554431]));
							fRec57[(fRec57_idx+i)&33554431] = fYec71[(fYec71_idx+i-iConst123)&33554431];
							fYec72[(fYec72_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec58[(fRec58_idx+i-iConst126)&33554431]));
							fRec58[(fRec58_idx+i)&33554431] = fYec72[(fYec72_idx+i-iConst125)&33554431];
							fYec73[(fYec73_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec59[(fRec59_idx+i-iConst128)&33554431]));
							fRec59[(fRec59_idx+i)&33554431] = fYec73[(fYec73_idx+i-iConst127)&33554431];
							fYec74[(fYec74_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec60[(fRec60_idx+i-iConst130)&33554431]));
							fRec60[(fRec60_idx+i)&33554431] = fYec74[(fYec74_idx+i-iConst129)&33554431];
							fYec75[(fYec75_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec61[(fRec61_idx+i-iConst132)&33554431]));
							fRec61[(fRec61_idx+i)&33554431] = fYec75[(fYec75_idx+i-iConst131)&33554431];
							fYec76[(fYec76_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec62[(fRec62_idx+i-iConst134)&33554431]));
							fRec62[(fRec62_idx+i)&33554431] = fYec76[(fYec76_idx+i-iConst133)&33554431];
							fYec77[(fYec77_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec63[(fRec63_idx+i-iConst136)&33554431]));
							fRec63[(fRec63_idx+i)&33554431] = fYec77[(fYec77_idx+i-iConst135)&33554431];
						}
						// post processing
						fRec63_idx_save = count;
						fYec77_idx_save = count;
						fRec62_idx_save = count;
						fYec76_idx_save = count;
						fRec61_idx_save = count;
						fYec75_idx_save = count;
						fRec60_idx_save = count;
						fYec74_idx_save = count;
						fRec59_idx_save = count;
						fYec73_idx_save = count;
						fRec58_idx_save = count;
						fYec72_idx_save = count;
						fRec57_idx_save = count;
						fYec71_idx_save = count;
						fRec56_idx_save = count;
						fYec70_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 40: { 
						// LOOP 0x3befb80
						// pre processing
						fYec78_idx = (fYec78_idx+fYec78_idx_save)&134217727;
						fRec64_idx = (fRec64_idx+fRec64_idx_save)&134217727;
						fYec79_idx = (fYec79_idx+fYec79_idx_save)&134217727;
						fRec65_idx = (fRec65_idx+fRec65_idx_save)&134217727;
						fYec80_idx = (fYec80_idx+fYec80_idx_save)&134217727;
						fRec66_idx = (fRec66_idx+fRec66_idx_save)&134217727;
						fYec81_idx = (fYec81_idx+fYec81_idx_save)&134217727;
						fRec67_idx = (fRec67_idx+fRec67_idx_save)&134217727;
						fYec82_idx = (fYec82_idx+fYec82_idx_save)&134217727;
						fRec68_idx = (fRec68_idx+fRec68_idx_save)&134217727;
						fYec83_idx = (fYec83_idx+fYec83_idx_save)&134217727;
						fRec69_idx = (fRec69_idx+fRec69_idx_save)&134217727;
						fYec84_idx = (fYec84_idx+fYec84_idx_save)&134217727;
						fRec70_idx = (fRec70_idx+fRec70_idx_save)&134217727;
						fYec85_idx = (fYec85_idx+fYec85_idx_save)&134217727;
						fRec71_idx = (fRec71_idx+fRec71_idx_save)&134217727;
						// exec code
						for (int i=0; i<count; i++) {
							fYec78[(fYec78_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec64[(fRec64_idx+i-iConst139)&134217727]));
							fRec64[(fRec64_idx+i)&134217727] = fYec78[(fYec78_idx+i-iConst138)&134217727];
							fYec79[(fYec79_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec65[(fRec65_idx+i-iConst141)&134217727]));
							fRec65[(fRec65_idx+i)&134217727] = fYec79[(fYec79_idx+i-iConst140)&134217727];
							fYec80[(fYec80_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec66[(fRec66_idx+i-iConst143)&134217727]));
							fRec66[(fRec66_idx+i)&134217727] = fYec80[(fYec80_idx+i-iConst142)&134217727];
							fYec81[(fYec81_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec67[(fRec67_idx+i-iConst145)&134217727]));
							fRec67[(fRec67_idx+i)&134217727] = fYec81[(fYec81_idx+i-iConst144)&134217727];
							fYec82[(fYec82_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec68[(fRec68_idx+i-iConst147)&134217727]));
							fRec68[(fRec68_idx+i)&134217727] = fYec82[(fYec82_idx+i-iConst146)&134217727];
							fYec83[(fYec83_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec69[(fRec69_idx+i-iConst149)&134217727]));
							fRec69[(fRec69_idx+i)&134217727] = fYec83[(fYec83_idx+i-iConst148)&134217727];
							fYec84[(fYec84_idx+i)&134217727] = (fZec15[i] + (fSlow13 * fRec70[(fRec70_idx+i-iConst151)&134217727]));
							fRec70[(fRec70_idx+i)&134217727] = fYec84[(fYec84_idx+i-iConst150)&134217727];
							fYec85[(fYec85_idx+i)&134217727] = (fZec14[i] + (fSlow13 * fRec71[(fRec71_idx+i-iConst153)&134217727]));
							fRec71[(fRec71_idx+i)&134217727] = fYec85[(fYec85_idx+i-iConst152)&134217727];
						}
						// post processing
						fRec71_idx_save = count;
						fYec85_idx_save = count;
						fRec70_idx_save = count;
						fYec84_idx_save = count;
						fRec69_idx_save = count;
						fYec83_idx_save = count;
						fRec68_idx_save = count;
						fYec82_idx_save = count;
						fRec67_idx_save = count;
						fYec81_idx_save = count;
						fRec66_idx_save = count;
						fYec80_idx_save = count;
						fRec65_idx_save = count;
						fYec79_idx_save = count;
						fRec64_idx_save = count;
						fYec78_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 41: { 
						// LOOP 0x3bfca40
						// pre processing
						fYec86_idx = (fYec86_idx+fYec86_idx_save)&268435455;
						fRec72_idx = (fRec72_idx+fRec72_idx_save)&268435455;
						fYec87_idx = (fYec87_idx+fYec87_idx_save)&268435455;
						fRec73_idx = (fRec73_idx+fRec73_idx_save)&268435455;
						fYec88_idx = (fYec88_idx+fYec88_idx_save)&268435455;
						fRec74_idx = (fRec74_idx+fRec74_idx_save)&268435455;
						fYec89_idx = (fYec89_idx+fYec89_idx_save)&268435455;
						fRec75_idx = (fRec75_idx+fRec75_idx_save)&268435455;
						fYec90_idx = (fYec90_idx+fYec90_idx_save)&268435455;
						fRec76_idx = (fRec76_idx+fRec76_idx_save)&268435455;
						fYec91_idx = (fYec91_idx+fYec91_idx_save)&268435455;
						fRec77_idx = (fRec77_idx+fRec77_idx_save)&268435455;
						fYec92_idx = (fYec92_idx+fYec92_idx_save)&268435455;
						fRec78_idx = (fRec78_idx+fRec78_idx_save)&268435455;
						fYec93_idx = (fYec93_idx+fYec93_idx_save)&268435455;
						fRec79_idx = (fRec79_idx+fRec79_idx_save)&268435455;
						// exec code
						for (int i=0; i<count; i++) {
							fYec86[(fYec86_idx+i)&268435455] = (fZec15[i] + (fSlow13 * fRec72[(fRec72_idx+i-iConst156)&268435455]));
							fRec72[(fRec72_idx+i)&268435455] = fYec86[(fYec86_idx+i-iConst155)&268435455];
							fYec87[(fYec87_idx+i)&268435455] = (fZec14[i] + (fSlow13 * fRec73[(fRec73_idx+i-iConst158)&268435455]));
							fRec73[(fRec73_idx+i)&268435455] = fYec87[(fYec87_idx+i-iConst157)&268435455];
							fYec88[(fYec88_idx+i)&268435455] = (fZec15[i] + (fSlow13 * fRec74[(fRec74_idx+i-iConst160)&268435455]));
							fRec74[(fRec74_idx+i)&268435455] = fYec88[(fYec88_idx+i-iConst159)&268435455];
							fYec89[(fYec89_idx+i)&268435455] = (fZec14[i] + (fSlow13 * fRec75[(fRec75_idx+i-iConst162)&268435455]));
							fRec75[(fRec75_idx+i)&268435455] = fYec89[(fYec89_idx+i-iConst161)&268435455];
							fYec90[(fYec90_idx+i)&268435455] = (fZec15[i] + (fSlow13 * fRec76[(fRec76_idx+i-iConst164)&268435455]));
							fRec76[(fRec76_idx+i)&268435455] = fYec90[(fYec90_idx+i-iConst163)&268435455];
							fYec91[(fYec91_idx+i)&268435455] = (fZec14[i] + (fSlow13 * fRec77[(fRec77_idx+i-iConst166)&268435455]));
							fRec77[(fRec77_idx+i)&268435455] = fYec91[(fYec91_idx+i-iConst165)&268435455];
							fYec92[(fYec92_idx+i)&268435455] = (fZec15[i] + (fSlow13 * fRec78[(fRec78_idx+i-iConst168)&268435455]));
							fRec78[(fRec78_idx+i)&268435455] = fYec92[(fYec92_idx+i-iConst167)&268435455];
							fYec93[(fYec93_idx+i)&268435455] = (fZec14[i] + (fSlow13 * fRec79[(fRec79_idx+i-iConst170)&268435455]));
							fRec79[(fRec79_idx+i)&268435455] = fYec93[(fYec93_idx+i-iConst169)&268435455];
						}
						// post processing
						fRec79_idx_save = count;
						fYec93_idx_save = count;
						fRec78_idx_save = count;
						fYec92_idx_save = count;
						fRec77_idx_save = count;
						fYec91_idx_save = count;
						fRec76_idx_save = count;
						fYec90_idx_save = count;
						fRec75_idx_save = count;
						fYec89_idx_save = count;
						fRec74_idx_save = count;
						fYec88_idx_save = count;
						fRec73_idx_save = count;
						fYec87_idx_save = count;
						fRec72_idx_save = count;
						fYec86_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 42: { 
						// LOOP 0x3c09910
						// pre processing
						fYec94_idx = (fYec94_idx+fYec94_idx_save)&8388607;
						fRec80_idx = (fRec80_idx+fRec80_idx_save)&8388607;
						fYec95_idx = (fYec95_idx+fYec95_idx_save)&8388607;
						fRec81_idx = (fRec81_idx+fRec81_idx_save)&8388607;
						fYec96_idx = (fYec96_idx+fYec96_idx_save)&8388607;
						fRec82_idx = (fRec82_idx+fRec82_idx_save)&8388607;
						fYec97_idx = (fYec97_idx+fYec97_idx_save)&8388607;
						fRec83_idx = (fRec83_idx+fRec83_idx_save)&8388607;
						fYec98_idx = (fYec98_idx+fYec98_idx_save)&8388607;
						fRec84_idx = (fRec84_idx+fRec84_idx_save)&8388607;
						fYec99_idx = (fYec99_idx+fYec99_idx_save)&8388607;
						fRec85_idx = (fRec85_idx+fRec85_idx_save)&8388607;
						fYec100_idx = (fYec100_idx+fYec100_idx_save)&8388607;
						fRec86_idx = (fRec86_idx+fRec86_idx_save)&8388607;
						fYec101_idx = (fYec101_idx+fYec101_idx_save)&8388607;
						fRec87_idx = (fRec87_idx+fRec87_idx_save)&8388607;
						// exec code
						for (int i=0; i<count; i++) {
							fYec94[(fYec94_idx+i)&8388607] = (fZec15[i] + (fSlow13 * fRec80[(fRec80_idx+i-iConst173)&8388607]));
							fRec80[(fRec80_idx+i)&8388607] = fYec94[(fYec94_idx+i-iConst172)&8388607];
							fYec95[(fYec95_idx+i)&8388607] = (fZec14[i] + (fSlow13 * fRec81[(fRec81_idx+i-iConst175)&8388607]));
							fRec81[(fRec81_idx+i)&8388607] = fYec95[(fYec95_idx+i-iConst174)&8388607];
							fYec96[(fYec96_idx+i)&8388607] = (fZec15[i] + (fSlow13 * fRec82[(fRec82_idx+i-iConst177)&8388607]));
							fRec82[(fRec82_idx+i)&8388607] = fYec96[(fYec96_idx+i-iConst176)&8388607];
							fYec97[(fYec97_idx+i)&8388607] = (fZec14[i] + (fSlow13 * fRec83[(fRec83_idx+i-iConst179)&8388607]));
							fRec83[(fRec83_idx+i)&8388607] = fYec97[(fYec97_idx+i-iConst178)&8388607];
							fYec98[(fYec98_idx+i)&8388607] = (fZec15[i] + (fSlow13 * fRec84[(fRec84_idx+i-iConst181)&8388607]));
							fRec84[(fRec84_idx+i)&8388607] = fYec98[(fYec98_idx+i-iConst180)&8388607];
							fYec99[(fYec99_idx+i)&8388607] = (fZec14[i] + (fSlow13 * fRec85[(fRec85_idx+i-iConst183)&8388607]));
							fRec85[(fRec85_idx+i)&8388607] = fYec99[(fYec99_idx+i-iConst182)&8388607];
							fYec100[(fYec100_idx+i)&8388607] = (fZec15[i] + (fSlow13 * fRec86[(fRec86_idx+i-iConst185)&8388607]));
							fRec86[(fRec86_idx+i)&8388607] = fYec100[(fYec100_idx+i-iConst184)&8388607];
							fYec101[(fYec101_idx+i)&8388607] = (fZec14[i] + (fSlow13 * fRec87[(fRec87_idx+i-iConst187)&8388607]));
							fRec87[(fRec87_idx+i)&8388607] = fYec101[(fYec101_idx+i-iConst186)&8388607];
						}
						// post processing
						fRec87_idx_save = count;
						fYec101_idx_save = count;
						fRec86_idx_save = count;
						fYec100_idx_save = count;
						fRec85_idx_save = count;
						fYec99_idx_save = count;
						fRec84_idx_save = count;
						fYec98_idx_save = count;
						fRec83_idx_save = count;
						fYec97_idx_save = count;
						fRec82_idx_save = count;
						fYec96_idx_save = count;
						fRec81_idx_save = count;
						fYec95_idx_save = count;
						fRec80_idx_save = count;
						fYec94_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 43: { 
						// LOOP 0x3c167c0
						// pre processing
						fYec102_idx = (fYec102_idx+fYec102_idx_save)&33554431;
						fRec88_idx = (fRec88_idx+fRec88_idx_save)&33554431;
						fYec103_idx = (fYec103_idx+fYec103_idx_save)&33554431;
						fRec89_idx = (fRec89_idx+fRec89_idx_save)&33554431;
						fYec104_idx = (fYec104_idx+fYec104_idx_save)&33554431;
						fRec90_idx = (fRec90_idx+fRec90_idx_save)&33554431;
						fYec105_idx = (fYec105_idx+fYec105_idx_save)&33554431;
						fRec91_idx = (fRec91_idx+fRec91_idx_save)&33554431;
						fYec106_idx = (fYec106_idx+fYec106_idx_save)&33554431;
						fRec92_idx = (fRec92_idx+fRec92_idx_save)&33554431;
						fYec107_idx = (fYec107_idx+fYec107_idx_save)&33554431;
						fRec93_idx = (fRec93_idx+fRec93_idx_save)&33554431;
						fYec108_idx = (fYec108_idx+fYec108_idx_save)&33554431;
						fRec94_idx = (fRec94_idx+fRec94_idx_save)&33554431;
						fYec109_idx = (fYec109_idx+fYec109_idx_save)&33554431;
						fRec95_idx = (fRec95_idx+fRec95_idx_save)&33554431;
						// exec code
						for (int i=0; i<count; i++) {
							fYec102[(fYec102_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec88[(fRec88_idx+i-iConst190)&33554431]));
							fRec88[(fRec88_idx+i)&33554431] = fYec102[(fYec102_idx+i-iConst189)&33554431];
							fYec103[(fYec103_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec89[(fRec89_idx+i-iConst192)&33554431]));
							fRec89[(fRec89_idx+i)&33554431] = fYec103[(fYec103_idx+i-iConst191)&33554431];
							fYec104[(fYec104_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec90[(fRec90_idx+i-iConst194)&33554431]));
							fRec90[(fRec90_idx+i)&33554431] = fYec104[(fYec104_idx+i-iConst193)&33554431];
							fYec105[(fYec105_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec91[(fRec91_idx+i-iConst196)&33554431]));
							fRec91[(fRec91_idx+i)&33554431] = fYec105[(fYec105_idx+i-iConst195)&33554431];
							fYec106[(fYec106_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec92[(fRec92_idx+i-iConst198)&33554431]));
							fRec92[(fRec92_idx+i)&33554431] = fYec106[(fYec106_idx+i-iConst197)&33554431];
							fYec107[(fYec107_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec93[(fRec93_idx+i-iConst200)&33554431]));
							fRec93[(fRec93_idx+i)&33554431] = fYec107[(fYec107_idx+i-iConst199)&33554431];
							fYec108[(fYec108_idx+i)&33554431] = (fZec15[i] + (fSlow13 * fRec94[(fRec94_idx+i-iConst202)&33554431]));
							fRec94[(fRec94_idx+i)&33554431] = fYec108[(fYec108_idx+i-iConst201)&33554431];
							fYec109[(fYec109_idx+i)&33554431] = (fZec14[i] + (fSlow13 * fRec95[(fRec95_idx+i-iConst204)&33554431]));
							fRec95[(fRec95_idx+i)&33554431] = fYec109[(fYec109_idx+i-iConst203)&33554431];
						}
						// post processing
						fRec95_idx_save = count;
						fYec109_idx_save = count;
						fRec94_idx_save = count;
						fYec108_idx_save = count;
						fRec93_idx_save = count;
						fYec107_idx_save = count;
						fRec92_idx_save = count;
						fYec106_idx_save = count;
						fRec91_idx_save = count;
						fYec105_idx_save = count;
						fRec90_idx_save = count;
						fYec104_idx_save = count;
						fRec89_idx_save = count;
						fYec103_idx_save = count;
						fRec88_idx_save = count;
						fYec102_idx_save = count;
						
						tasknum = WORK_STEALING_INDEX;
						fGraph.ActivateOutputTask(taskqueue, 44, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 45, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 46, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 47, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 48, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 49, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 50, tasknum);
						fGraph.ActivateOutputTask(taskqueue, 51, tasknum);
						fGraph.GetReadyTask(taskqueue, tasknum);
						break;
					} 
					case 44: { 
						// LOOP 0x3b6e560
						// exec code
						for (int i=0; i<count; i++) {
							output0[i] = (FAUSTFLOAT)(((((((((((fRec0[(fRec0_idx+i)&16777215] + fRec8[(fRec8_idx+i)&33554431]) + fRec16[(fRec16_idx+i)&67108863]) + fRec24[(fRec24_idx+i)&67108863]) + fRec32[(fRec32_idx+i)&134217727]) + fRec40[(fRec40_idx+i)&134217727]) + fRec48[(fRec48_idx+i)&134217727]) + fRec56[(fRec56_idx+i)&33554431]) + fRec64[(fRec64_idx+i)&134217727]) + fRec72[(fRec72_idx+i)&268435455]) + fRec80[(fRec80_idx+i)&8388607]) + fRec88[(fRec88_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 45: { 
						// LOOP 0x3c250d0
						// exec code
						for (int i=0; i<count; i++) {
							output1[i] = (FAUSTFLOAT)(((((((((((fRec1[(fRec1_idx+i)&16777215] + fRec9[(fRec9_idx+i)&33554431]) + fRec17[(fRec17_idx+i)&67108863]) + fRec25[(fRec25_idx+i)&67108863]) + fRec33[(fRec33_idx+i)&134217727]) + fRec41[(fRec41_idx+i)&134217727]) + fRec49[(fRec49_idx+i)&134217727]) + fRec57[(fRec57_idx+i)&33554431]) + fRec65[(fRec65_idx+i)&134217727]) + fRec73[(fRec73_idx+i)&268435455]) + fRec81[(fRec81_idx+i)&8388607]) + fRec89[(fRec89_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 46: { 
						// LOOP 0x3c29680
						// exec code
						for (int i=0; i<count; i++) {
							output2[i] = (FAUSTFLOAT)(((((((((((fRec2[(fRec2_idx+i)&16777215] + fRec10[(fRec10_idx+i)&33554431]) + fRec18[(fRec18_idx+i)&67108863]) + fRec26[(fRec26_idx+i)&67108863]) + fRec34[(fRec34_idx+i)&134217727]) + fRec42[(fRec42_idx+i)&134217727]) + fRec50[(fRec50_idx+i)&134217727]) + fRec58[(fRec58_idx+i)&33554431]) + fRec66[(fRec66_idx+i)&134217727]) + fRec74[(fRec74_idx+i)&268435455]) + fRec82[(fRec82_idx+i)&8388607]) + fRec90[(fRec90_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 47: { 
						// LOOP 0x3c2cb50
						// exec code
						for (int i=0; i<count; i++) {
							output3[i] = (FAUSTFLOAT)(((((((((((fRec3[(fRec3_idx+i)&16777215] + fRec11[(fRec11_idx+i)&33554431]) + fRec19[(fRec19_idx+i)&67108863]) + fRec27[(fRec27_idx+i)&67108863]) + fRec35[(fRec35_idx+i)&134217727]) + fRec43[(fRec43_idx+i)&134217727]) + fRec51[(fRec51_idx+i)&134217727]) + fRec59[(fRec59_idx+i)&33554431]) + fRec67[(fRec67_idx+i)&134217727]) + fRec75[(fRec75_idx+i)&268435455]) + fRec83[(fRec83_idx+i)&8388607]) + fRec91[(fRec91_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 48: { 
						// LOOP 0x3c300c0
						// exec code
						for (int i=0; i<count; i++) {
							output4[i] = (FAUSTFLOAT)(((((((((((fRec4[(fRec4_idx+i)&16777215] + fRec12[(fRec12_idx+i)&33554431]) + fRec20[(fRec20_idx+i)&67108863]) + fRec28[(fRec28_idx+i)&67108863]) + fRec36[(fRec36_idx+i)&134217727]) + fRec44[(fRec44_idx+i)&134217727]) + fRec52[(fRec52_idx+i)&134217727]) + fRec60[(fRec60_idx+i)&33554431]) + fRec68[(fRec68_idx+i)&134217727]) + fRec76[(fRec76_idx+i)&268435455]) + fRec84[(fRec84_idx+i)&8388607]) + fRec92[(fRec92_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 49: { 
						// LOOP 0x3c33620
						// exec code
						for (int i=0; i<count; i++) {
							output5[i] = (FAUSTFLOAT)(((((((((((fRec5[(fRec5_idx+i)&16777215] + fRec13[(fRec13_idx+i)&33554431]) + fRec21[(fRec21_idx+i)&67108863]) + fRec29[(fRec29_idx+i)&67108863]) + fRec37[(fRec37_idx+i)&134217727]) + fRec45[(fRec45_idx+i)&134217727]) + fRec53[(fRec53_idx+i)&134217727]) + fRec61[(fRec61_idx+i)&33554431]) + fRec69[(fRec69_idx+i)&134217727]) + fRec77[(fRec77_idx+i)&268435455]) + fRec85[(fRec85_idx+i)&8388607]) + fRec93[(fRec93_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 50: { 
						// LOOP 0x3c36b80
						// exec code
						for (int i=0; i<count; i++) {
							output6[i] = (FAUSTFLOAT)(((((((((((fRec6[(fRec6_idx+i)&16777215] + fRec14[(fRec14_idx+i)&33554431]) + fRec22[(fRec22_idx+i)&67108863]) + fRec30[(fRec30_idx+i)&67108863]) + fRec38[(fRec38_idx+i)&134217727]) + fRec46[(fRec46_idx+i)&134217727]) + fRec54[(fRec54_idx+i)&134217727]) + fRec62[(fRec62_idx+i)&33554431]) + fRec70[(fRec70_idx+i)&134217727]) + fRec78[(fRec78_idx+i)&268435455]) + fRec86[(fRec86_idx+i)&8388607]) + fRec94[(fRec94_idx+i)&33554431]);
						}
						
						fGraph.ActivateOneOutputTask(taskqueue, LAST_TASK_INDEX, tasknum);
						break;
					} 
					case 51: { 
						// LOOP 0x3c3a0e0
						// exec code
						for (int i=0; i<count; i++) {
							output7[i] = (FAUSTFLOAT)(((((((((((fRec7[(fRec7_idx+i)&16777215] + fRec15[(fRec15_idx+i)&33554431]) + fRec23[(fRec23_idx+i)&67108863]) + fRec31[(fRec31_idx+i)&67108863]) + fRec39[(fRec39_idx+i)&134217727]) + fRec47[(fRec47_idx+i)&134217727]) + fRec55[(fRec55_idx+i)&134217727]) + fRec63[(fRec63_idx+i)&33554431]) + fRec71[(fRec71_idx+i)&134217727]) + fRec79[(fRec79_idx+i)&268435455]) + fRec87[(fRec87_idx+i)&8388607]) + fRec95[(fRec95_idx+i)&33554431]);
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
    
    QApplication myApp(argc, argv);

	QTGUI* interface = new QTGUI();
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
	
    myApp.setStyleSheet(interface->styleSheet());
    myApp.exec();
    interface->stop();
    
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

