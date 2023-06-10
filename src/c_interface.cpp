#include "GS2Context.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

extern "C" {

	struct Response
	{
		bool success;
		const char* errmsg;
		unsigned char* bytecode;
		uint bytecodesize;
	};

	void* DLL_EXPORT get_context() {
		return new GS2Context();
	}

	Response DLL_EXPORT compile_code(void* context, const char* code, const char* type, const char* name) {
		Response result{};
		result.success = false;
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
				result.errmsg = new char[lenStr];
				strcpy(const_cast<char*>(result.errmsg), errMsg.c_str());

				result.bytecode = nullptr;
				result.bytecodesize = 0;
			} else {
				result.errmsg = nullptr;
				result.bytecode = new unsigned char[response.bytecode.size()];  // Allocate memory
				memcpy((void*)result.bytecode, response.bytecode.buffer(), response.bytecode.size());
				result.bytecodesize = response.bytecode.size();
			}
			result.success = response.success;
		}

		return result;
	}

	void DLL_EXPORT delete_context(void* context) {
		delete (GS2Context*)context;
	}

}