
#include "IntegerType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

IntegerType::IntegerType(bool signedness, const std::string &name) : DirectType(TypeName(name)), signedness(signedness) { }

std::string IntegerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    generator->addFeature(signedness ? ParserGenerator::FEATURE_READ_SIGNED : ParserGenerator::FEATURE_READ_UNSIGNED);
    return (
        indent+"skipWhitespace();\n"+
        indent+(signedness ? "return readSigned(value);" : "return readUnsigned(value);") // no newline = returned
    );
}

std::string IntegerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    generator->addFeature(signedness ? SerializerGenerator::FEATURE_WRITE_SIGNED : SerializerGenerator::FEATURE_WRITE_UNSIGNED);
    if (signedness) {
        return (
            indent+"long long safeValue = static_cast<long long>(value);\n"+
            indent+"writeSigned<unsigned long long>(safeValue);\n"
        );
    } else
        return indent+"writeUnsigned(value);\n";
}
