#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>

pid_t target;

void process_mem_usage(double& vm_usage, double& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        char target_str[20];
        sprintf(target_str, "/proc/%d/stat", target);
        std::ifstream ifs(target_str, std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}

int main(int argc, char **argv)
{
   using std::cout;
   using std::endl;

   target = atoi(argv[1]);

   double vm, rss;
   process_mem_usage(vm, rss);
   cout << "VM: " << vm << "; RSS: " << rss << endl;
}