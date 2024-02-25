#ifndef GS2TEST_TESTING_GTEST_BYTECODE_TESTS_COMPILERFIXTURE_H_
#define GS2TEST_TESTING_GTEST_BYTECODE_TESTS_COMPILERFIXTURE_H_

#include "GS2Context.h"
#include <gtest/gtest.h>

class GS2ContextFixture : public ::testing::Test {
	protected:
		GS2Context context;

		GS2ContextFixture() {
		}

		~GS2ContextFixture() override {
		}

		void SetUp() override {
			// Code here will be called immediately after the constructor (right before each test).
		}

		void TearDown() override {
			// Code here will be called immediately after each test (right before the destructor).
		}

		CompilerResponse compile(const std::string &script) {
			return context.compile(script);
		}

		CompilerResponse compileWithHeader(const std::string &script) {
			return context.compile(script, "weapon", "TestCode", true);
		}

		int printScriptErrors(const CompilerResponse &response) {
			if (!response.errors.empty()) {
				int errorIdx = 0;
				printf("Script Errors during compilation: %zu", response.errors.size());
				for (const auto &error: response.errors) {
					printf("Error %d: %s\n", ++errorIdx, error.msg().c_str());
				}
			}
			return response.errors.size();
		}
};

#endif //GS2TEST_TESTING_GTEST_BYTECODE_TESTS_COMPILERFIXTURE_H_
