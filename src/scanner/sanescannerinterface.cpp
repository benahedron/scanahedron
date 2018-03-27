#include "sanescannerinterface.h"
#include <sane/sane.h>
#include <sane/saneopts.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cmath>

#define SANE_SANITY(saneStatus)                                                                                                                             \
    if (saneStatus != SANE_STATUS_GOOD)                                                                                                                     \
    {                                                                                                                                                       \
        std::stringstream stream;                                                                                                                           \
        stream << "Sane operation failed: . (code:" << saneStatus << " = " << sane_strstatus(saneStatus) << ", at: " << __FILE__ << ":" << __LINE__ << ")"; \
        std::cout << stream.str() << std::endl;                                                                                                             \
        std::runtime_error(stream.str());                                                                                                                   \
    }

SHARED_STRUCT_PTR(SaneInternalScannerDevice);
struct SaneInternalScannerDevice : InternalScannerDevice
{
    const SANE_Device *device;
    SANE_Handle handle;

    virtual ~SaneInternalScannerDevice(){};
};

namespace
{
void authCallback(SANE_String_Const resource,
                  SANE_Char username[SANE_MAX_USERNAME_LEN],
                  SANE_Char password[SANE_MAX_PASSWORD_LEN])
{
    throw std::runtime_error("Not implemented.");
}

const unsigned int SANE_BUFFER_SIZE = 1024 * 1024 * 8; // 1MB
SANE_Byte buffer[SANE_BUFFER_SIZE];

const unsigned int SCANE_NAME_BUFFER_SIZE = 128;
SANE_Char stringValueBuffer[SCANE_NAME_BUFFER_SIZE];
}

SaneScannerInterface::SaneScannerInterface()
{
}

bool SaneScannerInterface::init()
{
    SANE_Int version_code = 0;
    sane_init(&version_code, authCallback);
    return true;
}

bool SaneScannerInterface::exit()
{
    for (auto device : openedDevices)
    {
        closeDevice(device);
    }
    openedDevices.clear();
    sane_exit();
    return true;
}

std::vector<ScannerDeviceDescriptorPtr> SaneScannerInterface::getDevices()
{
    std::vector<ScannerDeviceDescriptorPtr> result;
    const SANE_Device **deviceList;
    SANE_Status saneStatus = SANE_STATUS_GOOD;
    saneStatus = sane_get_devices(&deviceList, SANE_FALSE);
    int it = 0;

    if (saneStatus == SANE_STATUS_GOOD)
    {
        do
        {
            std::ostringstream stream;
            stream << deviceList[it]->name << " (" << deviceList[it]->vendor << " / " << deviceList[it]->model << ")";

            SaneInternalScannerDevicePtr saneDevice = SaneInternalScannerDevicePtr(new SaneInternalScannerDevice());
            saneDevice->device = deviceList[it];
            saneDevice->handle = 0;

            ScannerDeviceDescriptorPtr scanner(new ScannerDeviceDescriptor());
            scanner->descriptor = stream.str();
            scanner->device = saneDevice;
            result.push_back(scanner);
            it++;
        } while (deviceList[it] != nullptr);
    }
    return result;
}

const SaneScannerInterface::OptionMap &SaneScannerInterface::buildOptionMap(ScannerDeviceDescriptorPtr device)
{
    OptionMap map;
    openDevice(device);

    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    SANE_Handle handle = internalDevice->handle;
    SANE_Int numDevOptions;
    sane_control_option(handle, 0, SANE_ACTION_GET_VALUE, &numDevOptions, 0);

    std::vector<std::string> neededOptions;
    neededOptions.push_back(SANE_NAME_SCAN_TL_X);
    neededOptions.push_back(SANE_NAME_SCAN_TL_Y);
    neededOptions.push_back(SANE_NAME_SCAN_BR_X);
    neededOptions.push_back(SANE_NAME_SCAN_BR_Y);
    neededOptions.push_back(SANE_NAME_SCAN_RESOLUTION);
    neededOptions.push_back(SANE_NAME_SCAN_SOURCE);
    neededOptions.push_back(SANE_NAME_SCAN_MODE);

    for (auto i = 0; i < numDevOptions; ++i)
    {
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, i);
        if (!option->name)
        {
            continue;
        }
        std::string name = std::string(option->name);
        if (std::find(neededOptions.begin(), neededOptions.end(), name) != neededOptions.end())
        {
            map[name] = i;
        }
    }
    if (map.size() != neededOptions.size())
    {
        throw std::runtime_error("Not all required scanner options were found. Likely your device is not supported.");
    }
    optionMaps[device] = map;
    return optionMaps[device];
}

const SaneScannerInterface::OptionMap &SaneScannerInterface::getOptionMap(ScannerDeviceDescriptorPtr device)
{
    auto result = optionMaps.find(device);
    if (result == optionMaps.end())
    {
        return buildOptionMap(device);
    }
    else
    {
        return result->second;
    }
}

ScannerCapabilities SaneScannerInterface::getCapabilities(ScannerDeviceDescriptorPtr device)
{
    ScannerCapabilities capabilities;
    openDevice(device);

    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    SANE_Handle handle = internalDevice->handle;

    SANE_Int info;
    SANE_Status saneStatus;

    const OptionMap &options = getOptionMap(device);
    {
        int index = options.at(SANE_NAME_SCAN_TL_X);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        capabilities.minX = SANE_UNFIX(option->constraint.range->min);
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_X);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        capabilities.maxX = SANE_UNFIX(option->constraint.range->max);
    }
    {
        int index = options.at(SANE_NAME_SCAN_TL_Y);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        capabilities.minY = SANE_UNFIX(option->constraint.range->min);
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_Y);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        capabilities.maxY = SANE_UNFIX(option->constraint.range->max);
    }
    {
        int index = options.at(SANE_NAME_SCAN_RESOLUTION);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        auto list = option->constraint.word_list;
        int length = *list++;
        for (int i = 0; i < length; i++)
            capabilities.possibleResolutionsInDPI.push_back(list[i]);
    }
    {
        int index = options.at(SANE_NAME_SCAN_SOURCE);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        for (int i = 0; option->constraint.string_list[i] != nullptr; ++i)
        {
            capabilities.possibleSources.push_back(option->constraint.string_list[i]);
        }
    }
    {
        int index = options.at(SANE_NAME_SCAN_MODE);
        const SANE_Option_Descriptor *option = sane_get_option_descriptor(handle, index);
        for (int i = 0; option->constraint.string_list[i] != nullptr; ++i)
        {
            capabilities.possibleModes.push_back(option->constraint.string_list[i]);
        }
    }
    return capabilities;
}

ScannerConfiguration SaneScannerInterface::getConfiguration(ScannerDeviceDescriptorPtr device)
{
    ScannerConfiguration configuration;
    openDevice(device);

    SANE_Int info;
    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    SANE_Handle handle = internalDevice->handle;

    const OptionMap &options = getOptionMap(device);
    {
        int index = options.at(SANE_NAME_SCAN_TL_X);
        SANE_Fixed fixedValue = 0;
        sane_control_option(handle, index, SANE_ACTION_GET_VALUE, &fixedValue, &info);
        configuration.fromX = SANE_UNFIX(fixedValue);
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_X);
        SANE_Fixed fixedValue = 0;
        sane_control_option(handle, index, SANE_ACTION_GET_VALUE, &fixedValue, &info);
        configuration.toX = SANE_UNFIX(fixedValue);
    }
    {
        int index = options.at(SANE_NAME_SCAN_TL_Y);
        SANE_Fixed fixedValue = 0;
        sane_control_option(handle, index, SANE_ACTION_GET_VALUE, &fixedValue, &info);
        configuration.fromY = SANE_UNFIX(fixedValue);
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_Y);
        SANE_Fixed fixedValue = 0;
        sane_control_option(handle, index, SANE_ACTION_GET_VALUE, &fixedValue, &info);
        configuration.toY = SANE_UNFIX(fixedValue);
    }
    {
        int index = options.at(SANE_NAME_SCAN_RESOLUTION);
        SANE_Int intValue;
        sane_control_option(handle, index, SANE_ACTION_GET_VALUE, &intValue, &info);
        configuration.resolutionInDPI = intValue;
    }
    {
        int index = options.at(SANE_NAME_SCAN_SOURCE);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_GET_VALUE, stringValueBuffer, &info));
        configuration.source = std::string(stringValueBuffer);
    }
    {
        int index = options.at(SANE_NAME_SCAN_MODE);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_GET_VALUE, stringValueBuffer, &info));
        configuration.mode = std::string(stringValueBuffer);
    }

    return configuration;
}

void SaneScannerInterface::setConfiguration(ScannerDeviceDescriptorPtr device, const ScannerConfiguration &configuration)
{
    openDevice(device);

    SANE_Int info;
    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    SANE_Handle handle = internalDevice->handle;

    const OptionMap &options = getOptionMap(device);
    {
        int index = options.at(SANE_NAME_SCAN_TL_X);
        SANE_Fixed fixedValue = SANE_FIX(configuration.fromX);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, &fixedValue, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_X);
        SANE_Fixed fixedValue = SANE_FIX(configuration.toX);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, &fixedValue, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_TL_Y);
        SANE_Fixed fixedValue = SANE_FIX(configuration.fromY);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, &fixedValue, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_BR_Y);
        SANE_Fixed fixedValue = SANE_FIX(configuration.toY);
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, &fixedValue, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_RESOLUTION);
        SANE_Int intValue = configuration.resolutionInDPI;
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, &intValue, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_SOURCE);
        assert(configuration.source.length() <= SCANE_NAME_BUFFER_SIZE);
        std::memset(stringValueBuffer, 0, SCANE_NAME_BUFFER_SIZE);
        std::strncpy(stringValueBuffer, configuration.source.c_str(), configuration.source.length());
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, stringValueBuffer, &info));
    }
    {
        int index = options.at(SANE_NAME_SCAN_MODE);
        assert(configuration.mode.length() <= SCANE_NAME_BUFFER_SIZE);
        std::memset(stringValueBuffer, 0, SCANE_NAME_BUFFER_SIZE);
        std::strncpy(stringValueBuffer, configuration.mode.c_str(), configuration.mode.length());
        SANE_SANITY(sane_control_option(handle, index, SANE_ACTION_SET_VALUE, stringValueBuffer, &info));
    }
}

RawImagePtr SaneScannerInterface::scanToBuffer(ScannerDeviceDescriptorPtr device)
{
    openDevice(device);

    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    SANE_Handle handle = internalDevice->handle;

    SANE_Int usedBuffer = 0;
    SANE_Int saneStatus = SANE_STATUS_GOOD;

    SANE_SANITY(sane_start(internalDevice->handle));

    SANE_Parameters params;
    sane_get_parameters(handle, &params);

    unsigned int width = params.pixels_per_line;
    unsigned int height = params.lines;

    RawImagePtr rawImage(new RawImage(width, height));

    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int total = 0;
    while (true)
    {
        saneStatus = sane_read(internalDevice->handle, buffer, SANE_BUFFER_SIZE, &usedBuffer);
        if (saneStatus != SANE_STATUS_GOOD)
        {
            break;
        }
        unsigned int offset = 0;
        unsigned char *pivot = buffer;
        while (offset < usedBuffer && y < height)
        {
            for (unsigned int j = 0; j < 3; ++j)
                rawImage->pixels[(y * width + x) * 3 + j] = *pivot++;

            x = (x + 1) % width;
            if (x == 0)
            {
                y++;
            }

            offset += 3;
        }
        total += usedBuffer;
    }
    sane_cancel(internalDevice->handle);
    return rawImage;
}

void SaneScannerInterface::openDevice(ScannerDeviceDescriptorPtr device)
{
    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    if (internalDevice->handle == 0)
    {
        SANE_SANITY(sane_open(internalDevice->device->name, &internalDevice->handle));
        openedDevices.push_back(device);
    }
}

void SaneScannerInterface::closeDevice(ScannerDeviceDescriptorPtr device)
{
    SaneInternalScannerDevicePtr internalDevice = std::dynamic_pointer_cast<SaneInternalScannerDevice>(device->device);
    if (internalDevice->handle == 0)
    {
        sane_close(&internalDevice->handle);
        internalDevice->handle = 0;
    }
}
