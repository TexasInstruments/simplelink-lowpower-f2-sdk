/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ti_StateMachine_include
#define ti_StateMachine_include

/* Kernel Header files */
#include <semaphore.h>

/* Standard C Libraries */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
A simple state machine (SM) module for task-level execution.

This module allows to implement non-hierarchical state machines. States are
implemented as functions and may process events that are posted from interrupt
handlers and driver callbacks. The state machine executes in task context and
all events are dispatched with the same priority as the task.


Usage
=====

1. Draw a UML state chart of your application. Use a simple
   program like plantuml.

2. Declare the state machine and all the states you need in your
   application source file:

   StateMachine_Struct stateMachine;

   StateMachine_DEFINE_STATE(SetupState);
   StateMachine_DEFINE_STATE(RxState);
   StateMachine_DEFINE_STATE(TxState);

3. You might also want to define some events:

   typedef enum {
       PacketReceivedEvent = StateMachine_Event_00,
       TransmitPacketEvent = StateMachine_Event_01
   };

   Events will be processed while executing state handler functions.

4. Define state handler functions for each state in your application
   source file:

   void SetupState_function() { ... }
   void RxStateState_function() { ... }

5. Fill the state handler functions with content. Each state exeutes in 3
   phaes: entry, event handling, exit. A typical state looks like:

   void RxState_function() {
       // Entry: Start RX ...

       // Event handling
       for (;;) {
           StateMachine_EventMask events = StateMachine_pendEvent(&stateMachine, PacketReceivedEvent | TransmitPacketEvent);

           if (events & PacketReceivedEvent) {
               // handle packet
           }
           if (events & TransmitPacketEvent) {
               StateMachine_setNextState(&stateMachine, TxState);
               break;
           }
       }

       // Exit: Leave Rx ... ...
   }

6. Events are posted from hardware interrupt handlers or callbacks:

   void buttonCallbackFunction() {
       StateMachine_postEvent(&stateMachine, TransmitPacketEvent);
   }

7. Finally, setup and start the state machine:

   StateMachine_construct(&stateMachine);
   int exitCode = StateMachine_exec(&stateMachine, SetupState);

 */

#define STATEMACHINE_PEND_BLOCKING      true
#define STATEMACHINE_PEND_NON_BLOCKING  false

typedef void (*StateMachine_StateHandlerFunction)();
typedef StateMachine_StateHandlerFunction StateMachine_State;
typedef uint32_t StateMachine_EventMask;

typedef enum {
    StateMachine_Event00 = 1 << 0,
    StateMachine_Event01 = 1 << 1,
    StateMachine_Event02 = 1 << 2,
    StateMachine_Event03 = 1 << 3,
    StateMachine_Event04 = 1 << 4,
    StateMachine_Event05 = 1 << 5,
    StateMachine_Event06 = 1 << 6,
    StateMachine_Event07 = 1 << 7,
    StateMachine_Event08 = 1 << 8,
    StateMachine_Event09 = 1 << 9,
    StateMachine_Event10 = 1 << 10,
    StateMachine_Event11 = 1 << 11,
    StateMachine_Event12 = 1 << 12,
    StateMachine_Event13 = 1 << 13,
    StateMachine_Event14 = 1 << 14,
    StateMachine_Event15 = 1 << 15,
    StateMachine_Event16 = 1 << 16,
    StateMachine_Event17 = 1 << 17,
    StateMachine_Event18 = 1 << 18,
    StateMachine_Event19 = 1 << 19,
    StateMachine_Event20 = 1 << 20,
    StateMachine_Event21 = 1 << 21,
    StateMachine_Event22 = 1 << 22,
    StateMachine_Event23 = 1 << 23,
    StateMachine_Event24 = 1 << 24,
    StateMachine_Event25 = 1 << 25,
    StateMachine_Event26 = 1 << 26,
    StateMachine_Event27 = 1 << 27,
    StateMachine_Event28 = 1 << 28,
    StateMachine_Event29 = 1 << 29,
    StateMachine_Event_Timeout = 1 << 30,
    StateMachine_Event_Transition = (uint32_t)(1 << 31),
} StateMachine_Event;

typedef enum {
    StateMachine_Exit_Normal = 0,
    StateMachine_Exit_InvalidInitialState = -1,
    StateMachine_Exit_NoStateHandlerFunction = -2
} StateMachine_ExitCode;

/*
Declares a handler function for the state and defines a static pointer to this handler
function. The handler function is named {state}_function and the function pointer is named
\a state.

This macro must be placed into the same source file where the state handler function
is implemented.
*/
#define StateMachine_DECLARE_STATE(state) \
    void state##_function(); \
    static const StateMachine_State state = &state##_function;

typedef struct {
    volatile StateMachine_State currentState;
    volatile StateMachine_State nextState;
    int exitCode;

    uint32_t deferredEvents;
    volatile StateMachine_EventMask deferredEventsMask;
    volatile uint32_t pendingEvents;
    volatile StateMachine_EventMask ignoredEventsMask;
    volatile bool transitionPending;

    sem_t semaphore;

} StateMachine_Struct;

// Can be used to leave the state machine.
extern const StateMachine_State StateMachine_FinalState;

// Initializes a clean state machine in \a object.
void StateMachine_construct(StateMachine_Struct* object);

// Starts the state machine and returns an exit code on completion.
int StateMachine_exec(StateMachine_Struct* machine, StateMachine_State initialState);

// Exits the state machine by a transition to StateMachine_FinalState and with \a exitCode.
void StateMachine_exit(StateMachine_Struct* machine, int32_t exitcode);

// Returns the current state machine state.
void StateMachine_currentState(StateMachine_Struct* machine);

// Subscribe state machine events specified by \a eventmask and return occurred events.
// StateMachine_Event_Transition and StateMachine_Event_Timeout are always subscribed and cannot be masked away.
StateMachine_EventMask StateMachine_pendEvents(StateMachine_Struct* machine, StateMachine_EventMask eventmask, bool blocking);

// Causes a state machine event.
void StateMachine_postEvents(StateMachine_Struct* machine, StateMachine_EventMask eventmask);

// Specifies events that are not handled in this state, but deferred for later execution in the next state.
void StateMachine_setEventsDeferred(StateMachine_Struct* machine, StateMachine_EventMask eventmask);

// Specifies events that are dropped in this state.
void StateMachine_setEventsIgnored(StateMachine_Struct* machine, StateMachine_EventMask eventmask);

// Specifies an exit code.
void StateMachine_setExitCode(StateMachine_Struct* machine, int32_t value);

// Causes a transition to \a state and triggers StateMachine_Event_Transition in the current state.
void StateMachine_setNextState(StateMachine_Struct* machine, StateMachine_State state);

#endif
