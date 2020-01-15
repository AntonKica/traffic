#include "Input.h"
#include "GlobalSynchronization.h"

#include <mutex>


Input::Input()
{
}

void Input::initialize()
{
	keyboard.initialize();
	mouse.initialize();
}
void Input::update()
{
	keyboard.updateRecords();
	mouse.updateRecords();
}
