#include "glpsfuncs.h"
#include <iostream>

using namespace std;

int parse_lattice(const char*);

extern "C" void log_message(int level, const char *file, int line, const char *func, 
                 const char *msg)
{
    cout << func << ":" << msg << endl;
}

int main(int argc, char **argv)
{
    struct pcdata *pc_ = new pcdata();
    pc_->scaninfo = NULL;
    pc_->symtab = NULL;
    pc_->nast   = 0;
    pc_->al = NULL;
    pc_->ilat = -1;

    int err = glps_parse(pc_, argv[1]);
    
    return 0;
}

/**
 Local Variables:
 mode: c++
 tab-width: 4
 c-basic-offset: 4
 indent-tabs-mode: nil
 End:
*/
