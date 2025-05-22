#include <core/input.h>
#include <core/logger.h>


static PlatformStateT* _sgInputPlatformState;
// static InputLayoutT*   _sgInputButtons;


b8 hkInitInput(PlatformStateT* platformState, InputLayoutT* input) {
    HTRACE("input.c -> hkInitInput(PlatformStateT*, InputLayoutT*):b8");

    plInitInput(platformState, input);
    return TRUE;
}

void hkStopInput(InputLayoutT* input) {
    HTRACE("input.c -> hkStopInput(InputLayoutT*):void");
    HDEBUG("hkStopInput(): Stopping input subsystem.");

    plStopInput(_sgInputPlatformState);

    HINFO("Inuput subsytem has been stopped.");
}

void hkInputConfig(InputLayoutT* input) {
    HTRACE("input.c -> hkInputConfig(PlatformStateT*, InputLayoutT*):void");
    plInputConfig(_sgInputPlatformState, input);
}