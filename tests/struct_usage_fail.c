struct S { int a; };
int main() {
    struct S s;
    s.b = 4; // ERROR: member 'b' does not exist
    return 0;
}
