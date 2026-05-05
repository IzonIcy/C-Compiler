int main() {
    int x = 5;
    int y = x + 2;
    int* p = &x;
    int z = p + y; // ERROR: pointer arith with non-pointer
    return z;
}
