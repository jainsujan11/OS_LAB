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

#define TIME 1
#define RELEASE 0 
#define ADDITIONAL 1
#define QUIT 2

/* Overloaded functions to keep code simple */
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
/* Helper function commonly used in assg */
bool comp(const vector<int> &a,const vector<int>& b)
{
    assert(a.size() == b.size());
    for (int i = 0; i < static_cast<int>(a.size()); i++)
    {
        if(a[i] > b[i]) return true;
    }
    return false;
}


int m, n;
pthread_mutex_t rmtx = PTHREAD_MUTEX_INITIALIZER; /* used in mutual exclusion of global variables */
pthread_mutex_t pmtx = PTHREAD_MUTEX_INITIALIZER; /* used in printing statements */
/* Global Mutexes, Variables, Barriers specified in assg */
pthread_barrier_t BOS,REQB;
vector<pthread_barrier_t> ACKB;
vector<pthread_cond_t> cv;
vector<pthread_mutex_t> cmtx;
vector<vector<int>> NEED;
vector<int> done; /* used to prevent infinite wait on condition variable */

/* Global variables for copying */
int type;
int user;
vector<int> REQ;

void initall()
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
/* returns 0 if ADDITIONAL else 1 if RELEASE */
int check_resource_type(vector<int> &v)
{
    for(auto x : v ) if(x > 0) return 0;
    return 1;    
}
void* userthread(void* arg) {
    int i = (intptr_t)arg;
    pthread_mutex_lock(&pmtx);
    cout << "\tThread " << i << " born" << endl; 
    pthread_mutex_unlock(&pmtx);
    /* creating the file name */
    string num;
    if(i<10) num = "0" + to_string(i);
    else num = to_string(i);
    string fname = "input/thread" + num + ".txt";
    ifstream file(fname);
    /* 
       storing the MAX NEEDS in NEED vector 
       Nothing mentioned in ASSG on how to do it !! 
       so this is simplest and safe way every thread will write at distinct position 
       so no overwriting 
    */
    for (int j = 0; j < m; j++)
    {
        /*reading from file */
        file >> NEED[i][j];
    }
    /* Beginning of Session */
    pthread_barrier_wait(&BOS);
    while(1){
        int delay;char c; // denotes R or Q
        int flag; // denotes RELEASE or ADDITIONAL
        file >> delay;
        usleep(TIME*delay); // sleeping 
        pthread_mutex_lock(&rmtx);
        /* writing to global memory */
        done[i] = 0; /* flag variable to check if signal is not received before waiting. Set to zero even before sending request */
        file >> c;
        user = i;
        if(c == 'Q') type = QUIT;
        if(c == 'R')
        {
            // writing the requests 
            for (int j = 0; j < m; j++)
            {
                file >> REQ[j];    
            }
            flag = check_resource_type(REQ);
            type = ((flag==0)?ADDITIONAL:RELEASE);
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << i << " sends resource request: type = ";
            cout << (flag==0?"ADDITIONAL\n":"RELEASE\n");
            pthread_mutex_unlock(&pmtx);
        }
        pthread_barrier_wait(&REQB);
        pthread_barrier_wait(&ACKB[i]); // 2 way handshaking 
        pthread_mutex_unlock(&rmtx);
        if(c == 'Q') 
        {
            pthread_mutex_lock(&pmtx);
            cout << "\tThread " << i << " going to quit\n";
            pthread_mutex_unlock(&pmtx);
            file.close();
            break;
        }
        else if(c == 'R'){
            // ADDITIONAL request 
            if(flag == 0){
                // wait on condition variable 
                // if done becomes 1, then it means request is served, before sending every request done is set to 0
                pthread_mutex_lock(&cmtx[i]);
                while(done[i] == 0) pthread_cond_wait(&cv[i],&cmtx[i]);
                pthread_mutex_unlock(&cmtx[i]);
                pthread_mutex_lock(&pmtx);
                cout << "\tThread " << i << " is granted its last resource request\n";
                pthread_mutex_unlock(&pmtx);

            }
            // release request so no need to wait 
            else{
                pthread_mutex_lock(&pmtx);
                cout << "\tThread "<< i << " is done with its resource release request\n";
                pthread_mutex_unlock(&pmtx);

            }
        }
        else {
            // never supposed to happen 
            cerr << "Error in reading from file" << endl;
            return NULL;
        }
    }
    return NULL;
}
/* Bankers IsSafe algorithm */
int issafe(vector<int>& avail, vector<vector<int>>& need, vector<vector<int>>& alloc) {
    vector<bool> finish(n, false);
    vector<int> work = avail;
    int count = 0;
    while (count < n) {
        bool found = false;
        for (int p = 0; p < n; p++) {
            if (!finish[p] && !comp(need[p], work)) {
                work += alloc[p];
                finish[p] = true;
                count++;
                found = true;
            }
        }
        if (!found) return 0;
    }
    return 1;
}
/* Signals the cv when request is served */
void serve_req(int user)
{
    pthread_mutex_lock(&pmtx);
    cout << "Master thread grants resource request for thread " << user << endl;
    pthread_mutex_unlock(&pmtx);
    // used this so that signal is not sent before threads starts to wait on condition variable 
    pthread_mutex_lock(&cmtx[user]);
    done[user] = 1;
    pthread_cond_signal(&cv[user]);
    pthread_mutex_unlock(&cmtx[user]);
}
void start_simulation(vector<vector<int>>& ALLOC,vector<int>& AVAILABLE,vector<vector<int>>& NEED,queue<pair<int,vector<int>>>& q,vector<int>& alive,vector<int>& waiting)
{
    int active = n;
    while(active > 0)
    {
        pthread_barrier_wait(&REQB);
        // copied the global values to local variables
        int luser = user;
        int ltype = type;
        vector<int> lREQ = REQ;
        pthread_mutex_lock(&pmtx);
        cout << "Master thread stores resource request of thread " << luser << endl;
        pthread_mutex_unlock(&pmtx);
        pthread_barrier_wait(&ACKB[luser]); // 2 way handshaking 
        if(ltype == QUIT) 
        {
            /* release all the resources */
            active--;
            AVAILABLE += ALLOC[luser];
            NEED[luser] += ALLOC[luser];
            fill(ALLOC[luser].begin(), ALLOC[luser].end(), 0);
            fill(NEED[luser].begin(), NEED[luser].end(), 0);
            alive[luser] = 0;
            /* Printing info */
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
            if(active == 0) break;
        }
        else if(ltype == RELEASE){
            // directly allow the request 
            AVAILABLE -= lREQ; // subtracting because they are already negative 
            NEED[luser] -= lREQ;
            ALLOC[luser] += lREQ;
        }    
        else if(ltype == ADDITIONAL){
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
            // pushing in the queue 
            q.push({luser,lREQ});
            waiting.push_back(luser); // used for printing purpose 
        }
        else{
            // never suppose to occur 
            cerr << "Error Occured !!";
            return;
        }
        
        /* helper variables used to stores requests which can't be processed */
        queue<pair<int,vector<int>>> nq;
        vector<int> nwaiting;
        /* printing info */
        pthread_mutex_lock(&pmtx);
        cout << "\t\tWaiting Threads: ";
        for(auto x : waiting) cout << x << " ";
        cout << endl;
        cout << "Master thread tries to grant pending requests\n";
        pthread_mutex_unlock(&pmtx);

        while(q.size() > 0)
        {
            auto p = q.front();
            q.pop();
            auto &v = p.second;
            // check if v > AVAILABLE 
            if(comp(v,AVAILABLE) == 1) 
            {
                pthread_mutex_lock(&pmtx);
                cout << "\t+++ Insufficient resources to grant request of thread " << p.first << endl;
                pthread_mutex_unlock(&pmtx);
                // can't be processed now stored for later 
                nq.push(p);
                nwaiting.push_back(p.first);
            }
            else{
                // allow the request to be served  
                AVAILABLE -= v;
                NEED[p.first] -= v;
                ALLOC[p.first] += v;
                #ifndef _DLAVOID
                /* 
                   if no deadlock avoidance flag is set, then just serve this request without any checks.
                   MAY CAUSE DEADLOCKS !!  
                */
                serve_req(p.first);
                #endif
                #ifdef _DLAVOID
                /* run issafe algo after deadlock avoidance flag is set  */ 
                if(issafe(AVAILABLE,NEED,ALLOC))
                {
                    serve_req(p.first);
                }
                else{
                    // restore the state as issafe returns false
                    AVAILABLE += v;
                    NEED[p.first] += v;
                    ALLOC[p.first] -= v;
                    // push into nq as for future 
                    nq.push(p);
                    nwaiting.push_back(p.first);
                    pthread_mutex_lock(&pmtx);
                    cout << "\t+++ Unsafe to grant request of thread " << p.first << endl;
                    pthread_mutex_unlock(&pmtx);
                }
                #endif 
            }
        }
        /* storing the request which could not be served now */
        q = nq;
        waiting = nwaiting;
        /* printing info */
        pthread_mutex_lock(&pmtx);
        cout << "\t\tWaiting Threads: ";
        for(auto x : waiting) cout << x << " ";
        cout << endl;
        pthread_mutex_unlock(&pmtx);
    }
    return;
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

    initall(); /* initall called to initialise all the mutexes,barriers and condition variables and global variables */
    /* Creating all n userthreads */
    vector<pthread_t> threads(n);
    for (int i = 0; i < n; i++) {
        pthread_create(&threads[i], NULL, userthread, (void*)(intptr_t)i);
    }
    pthread_barrier_wait(&BOS);
    /* main function to perform all the allocation, passed with necessary arguments */
    start_simulation(ALLOC,AVAILABLE,NEED,q,alive,waiting);
    for (int i = 0; i < n; i++)
    {
        pthread_join(threads[i], NULL); // no need to cancel them as they will return 
    }
    delall();
    cout << "Hurray !! All Work done" << endl;
    cout << "Master/Main thread going to quit" << endl;
    return 0;
}
