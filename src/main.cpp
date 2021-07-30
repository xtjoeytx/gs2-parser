#include <cstdio>
#include <string>
#include <fstream>

#include "Parser.h"
#include "ast.h"
#include "GS2SourceVisitor.h"
#include "GS2CompilerVisitor.h"

int main(int argc, const char *argv[]) {
#ifdef YYDEBUG
//  yydebug = 1;
#endif

	std::string testStr;

	if (argc > 1)
	{
		std::ifstream t(argv[1]);
		std::string str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
		
		testStr = std::move(str);
	}

	{
		ParserData parserStruct;
		parserStruct.parse(testStr);

		StatementBlock* stmtBlock = parserStruct.prog;

		if (stmtBlock != nullptr)
		{
			GS2SourceVisitor visit;
			printf("Children: %zu\n", stmtBlock->statements.size());
			visit.Visit(stmtBlock);

			GS2CompilerVisitor compilerVisitor(&parserStruct);
			compilerVisitor.Visit(stmtBlock);

			auto byteCode = compilerVisitor.getByteCode("weapon", "TestCode");
			printf("Total length of bytecode w/ headers: %5zu\n", byteCode.length());

			auto buf = byteCode.buffer();

			FILE* file;
			if (argc > 2) {
				file = fopen(argv[2], "wb");
			}
			else file = fopen("weaponTestCode.dump", "wb");

			if (file)
			{
				uint8_t packetId = 140 + 32;
				fwrite(&packetId, sizeof(uint8_t), 1, file);
				fwrite(buf, sizeof(uint8_t), byteCode.length(), file);
				fclose(file);
			}
			else printf("Couldn't open file\n");
		}

#ifdef DBGALLOCATIONS
		checkNodeOwnership();
#endif
	}

#ifdef DBGALLOCATIONS
	checkForNodeLeaks();
#endif

	#ifdef _WIN32
	system("pause");
	#endif

	return 0;
}
