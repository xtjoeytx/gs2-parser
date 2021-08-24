#include <cstdio>
#include <string>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include "GS2Context.h"

struct CompileResonse
{
	Buffer bytecode;
	std::string errmsg;
	std::filesystem::path output_file;
};

CompileResonse compileFile(const std::filesystem::path& filePath)
{
	static GS2Context context;
	
	CompileResonse response{};

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

		//std::ifstream inputstream(filePath);
		//std::string script((std::istreambuf_iterator<char>(inputstream)), std::istreambuf_iterator<char>());

		response.bytecode = context.compile(script, "weapon", "TestCode", true);
		if (!context.hasErrors())
		{
			response.output_file = std::filesystem::relative(filePath.parent_path()) / filePath.stem().concat(".gs2bc");

			std::ofstream outstream(response.output_file, std::ofstream::out | std::ofstream::binary);
			outstream.write((const char *)response.bytecode.buffer(), response.bytecode.length());
			outstream.close();
		}
		else response.errmsg = context.getErrors()[0].msg();
	}
	else response.errmsg = "File does not exist";

	return response;
}

Buffer oneStopShop(const std::filesystem::path& inputPath)
{
	printf("Compiling file %s\n", inputPath.string().c_str());

	auto start = std::chrono::high_resolution_clock::now();
	auto compilerResponse = compileFile(inputPath);
	auto finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = finish - start;
	printf("Compiled in %f seconds\n", diff.count());
	
	if (compilerResponse.errmsg.empty())
	{
		printf(" -> saved to %s\n", compilerResponse.output_file.string().c_str());
	}
	else
	{
		printf(" -> [ERROR] %s\n", compilerResponse.errmsg.c_str());
	}

	return std::move(compilerResponse.bytecode);
}

int main(int argc, const char *argv[]) {
#ifdef YYDEBUG
//  yydebug = 1;
#endif

	Buffer buf;

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
			buf = oneStopShop(inputPath);
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

