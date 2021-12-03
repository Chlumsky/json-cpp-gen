
#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include "DirectType.h"

class StructureType : public DirectType {

public:
    StructureType(const std::string &name, const StructureType *baseType);
    StructureType(std::string &&name, const StructureType *baseType);
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return true; }
    inline virtual bool isEnum() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    void addMember(const std::string &name, const Type *type);
    const std::vector<std::pair<std::string, const Type *> > & getMembers() const;
    const Type * findMember(const std::string &name) const;
    void finalize();
    bool isFinalized() const;

private:
    std::map<std::string, const Type *> members;
    std::vector<std::pair<std::string, const Type *> > orderedMembers;
    bool finalized = false;

};
