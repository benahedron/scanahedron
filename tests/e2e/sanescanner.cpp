#include <iostream>

#include "scanner/sanescannerinterface.h"
#include "scanner/scanservice.h"

void dumpCapabilities(const ScannerCapabilities &capabilities)
{
    std::cout << "Capabilties :" << std::endl;
    std::cout << " xrange:" << capabilities.minX << " - " << capabilities.maxX << std::endl;
    std::cout << " yrange:" << capabilities.minY << " - " << capabilities.maxY << std::endl;
}

int main(int argc, char *argv[])
{
    auto interface = IScannerInterfacePtr(new SaneScannerInterface());
    auto scanner = ScanServicePtr(new ScanService(interface));
    auto device = scanner->getAvailableScanners()[0];
    if (device)
    {
        ScannerCapabilities capabilities = scanner->getCapabilities(device);
        dumpCapabilities(capabilities);

        ScannerConfiguration configuration = scanner->getConfiguration(device);
        configuration.resolutionInDPI = capabilities.possibleResolutionsInDPI[0];
        scanner->setConfiguration(device, configuration);

        const std::string path("output.png");
        scanner->scanToFile(device, path);
    }
    return 0;
}