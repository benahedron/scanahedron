#pragma once

#include "iscannerinterface.h"

SHARED_PTR(ScanService);

/**
 * The scan service allows scanner access through a simple interface.
 */
class ScanService
{
public:
  /**
   * Constructs a scan service with a specific implementation of the scanner interface
   */
  ScanService(IScannerInterfacePtr interface);

  ~ScanService();

  /**
   * Loads the available scanners
   */
  void loadScanners();

  /**
   * Retrieve a list of available scanners
   */
  const std::vector<ScannerDeviceDescriptorPtr> &getAvailableScanners();

  /**
   * Read the scanner capabilities
   */
  ScannerCapabilities getCapabilities(ScannerDeviceDescriptorPtr device);

  /**
   * Read the current scanner configuration
   */
  ScannerConfiguration getConfiguration(ScannerDeviceDescriptorPtr device);

  /**
   * Set the scanner configuration (not nessecary, if automatic settings are good enough)
   */
  void setConfiguration(ScannerDeviceDescriptorPtr device, const ScannerConfiguration &configuration);

  /**
   * Scan an image with the active configuration to a buffer and return it.
   * @return a raw image buffer with the scanned image
   */
  RawImagePtr scanToBuffer(ScannerDeviceDescriptorPtr device);

  /**
   * Scan to the given file (PNG) format. This is merely a wrapper around scan to buffer.
   * @return true, if the file was successfully stored on the disk.
   */
  bool scanToFile(ScannerDeviceDescriptorPtr device, const std::string &destinationPath);

private:
  /**
   * Retrieve the actual scanner, if a nullptr is passed, the defulat resp. first scanner is used.
   */
  ScannerDeviceDescriptorPtr getActualDevice(ScannerDeviceDescriptorPtr device);

  IScannerInterfacePtr interface;

  std::vector<ScannerDeviceDescriptorPtr> availableScanners;
};