/**
  Copyright (c) 2014 Lingyun Yang
=============================================================================*/

#include "statement.h"
#include "skipper.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>

using namespace std;

int parse_glad(glad::ast::statement_list& ast, const string& fname);

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    char const* filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        std::cerr << "Error: No input file provided." << std::endl;
        return 1;
    }

    glad::ast::statement_list ast;             // Our AST
    int success = parse_glad(ast, filename);

    std::cout << "-------- PRINT ---------------" << std::endl;
    glad::ast::printer pprint;
    pprint(ast);

    // std::cout << "is_absolute: " << INPDIR.is_absolute() << std::endl;
    // std::cout << "parent_path: " << INPDIR.parent_path() << std::endl;

    return 0;
}


