
#pragma once

#include <string>
#include <vector>
#include <map>

struct Settings {
    /// Specifies how the JSON data is received / outputted.
    enum class JsonIO {
        NULL_TERMINATED_STRING
        // Stream mode will be added in the future
    } jsonIOMode = JsonIO::NULL_TERMINATED_STRING;
    /// Parser / serialization errors can be thrown or returned. The latter should only be used for strictly no-throw codebases.
    bool noThrow = false;
    /// By default, not all of the JSON's syntax is checked to save performance. Enable strict syntax checking if you want to make sure that any invalid JSON file is detected.
    bool strictSyntaxCheck = false;
    /// If this is enabled, any time a JSON object is parsed into a C++ structure, but not all of its fields are present in the JSON, an error will be reported.
    bool checkMissingKeys = false;
    /// Unless repeating keys are checked, repeated instances of the same key will simply result in the field being overwritten.
    bool checkRepeatingKeys = false;
    /// When a JSON object contains an unknown element, it can either be ignored and its value silently skipped, or an error can be reported.
    bool ignoreExtraKeys = true;
    /// Unless enabled, parsing integers that are too large to fit the intended variable will simply truncate them.
    bool checkIntegerOverflow = true;
    /// In JSON, the forward slash character can be optionally escaped by a backslash. This is useful in some JavaScript applications.
    bool escapeForwardSlash = false;
    /// Empty optional values are normally serialized as null. However, in objects, they can be simply skipped.
    bool skipEmptyFields = true;
    /// The floating-point NAN value cannot be represented in JSON. The following dictates how it will be resolved in the serializer.
    enum class NanPolicy {
        SERIALIZER_ERROR,
        ZERO_VALUE, // 0
        NULL_VALUE, // null
        UPPERCASE_NAN_STRING_VALUE, // "NAN"
        LOWERCASE_NAN_STRING_VALUE, // "nan"
        MIXED_CASE_NAN_STRING_VALUE // "NaN"
    } nanPolicy = NanPolicy::SERIALIZER_ERROR;
    /// Floating-point infinity cannot be represented in JSON. The following dictates how it will be resolved in the serializer.
    enum class InfPolicy {
        SERIALIZER_ERROR,
        EXPONENT_OVERFLOW, // 1e999
        ZERO_VALUE, // 0
        NULL_VALUE, // null
        UPPERCASE_INF_STRING_VALUE, // "INF"
        LOWERCASE_INF_STRING_VALUE, // "inf"
        CAPITALIZED_INF_STRING_VALUE, // "Inf"
        UPPERCASE_INFINITY_STRING_VALUE, // "INFINITY"
        LOWERCASE_INFINITY_STRING_VALUE, // "infinity"
        CAPITALIZED_INFINITY_STRING_VALUE // "Infinity"
    } infPolicy = InfPolicy::EXPONENT_OVERFLOW;
    /// Line ending style of the generated C++ code (parsers and serializers)
    enum class LineEndingStyle {
        LF,
        CRLF,
        NATIVE,
        SAME_AS_INPUT
    } cppLineEndings = LineEndingStyle::SAME_AS_INPUT;
    /// Indentation style of the generated C++ code - the string represents one level of indentation
    std::string cppIndentation = "\t";
};

struct StringAPI {
    // $S = subject string
    std::string clear;
    // $S = subject string
    std::string getLength;
    // $S = subject string, $I = position of char
    std::string getCharAt;
    // $S = subject string, $X = char to append
    std::string appendChar;
    // $S = subject string, $X = C string to append
    std::string appendCStr;
    // $S = subject string, $X = string literal to append
    std::string appendStringLiteral;
    // $S = subject string, $X = string literal to compare
    std::string equalsStringLiteral;
    // $S = subject string, $I = iterator variable name, $Z = end variable name, $E = name of element (char) variable, $F = loop body
    std::string iterateChars;
};

struct ConstStringAPI {
    // $S = subject const string, $X = source string
    std::string copyFromString;
    // $S = subject const string, $X = source string
    std::string moveFromString;
    // $S = subject const string, $I = iterator variable name, $Z = end variable name, $E = name of element (char) variable, $F = loop body
    std::string iterateChars;
};

struct ArrayContainerAPI {
    // $T = element type, $S = subject array
    std::string clear;
    // $T = element type, $S = subject array
    std::string refAppended;
    // $T = element type, $S = subject array, $I = iterator variable name, $Z = end variable name, $E = name of element variable, $F = loop body
    std::string iterateElements;
};

struct FixedArrayContainerAPI {
    // $T = element type, $S = subject fixed array, $X = source array
    std::string copyFromArrayContainer;
    // $T = element type, $S = subject fixed array, $X = source array
    std::string moveFromArrayContainer;
    // $T = element type, $S = subject fixed array, $I = iterator variable name, $Z = end variable name, $E = name of element constref variable, $F = loop body
    std::string iterateElements;
};

struct StaticArrayContainerAPI {
    // $T = element type, $N = array length, $S = subject array, $I = array index
    std::string refByIndex;
};

struct ObjectContainerAPI {
    // $U = key string type, $T = element type, $S = subject object
    std::string clear;
    // $U = key string type, $T = element type, $S = subject object, $K = key
    std::string refByKey;
    // $U = key string type, $T = element type, $S = subject object, $I = iterator variable name, $Z = end variable name, $K = key variable name, $V = element constref variable name, $F = loop body
    std::string iterateElements;
};

struct OptionalContainerAPI {
    // $T = element type, $S = subject optional value
    std::string clear;
    // $T = element type, $S = subject optional value
    std::string refInitialized;
    // $T = element type, $S = subject optional value
    std::string hasValue;
    // $T = element type, $S = subject optional value
    std::string getValue;
};

struct Configuration {

    struct GeneratorDef {
        std::string name;
        //std::string baseClass; // TODO
        std::vector<std::string> types;
        std::vector<std::string> replacementIncludes;
        std::string headerOutput;
        std::string sourceOutput;
    };

    struct StringDef {
        std::string name;
        StringAPI api;
    };
    struct ConstStringDef {
        std::string name;
        std::string stringType;
        ConstStringAPI api;
    };
    struct ArrayContainerDef {
        std::string name;
        ArrayContainerAPI api;
    };
    struct FixedArrayContainerDef {
        std::string name;
        std::string arrayContainerType;
        FixedArrayContainerAPI api;
    };
    struct StaticArrayContainerDef {
        std::string name;
        StaticArrayContainerAPI api;
    };
    struct ObjectContainerDef {
        std::string name;
        std::string keyType;
        ObjectContainerAPI api;
    };
    struct ObjectMapContainerDef {
        std::string name;
        ObjectContainerAPI api;
    };
    struct OptionalContainerDef {
        std::string name;
        OptionalContainerAPI api;
    };

    std::vector<std::string> inputs;
    std::vector<std::string> includes;
    Settings settings;

    std::vector<GeneratorDef> parsers;
    std::vector<GeneratorDef> serializers;

    std::string stringType = "std::string";
    std::map<std::string, std::string> typeAliases;
    std::vector<StringDef> stringTypes;
    std::vector<ConstStringDef> constStringTypes;
    std::vector<ArrayContainerDef> arrayContainerTypes;
    std::vector<FixedArrayContainerDef> fixedArrayContainerTypes;
    std::vector<StaticArrayContainerDef> staticArrayContainerTypes;
    std::vector<ObjectContainerDef> objectContainerTypes;
    std::vector<ObjectMapContainerDef> objectMapContainerTypes;
    std::vector<OptionalContainerDef> optionalContainerTypes;

};
