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

bool insert_ast(glad::ast::statement_list& ast, const string& fname)
{
    const char *filename = fname.c_str();
    std::ifstream in(filename, std::ios_base::in);

    if (!in) {
        std::cerr << "Error: Could not open input file: "
                  << filename << std::endl;
        return 1;
    }

    std::string source_code; // We will read the contents here.
    in.unsetf(std::ios::skipws); // No white space skipping!
    std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(),
              std::back_inserter(source_code));

    typedef std::string::const_iterator iterator_type;
    iterator_type iter = source_code.begin();
    iterator_type end = source_code.end();

    // glad::ast::statement_list ast;             // Our AST

    glad::error_handler<iterator_type> error_handler(iter, end); // Our error handler
    //glad::function<iterator_type> function(error_handler); // Our parser
    glad::statement<iterator_type> function(error_handler); // Our parser
    glad::skipper<iterator_type> skipper;       // Our skipper

    bool success = phrase_parse(iter, end, +function, skipper, ast);

    std::cout << "-------------------------\n";

    if (success && iter == end) {
        std::cout << "Parsed Successfully\n";
    } else {
        std::cout << "Parse failure\n";
    }

    return true;
}

int parse_glad(glad::ast::statement_list& ast, const string& fname)
{
    boost::filesystem::path INPDIR(fname);

    bool success = insert_ast(ast, fname);
    size_t MAX_DEPTH = 3;
    size_t idepth = 0;
    while(true) {
        std::string filename = "";
        glad::ast::statement_list::iterator inciter = ast.end();
        for (glad::ast::statement_list::iterator iter = ast.begin(); 
             iter != ast.end(); ++iter)
        {
            if (glad::ast::include_statement *inc =
                boost::get<glad::ast::include_statement>(&(*iter)))
            {
                std::cout << "include:" <<  inc->filename.s  << std::endl;
                filename = inc->filename.s;
                inciter = iter;
                break;
            }
        }

        if (filename.length() > 0 && inciter != ast.end()) {
            idepth += 1;
            boost::filesystem::path subdir(filename);
            glad::ast::statement_list subast;
            std::string fname = filename;
            if (!subdir.is_absolute()) {
                // std::cout << "parent:" << INPDIR << std::endl
                //           << "parent dir:" << INPDIR.parent_path() << std::endl;
                boost::filesystem::path p(INPDIR.parent_path());
                p /= filename;
                fname = p.c_str();
            }
            // std::cout << "open:" << fname << ":" << 
            //     subdir.is_absolute() << std::endl;
            bool suc = insert_ast(subast, fname);
            glad::ast::statement_list::iterator n = ast.erase(inciter);
            ast.insert(n, subast.begin(), subast.end());
        } else {
            break;
        }

        if (idepth > 5) break;
    }

    std::cout << "-------- PRINT ---------------" << std::endl;
    glad::ast::printer pprint;
    pprint(ast);

    // std::cout << "is_absolute: " << INPDIR.is_absolute() << std::endl;
    // std::cout << "parent_path: " << INPDIR.parent_path() << std::endl;

    return 0;
}


