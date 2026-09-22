#include "FunctionName.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace sablelog::detail {
namespace {

[[nodiscard]] constexpr bool isSpace(char character) noexcept
{
    return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

[[nodiscard]] std::string_view trim(std::string_view value) noexcept
{
    while (!value.empty() && isSpace(value.front()))
    {
        value.remove_prefix(1U);
    }

    while (!value.empty() && isSpace(value.back()))
    {
        value.remove_suffix(1U);
    }

    return value;
}

[[nodiscard]] std::size_t matchingOpenParenthesis(std::string_view value, std::size_t closePosition) noexcept
{
    std::size_t depth = 0U;
    for (std::size_t index = closePosition + 1U; index > 0U; --index)
    {
        const auto character = value[index - 1U];
        if (character == ')')
        {
            ++depth;
        }
        else if (character == '(')
        {
            if (--depth == 0U)
            {
                return index - 1U;
            }
        }
    }

    return std::string_view::npos;
}

[[nodiscard]] bool isOperatorAngle(std::string_view value, std::size_t position) noexcept
{
    const auto prefix = trim(value.substr(0U, position));
    const auto operatorPosition = prefix.rfind("operator");
    if (operatorPosition == std::string_view::npos)
    {
        return false;
    }

    for (const auto character : prefix.substr(operatorPosition + 8U))
    {
        if (!isSpace(character) && character != '<' && character != '>' && character != '=')
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] std::string collapseTemplateArguments(std::string_view value)
{
    std::string collapsed;
    collapsed.reserve(value.size());

    for (std::size_t index = 0U; index < value.size(); ++index)
    {
        if (value[index] != '<' || isOperatorAngle(value, index))
        {
            collapsed.push_back(value[index]);
            continue;
        }

        std::size_t depth = 1U;
        std::size_t closePosition = index + 1U;
        for (; closePosition < value.size() && depth > 0U; ++closePosition)
        {
            if (value[closePosition] == '<')
            {
                ++depth;
            }
            else if (value[closePosition] == '>')
            {
                --depth;
            }
        }

        if (depth != 0U)
        {
            collapsed.push_back(value[index]);
            continue;
        }

        collapsed += "<...>";
        index = closePosition - 1U;
    }

    return collapsed;
}

void normalizeOperatorSpacing(std::string& name)
{
    const auto operatorPosition = name.rfind("operator");
    if (operatorPosition == std::string::npos)
    {
        return;
    }

    const auto spacingBegin = operatorPosition + 8U;
    auto nameBegin = spacingBegin;
    while (nameBegin < name.size() && isSpace(name[nameBegin]))
    {
        ++nameBegin;
    }

    if (nameBegin == spacingBegin || nameBegin == name.size())
    {
        return;
    }

    constexpr std::string_view punctuation{"()[]<>=!+-*/%&|^~,"};
    const auto replacement = punctuation.find(name[nameBegin]) == std::string_view::npos ? " " : "";
    name.replace(spacingBegin, nameBegin - spacingBegin, replacement);
}

[[nodiscard]] std::string compactQualifiedName(std::string_view value)
{
    auto collapsed = collapseTemplateArguments(trim(value));
    const auto operatorScope = collapsed.rfind("::operator");
    const auto globalOperator = collapsed.rfind("operator");

    std::size_t nameStart = 0U;
    if (operatorScope != std::string::npos)
    {
        const auto separator = collapsed.find_last_of(" \t\r\n", operatorScope);
        nameStart = separator == std::string::npos ? 0U : separator + 1U;
    }
    else if (globalOperator != std::string::npos)
    {
        nameStart = globalOperator;
    }
    else
    {
        const auto separator = collapsed.find_last_of(" \t\r\n");
        nameStart = separator == std::string::npos ? 0U : separator + 1U;
    }

    std::string qualified{trim(std::string_view{collapsed}.substr(nameStart))};
    if (qualified.empty())
    {
        return {};
    }

    normalizeOperatorSpacing(qualified);

    const auto compactOperatorScope = qualified.rfind("::operator");
    if (compactOperatorScope != std::string::npos)
    {
        const auto previousScope = qualified.rfind("::", compactOperatorScope - 1U);
        return previousScope == std::string::npos ? qualified : qualified.substr(previousScope + 2U);
    }

    const auto lastScope = qualified.rfind("::");
    if (lastScope == std::string::npos)
    {
        return qualified;
    }

    const auto previousScope = qualified.rfind("::", lastScope - 1U);
    return previousScope == std::string::npos ? qualified : qualified.substr(previousScope + 2U);
}

[[nodiscard]] std::string compactCallableName(std::string_view signature)
{
    signature = trim(signature);
    auto parameterClose = signature.rfind(')');
    if (parameterClose == std::string_view::npos)
    {
        return compactQualifiedName(signature);
    }

    auto parameterOpen = matchingOpenParenthesis(signature, parameterClose);
    if (parameterOpen == std::string_view::npos)
    {
        return {};
    }

    auto beforeParameters = trim(signature.substr(0U, parameterOpen));
    if (beforeParameters.ends_with("noexcept"))
    {
        signature = trim(beforeParameters.substr(0U, beforeParameters.size() - 8U));
        parameterClose = signature.rfind(')');
        if (parameterClose == std::string_view::npos)
        {
            return {};
        }

        parameterOpen = matchingOpenParenthesis(signature, parameterClose);
        if (parameterOpen == std::string_view::npos)
        {
            return {};
        }

        beforeParameters = trim(signature.substr(0U, parameterOpen));
    }

    if (beforeParameters.ends_with("operator"))
    {
        return compactQualifiedName(signature.substr(0U, parameterClose + 1U));
    }

    return compactQualifiedName(beforeParameters);
}

[[nodiscard]] std::size_t compilerSuffixPosition(std::string_view signature) noexcept
{
    std::size_t parenthesisDepth = 0U;
    for (std::size_t index = 0U; index + 1U < signature.size(); ++index)
    {
        if (signature[index] == '(')
        {
            ++parenthesisDepth;
            continue;
        }

        if (signature[index] == ')')
        {
            if (parenthesisDepth > 0U)
            {
                --parenthesisDepth;
            }
            continue;
        }

        if (parenthesisDepth != 0U || signature[index] != ' ' || signature[index + 1U] != '[')
        {
            continue;
        }

        const auto suffixEnd = signature.find(']', index + 2U);
        const auto assignment = signature.find(" = ", index + 2U);
        if (assignment != std::string_view::npos &&
            (suffixEnd == std::string_view::npos || assignment < suffixEnd))
        {
            return index;
        }
    }

    return std::string_view::npos;
}

[[nodiscard]] std::size_t lambdaPosition(std::string_view signature) noexcept
{
    constexpr std::array<std::string_view, 6U> markers{
        "<lambda", "{lambda", "(anonymous class)", "(lambda at ", "`lambda", "$_"};

    auto position = std::string_view::npos;
    for (const auto marker : markers)
    {
        const auto candidate = signature.find(marker);
        if (candidate != std::string_view::npos &&
            (position == std::string_view::npos || candidate < position))
        {
            position = candidate;
        }
    }

    return position;
}

[[nodiscard]] std::string compactLambdaName(std::string_view signature, std::size_t markerPosition)
{
    auto owner = trim(signature.substr(0U, markerPosition));
    while (owner.ends_with("::"))
    {
        owner = trim(owner.substr(0U, owner.size() - 2U));
    }

    auto compact = compactCallableName(owner);
    if (compact.empty())
    {
        return {};
    }

    compact += "::<lambda>";
    return compact;
}

} // namespace

std::string compactFunctionName(std::string_view signature)
{
    signature = trim(signature);
    if (signature.empty())
    {
        return {};
    }

    const auto compilerSuffix = compilerSuffixPosition(signature);
    if (compilerSuffix != std::string_view::npos)
    {
        signature = trim(signature.substr(0U, compilerSuffix));
    }

    const auto lambda = lambdaPosition(signature);
    if (lambda != std::string_view::npos)
    {
        return compactLambdaName(signature, lambda);
    }

    return compactCallableName(signature);
}

} // namespace sablelog::detail
