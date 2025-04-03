#include <iostream>
#include <iomanip>
#include <queue> 
#include <vector> 
#include <string> 
#include <fstream>
#include <cassert>
using namespace std;

#define NFFMIN 1000
#define SIZE 12288
struct node{
    int fno;int pno;int pid;
    node(){
        fno = 0;
        pno = -1;
        pid = -1;
    }
};
struct FFinfo {
    node FFLIST[SIZE];
    int NFF;
    int ptr; // pointer to first free frame 
    FFinfo() {
        NFF = SIZE;
        ptr = 0;
    }
    
    void deallocate(int frame) {
        NFF++;
        ptr--;
        FFLIST[ptr].fno = frame;
        FFLIST[ptr].pid = FFLIST[ptr].pno = -1;
    }   
    int allocate(int page,int pid)
    {
        NFF--;
        int fno = FFLIST[ptr].fno;
        ptr++;
        return fno;
    }
};
struct PCB{
    unsigned short int PT[2048];
    unsigned short int counter[2048];
    PCB(){
        for (int i = 0; i < 2048; i++)
        {
            PT[i] = 0;
            counter[i] = 0;
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
    void set_ref(int pg)
    {
        PT[pg] |= (1<<14);
    }
    void set_cnt(int pg)
    {
        counter[pg] = 0xffff;
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
            if((PT[pg] & (1<<14))) temp = temp^(1<<14);
            PT[pg] = 0;
            return temp; // return the frame which was stored 
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
queue<int> Q;
int n,m;
FFinfo FF;
vector<int> access, replacements, faults;
vector<array<int,4>> attempt;

void init()
{
    for (int i = 0; i < SIZE ; i++)
    {
        FF.FFLIST[i].fno = i;
    }
    table.resize(n);
    access.assign(n,0);
    faults.assign(n,0);
    replacements.assign(n,0);
    attempt.assign(n,array<int,4>{0});
    for (int i = 0; i < n; i++)
    {
        Q.push(i);
        for (int j = 0; j < 10; j++)
        {
            int fno = FF.allocate(j,i);
            table[i].set(j,fno);
            table[i].set_ref(j);
            table[i].set_cnt(j);
        }
    }
    
}

int min_history(int cur,unsigned short int& mini)
{
    mini = 0xffff;
    int page = 0;
    for (int j = 10; j < 2048; j++)
    {
        if((table[cur].PT[j]&(1<<15)))
        {
            if(mini > table[cur].counter[j])
            {
                mini = table[cur].counter[j];
                page = j;
            }
        }
    }
    return page;
}

int attempt_1(int cur,int p)
{
    assert(FF.ptr == 11288);
    for (int j = FF.ptr; j < SIZE; j++)
    {
        if(FF.FFLIST[j].pid == cur && FF.FFLIST[j].pno == p)
        {
            #ifdef VERBOSE
                cout << "\t\tAttempt 1: Page found in free frame " << FF.FFLIST[j].fno << endl;
            #endif
            attempt[cur][0]++;
            return j;
        }
    }
    return -1;
}
int attempt_2(int cur)
{
    assert(FF.ptr == 11288);
    for (int j = FF.ptr; j < SIZE; j++)
    {
        if(FF.FFLIST[j].pid == -1 && FF.FFLIST[j].pno == -1)
        {
            #ifdef VERBOSE
                cout << "\t\tAttempt 2: Free frame " << FF.FFLIST[j].fno << " owned by no process found\n";
            #endif 
            attempt[cur][1]++;
            return j;
        }
    }
    return -1;
}
int attempt_3(int cur)
{
    assert(FF.ptr == 11288);
    for (int j = FF.ptr; j < SIZE; j++)
    {
        if(FF.FFLIST[j].pid == cur)
        {
            #ifdef VERBOSE
                cout << "\t\tAttempt 3: Own page " << FF.FFLIST[j].pno << " found in free frame " << FF.FFLIST[j].fno << endl;
            #endif
            attempt[cur][2]++;
            return j;
        }
    }
    return -1;
}
void page_fault(int m,int cur)
{
    if(FF.NFF > NFFMIN)
    {
        int p = table[cur].get_page(m);
        int fno = FF.allocate(p,cur);
        table[cur].set(p,fno);
        table[cur].set_ref(p);
        table[cur].set_cnt(p);
        #ifdef VERBOSE
            cout << "\tFault on Page" << setw(5) << p << ": Free Frame " << fno << " found\n";
        #endif
    }
    else if(FF.NFF == NFFMIN)
    {
        // find a valid page with min history
        replacements[cur]++;
        unsigned short int ans;
        int p = table[cur].get_page(m);
        int q = min_history(cur,ans);
        int g = table[cur].clear(q);
        #ifdef VERBOSE
            cout << "\tFault on Page" << setw(5) << p << ": To replace Page " << q << " at Frame " << g << " [history = " << ans << "]" << endl;
        #endif
        int idx = attempt_1(cur,p);
        
        // we have to store q info at this idx 
        if(idx == -1)
        {   
            idx = attempt_2(cur);
            if(idx == -1)
            {
                idx = attempt_3(cur);
                if(idx == -1)
                {
                    // choose a random number between [11288,12287] for now 
                    idx = rand()%1000 + 11288;
                    #ifdef VERBOSE
                        cout << "\t\tAttempt 4: Free Frame " << FF.FFLIST[idx].fno << " owned by Process " << FF.FFLIST[idx].pid << " chosen" <<  endl;
                    #endif
                    attempt[cur][3]++;
                }
            }
        }
        // update the page table 
        int fno = FF.FFLIST[idx].fno;
        table[cur].set(p,fno);
        table[cur].set_ref(p);
        table[cur].set_cnt(p);
        // swap the entry
        FF.FFLIST[idx].fno = g;
        FF.FFLIST[idx].pid = cur;
        FF.FFLIST[idx].pno = q;
    }
    
}

void shift(int cur)
{
    for (int j = 10; j < 2048; j++)
    {
        if((table[cur].PT[j]&(1<<15)))
        {
            table[cur].counter[j] >>= 1;
            if((table[cur].PT[j]&(1<<14)))
            {
                table[cur].counter[j] |= (1<<15);
                table[cur].PT[j] ^= (1<<14);
            }
        }
    }
}

void binary_search(int k,int cur)
{
    int l = code[cur].l; 
    int r = code[cur].r;
    while(l < r)
    {
        int m = (l+r)>>1;
        access[cur]++;
        if(!table[cur].retreive(m))
        {   
            faults[cur]++;
            page_fault(m,cur);
        }else{
            int page = table[cur].get_page(m);
            table[cur].set_ref(page);
        }
        if(k <= m) r=m;
        else l=m+1;
    }
    shift(cur);
}

void quit(int cur)
{

    for (int i = 0; i < 2048; i++)
    {
        int x = table[cur].clear(i);
        if(x != -1)
        {
            // this frame is freed now 
            FF.deallocate(x);
        }
    }
}
void simulate()
{
    while(!Q.empty())
    {
        int cur = Q.front();
        Q.pop();
        int k = code[cur].entries.front();
        #ifdef VERBOSE
            cout << "+++ " << "Process " << cur << ": Search " << m-(int)code[cur].entries.size()+1 <<  endl;
        #endif
        binary_search(k,cur);
        code[cur].entries.pop();
        if(!code[cur].entries.empty()) Q.push(cur);
        else quit(cur);
    }
}
int main(int argc, char const *argv[])
{
    srand(time(NULL));
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
    init();
    simulate();
    cout << "+++ Page access summary" << endl;
    cout << setw(8) << "PID"
        << setw(12) << "Accesses"
        << setw(13) << "Faults"
        << setw(20) << "Replacements"
        << setw(40) << "Attempts" << endl;
    int total_access = 0, total_faults = 0, total_replacements = 0;
    int total_a = 0, total_b = 0, total_c = 0, total_d = 0;
    for (int i = 0; i < n; i++) {
        int accesses = access[i];   
        int fault = faults[i];     
        double fault_percent = fault*1.0/accesses * 100;
        int replacement = replacements[i]; 
        double replacement_percent = replacement*1.0/accesses * 100; 
        int a = attempt[i][0], b = attempt[i][1], c = attempt[i][2], d = attempt[i][3]; 
        double a_perc = a*1.0/replacement*100, b_perc = b*1.0/replacement*100, c_perc = c*1.0/replacement*100, d_perc = d*1.0/replacement*100; 

        cout << setw(8) << i
            << setw(10) << accesses
            << setw(8) << fault << "   (" << fixed << setprecision(2) << fault_percent << "%)"
            << setw(6) << replacement << "   (" << replacement_percent << "%)"
            << setw(8) << a << " +"
            << setw(4) << b << " +"
            << setw(4) << c << " +"
            << setw(4) << d << "  ("
            << fixed << setprecision(2)
            << a_perc << "% + "
            << b_perc << "% + "
            << c_perc << "% + "
            << d_perc << "%)" << endl;


        total_access += accesses;
        total_faults += fault;
        total_replacements += replacement;
        total_a += a;
        total_b += b;
        total_c += c;
        total_d += d;
    }

    double total_fault_percent = total_faults * 100.0 / total_access;
    double total_replacement_percent = total_replacements * 100.0 / total_access;
    double total_a_perc = total_a * 100.0 / total_replacements;
    double total_b_perc = total_b * 100.0 / total_replacements;
    double total_c_perc = total_c * 100.0 / total_replacements;
    double total_d_perc = total_d * 100.0 / total_replacements;

    // Print total row
    cout << "\n";
    cout << setw(8) << "Total"
         << setw(10) << total_access
         << setw(8) << total_faults << "   (" << fixed << setprecision(2) << total_fault_percent << "%)"
         << setw(6) << total_replacements << "   (" << total_replacement_percent << "%)"
         << setw(8) << total_a << " +"
         << setw(4) << total_b << " +"
         << setw(5) << total_c << " +"
         << setw(3) << total_d << "  ("
         << fixed << setprecision(2)
         << total_a_perc << "% + "
         << total_b_perc << "% + "
         << total_c_perc << "% + "
         << total_d_perc << "%)" << endl;

    return 0;
}
