/*! \file synch.cc
//  \brief Routines for synchronizing threads.
//
//      Three kinds of synchronization routines are defined here:
//      semaphores, locks and condition variables.
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation. We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts. While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------
*/

#include "kernel/synch.h"
#include "kernel/msgerror.h"
#include "kernel/scheduler.h"
#include "kernel/system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
/*! 	Initializes a semaphore, so that it can be used for synchronization.
//
// \param debugName is an arbitrary name, useful for debugging only.
// \param initialValue is the initial value of the semaphore.
*/
//----------------------------------------------------------------------
Semaphore::Semaphore(char *debugName, uint32_t initialCount) {
  name = new char[strlen(debugName) + 1];
  strcpy(name, debugName);
  counter = initialCount;
  waiting_queue = new Listint;
  type = SEMAPHORE_TYPE;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
/*! 	De-allocates a semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
*/
//----------------------------------------------------------------------
Semaphore::~Semaphore() {
  type = INVALID_TYPE;
  if (!waiting_queue->IsEmpty()) {
    DEBUG('s',
          (char *) "Destructor of semaphore \"%s\", queue is not empty!!\n",
          name);
    Thread *t = (Thread *) waiting_queue->Remove();
    DEBUG('s', (char *) "Queue contents %s\n", t->GetName());
    waiting_queue->Append((void *) t);
  }
  ASSERT(waiting_queue->IsEmpty());
  delete[] name;
  delete waiting_queue;
}

//----------------------------------------------------------------------
// Semaphore::P
/*!
//      Decrement the value, and wait if it becomes < 0. Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
*/
//----------------------------------------------------------------------
#ifndef ETUDIANTS_TP
void
Semaphore::P() {
  printf("**** Warning: method Semaphore::P is not implemented yet\n");
  exit(ERROR);
}
#endif
#ifdef ETUDIANTS_TP
void Semaphore::P() {
  counter --;
  if (counter < 0){
    IntStatus old_status = g_machine->interrupt->GetStatus(); 
    g_machine->interrupt->SetStatus(INTERRUPTS_OFF);
    this->waiting_queue->Append(g_current_thread);
    g_current_thread->Sleep();
    g_machine->interrupt->SetStatus(old_status);
  }
}
#endif

//----------------------------------------------------------------------
// Semaphore::V
/*! 	Increment semaphore value, waking up a waiting thread if any.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that interrupts
//	are disabled when it is called.
*/
//----------------------------------------------------------------------
#ifndef ETUDIANTS_TP
void
Semaphore::V() {
  printf("**** Warning: method Semaphore::V is not implemented yet\n");
  exit(ERROR);
}
#endif
#ifdef ETUDIANTS_TP
void Semaphore::V() {
  if (counter < 0){
    Thread* thread = (Thread*) waiting_queue->getFirst()->item;
    waiting_queue->Remove();
    g_scheduler->ReadyToRun(thread);
  }
  counter ++;
}
#endif

//----------------------------------------------------------------------
// Lock::Lock
/*! 	Initialize a Lock, so that it can be used for synchronization.
//      The lock is initialy free
//  \param "debugName" is an arbitrary name, useful for debugging.
*/
//----------------------------------------------------------------------
Lock::Lock(char *debugName) {
  name = new char[strlen(debugName) + 1];
  strcpy(name, debugName);
  waiting_queue = new Listint;
  free = true;
  owner = NULL;
  type = LOCK_TYPE;
}

//----------------------------------------------------------------------
// Lock::~Lock
/*! 	De-allocate lock, when no longer needed. Assumes that no thread
//      is waiting on the lock.
*/
//----------------------------------------------------------------------
Lock::~Lock() {
  type = INVALID_TYPE;
  ASSERT(waiting_queue->IsEmpty());
  delete[] name;
  delete waiting_queue;
}

//----------------------------------------------------------------------
// Lock::Acquire
/*! 	Wait until the lock become free.  Checking the
//	state of the lock (free or busy) and modify it must be done
//	atomically, so we need to disable interrupts before checking
//	the value of free.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
*/
//----------------------------------------------------------------------
#ifndef ETUDIANTS_TP
void Lock::Acquire() {
  printf("**** Warning: method Lock::Acquire is not implemented yet\n");
  exit(ERROR);
}
#endif
#ifdef ETUDIANTS_TP
void Lock::Acquire() {
  IntStatus old_status = g_machine->interrupt->GetStatus(); 
  g_machine->interrupt->SetStatus(INTERRUPTS_OFF);

  while (!this->free) {
    waiting_queue->Append(g_current_thread);
    g_current_thread->Sleep();
  }
  owner = g_current_thread;
  free=false;
  g_machine->interrupt->SetStatus(old_status);
} 
#endif
//----------------------------------------------------------------------
// Lock::Release
/*! 	Wake up a waiter if necessary, or release it if no thread is waiting.
//      We check that the lock is held by the g_current_thread.
//	As with Acquire, this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
*/
//----------------------------------------------------------------------
#ifndef ETUDIANTS_TP
void Lock::Release() {
  printf("**** Warning: method Lock::Release is not implemented yet\n");
  exit(ERROR);
}
#endif
#ifdef ETUDIANTS_TP
void Lock::Release()
{
  ASSERT(isHeldByCurrentThread());
  if (waiting_queue->IsEmpty())
  {
    free = true;
    owner = NULL;
    return;
  }

  Thread *thread = (Thread *)waiting_queue->getFirst()->item;
  waiting_queue->Remove();
  g_scheduler->ReadyToRun(thread);
  this->owner = thread;
}
#endif
//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
/*! To check if current thread hold the lock
 */
//----------------------------------------------------------------------
bool
Lock::isHeldByCurrentThread() {
  return (g_current_thread == owner);
}

//----------------------------------------------------------------------
// Condition::Condition
/*! 	Initializes a Condition, so that it can be used for synchronization.
//
//    \param  "debugName" is an arbitrary name, useful for debugging.
*/
//----------------------------------------------------------------------
Condition::Condition(char *debugName) {
  name = new char[strlen(debugName) + 1];
  strcpy(name, debugName);
  waiting_queue = new Listint;
  type = CONDITION_TYPE;
}

//----------------------------------------------------------------------
// Condition::~Condition
/*! 	De-allocate condition, when no longer needed.
//      Assumes that nobody is waiting on the condition.
*/
//----------------------------------------------------------------------
Condition::~Condition() {
  type = INVALID_TYPE;
  ASSERT(waiting_queue->IsEmpty());
  delete[] name;
  delete waiting_queue;
}

//----------------------------------------------------------------------
// Condition::Wait
/*! Block the calling thread (put it in the wait queue).
//  This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
#ifndef ETUDIANTS_TP
void Condition::Wait()
{
  printf("**** Warning: method Condition::Wait is not implemented yet\n");
  exit(ERROR);
}
#endif

#ifdef ETUDIANTS_TP
void Condition::Wait()
{
  IntStatus real_Status = g_machine->interrupt->GetStatus();
  g_machine->interrupt->SetStatus(INTERRUPTS_OFF);

  this->waiting_queue->Append(g_current_thread);
  g_current_thread->Sleep();

  g_machine->interrupt->SetStatus(real_Status);
}
#endif


//----------------------------------------------------------------------
// Condition::Signal
/*! Wake up the first thread of the wait queue (if any).
// This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
void
Condition::Signal() {
  printf("**** Warning: method Condition::Signal is not implemented yet\n");
  exit(ERROR);
}

//----------------------------------------------------------------------
// Condition::Broadcast
/*! Wake up all threads waiting in the waitqueue of the condition
// This operation must be atomic, so we need to disable interrupts.
*/
//----------------------------------------------------------------------
void
Condition::Broadcast() {
  printf("**** Warning: method Condition::Broadcast is not implemented yet\n");
  exit(ERROR);
}
