#include <gtest/gtest.h>
#include <filesystem>
#include "../fixtures/GS2ContextFixture.h"
#include "fmt/format.h"

bool checkBufferContentsEquals(const Buffer& buf1, const Buffer& buf2)
{
	if (buf1.length() != buf2.length())
		return false;

	return memcmp(buf1.buffer(), buf2.buffer(), buf1.length()) == 0;
}

Buffer readBufferFromFile(const std::filesystem::path& filePath)
{
	Buffer buffer;

	FILE *file = fopen(filePath.string().c_str(), "r");
	if (file)
	{
		static char buf[16384];
		size_t size = 0;
		while ((size = fread(buf, 1, sizeof(buf), file)) > 0)
		{
			buffer.write(buf, size);
		}

		fclose(file);
	}

	return buffer;
}

void writeBufferToFile(const std::filesystem::path& filePath, const Buffer& buffer)
{
	FILE *file = fopen(filePath.string().c_str(), "wb");
	if (file)
	{
		fwrite(buffer.buffer(), sizeof(uint8_t), buffer.length(), file);
		fclose(file);
	}
}

std::string readFile(const std::filesystem::path& filePath)
{
	std::string script;

	auto fileName = filePath.string();
	FILE *file = fopen(fileName.c_str(), "r");
	if (file)
	{
		static char buf[16384];
		size_t size = 0;
		while ((size = fread(buf, 1, sizeof(buf), file)) > 0)
		{
			script.append(&buf[0], size);
		}

		fclose(file);
	}

	return script;
}

TEST_F(GS2ContextFixture, ValidateBytecodeSamples)
{
	std::filesystem::path samplePath = "/home/joey/projects/gs2-parser/testing/samples";

	for (const auto & entry : std::filesystem::directory_iterator(samplePath))
	{
		if (entry.path().extension() != ".gs2")
			continue;

		fmt::print("Compiling sample file {}..\n", entry.path().string());

		auto script = readFile(entry.path());
		auto result = compile(script);
		EXPECT_EQ(result.errors.size(), 0);

		const std::filesystem::path outputFile = (entry.path().parent_path() / entry.path().stem()).string() + ".gs2bc";
		if (exists(outputFile)) {
			auto expectedBuffer = readBufferFromFile(outputFile);
			EXPECT_TRUE(checkBufferContentsEquals(result.bytecode, expectedBuffer));
			continue;
		}

		// Generate the bytecode if the file does not exist
		fmt::print("	Could not find pre-compiled bytecode sample at {}\n", outputFile.filename().string());
		fmt::print("	Compiling current script as sample code\n");
		writeBufferToFile(outputFile, result.bytecode);
	}
}