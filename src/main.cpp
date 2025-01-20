#include <cstdio>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include "GS2Context.h"


struct Response
{
	CompilerResponse response;
	std::filesystem::path output_file;
	std::string errmsg;
};

Response compileFile(const std::filesystem::path& filePath)
{
	static GS2Context context;

	Response result{};

	if (std::filesystem::exists(filePath))
	{
		std::string script;
		auto fileName = filePath.string();
		FILE* file = fopen(fileName.c_str(), "r");
		if (file)
		{
			static char buf[16384];
			size_t size = 0;
			while ((size = fread(buf, 1, sizeof(buf), file)) > 0)
			{
				script.append(&buf[0], size);
			}

			fclose(file);
			file = nullptr;
		}

		result.response = context.compile(script, "weapon", "TestCode", true);
		if (result.response.errors.empty())
		{
			result.output_file = std::filesystem::relative(filePath.parent_path()) / filePath.stem().concat(".gs2bc");

			std::ofstream outstream(result.output_file, std::ofstream::out | std::ofstream::binary);
			outstream.write((const char *)result.response.bytecode.buffer(), result.response.bytecode.length());
			outstream.close();
		}
		else
		{
			result.errmsg.clear();
			for (const auto &err : result.response.errors)
				result.errmsg.append(err.msg()).append("\n");
		}
	}
	else result.errmsg = "File does not exist";

	return result;
}

Buffer oneStopShop(const std::filesystem::path& inputPath)
{
	printf("Compiling file %s\n", inputPath.string().c_str());

	auto start = std::chrono::high_resolution_clock::now();
	auto result = compileFile(inputPath);
	auto finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = finish - start;
	printf("Compiled in %f seconds\n", diff.count());
	
	if (result.errmsg.empty())
	{
		printf(" -> saved to %s\n", result.output_file.string().c_str());
	}
	else
	{
		printf(" -> [ERROR] %s\n", result.errmsg.c_str());
	}

	return std::move(result.response.bytecode);
}

int main(int argc, const char *argv[]) {
#ifdef YYDEBUG
//  yydebug = 1;
#endif

//	auto ret = std::format("Test {}", 42);
//	printf("Test: %s\n", ret.c_str());
	//auto ret = std::format("test {}", 3);
	//printf("Test: %s\n", ret.c_str());

	Buffer buf;

	printf("Argc: %d\n", argc);
	if (argc >= 1)
		printf("Args: %s\n", argv[1]);

	if (argc > 1)
	{
		auto inputPath = std::filesystem::relative(argv[1]);

		if (std::filesystem::is_directory(inputPath))
		{
			printf("Scanning directory: %s\n", inputPath.string().c_str());

			auto iter = std::filesystem::directory_iterator(inputPath);
			for (const auto& it : iter)
			{
				if (it.path().extension() != ".txt")
				{
					printf("Skipping file %s\n", it.path().string().c_str());
					continue;
				}

				oneStopShop(it.path());
			}
		}
		else
		{
			//for (int i = 0; i < 1000; i++)
			{
				buf = oneStopShop(inputPath);
			}
		}
	}

	if (argc > 2)
	{
		FILE* file;
		file = fopen(argv[2], "wb");

		if (file)
		{
			//uint8_t packetId = 140 + 32;
			//fwrite(&packetId, sizeof(uint8_t), 1, file);
			fwrite(buf.buffer(), sizeof(uint8_t), buf.length(), file);
			fclose(file);

			printf("Written bytecode to %s\n", argv[2]);
		}
		else printf("Couldn't open output file %s\n", argv[2]);
	}

	printf("Total length of bytecode w/ headers: %5zu\n", buf.length());

#ifdef DBGALLOCATIONS
	checkNodeOwnership();
	checkForNodeLeaks();
#endif

	return 0;
}

