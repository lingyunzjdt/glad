/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#ifndef GLAD_STATEMENT_H_
#define GLAD_STATEMENT_H_

#include "expression.h"

namespace glad
{
    ////////////////////////////////////////////////////////////////////////////
    //  The statement grammar
    ////////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct statement : qi::grammar<Iterator, ast::statement_list(), skipper<Iterator> >
    {
        statement(error_handler<Iterator>& error_handler);

        expression<Iterator> expr;
        qi::rule<Iterator, ast::statement_list(), skipper<Iterator> >
            statement_list, compound_statement;

        qi::rule<Iterator, ast::statement(), skipper<Iterator> > statement_;
        qi::rule<Iterator, ast::assignment(), skipper<Iterator> > assignment;
        qi::rule<Iterator, ast::property(), skipper<Iterator> > property;
        qi::rule<Iterator, ast::element_statement(), skipper<Iterator> > element_statement;
        qi::rule<Iterator, ast::action_statement(), skipper<Iterator> > action_statement;
        qi::rule<Iterator, ast::beamline_statement(), skipper<Iterator> > beamline_statement;
        qi::rule<Iterator, ast::include_statement(), skipper<Iterator> > include_statement;
        
        qi::rule<Iterator, std::string(), skipper<Iterator> > identifier;
    };
}

#endif


