#define MAXLEN 1024
typedef struct {
    int element [MAXLEN];
    int front;
    int back;
} queue;
queue initq ()
{
    queue Q;
    Q.front = 0;
    Q.back = MAXLEN - 1;
    return Q;
}
int isEmpty (queue Q)
{
    if (Q.front == (Q.back + 1) % MAXLEN) return 1;
    else return 0;
}
int isFull (queue Q)
{
    if (Q.front == (Q.back + 2) % MAXLEN) return 1;
    else return 0;
}
int front (queue Q)
{
    if (isEmpty(Q)) {
        return -1;
    }
    return Q.element [Q.front];
}
queue enqueue (queue Q, char ch) {
    ++Q.back;
    if (Q.back == MAXLEN) Q.back = 0;
    Q.element[Q.back] = ch;
    return Q;
}
queue dequeue (queue Q) {
    ++Q.front;
    if (Q.front == MAXLEN) Q.front = 0;
    return Q;
}