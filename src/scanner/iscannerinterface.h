#pragma once

#include "iscannertypes.h"

SHARED_PTR(IScannerInterface);
/**
 * Simple scanner interface.
 */
class IScannerInterface
{
public:
  /**
   * Initialize the scanner interface
   */
  virtual bool init() = 0;

  /**
   * Release the scan interface.
   */
  virtual bool exit() = 0;

  /**
   * Access list of devices.
   */
  virtual std::vector<ScannerDeviceDescriptorPtr> getDevices() = 0;

  /**
   * Access the current scanner's possible options
   */
  virtual ScannerCapabilities getCapabilities(ScannerDeviceDescriptorPtr device) = 0;

  /**
   * Retrieve the scanner's current configruation
   */
  virtual ScannerConfiguration getConfiguration(ScannerDeviceDescriptorPtr device) = 0;

  /**
   * Set the scanner's configuration for the next scan operations.
   */
  virtual void setConfiguration(ScannerDeviceDescriptorPtr device, const ScannerConfiguration &configruation) = 0;

  /**
   * Scan an image with the active configuration to a buffer and return it.
   * @return an image buffer with the scanned image
   */
  virtual RawImagePtr scanToBuffer(ScannerDeviceDescriptorPtr device) = 0;
};