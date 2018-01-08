/*
 * === DS2890 ===
 * Based on datasheet
 * useful: https://www.kernel.org/doc/Documentation/w1/w1.generic
 */
#include <fstream>
#include <string>
#include "sysfs_w1.h"

struct position_read_result { uint8_t config, value; };

TPotentiometerOnewireDevice::TPotentiometerOnewireDevice(const string& device_name)
    : TSysfsOnewireDevice(device_name)
{
    Family = TOnewireFamilyType::ProgResPotentiometer;
    DeviceMQTTParams.push_back("/meta/type");
    DeviceMQTTParams.push_back("range");
    DeviceMQTTParams.push_back("/meta/order");
    DeviceMQTTParams.push_back("2");
    DeviceMQTTParams.push_back("/meta/max");
    // TODO this value should be red from the chip
    DeviceMQTTParams.push_back("100000");
}

TMaybe<float> TPotentiometerOnewireDevice::Read() const
{
    std::string fileName=DeviceDir +"/rw";
    std::fstream file;
    file.rdbuf()->pubsetbuf(0, 0);
    file.open(fileName.c_str());
    position_read_result result;
    if (file.is_open()) {
        uint8_t buffer = CMD_READ_POSITION;
        file.write((char *) &buffer, 1);
        file.read((char *) &result, 2); // first byte - control register, second byte - desired value
        file.close();
    }

    return SINGLE_STEP_RESISTENCE * result.value;
}

/*
 * By datasheet write sequence is following:
 * 1) write CMD_WRITE_POSITION
 * 2) write value
 * 3) read saved value
 * 4) write CMD_RELEASE to sumbit written value is correct
 *
 * Meanwhile in w1 linux driver (https://github.com/torvalds/linux/blob/master/drivers/w1/w1.c) in function rw_write
 * before each write attempt reset is made. That makes it impossible to submit written value.
 *
 * In this implementation INC and DEC are used to set required value.
 */
TMaybe<float> TPotentiometerOnewireDevice::Write(float val) const
{
    std::string fileName=DeviceDir +"/rw";
    std::fstream file;
    file.rdbuf()->pubsetbuf(0, 0);
    file.open(fileName.c_str());

    // first read current value
    uint8_t cmd_read = CMD_READ_POSITION;
    position_read_result read_result;
    file.write((char *) &cmd_read, 1);
    file.read((char *) &read_result, 2); // first byte - control register, second byte - desired value

    uint8_t int_val = val / SINGLE_STEP_RESISTENCE + 0.5;
    int delta = int_val - read_result.value;

    if (delta != 0 && file.is_open()) {
        uint8_t cmd_step = delta > 0 ? CMD_INCREMENT : CMD_DECREMENT;
        uint8_t step_result = read_result.value;
        if (delta < 0) {
            delta = -delta;
        }
        for (auto i = 1; i <= 300; i++) {
            // 300 iterations max, not 255 because there may be errors on chip update
            file.write((char *) &cmd_step, 1);
            file.read((char *) &step_result, 1);
            if (step_result == int_val) {
                // exit when value match
                break;
            }
        }

        file.close();
    }

    return SINGLE_STEP_RESISTENCE * int_val;
}
