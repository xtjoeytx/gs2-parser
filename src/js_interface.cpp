#include <emscripten/bind.h>
#include "GS2Context.h"
using namespace emscripten;

EMSCRIPTEN_BINDINGS(module) {
    register_vector<std::string>("VectorString");
}

EMSCRIPTEN_BINDINGS(GS2Context_bindings) {
    class_<GS2Context>("GS2Context")
        .constructor<>()
        .function("compile", select_overload<CompilerResponse(const std::string&, const std::string&, const std::string&, bool)>(&GS2Context::compile), emscripten::return_value_policy::take_ownership())
        .function("compile", select_overload<CompilerResponse(const std::string&)>(&GS2Context::compile), emscripten::return_value_policy::take_ownership());
}

emscripten::val getBytecodeFromBuffer(const CompilerResponse &response) {
    const Buffer &buf = response.bytecode;
    return emscripten::val(emscripten::typed_memory_view(buf.size(), buf.buffer()));
}

/* getErrors is simply a list of strings for now */
std::vector<std::string> getErrors(const CompilerResponse &response) {
    std::vector<std::string> errors;
    for (const GS2CompilerError &error : response.errors) {
        errors.push_back(error.msg());
    }

    return errors;
}

EMSCRIPTEN_BINDINGS(CompilerResponse_bindings) {
    /* For now, let's just have a test property set to 1 */
    class_<CompilerResponse>("CompilerResponse")
        .property("success", &CompilerResponse::success)
        .function("getBytecode", &getBytecodeFromBuffer, emscripten::return_value_policy::take_ownership())
        .function("getErrors", &getErrors, emscripten::return_value_policy::take_ownership());
}