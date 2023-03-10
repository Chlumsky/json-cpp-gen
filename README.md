
# JSON-CPP-gen

This is a program that parses C++ structures from a header file
and automatically generates C++ code capable of serializing said structures
into the JSON format and parsing it back.

For example, if provided with the following structure as input,

```c++
struct User {
    int id;
    std::string name;
    std::vector<int> friendIds;
};
```

JSON-CPP-gen can generate a `UserParser` class that can be used as

```c++
User user;
auto error = UserParser::parse(user, jsonString);
```

to automatically parse a JSON string that matches the stucture, e.g.

```json
{
    "id": 137,
    "name": "John Smith",
    "friendIds": [ 63, 51, 206 ]
}
```

And analogously, it can generate a `UserSerializer` class for
the inverse operation.
Of course, the input may be more complex - see [`Configuration`](src/Configuration.h)
and the generated [`ConfigurationParser`](generated/ConfigurationParser.h)
for a more complex example.

The generated parsers and serializers are highly efficient because
no intermediate DOM or other auxiliary data structures are constructed -
data is read directly to / from the input structures.

## How to use

To build the program, simply use the provided [CMake file](CMakeLists.txt).
JSON-CPP-gen has no dependencies besides the C++ standard library.

To run the program, you must provide a JSON configuration file
that corresponds to the [`Configuration`](src/Configuration.h) structure
as its command line argument.
For example, to generate the `UserParser` and `UserSerializer` classes
described above, you could use a configuration file containing:

```json
{
    "inputs": [ "User.h" ],
    "includes": [ ],
    "settings": { },
    "parsers": [ {
        "name": "UserParser",
        "types": [ "User" ],
        "headerOutput": "UserParser.h",
        "sourceOutput": "UserParser.cpp"
    } ],
    "serializers": [ {
        "name": "UserSerializer",
        "types": [ "User" ],
        "headerOutput": "UserSerializer.h",
        "sourceOutput": "UserSerializer.cpp"
    } ],
    "stringType": "std::string"
}
```

All (non-absolute) file names are relative to the configuration file.
`includes` may contain additional include files that should be present
in the output files and `settings` may contain code generator
[settings](src/Configuration.h).
The `includes`, `settings`, and `stringType` fields could be omitted in this case
since they are empty or equal to the default value.
Even `parsers` or `serializers` could be omitted if you only needed one of the two.
Additionally, custom string and container data types can be defined
in the configuration file - see [below](#Custom-types).
Another example is the [configuration file](configuration-parser-config.json)
for the [`ConfigurationParser`](generated/ConfigurationParser.h) class.

## Features

- Supported C++ types:
    - All fundamental integer and floating-point types, `bool`, and `size_t`
    - Structures (`struct`)
    - Enumerations (`enum` and `enum class`) - serialized as strings
    - Static arrays, including multi-dimensional
    - Standard library types:
        - `std::string` for strings
        - `std::vector`, `std::deque`, `std::list` for dynamic-sized arrays
        - `std::array` for static-sized arrays
        - `std::map` for objects with arbitrary keys and homogeneous values
        - `std::optional` as well as `std::auto_ptr`, `std::shared_ptr`, and `std::unique_ptr` for optional values
    - Custom string, array, object, and optional types if defined in the configuration file, see [below](#Custom-types)
- Namespaces
- Structure inheritance (basic support)
- Full UTF-8 & UTF-16 support

Currently **NOT** supported but planned features:
- Out of order structure declaration - input header files must currently be in the correct order - WIP
- Anonymous structures
- Structure / enum declaration and variable declaration in one statement
- Omitting specific member variables from parsing and serialization - planned via annotations
- Structure members under different name in JSON - planned via annotations
- Classes - currently ignored due to their data being typically private but explicit enablement via annotations is planned - access to private members must be ensured by the user
- `#define`, `typedef`, `using` aliases - basic support planned

What will (probably) never be supported:
- Heterogeneous JSON objects - not really representable by static stuctures
- Unions - impossible to serialize without more advanced logic
- Raw pointers - no point in serializing memory addresses and unclear memory management otherwise
- Template structures
- More complex expression / macro evaluation (e.g. array of length `20*sizeof(User)`)

## Custom types

Even though the standard library string and container types are supported by default,
you are **not** locked into using these exclusively in order for the automatic
parser and serializer generation to work. In fact, you may configure JSON-CPP-gen
even to produce code where none of the C++ standard library is used.

However, in order to do that, you must provide a set of replacement classes,
and for each an API that dictates how it should be used.
The API consists of code snippets for specific operations with placeholders.
See comments in [Configuration.h](src/Configuration.h) for a full list of the placeholders.
The following are examples for each category of definable custom types
and how they should be written in the configuration file.
You can specify multiple types for each category.

These are mostly demonstrated on types from the standard library,
which however should not be put into the configuration file as they are present implicitly
and would cause collisions.

### String (dynamic)

```json
"stringTypes": [ {
    "name": "std::string",
    "api": {
        "clear": "$S.clear()",
        "getLength": "$S.size()",
        "getCharAt": "$S[$I]",
        "appendChar": "$S.push_back($X)",
        "appendCStr": "$S += $X",
        "iterateChars": "for (char $E : $S) { $F }",
        "equalsStringLiteral": "$S == $X"
    }
} ]
```

### Constant string

Represents a string type that cannot be constructed incrementally.
Does not have an analogue in the standard library.
An intermediate (dynamic) string type must be specified for the parser (can be `std::string`).

```json
"constStringTypes": [ {
    "name": "ConstString",
    "stringType": "std::string",
    "api": {
        "copyFromString": "$S = $X",
        "moveFromString": "$S = std::move($X)",
        "iterateChars": "for (char $E : $S) { $F }"
    }
} ]
```

### Array (dynamic)

```json
"arrayContainerTypes": [ {
    "name": "std::vector<$T>",
    "api": {
        "clear": "$S.clear()",
        "refAppended": "($S.emplace_back(), $S.back())",
        "iterateElements": "for ($T const &$E : $S) { $F }"
    }
} ]
```

Note: The `refAppended` operation must add an empty element at the end of the array
and return a modifiable reference to the new element.

### Fixed-length array

Represents a non-statically fixed-length array that cannot be constructed incrementally.
Does not have an analogue in the standard library.
An intermediate (dynamic) array type must be specified for the parser
(can be `std::vector` or another implicit array type).

```json
"fixedArrayContainerTypes": [ {
    "name": "FixedArray<$T>",
    "arrayContainerType": "std::vector",
    "api": {
        "copyFromArrayContainer": "$S = $X",
        "moveFromArrayContainer": "$S = std::move($X)",
        "iterateElements": "for ($T const &$E : $S) { $F }"
    }
} ]
```

### Static length array

```json
"staticArrayContainerTypes": [ {
    "name": "std::array<$T, $N>",
    "api": {
        "refByIndex": "$S[$I]"
    }
} ]
```

Note: `$N` is array length.

### Object (with implicit key type)

```json
"objectContainerTypes": [ {
    "name": "std::map<std::string, $T>",
    "keyType": "std::string",
    "api": {
        "clear": "$S.clear()",
        "refByKey": "$S[$K]",
        "iterateElements": "for (const std::pair<$U, $T> &$I : $S) { $U const &$K = $I.first; $T const &$V = $I.second; $F }"
    }
} ]
```

Notes: A key type must be specified as `keyType` (can be `std::string`).
The `refByKey` operation must create the element if it doesn't already exist
and return a modifiable reference to its value.
The `iterateElements` operation must provide keys and values separately as `$K` and `$V`.

### Object map (with explicit key type)

```json
"objectMapContainerTypes": [ {
    "name": "std::map<$K, $T>",
    "api": {
        "clear": "$S.clear()",
        "refByKey": "$S[$K]",
        "iterateElements": "for (const std::pair<$U, $T> &$I : $S) { $U const &$K = $I.first; $T const &$V = $I.second; $F }"
    }
} ]
```

### Optional value

```json
"optionalContainerTypes": [ {
    "name": "std::optional<$T>",
    "api": {
        "clear": "$S.reset()",
        "refInitialized": "($S = $T()).value()",
        "hasValue": "$S.has_value()",
        "getValue": "$S.value()"
    }
} ]
```

Note: The `refInitialized` operation must initialize the container
with a default-constructed value and return a modifiable reference to the value.
