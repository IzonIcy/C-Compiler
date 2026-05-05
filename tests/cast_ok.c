int main() {
    int x = 123;
    void *p = (void*)&x;
    int *ip = (int*)p;
    return *ip;
}
