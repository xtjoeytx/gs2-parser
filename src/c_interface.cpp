#include "GS2Context.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {
	struct Response
	{
		bool Success;
		const char* ErrMsg;
		unsigned char* ByteCode;
		uint32_t ByteCodeSize;
	};

	void* DLL_EXPORT get_context() {
		return new GS2Context();
	}

	Response DLL_EXPORT compile_code(void* context, const char* code, const char* type, const char* name) {
		Response result{};
		result.Success = false;

		auto gs2Context = (GS2Context*)context;

		if (gs2Context != nullptr) {
			std::string script = code;
			std::string errMsg;
			auto response = gs2Context->compile(script, type, name, true);

			if (!response.errors.empty()) {
				errMsg.clear();
				for (const auto& err : response.errors)
					errMsg.append(err.msg()).append("\n");

				int lenStr = errMsg.length() + 1;
				result.ErrMsg = new char[lenStr];
				strcpy(const_cast<char*>(result.ErrMsg), errMsg.c_str());

				result.ByteCode = nullptr;
				result.ByteCodeSize = 0;
			} else {
				result.ErrMsg = nullptr;
				result.ByteCode = new unsigned char[response.bytecode.size()];  // Allocate memory
				memcpy((void*)result.ByteCode, response.bytecode.buffer(), response.bytecode.size());
				result.ByteCodeSize = response.bytecode.size();
			}
			result.Success = response.success;
		}

		return result;
	}

	void DLL_EXPORT delete_context(void* context) {
		delete (GS2Context*)context;
	}
}