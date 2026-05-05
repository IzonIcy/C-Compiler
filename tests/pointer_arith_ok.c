int main() {
    int arr[5];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    int *p = arr;
    *(p+3) = 7; // pointer arithmetic (should succeed)
    return arr[3]; // should return 7
}
