
#include "StringType.h"

#include "../pattern-fill.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

static StringAPI stdStringAPI() {
    StringAPI api;
    api.clear = "$S.clear()";
    api.getLength = "$S.size()";
    api.getCharAt = "$S[$I]";
    api.appendChar = "$S.push_back($X)";
    api.appendCStr = "$S += $X";
    api.appendStringLiteral = "$S += $X";
    api.equalsStringLiteral = "$S == $X";
    api.iterateChars = "for (std::string::const_iterator $I = $S.begin(), $Z = $S.end(); $I != $Z; ++$I) { char $E = *$I; $F }";
    return api;
}

const StringType StringType::STD_STRING("std::string", stdStringAPI());

StringType::StringType(const std::string &name, const StringAPI &api) : Type(TypeName(name)), api(api) {
    if (this->api.appendStringLiteral.empty())
        this->api.appendStringLiteral = this->api.appendCStr;
}

StringType::StringType(std::string &&name, const StringAPI &api) : Type(TypeName((std::string &&) name)), api(api) {
    if (this->api.appendStringLiteral.empty())
        this->api.appendStringLiteral = this->api.appendCStr;
}

std::string StringType::generateUnescapeBody(ParserGenerator *generator, const char *outputName, const std::string &indent) const {
    std::string body;
    body += indent+"++cur;\n";
    body += indent+"switch (*cur++) {\n";
    body += indent+INDENT "case '\\0':\n";
    body += indent+INDENT INDENT "--cur;\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::UNEXPECTED_END_OF_FILE)+";\n";
    body += indent+INDENT "case 'B': case 'b': "+generateAppendChar(outputName, "'\\b'")+"; break;\n";
    body += indent+INDENT "case 'F': case 'f': "+generateAppendChar(outputName, "'\\f'")+"; break;\n";
    body += indent+INDENT "case 'N': case 'n': "+generateAppendChar(outputName, "'\\n'")+"; break;\n";
    body += indent+INDENT "case 'R': case 'r': "+generateAppendChar(outputName, "'\\r'")+"; break;\n";
    body += indent+INDENT "case 'T': case 't': "+generateAppendChar(outputName, "'\\t'")+"; break;\n";
    body += indent+INDENT "case 'U': case 'u': {\n";
    body += indent+INDENT INDENT+"unsigned long cp;\n";
    body += indent+INDENT INDENT+"int wc;\n";
    if (generator->settings().noThrow) {
        body += indent+INDENT INDENT+"if (!readHexQuad(wc))\n";
        body += indent+INDENT INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+INDENT INDENT+"readHexQuad(wc);\n";
    body += indent+INDENT INDENT+"if ((wc&0xfc00) == 0xd800) {\n";
    body += indent+INDENT INDENT INDENT+"if (!(cur[0] == '\\\\' && (cur[1] == 'u' || cur[1] == 'U')))\n";
    body += indent+INDENT INDENT INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::UTF16_ENCODING_ERROR)+";\n";
    body += indent+INDENT INDENT INDENT+"cp = (unsigned long) ((wc&0x03ff)<<10);\n";
    body += indent+INDENT INDENT INDENT+"cur += 2;\n";
    if (generator->settings().noThrow) {
        body += indent+INDENT INDENT INDENT+"if (!readHexQuad(wc))\n";
        body += indent+INDENT INDENT INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+INDENT INDENT INDENT+"readHexQuad(wc);\n";
    body += indent+INDENT INDENT INDENT+"if ((wc&0xfc00) != 0xdc00)\n";
    body += indent+INDENT INDENT INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::UTF16_ENCODING_ERROR)+";\n";
    body += indent+INDENT INDENT INDENT+"cp = 0x010000+(cp|(unsigned long) (wc&0x03ff));\n";
    body += indent+INDENT INDENT+"} else\n";
    body += indent+INDENT INDENT INDENT+"cp = (unsigned long) wc;\n";
    body += indent+INDENT INDENT+"if (cp&0xffffff80) {\n";
    body += indent+INDENT INDENT INDENT+"int len;\n";
    body += indent+INDENT INDENT INDENT+"for (len = 1; cp>>(5*len+1) && len < 6; ++len);\n";
    body += indent+INDENT INDENT INDENT+generateAppendChar(outputName, "(char) (0xff<<(8-len)|cp>>6*(len-1))")+";\n";
    body += indent+INDENT INDENT INDENT+"for (int i = 1; i < len; ++i)\n";
    body += indent+INDENT INDENT INDENT INDENT+generateAppendChar(outputName, "(char) (0x80|(cp>>6*(len-i-1)&0x3f))")+";\n";
    body += indent+INDENT INDENT+"} else\n";
    body += indent+INDENT INDENT INDENT+generateAppendChar(outputName, "(char) cp")+";\n";
    body += indent+INDENT INDENT+"break;\n";
    body += indent+INDENT "}\n";
    body += indent+INDENT "default:\n";
    body += indent+INDENT INDENT+generateAppendChar(outputName, "cur[-1]")+";\n";
    body += indent+"}\n";
    return body;
}

std::string StringType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('\"'))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::STRING_EXPECTED)+";\n";
    body += indent+generateClear("value")+";\n";
    body += indent+"while (*cur != '\"') {\n";
    body += indent+INDENT "if (*cur == '\\\\') {\n";
    body += generateUnescapeBody(generator, "value", indent+INDENT INDENT);
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
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'\"'")+";\n";
    body += indent+generateIterateChars("value", "i", "end", "c", "writeEscaped(c);")+"\n";
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'\"'")+";\n";
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

std::string StringType::generateAppendStringLiteral(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.appendStringLiteral, r, ARRAY_LENGTH(r));
}

std::string StringType::generateEqualsStringLiteral(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.equalsStringLiteral, r, ARRAY_LENGTH(r));
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
