typedef int foo;
typedef foo bar;
typedef missing baz;

int main() {
    baz x = 42;
    return 0;
}
