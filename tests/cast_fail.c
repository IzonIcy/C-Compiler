struct S { int a; };
struct T { int b; };
int main() {
    struct S s;
    struct T *tp = (struct T*)&s; // ERROR: invalid cast between incompatible struct pointers
    return 0;
}
