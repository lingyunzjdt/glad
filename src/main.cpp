/**
  Copyright (c) 2014 Lingyun Yang
=============================================================================*/

#include "statement.h"
#include "skipper.h"
#include <boost/lexical_cast.hpp>
#include <fstream>

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

    glad::ast::statement_list ast;             // Our AST

    glad::error_handler<iterator_type> error_handler(iter, end); // Our error handler
    //glad::function<iterator_type> function(error_handler); // Our parser
    glad::statement<iterator_type> function(error_handler); // Our parser
    glad::skipper<iterator_type> skipper;       // Our skipper

    bool success = phrase_parse(iter, end, +function, skipper, ast);

    glad::ast::printer pprint;
    pprint(ast);
    pprint.print_locals();

    std::cout << "-------------------------\n";

    if (success && iter == end) {
        std::cout << "Parsed Successfully\n";
    } else {
        std::cout << "Parse failure\n";
    }
    return 0;
}


