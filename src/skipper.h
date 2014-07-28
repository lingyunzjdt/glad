/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#ifndef GLAD_SKIPPER_H_
#define GLAD_SKIPPER_H_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>

namespace glad
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    ////////////////////////////////////////////////////////////////////////////
    //  The skipper grammar
    ////////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct skipper : qi::grammar<Iterator>
    {
        skipper() : skipper::base_type(start)
        {
            qi::char_type char_;
            ascii::space_type space;
            start =
                    space                               // tab/space/cr/lf
                |   "/*" >> *(char_ - "*/") >> "*/"     // C-style comments
                |   ('#' >> *(char_ - qi::eol) >> qi::eol | qi::blank)
                ;
        }

        qi::rule<Iterator> start;
    };
}

#endif


