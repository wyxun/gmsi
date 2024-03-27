#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

    volatile bool smEvent;

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
// A timeout \a timeoutTicks has to be specified. Valid values are Clock ticks, BIOS_WAIT_FOREVER or BIOS_NO_WAIT.
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
