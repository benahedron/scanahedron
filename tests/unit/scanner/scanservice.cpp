#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "scanner/iscannerinterface.h"
#include "scanner/scanservice.h"
#include "utils/types.h"

using ::testing::Return;
using ::testing::_;

class MockScannerInterface : public IScannerInterface
{
public:
  MOCK_METHOD0(init, bool());
  MOCK_METHOD0(exit, bool());
  MOCK_METHOD0(getDevices, std::vector<ScannerDeviceDescriptorPtr>());
  MOCK_METHOD1(getCapabilities, ScannerCapabilities(ScannerDeviceDescriptorPtr));
  MOCK_METHOD1(getConfiguration, ScannerConfiguration(ScannerDeviceDescriptorPtr));
  MOCK_METHOD2(setConfiguration, void(ScannerDeviceDescriptorPtr, const ScannerConfiguration &));
  MOCK_METHOD1(scanToBuffer, RawImagePtr(ScannerDeviceDescriptorPtr));
};

SHARED_PTR(MockScannerInterface);

TEST(ScannerService, ConstructionCallsInitAndDoesNotThrowIfOk)
{
  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, exit()).Times(1);

  ScanService service(interface);
}

TEST(ScannerService, CannotConstructWithoutInterface)
{
  ASSERT_ANY_THROW(ScanService service(nullptr));
}

TEST(ScannerService, ConstructorThrowsIfCannotInit)
{
  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(false));

  ASSERT_ANY_THROW(ScanService service(interface));
}

TEST(ScannerService, DestructionCallsExit)
{
  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
  }
}

TEST(ScannerService, LoadScannersQueriesTheAvailableDevice)
{
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));
  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getDevices()).Times(1).WillRepeatedly(Return(available));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    service.loadScanners();

    ASSERT_EQ(available.size(), 2);
    ASSERT_EQ(available.size(), service.getAvailableScanners().size());
  }
}

TEST(ScannerService, GetAvailableWillLazyLoadAllAvailableDevice)
{
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));
  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getDevices()).Times(1).WillRepeatedly(Return(available));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    auto result = service.getAvailableScanners();

    ASSERT_EQ(available.size(), 2);
    ASSERT_EQ(available.size(), result.size());
    ASSERT_EQ(available[1], result[1]);
  }
}

TEST(ScannerService, GetCapabilities)
{
  ScannerCapabilities capabilities;
  capabilities.minX = 13.0;
  capabilities.maxY = 25.0;
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));

  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getCapabilities(available[0])).Times(1).WillRepeatedly(Return(capabilities));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    auto result = service.getCapabilities(available[0]);

    ASSERT_EQ(result.minX, capabilities.minX);
    ASSERT_EQ(result.maxY, capabilities.maxY);
  }
}

TEST(ScannerService, GetCapabilitiesWillUseFirstScannerIfNoneGiven)
{
  ScannerCapabilities capabilities;
  capabilities.minX = 10.6;
  capabilities.maxY = 25.0;
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));

  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getDevices()).Times(1).WillRepeatedly(Return(available));
  EXPECT_CALL(*interface, getCapabilities(available[0])).Times(1).WillRepeatedly(Return(capabilities));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    auto result = service.getCapabilities(nullptr);
    ASSERT_EQ(result.minX, capabilities.minX);
    ASSERT_EQ(result.maxY, capabilities.maxY);
  }
}

TEST(ScannerService, GetConfiguration)
{
  ScannerConfiguration configuration;
  configuration.fromX = 13.0;
  configuration.toY = 25.0;
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));

  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getConfiguration(available[0])).Times(1).WillRepeatedly(Return(configuration));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    auto result = service.getConfiguration(available[0]);

    ASSERT_EQ(result.fromX, configuration.fromX);
    ASSERT_EQ(result.toY, configuration.toY);
  }
}

TEST(ScannerService, SetConfiguration)
{
  ScannerConfiguration configuration;
  configuration.fromX = 13.0;
  configuration.toY = 25.0;
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));

  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getDevices()).Times(1).WillRepeatedly(Return(available));
  EXPECT_CALL(*interface, setConfiguration(available[0], _)).Times(1);
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    service.setConfiguration(nullptr, configuration);
  }
}

TEST(ScannerService, ScanToBuffer)
{
  auto buffer = RawImagePtr(new RawImage(5, 5, 3));
  std::vector<ScannerDeviceDescriptorPtr> available;
  available.push_back(ScannerDeviceDescriptorPtr(new ScannerDeviceDescriptor()));

  MockScannerInterfacePtr interface(new MockScannerInterface());
  EXPECT_CALL(*interface, init()).Times(1).WillRepeatedly(Return(true));
  EXPECT_CALL(*interface, getDevices()).Times(1).WillRepeatedly(Return(available));
  EXPECT_CALL(*interface, scanToBuffer(available[0])).Times(1).WillOnce(Return(buffer));
  EXPECT_CALL(*interface, exit()).Times(1);
  {
    ScanService service(interface);
    auto result = service.scanToBuffer(nullptr);
    ASSERT_EQ(result, buffer);
  }
}
