#include <gtest/gtest.h>
#include "../fixtures/GS2BytecodeFixture.h"

TEST_F(GS2BytecodeFixture, GetLastOp) {
	resetBytecode();

	EXPECT_EQ(opcode::OP_NONE, bytecode.getLastOp());

	bytecode.emit((char) 5); // 1
	bytecode.emit((short) 5); // 2
	bytecode.emit(-5); // 4
	bytecode.emit(opcode::OP_ABS); // 1

	EXPECT_EQ(opcode::OP_ABS, bytecode.getLastOp());
	EXPECT_EQ(8, bytecode.getBytecodeBuffer().length());
}

TEST_F(GS2BytecodeFixture, GetStringConst) {
	resetBytecode();

	const char *testStrings[] = {
		"test str",
		"new test str",
		"one more test str"
	};

	auto idx1 = bytecode.getStringConst(testStrings[0]);
	auto idx2 = bytecode.getStringConst(testStrings[1]);
	auto idx3 = bytecode.getStringConst(testStrings[2]);
	auto idx4 = bytecode.getStringConst(testStrings[1]);

	EXPECT_EQ(0, idx1);
	EXPECT_EQ(1, idx2);
	EXPECT_EQ(2, idx3);
	EXPECT_EQ(1, idx4);

	EXPECT_EQ(0, bytecode.getStringConst(testStrings[0]));
	EXPECT_EQ(1, bytecode.getStringConst(testStrings[1]));
	EXPECT_EQ(2, bytecode.getStringConst(testStrings[2]));
	EXPECT_EQ(1, bytecode.getStringConst(testStrings[1]));
}

TEST_F(GS2BytecodeFixture, EmitConversionOpTest) {
	resetBytecode();
	bool result;

	// any -> float
	result = bytecode.emitConversionOp(ExpressionType::EXPR_STRING, ExpressionType::EXPR_NUMBER);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_OBJECT, ExpressionType::EXPR_NUMBER);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_INTEGER, ExpressionType::EXPR_NUMBER);
	EXPECT_FALSE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_NUMBER, ExpressionType::EXPR_NUMBER);
	EXPECT_FALSE(result);

	// any -> str
	result = bytecode.emitConversionOp(ExpressionType::EXPR_NUMBER, ExpressionType::EXPR_STRING);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_INTEGER, ExpressionType::EXPR_STRING);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_OBJECT, ExpressionType::EXPR_STRING);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_STRING, ExpressionType::EXPR_STRING);
	EXPECT_FALSE(result);

	// any -> obj
	result = bytecode.emitConversionOp(ExpressionType::EXPR_NUMBER, ExpressionType::EXPR_OBJECT);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_INTEGER, ExpressionType::EXPR_OBJECT);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_STRING, ExpressionType::EXPR_OBJECT);
	EXPECT_TRUE(result);

	result = bytecode.emitConversionOp(ExpressionType::EXPR_OBJECT, ExpressionType::EXPR_OBJECT);
	EXPECT_FALSE(result);
}

TEST_F(GS2BytecodeFixture, EmitDynamicNumber) {
	resetBytecode();

	// positive numbers
	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(117); // size = 1
	EXPECT_EQ(3, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(133); // size = 2
	EXPECT_EQ(7, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(650000); // size = 4
	EXPECT_EQ(13, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumber(15); // size = 1
	EXPECT_EQ(16, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumber(32767); // size = 2
	EXPECT_EQ(20, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumber(650000); // size = 4
	EXPECT_EQ(26, bytecode.getBytecodePos());

	// negative numbers
	resetBytecode();

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(-117); // size = 1
	EXPECT_EQ(3, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(-133); // size = 2
	EXPECT_EQ(7, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDynamicNumber(-650000); // size = 4
	EXPECT_EQ(13, bytecode.getBytecodePos());
}

TEST_F(GS2BytecodeFixture, EmitDynamicNumberInvalid) {
	resetBytecode();
	EXPECT_DEATH(bytecode.emitDynamicNumber(25535), "");
}

TEST_F(GS2BytecodeFixture, EmitDynamicNumberUnsigned) {
	resetBytecode();

	// positive numbers
	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumberUnsigned(std::numeric_limits<uint8_t>::max()); // size = 1
	EXPECT_EQ(3, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumberUnsigned(std::numeric_limits<uint16_t>::max()); // size = 2
	EXPECT_EQ(7, bytecode.getBytecodePos());

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumberUnsigned(std::numeric_limits<uint32_t>::max()); // size = 4
	EXPECT_EQ(13, bytecode.getBytecodePos());

	// negative numbers
	resetBytecode();

	bytecode.emit(opcode::OP_TYPE_VAR);
	bytecode.emitDynamicNumberUnsigned(-1); // size = 4
	EXPECT_EQ(6, bytecode.getBytecodePos());
}

TEST_F(GS2BytecodeFixture, EmitDouble) {
	resetBytecode();

	bytecode.emit(opcode::OP_TYPE_NUMBER);
	bytecode.emitDoubleNumber("5.73");

	const auto *buffer = bytecode.getBytecodeBuffer().buffer();

	EXPECT_EQ(buffer[0], opcode::OP_TYPE_NUMBER);
	EXPECT_EQ(buffer[1], 0xF6);
	EXPECT_EQ(std::string(buffer + 2, buffer + 6), "5.73");
}

TEST_F(GS2BytecodeFixture, EmitString) {
	resetBytecode();

	const char *testStr = "Test string";

	bytecode.emit(testStr);

	auto buffer = bytecode.getBytecodeBuffer().buffer();
	auto actual = std::string(buffer, buffer + strlen(testStr));

	EXPECT_EQ(testStr, actual);
}