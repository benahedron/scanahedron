#pragma once

#include "utils/types.h"

SHARED_STRUCT_PTR(ScannerDeviceDescriptor);
SHARED_STRUCT_PTR(InternalScannerDevice);

/**
 * Blank base struct for device identification at library level
 */
struct InternalScannerDevice
{
    virtual ~InternalScannerDevice() {}
};

/**
 * Simple Scanner descriptor with a reference to the precise device
 */
struct ScannerDeviceDescriptor
{
    std::string descriptor;
    InternalScannerDevicePtr device;
};

/**
 * Descriptor for the capabilities of the scanner
 * 
 */
struct ScannerCapabilities
{
    double minX = -1;
    double minY = -1;
    double maxX = -1;
    double maxY = -1;
    std::vector<int> possibleResolutionsInDPI;
    std::vector<std::string> possibleSources;
    std::vector<std::string> possibleModes;
};

/**
 * Specific scanner configuration at a given moment of time.
 * Used to set/get the state.
 */
struct ScannerConfiguration
{
    double fromX = -1;
    double fromY = -1;
    double toX = -1;
    double toY = -1;
    int resolutionInDPI = -1;
    std::string source;
    std::string mode;
};