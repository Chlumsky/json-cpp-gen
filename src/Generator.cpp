
#include "Generator.h"

#include <cctype>
#include "NameFormat.h"

const char *const Generator::signature =
    "// Generated by json-cpp-gen by Viktor Chlumsky\n"
    "// https://github.com/Chlumsky/json-cpp-gen\n\n";

const unsigned Generator::FEATURE_CSTDLIB = 0x01;

Generator::Generator(const std::string &className, const StringType *stringType, const Settings &settings) : mStringType(stringType), mSettings(settings), featureBits(0) {
    std::string namePart;
    for (char c : className) {
        if (c == ':') {
            if (!namePart.empty()) {
                classNamespaces.push_back((std::string &&) namePart);
                namePart.clear();
            }
        } else
            namePart.push_back(c);
    }
    this->className = (std::string &&) namePart;
}

void Generator::addTypeInclude(const std::string &includeAddress) {
    typeIncludes.push_back(includeAddress);
}

void Generator::addFeature(unsigned featureBit) {
    featureBits |= featureBit;
}

std::string Generator::getJsonMemberNameLiteral(const std::string &memberName) const {
    return "\""+memberName+"\"";
}

std::string Generator::getJsonEnumValueLiteral(const std::string &enumValue) const {
    return "\""+enumValue+"\"";
}

void Generator::resolveVirtualTypename(const Type *type, const Type *parentType, const std::string &memberName) {
    if (type->name().substance() == TypeName::VIRTUAL && resolvedVirtualTypenames.insert(type->name().body()).second) {
        std::string typeName = parentType->name().body()+"::"+memberName;
        for (char c : type->name().suffix()) {
            if (c == '[')
                typeName += "[0]";
        }
        virtualTypedefs.emplace_back((std::string &&) typeName, type->name().body());
    }
}

std::string Generator::generateVirtualTypedefs(const std::string &indent) {
    std::string code;
    if (!virtualTypedefs.empty()) {
        code += indent+"template <typename T> struct NonRef { typedef T Type; };\n";
        code += indent+"template <typename T> struct NonRef<T &> { typedef T Type; };\n";
        code += indent+"template <typename T> struct NonRef<const T &> { typedef T Type; };\n\n";
        for (const std::pair<std::string, std::string> &virtualTypedef : virtualTypedefs)
            code += indent+"typedef typename NonRef<decltype("+virtualTypedef.first+")>::Type "+virtualTypedef.second+";\n";
        code += "\n";
    }
    return code;
}

std::string Generator::generateFunctionName(const char *prefix, const Type *type) {
    std::string functionName = prefix+formatName(type->name().body(), NameFormat::CAMELCASE_CAPITAL);
    for (char c : type->name().suffix()) {
        if (isalnum(c))
            functionName.push_back(c);
        else if (c == '[')
            functionName.push_back('_');
    }
    // Resolve collision
    if (usedFunctionNames.find(functionName) != usedFunctionNames.end()) {
        std::string baseFunctionName = functionName;
        if (!baseFunctionName.empty() && baseFunctionName.back() >= '0' && baseFunctionName.back() <= '9')
            baseFunctionName.push_back('_');
        int i = 0;
        do {
            functionName = baseFunctionName+std::to_string(++i);
        } while (usedFunctionNames.find(functionName) != usedFunctionNames.end());
    }
    usedFunctionNames.insert(functionName);
    return functionName;
}

std::string Generator::beginNamespace() const {
    std::string code;
    for (const std::string &namespaceName : classNamespaces)
        code += "namespace "+namespaceName+" {\n\n";
    return code;
}

std::string Generator::endNamespace() const {
    std::string code;
    for (size_t n = classNamespaces.size(); n; --n)
        code += "\n}\n";
    return code;
}

static bool isWordChar(char c) {
    return isalnum(c) || c == '_' || c&0x80;
}

static bool matchSubstr(const std::string &str, size_t pos, const char *needle) {
    while (pos < str.size() && *needle && str[pos++] == *needle++);
    return !*needle;
}

std::string Generator::safeName(const std::string &name) {
    if (name.empty() || name.front() == ':')
        return name;
    bool nameStart = true;
    for (size_t i = 0; i < name.size() && (isWordChar(name[i]) || name[i] == ':'); ++i) {
        if (nameStart && (
            (matchSubstr(name, i, "Error") && !isWordChar(name.c_str()[i+sizeof("Error")-1])) ||
            (matchSubstr(name, i, "NonRef") && !isWordChar(name.c_str()[i+sizeof("NonRef")-1])) ||
            (matchSubstr(name, i, UNNAMED_PREFIX) && name.c_str()[i+sizeof(UNNAMED_PREFIX)-1] >= '0' && name.c_str()[i+sizeof(UNNAMED_PREFIX)-1] <= '9')
        )) {
            // Add global namespace if name contains names used by parsers or serializers
            return "::"+name;
        }
        nameStart = name[i] == ':';
    }
    return name;
}

std::string Generator::charLiteral(char c) {
    switch (c) {
        case '\0': return "'\\0'";
        case '\b': return "'\\b'";
        case '\f': return "'\\f'";
        case '\n': return "'\\n'";
        case '\r': return "'\\r'";
        case '\t': return "'\\t'";
        case '\'': return "'\\''";
        case '\\': return "'\\\\'";
    }
    if (c >= 0x20 && c < 0x7f) {
        char buffer[] = "'#'";
        buffer[1] = c;
        return buffer;
    } {
        char buffer[] = "'\\x##'";
        buffer[3] = "0123456789abcdef"[c>>4&0x0f];
        buffer[4] = "0123456789abcdef"[c&0x0f];
        return buffer;
    }
}
