#ifndef GS2TEST_TESTING_GTEST_FIXTURES_GS2BYTECODEFIXTURE_H_
#define GS2TEST_TESTING_GTEST_FIXTURES_GS2BYTECODEFIXTURE_H_

#include <gtest/gtest.h>
#include "GS2Bytecode.h"

class GS2BytecodeMock : public GS2Bytecode {
	public:
		GS2BytecodeMock() : GS2Bytecode() { }

	protected:
};

class GS2BytecodeFixture : public ::testing::Test {
	protected:
		GS2BytecodeMock bytecode;

		GS2BytecodeFixture() {
		}

		~GS2BytecodeFixture() override {
		}

		void SetUp() override {
			// Code here will be called immediately after the constructor (right before each test).
		}

		void TearDown() override {
			// Code here will be called immediately after each test (right before the destructor).
		}

		void resetBytecode() {
			bytecode = GS2BytecodeMock{};
		}
};

#endif //GS2TEST_TESTING_GTEST_FIXTURES_GS2BYTECODEFIXTURE_H_
