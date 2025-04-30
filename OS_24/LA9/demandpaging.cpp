#include <iostream>
#include <iomanip>
#include <queue> 
#include <vector> 
#include <string> 
#include <fstream>
using namespace std;
/* Kernel variables */
int page_access=0, page_faults=0, swaps=0;
int deg_of_mult;
int active = 0;
int n,m;
/* Page Table and functions associated with it */
struct PCB{
    // first 10 entries are reserved 
    // for accessing a entry use 10 + i/1024 to find page in which it is present 
    unsigned short int PT[2048];
    PCB(){
        for (int i = 0; i < 2048; i++)
        {
            PT[i] = 0;
        }
    }
    int get_page(int val)
    {
        return 10 + val/1024;
    }
    void set(int pg,int frame)
    {
        for (int i = 0; i < 14; i++)
        {
            if(((frame)&(1<<i))!=((PT[pg])&(1<<i))) PT[pg] ^= (1<<i);    
        }
        PT[pg] |= (1<<15);
    }
    int retreive(int val)
    {
        int idx = get_page(val);
        int fl  = (PT[idx]&(1<<15));
        return fl;
    }
    int clear(int pg)
    {
        if((PT[pg] & (1<<15)))
        {
            int temp = PT[pg]^(1<<15);
            PT[pg] = 0;
            return temp;
        }
        return -1;
    }
};
/* Simulation data */
struct info
{
    int l,r;
    queue<int> entries;
};
vector<PCB> table;
vector<info> code;
/* 3 queues */
queue<int> Q,free_frames,swapped_out;

void init()
{
    // kernel data initialisation 
    active = n;
    deg_of_mult = n+1;
    for (int i = 0; i < 12288; i++)
    {
        free_frames.push(i);
    }
    table.resize(n);
    for (int i = 0; i < n; i++)
    {
        Q.push(i);
        for (int j = 0; j < 10; j++)
        {
            table[i].set(j,free_frames.front());
            free_frames.pop();
        }
    }
}

int page_fault(int m,int cur)
{
    // assign a free frame from queue 
    if(free_frames.empty()) return 0;
    int fr = free_frames.front();
    free_frames.pop();
    table[cur].set(table[cur].get_page(m),fr);
    return 1;
}

int binary_search(int k,int cur)
{
    int l = code[cur].l; 
    int r = code[cur].r;
    while(l<r)
    {
        int m = (l+r)>>1;
        // check if m is available 
        page_access++;
        if(!table[cur].retreive(m)){
            // if not present in frame 
            page_faults++;
            if(!page_fault(m,cur)) return 0; // no frame free, so swap out 
        }
        if(k <= m) r=m;
        else l=m+1;
    }
    return 1;
}
void quit(int cur)
{
    // as per assignment
    active--;
    for (int i = 0; i < 2048; i++)
    {
        int x = table[cur].clear(i);
        if(x != -1) free_frames.push(x);
    }
}
void swap_out(int cur)
{
    // as per assignment
    active--;
    cout << "+++ Swapping out process";
    cout << right;
    cout << setw(5) << cur << " " << "[" << active << " active processes]" << endl;
    swaps++;
    deg_of_mult = min(deg_of_mult,active);
    for (int i = 0; i < 2048; i++)
    {
        int x = table[cur].clear(i);
        if(x != -1) free_frames.push(x);
    }
    swapped_out.push(cur);
}
void swap_in()
{
    if(swapped_out.empty()) return;
    active++;
    cout << "+++ Swapping in process";
    cout << right;
    int cur = swapped_out.front();
    cout << setw(6) << cur << " " << "[" << active << " active processes]" << endl;
    swapped_out.pop();
    #ifdef VERBOSE
        cout << "\tSearch " << m-(int)code[cur].entries.size()+1 << " by Process " << cur << endl;
    #endif
    int k = code[cur].entries.front();
    // simulate binary search 
    if(binary_search(k,cur)) 
    {
        code[cur].entries.pop();
        if(!code[cur].entries.empty()) Q.push(cur);
        else{
            quit(cur);
            swap_in();
        }
    }
    else{
        cout << "Not expected to happen !! :( " << endl;
        exit(1);
    }
    return;
}
void simulate()
{
    while(!Q.empty())
    {
        int cur = Q.front();
        Q.pop();
        #ifdef VERBOSE
            cout << "\tSearch " << m-(int)code[cur].entries.size()+1 << " by Process " << cur << endl;
        #endif
        int k = code[cur].entries.front();
        // simulate binary search 
        if(binary_search(k,cur)) 
        {
            // successful search meaning page got a frame 
            code[cur].entries.pop();
            // check if last frame 
            if(!code[cur].entries.empty()) Q.push(cur);
            else{
                // if yes 
                quit(cur);
                swap_in();
            }
        }
        else{
            // simulate a swap out
            swap_out(cur);
        }
    }
}

int main(int argc, char const *argv[])
{
    ifstream file("search.txt");
    if (!file) {
        cerr << "Error opening file\n";
        return EXIT_FAILURE;
    }
    file >> n >> m;
    code.resize(n);
    // reading entries from file 
    for (int i = 0; i < n; i++)
    {
        int s;
        file >> s;
        code[i].l = 0;
        code[i].r = s-1;
        for (int j = 0; j < m; j++)
        {
            int idx;
            file >> idx;
            code[i].entries.push(idx);
        }
    }
    cout << "+++ Simulation data read from file" << endl;
    init();
    cout << "+++ Kernel data initialized" << endl;
    simulate();
    cout << "+++ Page access summary" << endl;
    cout << "\t\tTotal number of page accesses  =  " << page_access << endl;
    cout << "\t\tTotal number of page faults    =  " << page_faults << endl;
    cout << "\t\tTotal number of swaps          =  " << swaps << endl;
    cout << "\t\tDegree of multiprogramming     =  " << deg_of_mult << endl;
    return 0;
}
