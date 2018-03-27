#include "scanservice.h"
#include "iscannerinterface.h"

#include <png.hpp>
#include <iostream>

ScanService::ScanService(IScannerInterfacePtr interface_)
    : interface(interface_)
{
    if (!interface)
    {
        throw std::runtime_error("No scanner interface given!");
    }
    if (!interface->init())
    {
        throw std::runtime_error("Could not initialize scanner interface!");
    }
}

ScanService::~ScanService()
{
    if (interface)
    {
        interface->exit();
    }
}

void ScanService::loadScanners()
{
    availableScanners = interface->getDevices();
}

const std::vector<ScannerDeviceDescriptorPtr> &ScanService::getAvailableScanners()
{
    if (availableScanners.empty())
    {
        loadScanners();
    }
    return availableScanners;
}

ScannerDeviceDescriptorPtr ScanService::getActualDevice(ScannerDeviceDescriptorPtr device)
{
    if (device == nullptr)
    {
        const auto &scanners = getAvailableScanners();
        if (scanners.empty())
        {
            throw std::runtime_error("No scanner found!");
        }
        return scanners[0];
    }
    return device;
}

ScannerCapabilities ScanService::getCapabilities(ScannerDeviceDescriptorPtr device)
{
    ScannerDeviceDescriptorPtr actualDevice = getActualDevice(device);
    return interface->getCapabilities(actualDevice);
}

ScannerConfiguration ScanService::getConfiguration(ScannerDeviceDescriptorPtr device)
{
    ScannerDeviceDescriptorPtr actualDevice = getActualDevice(device);
    return interface->getConfiguration(actualDevice);
}

void ScanService::setConfiguration(ScannerDeviceDescriptorPtr device, const ScannerConfiguration &configuration)
{
    ScannerDeviceDescriptorPtr actualDevice = getActualDevice(device);
    interface->setConfiguration(actualDevice, configuration);
}

RawImagePtr ScanService::scanToBuffer(ScannerDeviceDescriptorPtr device)
{
    ScannerDeviceDescriptorPtr actualDevice = getActualDevice(device);
    return interface->scanToBuffer(actualDevice);
}

bool ScanService::scanToFile(ScannerDeviceDescriptorPtr device, const std::string &destinationPath)
{
    RawImagePtr buffer = scanToBuffer(device);

    if (buffer == nullptr)
    {
        return false;
    }

    png::image<png::rgb_pixel> image(buffer->width, buffer->height);
    unsigned char *pivot = buffer->pixels;
    for (int y = 0; y < buffer->height; ++y)
    {
        for (int x = 0; x < buffer->width; ++x)
        {
            if (buffer->bytesPerPixel == 3)
            {
                image[y][x] = png::rgb_pixel(*pivot++, *pivot++, *pivot++);
            }
            else if (buffer->bytesPerPixel == 1)
            {
                image[y][x] = png::rgb_pixel(*pivot, *pivot, *pivot++);
            }
        }
    }

    image.write(destinationPath);
    return true;
}
