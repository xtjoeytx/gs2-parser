#include <gtest/gtest.h>
#include "encoding/buffer.h"
#include "encoding/graalencoding.h"

const char *testString = "Test string";
const int len = strlen(testString);

TEST(Buffer, BufferDefaultConstructor) {
	Buffer buffer;

	EXPECT_EQ(buffer.length(), 0);
	EXPECT_EQ(buffer.size(), 0);
	EXPECT_EQ(buffer.buffer(), nullptr);

	buffer.write('a');

	EXPECT_EQ(buffer.length(), 1);
	EXPECT_EQ(buffer.size(), 128);
	EXPECT_NE(buffer.buffer(), nullptr);
}


TEST(Buffer, BufferSizeConstructor) {
	Buffer buffer(300);

	EXPECT_EQ(buffer.length(), 0);
	EXPECT_EQ(buffer.size(), 300);
	EXPECT_NE(buffer.buffer(), nullptr);

	const int len = strlen("Test string");
	buffer.write("Test string", len);

	EXPECT_EQ(buffer.length(), len);
	EXPECT_EQ(buffer.size(), 300);
	EXPECT_NE(buffer.buffer(), nullptr);
}

TEST(Buffer, BufferMoveConstructor) {
	Buffer buffer(300);
	buffer.write('a');
	buffer.write(testString, len);

	EXPECT_EQ(buffer.length(), len + 1);
	EXPECT_EQ(buffer.size(), 300);
	EXPECT_NE(buffer.buffer(), nullptr);

	Buffer newBuffer(std::move(buffer));

	// verify previous buffer cleared
	EXPECT_EQ(buffer.length(), 0);
	EXPECT_EQ(buffer.size(), 0);
	EXPECT_EQ(buffer.buffer(), nullptr);

	// verify new buffer copied over
	EXPECT_EQ(newBuffer.length(), len + 1);
	EXPECT_EQ(newBuffer.size(), 300);
	EXPECT_NE(newBuffer.buffer(), nullptr);

	// write to new buffer
	newBuffer.write(testString, len);

	EXPECT_EQ(newBuffer.length(), (len * 2) + 1);
	EXPECT_EQ(newBuffer.size(), 300);
	EXPECT_NE(newBuffer.buffer(), nullptr);

	//	buffer.write(testString, len);
//
//	EXPECT_EQ(buffer.length(), len);
//	EXPECT_EQ(buffer.size(), 300);
//	EXPECT_NE(buffer.buffer(), nullptr);
}

TEST(Buffer, BufferResize) {
	Buffer buffer(6);
	buffer.write('a');

	EXPECT_EQ(buffer.size(), 6);

	buffer.write(testString, len);

	EXPECT_EQ(buffer.length(), len + 1);
	EXPECT_EQ(buffer.size(), (6 + 1 + len));
	EXPECT_NE(buffer.buffer(), nullptr);

	const char *someLongStr = "aaaaaaaaaabbbbbbbbbbcccccccccddddddddeeeeeeeffffffggggghhhhhhiiiii";
	buffer.write(someLongStr, strlen(someLongStr));

	EXPECT_EQ(buffer.length(), len + strlen(someLongStr) + 1);
	EXPECT_EQ(buffer.size(), (6 + 1 + len) + (strlen(someLongStr) + len + 1));
	EXPECT_NE(buffer.buffer(), nullptr);
}

TEST(Buffer, BufferResize2) {
	char buf[100] = {0};

	Buffer buffer(128);
	buffer.write(buf, 128);

	EXPECT_EQ(buffer.length(), 128);
	EXPECT_EQ(buffer.size(), 128);
	EXPECT_NE(buffer.buffer(), nullptr);

	buffer.write('a');

	EXPECT_EQ(buffer.length(), 129);
	EXPECT_EQ(buffer.size(), 256);
	EXPECT_NE(buffer.buffer(), nullptr);
	EXPECT_EQ(buffer.buffer()[128], 'a');
}

TEST(Buffer, BufferWriteBuffer) {
	Buffer buffer(128);
	Buffer buffer2(128);

	buffer.write("hello", 5);
	buffer2.write(" world", 6);

	EXPECT_EQ("hello", std::string(buffer.buffer(), buffer.buffer() + 5));
	EXPECT_EQ(" world", std::string(buffer2.buffer(), buffer2.buffer() + 6));

	buffer.write(buffer2);

	EXPECT_EQ(buffer.length(), 11);
	EXPECT_EQ(buffer.size(), 128);
	EXPECT_EQ("hello world", std::string(buffer.buffer(), buffer.buffer() + 11));
}

TEST(Buffer, BufferWritePos) {
	Buffer buffer(128);

	buffer.write("hello", 5);
	buffer.setWritePos(0);

	EXPECT_EQ("hello", std::string(buffer.buffer(), buffer.buffer() + 5));

	buffer.write("world", 5);

	EXPECT_EQ(buffer.length(), 5);
	EXPECT_EQ(buffer.size(), 128);
	EXPECT_NE(buffer.buffer(), nullptr);
	EXPECT_EQ("world", std::string(buffer.buffer(), buffer.buffer() + 5));
}

TEST(Buffer, BufferRead) {
	Buffer buffer(128);
	buffer.write("hello world", 11);
	buffer.write('\0');

	char str[12];
	buffer.read(str, 12);

	EXPECT_EQ("hello world", std::string(str));
}

TEST(Buffer, BufferReadOutOfBounds) {
	Buffer buffer(128);
	buffer.write("hello world", 11);
	buffer.write('\0');

	char str[12] = {0};
	buffer.read(str, 12, buffer.size());

	EXPECT_NE("hello world", std::string(str));
	EXPECT_EQ("", std::string(str));
}
