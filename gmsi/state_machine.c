#include "state_machine.h"

bool IntMasterDisable(void)
{
    return true;
}

void IntMasterEnable(void)
{

}

void Power_idleFunc(void)
{
    
}
static void StateMachine_FinalState_defaultFunction();

const StateMachine_State StateMachine_FinalState = &StateMachine_FinalState_defaultFunction;
const StateMachine_State InvalidState = NULL;

void StateMachine_FinalState_defaultFunction() {}

static const StateMachine_Struct StateMachine_defaultObject = {
        .currentState = NULL,
        .nextState = NULL,
        .transitionPending = false,
        .exitCode = StateMachine_Exit_Normal,
        .deferredEvents = 0,
        .deferredEventsMask = 0,
        .ignoredEventsMask = 0,
        .pendingEvents = 0,
        .smEvent = false
};

void StateMachine_construct(StateMachine_Struct* machine)
{
    *machine = StateMachine_defaultObject;

    machine->smEvent = false;
}


int StateMachine_exec(StateMachine_Struct* machine, StateMachine_State initialState)
{
    if (initialState == InvalidState)
    {
        return StateMachine_Exit_InvalidInitialState;
    }

    bool previousHwiState = IntMasterDisable();

    machine->nextState = initialState;
    machine->currentState = initialState;
    machine->deferredEventsMask = 0;
    machine->ignoredEventsMask = 0;
    machine->deferredEvents = 0;
    machine->pendingEvents = 0;
    machine->exitCode = 0;
    machine->transitionPending = false;

    for (; machine->currentState != StateMachine_FinalState;)
    {
        // The loop body describes the transition from
        // currentState to nextState.

        // Treat deferred events from previous state as pending events in this state
        machine->pendingEvents = machine->deferredEvents;
        machine->deferredEvents = 0;
        machine->deferredEventsMask = 0;
        machine->ignoredEventsMask = 0;

        machine->currentState = machine->nextState;
        machine->transitionPending = false;

        if (machine->currentState == NULL)
        {
            if (!previousHwiState)
            {
                IntMasterEnable();
            }
            return StateMachine_Exit_NoStateHandlerFunction;
        }

        // Execute the state handler
        IntMasterEnable();
        machine->currentState();
        IntMasterDisable();
    }

    if (!previousHwiState)
    {
        IntMasterEnable();
    }

    return machine->exitCode;
}


void StateMachine_exit(StateMachine_Struct* machine, int32_t exitcode)
{
    StateMachine_setExitCode(machine, exitcode);
    StateMachine_setNextState(machine, StateMachine_FinalState);
}


StateMachine_EventMask StateMachine_pendEvents(StateMachine_Struct* machine, StateMachine_EventMask eventmask, bool blocking)
{
    StateMachine_EventMask effectiveEvents = 0;

    // These events are always subscribed
    eventmask |= StateMachine_Event_Transition | StateMachine_Event_Timeout;

    bool previousHwiState = IntMasterDisable();
    /* true  : Interrupts were already disabled when the function was called. */
    /* false : Interrupts were enabled and are now disabled. */

    // Wait either for timeout or for matching subscribed events
    for (;;)
    {
        /* Check smEvent */
        while ((machine->smEvent == false) && (blocking == true)) {
            IntMasterEnable();
            Power_idleFunc();
            IntMasterDisable();
        }
        machine->smEvent = false;

        if (!blocking)
        {
            machine->pendingEvents |= StateMachine_Event_Timeout;
        }

        effectiveEvents = machine->pendingEvents & eventmask;
        if (effectiveEvents != 0)
        {
            // Consider all returned events not pending anymore.
            machine->pendingEvents &= ~effectiveEvents;
            break;
        }
    }

    if (!previousHwiState)
    {
        IntMasterEnable();
    }

    return effectiveEvents;
}

void StateMachine_postEvents(StateMachine_Struct* machine, uint32_t eventmask)
{
    uint32_t pendingEvents;

    // Ignored events are not handled
    eventmask &= ~machine->ignoredEventsMask;
    if (eventmask != 0)
    {
        bool previousHwiState = IntMasterDisable();
        /* true  : Interrupts were already disabled when the function was called. */
        /* false : Interrupts were enabled and are now disabled. */

        machine->deferredEvents |= eventmask & machine->deferredEventsMask;
        pendingEvents = machine->pendingEvents;
        machine->pendingEvents = pendingEvents | (eventmask & (~machine->deferredEventsMask));


        // Wakeup the waiting thread in StateMachine_pendEvent() or post for later.
        if (eventmask & ~machine->deferredEventsMask)
        {
            machine->smEvent = true;
        }

        if (!previousHwiState)
        {
            IntMasterEnable();
        }
    }
}


void StateMachine_setEventsDeferred(StateMachine_Struct* machine, StateMachine_EventMask eventmask)
{
    // Transition and timeout events cannot be deferred
    eventmask &= ~StateMachine_Event_Transition;

    bool previousHwiState = IntMasterDisable();
    /* true  : Interrupts were already disabled when the function was called. */
    /* false : Interrupts were enabled and are now disabled. */

    {
        // When calling this function multiple times with different values,
        // then there might be already events that are now no longer deferred.
        // Set them pending.
        StateMachine_EventMask raisedEvents = machine->deferredEvents & ~eventmask;
        machine->pendingEvents |= raisedEvents;
        machine->deferredEvents &= eventmask;

        // Keep only events that are not to be deferred
        machine->pendingEvents &= ~eventmask;
        // Remember mask for future events
        machine->deferredEventsMask = eventmask;

        if (raisedEvents != 0)
        {
            machine->smEvent = true;
        }
    }

    if(!previousHwiState)
    {
        IntMasterEnable();
    }
}

void StateMachine_setEventsIgnored(StateMachine_Struct* machine, StateMachine_EventMask eventmask)
{
    // Transition and timeout events cannot be masked
    eventmask &= ~StateMachine_Event_Transition;

    bool previousHwiState = IntMasterDisable();
    {
        // Ignore already pending events
        machine->pendingEvents &= eventmask;
        // Remember event mask for future events
        machine->ignoredEventsMask = eventmask;
    }
    if (!previousHwiState)
    {
        IntMasterEnable();
    }
}

void StateMachine_setExitCode(StateMachine_Struct* machine, int32_t value)
{
    machine->exitCode = value;
}

void StateMachine_setNextState(StateMachine_Struct* machine, StateMachine_State state)
{
    bool previousHwiState = IntMasterDisable();
    {
        // Allow only one transition at a time
        if (!machine->transitionPending)
        {
            machine->transitionPending = true;
            machine->nextState = state;
            StateMachine_postEvents(machine, StateMachine_Event_Transition);
        }
    }
    if (!previousHwiState)
    {
        IntMasterEnable();
    }
}