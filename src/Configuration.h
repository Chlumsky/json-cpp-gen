
#pragma once

#include <string>
#include <vector>
#include "NameFormat.h"

struct Settings {
    /// Specifies how the JSON data is received / outputted.
    enum class JsonIO {
        NULL_TERMINATED_STRING
        // Stream mode will be added in the future
    } jsonIOMode = JsonIO::NULL_TERMINATED_STRING;
    /// Specifies how keys of structure member variables are formatted in the JSON file.
    NameFormat keyFormat = NameFormat::ANY;
    /// Specifies how enum values are formatted in the JSON file.
    NameFormat enumFormat = NameFormat::ANY;
    /// Parser / serialization errors can be thrown or returned. The latter should only be used for strictly no-throw codebases.
    bool noThrow = false;
    /// By default, errors are enumerated error codes. In verbose mode, they are instead a structure with the code and a message string.
    bool verboseErrors = false; // TODO IMPLEMENT
    /// By default, not all of the JSON's syntax is checked to save performance. Enable strict syntax checking if you want to make sure that any invalid JSON file is detected.
    bool strictSyntaxCheck = false;
    /// If this is enabled, any time a JSON object is parsed into a C++ structure, but not all of its fields are present in the JSON, an error will be reported.
    bool checkMissingKeys = true; // TODO IMPLEMENT
    /// Unless repeating keys are checked, repeated instances of the same key will simply result in the field being overwritten.
    bool checkRepeatingKeys = true; // TODO IMPLEMENT
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
    } nanPolicy = NanPolicy::SERIALIZER_ERROR; // TODO IMPLEMENT
    /// Floating-point infinity cannon be represented in JSON. The following dictates how it will be resolved in the serializer.
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
    } infPolicy = InfPolicy::EXPONENT_OVERFLOW; // TODO IMPLEMENT
};

struct StringAPI {
    // $S = subject string
    std::string clear;
    // $S = subject string, $X = char to append
    std::string appendChar;
    // $S = subject string, $X = C string to append
    std::string appendCStr;
    // $S = subject string, $E = name of element (char) variable, $F = loop body
    std::string iterateChars;
};

struct ConstStringAPI {
    // $S = subject fixed string, $X = source string
    std::string copyFromString;
    // $S = subject fixed string, $X = source string
    std::string moveFromString;
    // $S = subject fixed string, $E = name of element (char) variable, $F = loop body
    std::string iterateChars;
};

struct ArrayContainerAPI {
    // $T = element type, $S = subject array
    std::string clear;
    // $T = element type, $S = subject array
    std::string refAppended;
    // $T = element type, $S = subject array, $E = name of element variable, $F = loop body
    std::string iterateElements;
};

struct FixedArrayContainerAPI {
    // $T = element type, $S = subject fixed array, $X = source array
    std::string copyFromArrayContainer;
    // $T = element type, $S = subject fixed array, $X = source array
    std::string moveFromArrayContainer;
    // $T = element type, $S = subject fixed array, $E = name of element constref variable, $F = loop body
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
    // $U = key string type, $T = element type, $S = subject object, $I = iterator variable name, $K = key variable name, $V = element constref variable name, $F = loop body
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
        std::string baseClass;
        std::vector<std::string> types;
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
    std::vector<StringDef> stringTypes;
    std::vector<ConstStringDef> constStringTypes;
    std::vector<ArrayContainerDef> arrayContainerTypes;
    std::vector<FixedArrayContainerDef> fixedArrayContainerTypes;
    std::vector<StaticArrayContainerDef> staticArrayContainerTypes;
    std::vector<ObjectContainerDef> objectContainerTypes;
    std::vector<ObjectMapContainerDef> objectMapContainerTypes;
    std::vector<OptionalContainerDef> optionalContainerTypes;

};
