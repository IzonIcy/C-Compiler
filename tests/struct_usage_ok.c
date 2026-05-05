struct S { int a; float b; };
int get_a(struct S s) { return s.a; }
int main() {
    struct S s;
    s.a = 42;
    s.b = 3.14;
    return get_a(s);
}
