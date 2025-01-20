#ifndef FORMAT_STRING_H
#define FORMAT_STRING_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdarg>
#include <cstdio> // for std::vsnprintf

/**
 * @brief Formats a string using a printf-style format string and variadic arguments.
 *
 * Remove the dependency on std::format by using std::vsnprintf to format a string
 *
 * @param fmt The printf-style format string.
 * @param ... Variadic arguments matching the format string specifiers.
 * @return A formatted std::string.
 * @throws std::runtime_error If an error occurs during formatting.
 *
 * @note This function supports standard printf-style formatting specifiers (e.g.,
 * `%d`, `%s`, `%f`, etc.). Ensure that the number and types of arguments match
 * the format specifiers to avoid undefined behavior.
 *
 * @example
 * ```cpp
 * std::string result = printf_format_string("Hello, %s! You have %d new messages.", "Alice", 5);
 * std::cout << result; // Output: Hello, Alice! You have 5 new messages.
 * ```
 */
inline std::string printf_format_string(const char* fmt, ...) {
    // Start variable arguments processing
    va_list args;
    va_start(args, fmt);

    // Estimate required buffer size
    size_t size = std::vsnprintf(nullptr, 0, fmt, args) + 1; // +1 for null terminator
    va_end(args);

    if (size <= 0) {
        throw std::runtime_error("Error during formatting.");
    }

    // Allocate the required size
    std::vector<char> buffer(size);

    // Format the string
    va_start(args, fmt);
    std::vsnprintf(buffer.data(), size, fmt, args);
    va_end(args);

    // Return the formatted string
    return std::string(buffer.data());
}

#endif // FORMAT_STRING_H
