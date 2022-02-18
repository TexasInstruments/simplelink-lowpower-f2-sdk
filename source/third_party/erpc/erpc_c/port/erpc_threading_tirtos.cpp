/*
 * Copyright (c) 2018 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define ti_sysbios_knl_Task__internalaccess     /* modify Task.fxn */
#include "erpc_threading.h"
#include <cassert>
#include <errno.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>

#if ERPC_THREADS_IS(TIRTOS)

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Thread::Thread(const char *name)
: m_name(name)
, m_entry(0)
, m_arg(0)
, m_stackSize(0)
, m_priority(0)
, m_task(0)
{
}

Thread::Thread(thread_entry_t entry, uint32_t priority, uint32_t stackSize, const char *name)
: m_name(name)
, m_entry(entry)
, m_arg(0)
, m_stackSize(stackSize)
, m_priority(priority)
, m_task(0)
{
}

Thread::~Thread(void) {}

void Thread::init(thread_entry_t entry, uint32_t priority, uint32_t stackSize)
{
    m_entry = entry;
    m_stackSize = stackSize;
    m_priority = priority;
}

void Thread::start(void *arg)
{
    m_arg = arg;

    Task_Params task_p;
    Task_Params_init(&task_p);
    task_p.arg0 = (UArg)m_entry;
    task_p.arg1 = (UArg)this;
    task_p.stackSize = m_stackSize;
    task_p.priority = -1;

    m_task = Task_create(threadEntryPointStub, &task_p, NULL);
    assert(m_task != NULL);

    /*  Replace task object fxn field with address of startroutine.
     *  This allows all POSIX threads to render in the execution
     *  graph with the startroutine function name instead of
     *  _pthread_runStub.
     */
    m_task->fxn = (Task_FuncPtr)m_entry;

    Task_setPri(m_task, m_priority);
}

bool Thread::operator==(Thread &o)
{
    return m_task == o.m_task;
}

Thread *Thread::getCurrentThread()
{
    return reinterpret_cast<Thread *>(Task_getArg1(Task_self()));
}

void Thread::sleep(uint32_t usecs)
{
    UInt32 timeout;

    /* Clock_tickPeriod is the Clock period in microseconds */
    timeout = (UInt32)((usecs + Clock_tickPeriod - 1) / Clock_tickPeriod);

    /* must add one tick to ensure a full duration of timeout ticks */
    Task_sleep(timeout + 1);
}

void Thread::threadEntryPoint(void)
{
    if (m_entry)
    {
        m_entry(m_arg);
    }
}

void Thread::threadEntryPointStub(UArg arg0, UArg arg1)
{
    Thread *_this = reinterpret_cast<Thread *>(arg1);
    assert(_this != NULL);

    _this->threadEntryPoint();
}

Mutex::Mutex(void)
: m_owner(NULL)
, m_count(0)
{
    Semaphore_Params sem_p;

    Semaphore_Params_init(&sem_p);
    sem_p.mode = Semaphore_Mode_BINARY;

    Semaphore_construct(&m_mutex, 1, NULL);
}

Mutex::~Mutex(void)
{
    Semaphore_destruct(&m_mutex);
}

bool Mutex::tryLock(void)
{
    return acquire_mutex(0);
}

bool Mutex::lock(void)
{
    return acquire_mutex(BIOS_WAIT_FOREVER);
}

bool Mutex::acquire_mutex(UInt32 timeout)
{
    Thread *thisThread = Thread::getCurrentThread();

    if (m_owner == thisThread) {
        m_count++;
        return true;
    }
    
    if (!Semaphore_pend(Semaphore_handle(&m_mutex), 0)) {
        return false;
    }

    /* this thread has acquired ownership of the mutex */
    m_owner = thisThread;
    m_count = 1;
    return true;
}

bool Mutex::unlock(void)
{
    Thread *thisThread = Thread::getCurrentThread();

    if (m_owner != thisThread) {
        return false;
    }

    if (--m_count == 0) {
        m_owner = NULL;
        Semaphore_post(Semaphore_handle(&m_mutex));
    }
    return true;
}

Semaphore::Semaphore(int count)
{
    Semaphore_Params sem_p;

    Semaphore_Params_init(&sem_p);
    sem_p.mode = Semaphore_Mode_COUNTING;

    Semaphore_construct(&m_sem, count, NULL);
}

Semaphore::~Semaphore(void)
{
    Semaphore_destruct(&m_sem);
}

void Semaphore::put(void)
{
    Semaphore_post(Semaphore_handle(&m_sem));
}

bool Semaphore::get(uint32_t usecs)
{
    bool rc;
    UInt32 timeout;

    if (usecs == kWaitForever) {
        timeout = BIOS_WAIT_FOREVER;
    }
    else {
        /* Clock_tickPeriod is the Clock period in microseconds */
        timeout = (UInt32)((usecs + Clock_tickPeriod - 1) / Clock_tickPeriod);

        /* must add one tick to ensure a full duration of timeout ticks */
        timeout += 1;
    }

    rc = Semaphore_pend(Semaphore_handle(&m_sem), timeout);

    return rc;
}

int Semaphore::getCount(void) const
{
    /* must cast away const reference to m_sem */
    Semaphore_Handle sem = Semaphore_handle((Semaphore_Struct *)(&m_sem));
    return static_cast<int>(Semaphore_getCount(sem));
}
#endif /* ERPC_THREADS_IS(TIRTOS) */
