#include <iostream> 
#include <iomanip>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h> 
using namespace std; 


/* used to store login details in a array */
#define MAX_USERS 1000
#define MAX_LINE 1024
#define MAX_LOGIN 64
typedef struct {
    unsigned int uid;
    char login[MAX_LOGIN];
} UserMap;
UserMap users[MAX_USERS];
int user_count = 0;

// reads /etc/passwd and fills the users array
void load_passwd_file() {
    FILE *fp = fopen("/etc/passwd", "r");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char *login = strtok(line, ":");
        strtok(NULL, ":"); // skip password field
        char *uid_str = strtok(NULL, ":");

        if (login && uid_str) {
            unsigned int uid = atoi(uid_str);
            if (user_count < MAX_USERS) {
                users[user_count].uid = uid;
                strncpy(users[user_count].login, login, MAX_LOGIN - 1);
                users[user_count].login[MAX_LOGIN - 1] = '\0'; // Ensure null-termination
                user_count++;
            }
        }
    }

    fclose(fp);
}
// Looks up a login name by UID
const char* get_login_by_uid(unsigned int uid) {
    // searches the users array (made above) for the given uid
    for (int i = 0; i < user_count; ++i) {
        if (users[i].uid == uid) {
            return users[i].login;
        }
    }
    return "UNKNOWN";
}
vector<const char*> history; // stores the history of directories traversed
void print_path(char* name)
{
    for (auto x : history)
    {
        cout << x << "/";
    }
    cout << name << endl;
}
int has_extension(const char *filename, const char *ext) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0; 
    return strcmp(dot + 1, ext) == 0; 
}
int serial_no = 0;
// main recursive function 
void func(const char * dirname,const char* ext)
{
    DIR *dp;
    struct dirent *ep;
    dp = opendir (dirname);
    if (dp != NULL)
    {
        while (ep = readdir (dp)){
            // cout << ep->d_name << endl;
            if(ep->d_type == DT_DIR)
            {
                if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0) continue; // preventing infinite loop
                // getting full path 
                string full_path = "";
                for (auto x : history) {
                    full_path += x;
                    full_path += "/";
                }
                full_path+=ep->d_name;
                history.push_back(ep->d_name);
                func(full_path.c_str(),ext);
                history.pop_back();
            }
            else if(ep->d_type == DT_REG)
            {
                // check its file ext and print info using stat 
                if(has_extension(ep->d_name,ext))
                {
                    serial_no++;    
                    struct stat sb;
                    // getting full path 
                    string full_path = "";
                    for (auto x : history) {
                        full_path += x;
                        full_path += "/";
                    }
                    full_path+=ep->d_name;
                    if (stat(full_path.c_str(), &sb) == -1)
                    {
                        perror("stat()");
                        exit(EXIT_FAILURE);
                    }
                    // printing info 
                    const char * ownner = get_login_by_uid(sb.st_uid);
                    cout << left << setw(10) << serial_no << ": ";
                    cout << left << setw(20) << ownner << ": ";
                    cout << left << setw(20) << sb.st_size;
                    print_path(ep->d_name);
                }
            }
        }
        (void) closedir (dp);
    }
    else{
        perror ("Couldn't open the directory");
    }
}

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        cerr << "run with directory and extension" << endl;
        exit(1);
    }
    history.push_back(argv[1]);
    // load the passwd file
    load_passwd_file();
    cout << left << setw(10) << "NO" << ": ";
    cout << left << setw(20) << "OWNER" << ": ";
    cout << left << setw(20) << "SIZE";
    cout << "NAME\n";
    cout << left << setw(10) << "--" << "  ";
    cout << left << setw(20) << "-----" << "  ";
    cout << left << setw(20) << "----";
    cout << "----\n";
    // calling the recursive function
    func(argv[1],argv[2]);
    cout << "+++ " << serial_no << " files match the extension " << argv[2] << endl;
    return 0;
}
