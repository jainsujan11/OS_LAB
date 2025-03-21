#include <iostream>
#include <vector>
#include <string> 
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <queue> 
#include <pthread.h>
#include <cassert> 
using namespace std;

#define TIME 100000
#define RELEASE 0 
#define ADDITIONAL 1


vector<int>& operator-=(vector<int>& a, const vector<int>& b) {
    for (size_t i = 0; i < a.size(); i++) {
        a[i] -= b[i];
    }
    return a;
}
vector<int>& operator+=(vector<int>& a, const vector<int>& b) {
    for (size_t i = 0; i < a.size(); i++) {
        a[i] += b[i];
    }
    return a;
}




int m, n;
pthread_mutex_t rmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t BOS,REQB;
vector<pthread_barrier_t> ACKB;
vector<pthread_cond_t> cv;
vector<pthread_mutex_t> cmtx;
vector<int> done;
vector<vector<int>> NEED;

/* Global variables */
char type;
int user;
vector<int> REQ;





bool comp(const vector<int> &a,const vector<int>& b)
{
    assert(a.size() == b.size());
    for (int i = 0; i < static_cast<int>(a.size()); i++)
    {
        if(a[i] > b[i]) return true;
    }
    return false;
}
int check(vector<int> &v)
{
    for(auto x : v ) if(x > 0) return 0;
    return 1;    
}
void* userthread(void* arg) {
    int i = (intptr_t)arg;
    pthread_mutex_lock(&pmtx);
    cout << "\tThread " << i << " born" << endl; 
    pthread_mutex_unlock(&pmtx);
    string num;
    if(i<10) num = "0" + to_string(i);
    else num = to_string(i);
    string fname = "input/thread" + num + ".txt";
    ifstream file(fname);
    for (int j = 0; j < m; j++)
    {
        file >> NEED[i][j];
    }
    pthread_barrier_wait(&BOS);
    while(1){
        int delay;
        file >> delay;
        usleep(TIME*delay);
        pthread_mutex_lock(&rmtx);
        done[i] = 0;
        file >> type;
        user = i;
        int flag;
        if(type == 'R')
        {
            for (int j = 0; j < m; j++)
            {
                file >> REQ[j];    
            }
            flag = check(REQ);
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << i << " sends resource request: type = ";
            cout << (flag==0?"ADDITIONAL\n":"RELEASE\n");
            pthread_mutex_unlock(&pmtx);
        }
        pthread_barrier_wait(&REQB);
        pthread_barrier_wait(&ACKB[i]);
        if(type == 'Q')
        {
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << i << " going to quit\n";
            pthread_mutex_unlock(&pmtx);
            pthread_mutex_unlock(&rmtx);
            break;
        }
        pthread_mutex_unlock(&rmtx);
        if(flag == 0){
            // blocking req 
            pthread_mutex_lock(&cmtx[i]);
            while(done[i] == 0) pthread_cond_wait(&cv[i],&cmtx[i]);
            pthread_mutex_unlock(&cmtx[i]);
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << i << " is granted its last resource request\n";
            pthread_mutex_unlock(&pmtx);

        }
        else{
            pthread_mutex_lock(&pmtx);
            cout << "\tThread "<< i << " is done with its resource release request\n";
            pthread_mutex_unlock(&pmtx);

        }
    }
    return NULL;
}
void init()
{
    // start doing book keeping 
    pthread_mutex_trylock(&rmtx);   
    pthread_mutex_unlock(&rmtx);
    pthread_mutex_trylock(&pmtx);
    pthread_mutex_unlock(&pmtx);
    pthread_barrier_init(&BOS, NULL, n + 1);
    pthread_barrier_init(&REQB, NULL, 2);
    ACKB.resize(n);
    cv.resize(n);
    cmtx.resize(n);
    done.resize(n);
    for (int i = 0; i < n; i++) {
        done[i] = 0;
        pthread_barrier_init(&ACKB[i], NULL, 2);
        pthread_cond_init(&cv[i], NULL);
        pthread_mutex_init(&cmtx[i], NULL);
        pthread_mutex_trylock(&cmtx[i]);
        pthread_mutex_unlock(&cmtx[i]);
    }
}
int issafe(vector<int>& avail, vector<vector<int>>& need, vector<vector<int>>& alloc) {
    int P = need.size(), R = avail.size();
    vector<bool> finish(P, false);
    vector<int> work = avail;
    int count = 0;

    while (count < P) {
        bool found = false;
        for (int p = 0; p < P; p++) {
            if (!finish[p]) {
                int j;
                for (j = 0; j < R; j++) {
                    if (need[p][j] > work[j])
                        break;
                }
                if (j == R) {
                    for (int k = 0; k < R; k++)
                        work[k] += alloc[p][k];
                    finish[p] = true;
                    count++;
                    found = true;
                }
            }
        }
        if (!found)
            return 0;
    }
    return 1;
}

void start_simulation()
{
    return;
}

void delall()
{
    for (int i = 0; i < n; i++) {
        pthread_barrier_destroy(&ACKB[i]);
        pthread_cond_destroy(&cv[i]);
        pthread_mutex_destroy(&cmtx[i]);
    }
    pthread_barrier_destroy(&BOS);
    pthread_barrier_destroy(&REQB);
    pthread_mutex_destroy(&pmtx);
    pthread_mutex_destroy(&rmtx);
}
int main() {
    ifstream file("input/system.txt");
    if (!file) {
        cerr << "Error opening file\n";
        return EXIT_FAILURE;
    }
    file >> m >> n;
    /* Alloc, Need, and Available declarations */
    vector<vector<int>> ALLOC(n,vector<int>(m,0));
    vector<int> AVAILABLE(m,0);
    NEED.resize(n,vector<int>(m,0));
    /* Queue for storing the requests */
    queue<pair<int,vector<int>>> q;
    /* Global vector for sending requests */
    REQ.resize(m);
    /* Vectors used for storing info helpful in printing */
    vector<int> alive(n,1);
    vector<int> waiting;
    /* Initializing Available vector from system.txt */
    for (int i = 0; i < m; i++) { file >> AVAILABLE[i]; }
    file.close();

    init(); /* init called to initialise all the mutexes,barriers and condition variables */
    /* Creating all n userthreads */
    vector<pthread_t> threads(n);
    for (int i = 0; i < n; i++) {
        pthread_create(&threads[i], NULL, userthread, (void*)(intptr_t)i);
    }
    pthread_barrier_wait(&BOS);
    int active = n;
    while(active > 0)
    {
        pthread_barrier_wait(&REQB);
        // copied to local 
        int luser = user;
        char ltype = type;
        vector<int> lREQ = REQ;
        pthread_mutex_lock(&pmtx);
        cout << "Master thread stores resource request of thread " << luser << endl;
        pthread_mutex_unlock(&pmtx);

        pthread_barrier_wait(&ACKB[luser]);
        if(ltype == 'Q') 
        {
            active--;
            // release all the resources 
            AVAILABLE += ALLOC[luser];
            NEED[luser] += ALLOC[luser];
            fill(ALLOC[luser].begin(), ALLOC[luser].end(), 0);
            fill(NEED[luser].begin(), NEED[luser].end(), 0);
            alive[luser] = 0;
            pthread_mutex_lock(&pmtx);
            cout << active << " threads left: ";
            for (int i = 0; i < n; i++)
            {
                if(alive[i]) cout << i << " ";
            }
            cout << endl;
            cout << "Available resources: ";
            for(auto x : AVAILABLE) cout << x << " ";
            cout << endl;
            pthread_mutex_unlock(&pmtx);
        }
        else{
            if(check(lREQ) == 1)
            {
                // means release 
                AVAILABLE -= lREQ; // basically adding as negative 
                ALLOC[luser] += lREQ;
                NEED[luser] -= lREQ;
            }    
            else{
                // first release the non negative entries 
                for (int j = 0; j < m; j++)
                {
                    if(lREQ[j] <= 0)
                    {
                        AVAILABLE[j] -= lREQ[j];
                        ALLOC[luser][j] += lREQ[j];
                        NEED[luser][j] -= lREQ[j];
                        lREQ[j] = 0;
                    }
                }
                q.push({luser,lREQ});
                waiting.push_back(luser);
            }
        }
        queue<pair<int,vector<int>>> nq;
        vector<int> nwaiting;
        pthread_mutex_lock(&pmtx);
        cout << "\t\tWaiting Threads: ";
        for(auto x : waiting) cout << x << " ";
        cout << endl;
        cout << "Master thread tries to grant pending requests\n";
        pthread_mutex_unlock(&pmtx);

        while(q.size() > 0)
        {
            // pthread_mutex_lock(&pmtx);
            // cout << "\t\t**AVAILABLE: ";
            // for(auto x : AVAILABLE) cout << x << " ";
            // cout << endl;
            // pthread_mutex_unlock(&pmtx);
            auto p = q.front();
            q.pop();
            auto &v = p.second;
            if(comp(v,AVAILABLE)) 
            {
                pthread_mutex_lock(&pmtx);
                cout << "\t+++ Insufficient resources to grant request of thread " << p.first << endl;
                pthread_mutex_unlock(&pmtx);

                nq.push(p);
                nwaiting.push_back(p.first);
            }
            else{
                AVAILABLE -= v;
                NEED[p.first] -= v;
                ALLOC[p.first] += v;
                #ifndef _DLAVOID
                // if no deadlock avoidance flag is set then serve this request
                pthread_mutex_lock(&pmtx);
                cout << "Master thread grants resource request for thread " << p.first << endl;
                pthread_mutex_unlock(&pmtx);

                pthread_mutex_lock(&cmtx[p.first]);
                done[p.first] = 1;
                pthread_cond_signal(&cv[p.first]);
                pthread_mutex_unlock(&cmtx[p.first]);
                #endif
                #ifdef _DLAVOID
                // run issafe algo 
                if(issafe(AVAILABLE,NEED,ALLOC))
                {
                    pthread_mutex_lock(&pmtx);
                    cout << "Master thread grants resource request for thread " << p.first << endl;
                    pthread_mutex_unlock(&pmtx);    
                    pthread_mutex_lock(&cmtx[p.first]);
                    done[p.first] = 1;
                    pthread_cond_signal(&cv[p.first]);
                    pthread_mutex_unlock(&cmtx[p.first]);
                }
                else{
                    AVAILABLE += v;
                    NEED[p.first] += v;
                    ALLOC[p.first] -= v;
                    nq.push(p);
                    nwaiting.push_back(p.first);
                    pthread_mutex_lock(&pmtx);
                    cout << "\t+++ Unsafe to grant request of thread " << p.first << endl;
                    pthread_mutex_unlock(&pmtx);
                }
                #endif 
            }
        }
        q = nq;
        waiting = nwaiting;
        pthread_mutex_lock(&pmtx);
        cout << "\t\tWaiting Threads: ";
        for(auto x : waiting) cout << x << " ";
        cout << endl;
        pthread_mutex_unlock(&pmtx);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(threads[i], NULL); // no need to cancel them as they will return 
    }
    delall();
    cout << "Hurray !! All Work done" << endl;
    cout << "Master thread going to quit" << endl;
    return 0;
}
