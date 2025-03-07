//===- llvm/unittest/Support/DataExtractorTest.cpp - DataExtractor tests --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/DataExtractor.h"
#include "gtest/gtest.h"
using namespace llvm;

namespace {

// Test fixture
template <typename T>
class DataExtractorTest : public ::testing::Test { };

// Test DataExtractor with both types which can be used for offsets.
typedef ::testing::Types</*uint32_t, */uint64_t> TestTypes;
TYPED_TEST_CASE(DataExtractorTest, TestTypes);

const char numberData[] = "\x80\x90\xFF\xFF\x80\x00\x00\x00";
const char stringData[] = "hellohello\0hello";
const char leb128data[] = "\xA6\x49";
const char bigleb128data[] = "\xAA\xA9\xFF\xAA\xFF\xAA\xFF\x4A";

TYPED_TEST(DataExtractorTest, OffsetOverflow) {
  DataExtractor DE(StringRef(numberData, sizeof(numberData)-1), false, 8);
  EXPECT_FALSE(DE.isValidOffsetForDataOfSize(-2U, 5));
}

TYPED_TEST(DataExtractorTest, UnsignedNumbers) {
  DataExtractor DE(StringRef(numberData, sizeof(numberData)-1), false, 8);
  TypeParam offset = 0;

  EXPECT_EQ(0x80U, DE.getU8(&offset));
  EXPECT_EQ(1U, offset);
  offset = 0;
  EXPECT_EQ(0x8090U, DE.getU16(&offset));
  EXPECT_EQ(2U, offset);
  offset = 0;
  EXPECT_EQ(0x8090FFFFU, DE.getU32(&offset));
  EXPECT_EQ(4U, offset);
  offset = 0;
  EXPECT_EQ(0x8090FFFF80000000ULL, DE.getU64(&offset));
  EXPECT_EQ(8U, offset);
  offset = 0;
  EXPECT_EQ(0x8090FFFF80000000ULL, DE.getAddress(&offset));
  EXPECT_EQ(8U, offset);
  offset = 0;

  uint32_t data[2];
  EXPECT_EQ(data, DE.getU32(&offset, data, 2));
  EXPECT_EQ(0x8090FFFFU, data[0]);
  EXPECT_EQ(0x80000000U, data[1]);
  EXPECT_EQ(8U, offset);
  offset = 0;

  // Now for little endian.
  DE = DataExtractor(StringRef(numberData, sizeof(numberData)-1), true, 4);
  EXPECT_EQ(0x9080U, DE.getU16(&offset));
  EXPECT_EQ(2U, offset);
  offset = 0;
  EXPECT_EQ(0xFFFF9080U, DE.getU32(&offset));
  EXPECT_EQ(4U, offset);
  offset = 0;
  EXPECT_EQ(0x80FFFF9080ULL, DE.getU64(&offset));
  EXPECT_EQ(8U, offset);
  offset = 0;
  EXPECT_EQ(0xFFFF9080U, DE.getAddress(&offset));
  EXPECT_EQ(4U, offset);
  offset = 0;

  EXPECT_EQ(data, DE.getU32(&offset, data, 2));
  EXPECT_EQ(0xFFFF9080U, data[0]);
  EXPECT_EQ(0x80U, data[1]);
  EXPECT_EQ(8U, offset);
}

TYPED_TEST(DataExtractorTest, SignedNumbers) {
  DataExtractor DE(StringRef(numberData, sizeof(numberData)-1), false, 8);
  TypeParam offset = 0;

  EXPECT_EQ(-128, DE.getSigned(&offset, 1));
  EXPECT_EQ(1U, offset);
  offset = 0;
  EXPECT_EQ(-32624, DE.getSigned(&offset, 2));
  EXPECT_EQ(2U, offset);
  offset = 0;
  EXPECT_EQ(-2137980929, DE.getSigned(&offset, 4));
  EXPECT_EQ(4U, offset);
  offset = 0;
  EXPECT_EQ(-9182558167379214336LL, DE.getSigned(&offset, 8));
  EXPECT_EQ(8U, offset);
}

TYPED_TEST(DataExtractorTest, Strings) {
  DataExtractor DE(StringRef(stringData, sizeof(stringData)-1), false, 8);
  TypeParam offset = 0;

  EXPECT_EQ(stringData, DE.getCStr(&offset));
  EXPECT_EQ(11U, offset);
  EXPECT_EQ(nullptr, DE.getCStr(&offset));
  EXPECT_EQ(11U, offset);
}

TYPED_TEST(DataExtractorTest, LEB128) {
  DataExtractor DE(StringRef(leb128data, sizeof(leb128data)-1), false, 8);
  TypeParam offset = 0;

  EXPECT_EQ(9382ULL, DE.getULEB128(&offset));
  EXPECT_EQ(2U, offset);
  offset = 0;
  EXPECT_EQ(-7002LL, DE.getSLEB128(&offset));
  EXPECT_EQ(2U, offset);

  DataExtractor BDE(StringRef(bigleb128data, sizeof(bigleb128data)-1), false,8);
  offset = 0;
  EXPECT_EQ(42218325750568106ULL, BDE.getULEB128(&offset));
  EXPECT_EQ(8U, offset);
  offset = 0;
  EXPECT_EQ(-29839268287359830LL, BDE.getSLEB128(&offset));
  EXPECT_EQ(8U, offset);
}

TYPED_TEST(DataExtractorTest, LEB128_error) {
  DataExtractor DE(StringRef("\x81"), false, 8);
  TypeParam Offset = 0;
  EXPECT_EQ(0U, DE.getULEB128(&Offset));
  EXPECT_EQ(0U, Offset);

  Offset = 0;
  EXPECT_EQ(0U, DE.getSLEB128(&Offset));
  EXPECT_EQ(0U, Offset);
}
}
