#include "state_manager.h"
#include <stdio.h>

static SystemState current_state = STATE_BOOTING;

void set_system_state(SystemState state) { 
    current_state = state; 
    printf("[STATE Driver] System State Changed to %d\n", current_state);
}
SystemState get_system_state() { 
    return current_state; 
}