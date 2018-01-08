#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "sysfs_w1.h"

bool operator== (const TSysfsOnewireDevice & first, const TSysfsOnewireDevice & second)
{
    return first.DeviceName == second.DeviceName;
}


TSysfsOnewireDevice::TSysfsOnewireDevice(const string& device_name)
    : DeviceName(device_name)
{
    //FIXME: fill family number
    DeviceId = DeviceName;
    DeviceDir = SysfsOnewireDevicesPath + DeviceName;
}

TSysfsOnewireDevice *TSysfsOnewireDevice::createInstance(const string& device_name)
{
	if (StringStartsWith(device_name, "28-") ||
	    StringStartsWith(device_name, "10-") ||
	    StringStartsWith(device_name, "22-"))
	{
		return new TTemperatureOnewireDevice(device_name);
	}
	if (StringStartsWith(device_name, "2c-"))
	{
		return new TPotentiometerOnewireDevice(device_name);
	}
	return NULL;
}

void TSysfsOnewireManager::RescanBus()
{
}
