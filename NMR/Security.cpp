#include "App.h"


void CSecurity::SafetyExit() {
    memset(App, 0, sizeof(CApp));
    exit(0); memset(GetModuleHandleA(NULL), 0, sizeof(unsigned char) * 10000); TerminateProcess(GetCurrentProcess(), 0); App->Security->NtTerminateProcess(GetCurrentProcess(), 0);   App->Security->NullFunction();
}