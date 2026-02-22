#include "compiler/GS2Context.h"

#ifdef _WIN32
  #define DLL_EXPORT extern "C" __declspec(dllexport)
#else
  #define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

struct Response {
    bool Success;
    const char *ErrMsg;
    unsigned char *ByteCode;
    uint32_t ByteCodeSize;
};

DLL_EXPORT void *get_context() {
    return new GS2Context();
}

DLL_EXPORT Response compile_code_no_header(void *context, const char *code) {
    Response result{};
    result.Success = false;

    if (const auto gs2Context = static_cast<GS2Context *>(context); gs2Context != nullptr) {
		const std::string script = code;
        std::string errMsg;
        const auto response = gs2Context->compile(script);

        if (!response.errors.empty()) {
            errMsg.clear();
            for (const auto &err: response.errors)
                    errMsg.append(err.msg()).append("\n");

            const int lenStr = errMsg.length() + 1;
            result.ErrMsg = new char[lenStr];
            strcpy(const_cast<char *>(result.ErrMsg), errMsg.c_str());

            result.ByteCode = nullptr;
            result.ByteCodeSize = 0;
        } else {
            result.ErrMsg = nullptr;
            result.ByteCode = new unsigned char[response.bytecode.length()];  // Allocate memory
            memcpy(result.ByteCode, response.bytecode.buffer(), response.bytecode.length());
            result.ByteCodeSize = response.bytecode.length();
        }
        result.Success = response.success;
    }

    return result;
}

DLL_EXPORT Response compile_code(void *context, const char *code, const char *type, const char *name) {
    Response result{};
    result.Success = false;

    if (const auto gs2Context = static_cast<GS2Context *>(context);
        gs2Context != nullptr) {
		const std::string script = code;
        std::string errMsg;
        const auto response = gs2Context->compile(script, type, name, true);

        if (!response.errors.empty()) {
            errMsg.clear();
            for (const auto &err: response.errors)
                errMsg.append(err.msg()).append("\n");

            const int lenStr = errMsg.length() + 1;
            result.ErrMsg = new char[lenStr];
            strcpy(const_cast<char *>(result.ErrMsg), errMsg.c_str());

            result.ByteCode = nullptr;
            result.ByteCodeSize = 0;
        } else {
            result.ErrMsg = nullptr;
            result.ByteCode = new unsigned char[response.bytecode.length()];  // Allocate memory
            memcpy(result.ByteCode, response.bytecode.buffer(), response.bytecode.length());
            result.ByteCodeSize = response.bytecode.length();
        }
        result.Success = response.success;
    }

    return result;
}

DLL_EXPORT void delete_context(void *context) {
    delete static_cast<GS2Context *>(context);
}