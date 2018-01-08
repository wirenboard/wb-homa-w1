#pragma once
#include <string>
#include <map>
#include <wbmqtt/utils.h>

using namespace std;

const string SysfsOnewireDevicesPath = "/sys/bus/w1/devices/";

enum class TOnewireFamilyType
{
    ProgResThermometer = 0x28,
    ProgResPotentiometer = 0x2c,
    Unknown = 0x00,
};

class TSysfsOnewireDevice
{
public:
    TSysfsOnewireDevice(const string& device_name);

    inline TOnewireFamilyType GetDeviceFamily() const {return Family;};
    inline const string & GetDeviceId() const {return DeviceId;};

    virtual TMaybe<float> Read() const {return NotDefinedMaybe; }
    virtual TMaybe<float> Write(float val) const {return NotDefinedMaybe; }

    static TSysfsOnewireDevice *createInstance(const string& device_name);
    inline vector<string> getDeviceMQTTParams() const {return DeviceMQTTParams; }

    friend bool operator== (const TSysfsOnewireDevice & first, const TSysfsOnewireDevice & second);
protected:
    string DeviceName;
    TOnewireFamilyType Family;
    string DeviceId;
    string DeviceDir;
    vector<string> DeviceMQTTParams;
};

class TTemperatureOnewireDevice : public TSysfsOnewireDevice {
public:
	TTemperatureOnewireDevice(const string& device_name);
	TMaybe<float> Read() const;
	vector<string> getDeviceMQTTParams();
};

class TPotentiometerOnewireDevice : public TSysfsOnewireDevice {
public:
    const uint8_t CMD_READ_POSITION = 0xF0;
    const uint8_t CMD_WRITE_POSITION = 0x0F;
    const uint8_t CMD_WRITE_CONTROL_REGISTER = 0x55;
    const uint8_t CMD_READ_CONTROL_REGISTER = 0xAA;
    const uint8_t CMD_RELEASE = 0x96;
    const uint8_t CMD_INCREMENT = 0xC3;
    const uint8_t CMD_DECREMENT = 0x99;
    const float SINGLE_STEP_RESISTENCE = 100000/255;

    TPotentiometerOnewireDevice(const string& device_name);
    TMaybe<float> Read() const;
    TMaybe<float> Write(float val) const;
    vector<string> getDeviceMQTTParams();
};

class TSysfsOnewireManager
{
public:
    TSysfsOnewireManager()  {};

    void RescanBus();


private:
    // FIXME: once found, device will be kept indefinetely
    // consider some kind of ref-counting smart vectors instead

    map<const string, TSysfsOnewireDevice> Devices;

};


class TOneWireReadErrorException : public std::exception
{
public:
   TOneWireReadErrorException(const std::string& deviceFileName) : Message("1-Wire system : error reading value from ") {Message.append(deviceFileName);}
   virtual ~TOneWireReadErrorException() throw() {}
   virtual const char* what() const throw() {return Message.c_str();}
protected:
   std::string Message;
};

class TOneWireWriteErrorException : public std::exception
{
public:
   TOneWireWriteErrorException(const std::string& deviceFileName) : Message("1-Wire system : error writing value from ") {Message.append(deviceFileName);}
   virtual ~TOneWireWriteErrorException() throw() {}
   virtual const char* what() const throw() {return Message.c_str();}
protected:
   std::string Message;
};
