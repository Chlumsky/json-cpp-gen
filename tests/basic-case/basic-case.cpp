
#include "../common.h"
#include "structures.h"
#include <basic-case/Parser.h>
#include <basic-case/Serializer.h>

using namespace basic_case;

void test_basic_case() {
    std::shared_ptr<Animal> mothersPet(new Animal {
        Animal::DOG,
        Gender::MALE,
        false,
        5,
        .4,
        { {
            { .8, 1, .25 },
            { .2, .85, .85 }
        } }
    });
    std::shared_ptr<Animal> childrensPet(new Animal {
        Animal::CAT,
        Gender::FEMALE,
        true,
        2,
        .9,
        { {
            { 0, .5, 1 },
            { 0, .5, 1 }
        } }
    });
    std::shared_ptr<Person> billy(new Person {
        "Billy",
        Gender::MALE,
        12,
        { },
        {
            { .4, .2, 0 },
            { .4, .2, 0 }
        },
        nullptr,
        { },
        { { { "Whisker", childrensPet } } }
    });
    std::shared_ptr<Person> lucy(new Person {
        "Lucy",
        Gender::FEMALE,
        8,
        { },
        {
            { .3, .8, .1 },
            { .3, .8, .1 }
        },
        nullptr,
        { },
        { { { "Whisker", childrensPet } } }
    });
    std::shared_ptr<Person> mother(new Person {
        "Anne",
        Gender::FEMALE,
        33,
        { },
        {
            { .4, .2, 0 },
            { .4, .2, 0 }
        },
        nullptr,
        { billy, lucy },
        { { { "Rex", mothersPet } } }
    });
    std::shared_ptr<Person> father(new Person {
        "Jack",
        Gender::MALE,
        36,
        { "MSc.", "PhD." },
        {
            { .3, .8, .1 },
            { .3, .8, .1 }
        },
        mother,
        { billy, lucy },
        std::nullopt
    });
    BigStruct o, i = {
        {
            {
                {
                    "Smith",
                    {
                        mother,
                        father,
                        billy,
                        lucy
                    }, {
                        { "Rex", mothersPet },
                        { "Whisker", childrensPet }
                    },
                    14842560000
                }
            }, {
                {
                    Animal::LIZARD,
                    Gender::MALE,
                    false,
                    4,
                    0,
                    { {
                        { 0, .75, .25 },
                        { 0, .75, .25 }
                    } }
                }, {
                    Animal::MARSUPIAL,
                    Gender::FEMALE,
                    false,
                    9,
                    .35,
                    { {
                        { .25, .2, .1 },
                        { .25, .2, .1 }
                    } }
                }
            }
        }
    };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("basic-case", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(o == i);
}
