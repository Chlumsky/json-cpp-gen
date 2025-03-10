
#pragma once

#include <array>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>

namespace basic_case {

enum class Gender {
    MALE,
    FEMALE
};

struct Color {
    float r, g, b;

    bool operator==(const Color &) const = default;
};

struct Animal {
    enum Type {
        DOG,
        CAT,
        LIZARD,
        MARSUPIAL
    } type;
    Gender gender;
    bool neutered;
    int age;
    double fluffiness;
    std::array<Color, 2> eyeColor;

    bool operator==(const Animal &) const = default;
};

typedef std::map<std::string, std::shared_ptr<Animal> > PetsByName;

struct Person {
    std::string name;
    Gender gender;
    unsigned age;
    std::vector<std::string> titles;
    Color eyeColor[2];
    std::shared_ptr<Person> wife;
    std::vector<std::shared_ptr<Person> > children;
    std::optional<PetsByName> pets;
};

using PersonList = std::vector<std::shared_ptr<Person> >;

struct Family {
    std::string surname;
    PersonList members;
    PetsByName pets;
    long long bankBalance;
};

struct BigStruct {
    struct {
        std::vector<Family> households;
        std::vector<Animal> wildlife;
    } neighborhood;
};

// Equality operators

inline bool samePets(const PetsByName &a, const PetsByName &b) {
    if (a.size() != b.size())
        return false;
    for (PetsByName::const_iterator aIt = a.begin(), bIt = b.begin(); aIt != a.end(); ++aIt, ++bIt) {
        if (!(aIt->first == bIt->first && ((!aIt->second && !bIt->second) || *aIt->second == *bIt->second)))
            return false;
    }
    return true;
}

inline bool operator==(const Person &a, const Person &b) {
    if (a.children.size() != b.children.size())
        return false;
    for (size_t i = 0; i < a.children.size(); ++i) {
        if (!((!a.children[i] && b.children[i]) || *a.children[i] == *b.children[i]))
            return false;
    }
    return (
        a.name == b.name &&
        a.gender == b.gender &&
        a.age == b.age &&
        a.titles == b.titles &&
        a.eyeColor[0] == b.eyeColor[0] &&
        a.eyeColor[1] == b.eyeColor[1] &&
        ((!a.wife && !b.wife) || *a.wife == *b.wife) &&
        ((!a.pets.has_value() && !b.pets.has_value()) || samePets(a.pets.value(), b.pets.value()))
    );
}

inline bool operator==(const Family &a, const Family &b) {
    if (a.members.size() != b.members.size())
        return false;
    for (size_t i = 0; i < a.members.size(); ++i) {
        if (!((!a.members[i] && b.members[i]) || *a.members[i] == *b.members[i]))
            return false;
    }
    return (
        a.surname == b.surname &&
        samePets(a.pets, b.pets) &&
        a.bankBalance == b.bankBalance
    );
}

inline bool operator==(const BigStruct &a, const BigStruct &b) {
    return (
        a.neighborhood.households == b.neighborhood.households &&
        a.neighborhood.wildlife == b.neighborhood.wildlife
    );
}

}
