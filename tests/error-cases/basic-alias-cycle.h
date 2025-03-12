
typedef A B;
typedef B C;
typedef C A;

struct All {
    A a;
    B b;
    C c;
    int canary;
};
