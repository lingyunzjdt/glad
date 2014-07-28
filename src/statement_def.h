/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#include "statement.h"
#include "error_handler.h"
#include "annotation.h"

namespace glad
{
    template <typename Iterator>
    statement<Iterator>::statement(error_handler<Iterator>& error_handler)
      : statement::base_type(statement_list), expr(error_handler)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;

        qi::_val_type _val;
        qi::raw_type raw;
        qi::lexeme_type lexeme;
        qi::alpha_type alpha;
        qi::alnum_type alnum;
        qi::lit_type lit;

        using qi::uint_;
        using qi::on_error;
        using qi::on_success;
        using qi::fail;
        using boost::phoenix::function;

        typedef function<glad::error_handler<Iterator> > error_handler_function;
        typedef function<glad::annotation<Iterator> > annotation_function;

        statement_list =
            +statement_
            ;

        statement_ =
            // variable_declaration
                element_statement
            |   assignment
            |   compound_statement
            |   action_statement
            |   beamline_statement
            ;

        identifier =
                !expr.keywords
            >>  raw[lexeme[(alpha | '_') >> *(alnum | '_')]]
            ;

        /* variable_declaration = */
        /*         lexeme["int" >> !(alnum | '_')] // make sure we have whole words */
        /*     >   identifier */
        /*     >   -('=' > expr) */
        /*     >   ';' */
        /*     ; */

        assignment =
                identifier
            >>   '='
            >>   expr
            >>   ';'
            ;

        property = 
            (identifier >> '=' >> expr)
            | (identifier >> '=' >> '(' >> expr.argument_list >> ')')
            ;

        element_statement = 
                identifier
            >>   ':'
            >>  identifier
            >> *( ',' > property )
            >>   ';'
            ;

        action_statement = 
                identifier
            >> +( ',' > property )
            >>   ';'
            ;

        beamline_statement = 
            (identifier >> ':' >> lit("line"))
            >> '='
            >> expr
            >> ';'
            ;
            
        compound_statement =
            '{' >> -statement_list >> '}'
            ;

        // Debugging and error handling and reporting support.
        BOOST_SPIRIT_DEBUG_NODES(
            (statement_list)
            (identifier)
            (assignment)
            (element_statement)
            (action_statement)
            (beamline_statement)
            (property)
        );

        // Error handling: on error in statement_list, call error_handler.
        on_error<fail>(statement_list,
            error_handler_function(error_handler)(
                "Error! Expecting ", _4, _3));

        // Annotation: on success in variable_declaration,
        // assignment and return_statement, call annotation.
        // on_success(variable_declaration,
        //     annotation_function(error_handler.iters)(_val, _1));
        // on_success(assignment,
        //     annotation_function(error_handler.iters)(_val, _1));
        // on_success(return_statement,
        //     annotation_function(error_handler.iters)(_val, _1));

        //debug(identifier);
        debug(assignment);
        debug(element_statement);
        debug(property);
        debug(beamline_statement);
    }
}


