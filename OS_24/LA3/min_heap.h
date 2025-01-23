#define MAX_SIZE 1024
int size=0;

int LeftChild(int i) { return 2 * i; }
int RightChild(int i) { return 2 * i + 1; }
int Parent(int i) { return i / 2; }
int getMin(int H[]) { return H[1]; }
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void shiftUp(int H[],int n);

void shiftDown(int H[],int n);

void Insert(int H[], int newNum) {
    H[size + 1] = newNum;
    size++;
    shiftUp(H,size);
}

void Pop(int H[]) {
    H[1] = H[size];
    size--;
    shiftDown(H, 1);
}