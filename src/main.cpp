#ifndef NO_NODE
#include <node.h>
#include <iostream>
#include "scanner/sanescannerinterface.h"
#include "scanner/scanservice.h"

namespace carbonpaper
{

using v8::Array;
using v8::Boolean;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Uint8Array;
using v8::Value;

ScanServicePtr scanService;

/**
 * Get a device descriptor via the device's name
 */
ScannerDeviceDescriptorPtr getDeviceByName(const std::string &deviceName)
{
  for (const auto &device : scanService->getAvailableScanners())
  {
    if (deviceName == device->descriptor)
    {
      return device;
    }
  }
  return nullptr;
}

/**
 * Access the list of existing scanners
 */
void getScanners(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Local<Array> deviceNames = Array::New(isolate);

  unsigned int i = 0;
  for (const auto &device : scanService->getAvailableScanners())
  {
    deviceNames->Set(i++, String::NewFromUtf8(isolate, device->descriptor.c_str()));
  }

  args.GetReturnValue().Set(deviceNames);
}

/**
 * Get a scanner descriptor out of the data in the given argument (device name)
 */
ScannerDeviceDescriptorPtr getDeviceDescriptor(Isolate *isolate, Local<Value> argument)
{

  ScannerDeviceDescriptorPtr usedDevice = nullptr;
  if (!argument->IsNull())
  {
    v8::String::Utf8Value paramDeviceName(argument);
    std::string deviceName = std::string(*paramDeviceName);
    usedDevice = getDeviceByName(deviceName);
    if (usedDevice == nullptr)
    {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Scanner with the given name not found!")));
      return nullptr;
    }
  }
  return usedDevice;
}
/**
 * Get the complete scanner capabilities for a given scanner.
 * 
 * Expects javascript arguments: 
 *  - deviceName (string)
 * 
 * The result is a dict with the following data:
 * - minX (in mm)
 * - minY (in mm)
 * - maxX (in mm)
 * - maxY (in mm)
 * - possibleResolutionsInDPI, list of resolutions that the scanner supports.
 * - possibleSources, list of scan sources that the scanner supports (e.g. feeder).
 * - possibleModes, list of scan modes that the scanner supports.
 */
void getCapabilities(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  ScannerDeviceDescriptorPtr usedDevice = getDeviceDescriptor(isolate, args[0]);
  if (usedDevice == nullptr)
  {
    return;
  }

  ScannerCapabilities capabilities = scanService->getCapabilities(usedDevice);

  Local<Object> obj = Object::New(isolate);
  obj->Set(String::NewFromUtf8(isolate, "minX"), Number::New(isolate, capabilities.minX));
  obj->Set(String::NewFromUtf8(isolate, "minY"), Number::New(isolate, capabilities.minY));
  obj->Set(String::NewFromUtf8(isolate, "maxX"), Number::New(isolate, capabilities.maxX));
  obj->Set(String::NewFromUtf8(isolate, "maxY"), Number::New(isolate, capabilities.maxY));

  Local<Array> resolutions = Array::New(isolate);
  unsigned int i = 0;
  for (const auto &resolution : capabilities.possibleResolutionsInDPI)
  {
    resolutions->Set(i++, Number::New(isolate, resolution));
  }
  obj->Set(String::NewFromUtf8(isolate, "possibleResolutionsInDPI"), resolutions);

  Local<Array> sources = Array::New(isolate);
  i = 0;
  for (const auto &source : capabilities.possibleSources)
  {
    sources->Set(i++, String::NewFromUtf8(isolate, source.c_str()));
  }
  obj->Set(String::NewFromUtf8(isolate, "possibleSources"), sources);

  Local<Array> modes = Array::New(isolate);
  i = 0;
  for (const auto &mode : capabilities.possibleModes)
  {
    modes->Set(i++, String::NewFromUtf8(isolate, mode.c_str()));
  }
  obj->Set(String::NewFromUtf8(isolate, "possibleModes"), modes);

  args.GetReturnValue().Set(obj);
}

/**
 * Get the current scanner configuration for a given scanner.
 * 
 * Expects javascript arguments: 
 *  - deviceName (string)
 * 
 * The result is a dict with the following data:
 * - fromX (in mm)
 * - fromY (in mm)
 * - toX (in mm)
 * - toY (in mm)
 * - resolutionInDPI
 * - source
 * - mode
 */
void getConfiguration(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  ScannerDeviceDescriptorPtr usedDevice = getDeviceDescriptor(isolate, args[0]);
  if (usedDevice == nullptr)
  {
    return;
  }

  ScannerConfiguration configuration = scanService->getConfiguration(usedDevice);

  Local<Object> obj = Object::New(isolate);
  obj->Set(String::NewFromUtf8(isolate, "fromX"), Number::New(isolate, configuration.fromX));
  obj->Set(String::NewFromUtf8(isolate, "fromY"), Number::New(isolate, configuration.fromY));
  obj->Set(String::NewFromUtf8(isolate, "toX"), Number::New(isolate, configuration.toX));
  obj->Set(String::NewFromUtf8(isolate, "toY"), Number::New(isolate, configuration.toY));
  obj->Set(String::NewFromUtf8(isolate, "resolutionInDPI"), Number::New(isolate, configuration.resolutionInDPI));
  obj->Set(String::NewFromUtf8(isolate, "source"), String::NewFromUtf8(isolate, configuration.source.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "mode"), String::NewFromUtf8(isolate, configuration.mode.c_str()));
  args.GetReturnValue().Set(obj);
}

/**
 * Set the current scanner configuration for a given scanner.
 * 
 * Expects javascript arguments: 
 *  - deviceName (string)
 *  - configuration dict with the structure:
 *     - fromX (in mm)
 *     - fromY (in mm)
 *     - toX (in mm)
 *     - toY (in mm)
 *     - resolutionInDPI
 *     - source
 *     - mode
 */
void setConfiguration(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  ScannerDeviceDescriptorPtr usedDevice = getDeviceDescriptor(isolate, args[0]);
  if (usedDevice == nullptr)
  {
    return;
  }

  if (!args[1]->IsObject())
  {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Expecting: setConfiguratoin(deviceName:string, configuration:object)")));
    return;
  }

  ScannerConfiguration configuration = scanService->getConfiguration(usedDevice);

  Local<Object> obj = args[1]->ToObject();
  if (obj->Has(String::NewFromUtf8(isolate, "fromX")))
  {
    configuration.fromX = obj->Get(String::NewFromUtf8(isolate, "fromX"))->NumberValue();
  }
  if (obj->Has(String::NewFromUtf8(isolate, "toX")))
  {
    configuration.toX = obj->Get(String::NewFromUtf8(isolate, "toX"))->NumberValue();
  }
  if (obj->Has(String::NewFromUtf8(isolate, "fromY")))
  {
    configuration.fromY = obj->Get(String::NewFromUtf8(isolate, "fromY"))->NumberValue();
  }
  if (obj->Has(String::NewFromUtf8(isolate, "toY")))
  {
    configuration.toY = obj->Get(String::NewFromUtf8(isolate, "toY"))->NumberValue();
  }
  if (obj->Has(String::NewFromUtf8(isolate, "resolutionInDPI")))
  {
    configuration.resolutionInDPI = obj->Get(String::NewFromUtf8(isolate, "resolutionInDPI"))->Uint32Value();
  }
  if (obj->Has(String::NewFromUtf8(isolate, "source")))
  {
    v8::String::Utf8Value source(obj->Get(String::NewFromUtf8(isolate, "source"))->ToString());
    configuration.source = *source;
  }
  if (obj->Has(String::NewFromUtf8(isolate, "mode")))
  {
    v8::String::Utf8Value mode(obj->Get(String::NewFromUtf8(isolate, "mode"))->ToString());
    configuration.mode = *mode;
  }

  scanService->setConfiguration(usedDevice, configuration);
}

/**
 * Scan to a given file.
 * 
 * Expects javascript arguments: 
 *  - deviceName (string)
 *  - fileName (string)
 */
void scanToFile(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  if (args.Length() != 2 && !(args[0]->IsString() || args[0]->IsNull()) && !args[1]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Expecting: scanToFile(deviceName:string, filepath:string)")));
    return;
  }

  v8::String::Utf8Value paramFilePath(args[1]);
  std::string filePath = std::string(*paramFilePath);

  ScannerDeviceDescriptorPtr usedDevice = getDeviceDescriptor(isolate, args[0]);
  if (usedDevice == nullptr)
  {
    return;
  }

  bool result = scanService->scanToFile(usedDevice, filePath);

  args.GetReturnValue().Set(Boolean::New(isolate, result));
}

/**
 * Scan to a buffer
 * 
 * Expects javascript arguments: 
 *  - deviceName (string)
 * 
 * The result is a dict with the following data:
 * - width (in pixel)
 * - pixel (in pixel)
 * - bytesPerPixel
 * - pixel[] (Uint8Array with the RGB pixel data (line by line))
 */
void scanToBuffer(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  ScannerDeviceDescriptorPtr usedDevice = getDeviceDescriptor(isolate, args[0]);
  if (usedDevice == nullptr)
  {
    return;
  }

  RawImagePtr rawImage = scanService->scanToBuffer(usedDevice);
  auto bytes = rawImage->width * rawImage->height * rawImage->bytesPerPixel;
  v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, rawImage->pixels, bytes);
  v8::Local<v8::Uint8Array> array = v8::Uint8Array::New(buffer, 0, bytes);

  Local<Object> obj = Object::New(isolate);
  obj->Set(String::NewFromUtf8(isolate, "width"), Uint32::New(isolate, rawImage->width));
  obj->Set(String::NewFromUtf8(isolate, "height"), Uint32::New(isolate, rawImage->height));
  obj->Set(String::NewFromUtf8(isolate, "bytesPerPixel"), Uint32::New(isolate, rawImage->bytesPerPixel));
  obj->Set(String::NewFromUtf8(isolate, "pixels"), array);
  args.GetReturnValue().Set(obj);
}

/**
 * Setup the interface / scanner service
 */
void init(Local<Object> exports)
{
  auto interface = IScannerInterfacePtr(new SaneScannerInterface());
  scanService = ScanServicePtr(new ScanService(interface));

  NODE_SET_METHOD(exports, "getScanners", getScanners);
  NODE_SET_METHOD(exports, "getCapabilities", getCapabilities);
  NODE_SET_METHOD(exports, "getConfiguration", getConfiguration);
  NODE_SET_METHOD(exports, "setConfiguration", setConfiguration);
  NODE_SET_METHOD(exports, "scanToFile", scanToFile);
  NODE_SET_METHOD(exports, "scanToBuffer", scanToBuffer);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)

} // namespace demo

#endif //NO_NODE