#pragma once

#include "iscannerinterface.h"

/**
 * SANE specific implementation of the scanner interface
 */
class SaneScannerInterface : public IScannerInterface
{
public:
  SaneScannerInterface();

  /**
   * Initialize the scanner interface
   */
  virtual bool init();

  /**
   * Release the scan interface.
   */
  virtual bool exit();

  /**
   * Access list of devices.
   */
  virtual std::vector<ScannerDeviceDescriptorPtr> getDevices();

  /**
   * Access the current scanner's possible options
   */
  virtual ScannerCapabilities getCapabilities(ScannerDeviceDescriptorPtr device);

  /**
   * Retrieve the scanner's current configruation
   */
  virtual ScannerConfiguration getConfiguration(ScannerDeviceDescriptorPtr device);

  /**
   * Set the scanner's configuration for the next scan operations.
   */
  virtual void setConfiguration(ScannerDeviceDescriptorPtr device, const ScannerConfiguration &configruation);

  /**
   * Scan an image with the active configuration to a buffer and return it.
   * @return an image buffer with the scanned image
   */
  virtual RawImagePtr scanToBuffer(ScannerDeviceDescriptorPtr device);

  typedef std::map<std::string, unsigned int> OptionMap;

private:
  /**
   * Asserts the a device is opened/resource if ready. 
   * Can be called multiple time.
   */
  void openDevice(ScannerDeviceDescriptorPtr device);

  /**
   * Close/release a device
   */
  void closeDevice(ScannerDeviceDescriptorPtr device);

  /**
   * Get the device information an create a new option map
   */
  const OptionMap &buildOptionMap(ScannerDeviceDescriptorPtr device);

  /**
   * Access the option map for a given device (allows lazy access)
   */
  const OptionMap &getOptionMap(ScannerDeviceDescriptorPtr device);

  /**
   * Keeps track of the opened devices
   */
  std::vector<ScannerDeviceDescriptorPtr> openedDevices;

  /**
   * Option maps for the opened devices
   */
  std::map<ScannerDeviceDescriptorPtr, OptionMap> optionMaps;
};