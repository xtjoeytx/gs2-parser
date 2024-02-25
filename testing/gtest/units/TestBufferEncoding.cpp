#include <gtest/gtest.h>
#include "encoding/buffer.h"
#include "encoding/graalencoding.h"

TEST(Buffer, GraalByteEncoding) {
	Buffer buffer(1);
	buffer.Write<GraalByte>(76);
	buffer.Write<GraalByte>(32);

	EXPECT_EQ(76 + 32, buffer.buffer()[0]);
	EXPECT_EQ(76, buffer.Read<GraalByte>(0));
	EXPECT_EQ(64, buffer.buffer()[1]);
	EXPECT_EQ(32, buffer.Read<GraalByte>(1));
}

TEST(Buffer, GraalByteEncodingBounds) {
	Buffer buffer(1);
	buffer.Write<GraalByte>(240);

	EXPECT_EQ(1, buffer.length());
	EXPECT_EQ(240, buffer.buffer()[0]);
	EXPECT_EQ(208, buffer.Read<GraalByte>(0)); // limited by maxread being 223, take 240 - 32 = 208
}