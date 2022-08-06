#include <RemoteControlInputs.h>

RemoteControlInputSet::RemoteControlInputSet():InputSet(INPUT_COUNT),
powerBtn(POWER_BUTTON_SENSE_PIN,1,100)
{
    powerBtn.setStateValue(1,3800);
}