
#include "StringType.h"

#include "../pattern-fill.h"
#include "../ParserGenerator.h"

static StringAPI stdStringAPI() {
    StringAPI api;
    api.clear = "$S.clear()";
    api.getLength = "$S.size()";
    api.getCharAt = "$S[$I]";
    api.appendChar = "$S.push_back($X)";
    api.appendCStr = "$S += $X";
    api.iterateChars = "for (std::string::const_iterator $I = $S.begin(), $Z = $S.end(); $I != $Z; ++$I) { char $E = *$I; $F }";
    api.equalsStringLiteral = "$S == $X";
    return api;
}

const StringType StringType::STD_STRING("std::string", stdStringAPI());

StringType::StringType(const std::string &name, const StringAPI &api) : Type(TypeName(name)), api(api) { }

StringType::StringType(std::string &&name, const StringAPI &api) : Type(TypeName((std::string &&) name)), api(api) { }

std::string StringType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('\"'))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::STRING_EXPECTED)+";\n";
    body += indent+generateClear("value")+";\n";
    body += indent+"while (*cur != '\"') {\n";
    body += indent+INDENT "if (*cur == '\\\\') {\n";
    body += indent+INDENT INDENT "char buffer[8];\n";
    if (generator->settings().noThrow) {
        body += indent+INDENT INDENT "if (Error error = unescape(buffer))\n";
        body += indent+INDENT INDENT INDENT "return error;\n";
    } else
        body += indent+INDENT INDENT "unescape(buffer);\n";
    body += indent+INDENT INDENT+generateAppendCStr("value", "buffer")+";\n";
    body += indent+INDENT INDENT "continue;\n";
    body += indent+INDENT "}\n";
    body += indent+INDENT "if (!*cur)\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::UNEXPECTED_END_OF_FILE)+";\n";
    body += indent+INDENT+generateAppendChar("value", "*cur")+";\n";
    body += indent+INDENT+"++cur;\n";
    body += indent+"}\n";
    body += indent+"++cur;\n";
    return body;
}

std::string StringType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"write('\"');\n";
    body += indent+generateIterateChars("value", "i", "end", "c", "writeEscaped(c);")+"\n";
    body += indent+"write('\"');\n";
    return body;
}

std::string StringType::generateClear(const char *subject) const {
    Replacer r[] = {
        { 'S', subject }
    };
    return fillPattern(api.clear, r, ARRAY_LENGTH(r));
}

std::string StringType::generateGetLength(const char *subject) const {
    Replacer r[] = {
        { 'S', subject }
    };
    return fillPattern(api.getLength, r, ARRAY_LENGTH(r));
}

std::string StringType::generateGetCharAt(const char *subject, const char *index) const {
    Replacer r[] = {
        { 'S', subject },
        { 'I', index }
    };
    return fillPattern(api.getCharAt, r, ARRAY_LENGTH(r));
}

std::string StringType::generateAppendChar(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.appendChar, r, ARRAY_LENGTH(r));
}

std::string StringType::generateAppendCStr(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.appendCStr, r, ARRAY_LENGTH(r));
}

std::string StringType::generateIterateChars(const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'E', elementName },
        { 'F', body }
    };
    return fillPattern(api.iterateChars, r, ARRAY_LENGTH(r));
}

std::string StringType::generateEqualsStringLiteral(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.equalsStringLiteral, r, ARRAY_LENGTH(r));
}
