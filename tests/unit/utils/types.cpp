#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "utils/types.h"

using ::testing::Return;
using ::testing::_;

TEST(RawImage, ConstructionSuceeds)
{
    RawImage image(13, 14);

    ASSERT_EQ(image.width, 13);
    ASSERT_EQ(image.height, 14);
    ASSERT_EQ(image.bytesPerPixel, 3);
    ASSERT_NE(image.pixels, nullptr);
}