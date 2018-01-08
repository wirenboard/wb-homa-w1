#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "sysfs_w1.h"

TTemperatureOnewireDevice::TTemperatureOnewireDevice(const string& device_name)
    : TSysfsOnewireDevice(device_name)
{
    Family = TOnewireFamilyType::ProgResThermometer;
    DeviceMQTTParams.push_back("/meta/type");
    DeviceMQTTParams.push_back("temperature");
}

TMaybe<float> TTemperatureOnewireDevice::Read() const
{
    std::string data;
    bool bFoundCrcOk=false;

    static const std::string tag("t=");

    std::ifstream file;
    std::string fileName=DeviceDir +"/w1_slave";
    file.open(fileName.c_str());
    if (file.is_open()) {
        std::string sLine;
        while (!file.eof()) {
            getline(file, sLine);
            size_t tpos;
            if (sLine.find("crc=")!=std::string::npos) {
                if (sLine.find("YES")!=std::string::npos) {
                    bFoundCrcOk=true;
                }
            } else if ((tpos=sLine.find(tag))!=std::string::npos) {
                data = sLine.substr(tpos+tag.length());
            }
        }
        file.close();
    }


    if (bFoundCrcOk) {
        int data_int = std::stoi(data);

        if (data_int == 85000) {
            // wrong read
            return NotDefinedMaybe;
        }

        if (data_int == 127937) {
            // returned max possible temp, probably an error
            // (it happens for chineese clones)
            return NotDefinedMaybe;
        }


        return (float) data_int/1000.0f; // Temperature given by kernel is in thousandths of degrees
    }

    return NotDefinedMaybe;
}
