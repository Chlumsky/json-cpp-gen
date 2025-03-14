
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
    body += indent+"\tcase '\\0':\n";
    body += indent+"\t\t--cur;\n";
    body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::UNEXPECTED_END_OF_FILE)+";\n";
    body += indent+"\tcase 'B': case 'b': "+generateAppendChar(outputName, "'\\b'")+"; break;\n";
    body += indent+"\tcase 'F': case 'f': "+generateAppendChar(outputName, "'\\f'")+"; break;\n";
    body += indent+"\tcase 'N': case 'n': "+generateAppendChar(outputName, "'\\n'")+"; break;\n";
    body += indent+"\tcase 'R': case 'r': "+generateAppendChar(outputName, "'\\r'")+"; break;\n";
    body += indent+"\tcase 'T': case 't': "+generateAppendChar(outputName, "'\\t'")+"; break;\n";
    body += indent+"\tcase 'U': case 'u': {\n";
    body += indent+"\t\tunsigned long cp;\n";
    body += indent+"\t\tint wc;\n";
    if (generator->settings().noThrow) {
        body += indent+"\t\tif (!readHexQuad(wc))\n";
        body += indent+"\t\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+"\t\treadHexQuad(wc);\n";
    body += indent+"\t\tif ((wc&0xfc00) == 0xd800) {\n";
    body += indent+"\t\t\tif (!(cur[0] == '\\\\' && (cur[1] == 'u' || cur[1] == 'U')))\n";
    body += indent+"\t\t\t\t"+generator->generateErrorStatement(ParserGenerator::Error::UTF16_ENCODING_ERROR)+";\n";
    body += indent+"\t\t\tcp = (unsigned long) ((wc&0x03ff)<<10);\n";
    body += indent+"\t\t\tcur += 2;\n";
    if (generator->settings().noThrow) {
        body += indent+"\t\t\tif (!readHexQuad(wc))\n";
        body += indent+"\t\t\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+"\t\t\treadHexQuad(wc);\n";
    body += indent+"\t\t\tif ((wc&0xfc00) != 0xdc00)\n";
    body += indent+"\t\t\t\t"+generator->generateErrorStatement(ParserGenerator::Error::UTF16_ENCODING_ERROR)+";\n";
    body += indent+"\t\t\tcp = 0x010000+(cp|(unsigned long) (wc&0x03ff));\n";
    body += indent+"\t\t} else\n";
    body += indent+"\t\t\tcp = (unsigned long) wc;\n";
    body += indent+"\t\tif (cp&0xffffff80) {\n";
    body += indent+"\t\t\tint len;\n";
    body += indent+"\t\t\tfor (len = 1; cp>>(5*len+1) && len < 6; ++len);\n";
    body += indent+"\t\t\t"+generateAppendChar(outputName, "(char) (0xff<<(8-len)|cp>>6*(len-1))")+";\n";
    body += indent+"\t\t\tfor (int i = 1; i < len; ++i)\n";
    body += indent+"\t\t\t\t"+generateAppendChar(outputName, "(char) (0x80|(cp>>6*(len-i-1)&0x3f))")+";\n";
    body += indent+"\t\t} else\n";
    body += indent+"\t\t\t"+generateAppendChar(outputName, "(char) cp")+";\n";
    body += indent+"\t\tbreak;\n";
    body += indent+"\t}\n";
    body += indent+"\tdefault:\n";
    body += indent+"\t\t"+generateAppendChar(outputName, "cur[-1]")+";\n";
    body += indent+"}\n";
    return body;
}

std::string StringType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('\"'))\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::STRING_EXPECTED)+";\n";
    body += indent+generateClear("value")+";\n";
    body += indent+"while (*cur != '\"') {\n";
    body += indent+"\tif (*cur == '\\\\') {\n";
    body += generateUnescapeBody(generator, "value", indent+"\t\t");
    body += indent+"\t\tcontinue;\n";
    body += indent+"\t}\n";
    body += indent+"\tif (!*cur)\n";
    body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::UNEXPECTED_END_OF_FILE)+";\n";
    body += indent+"\t"+generateAppendChar("value", "*cur")+";\n";
    body += indent+"\t++cur;\n";
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
