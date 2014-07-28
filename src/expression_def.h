/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#include "expression.h"
#include "error_handler.h"
#include "annotation.h"
#include <boost/spirit/include/phoenix_function.hpp>

namespace glad
{
    template <typename Iterator>
    expression<Iterator>::expression(error_handler<Iterator>& error_handler)
      : expression::base_type(expr)
    {
        using qi::_1;
        using qi::_2;
        using qi::_3;
        using qi::_4;

        using qi::char_;
        using qi::uint_;
        using qi::_val;
        using qi::raw;
        using qi::lexeme;
        using qi::alpha;
        using qi::alnum;
        using qi::bool_;
        using qi::double_;

        using qi::on_error;
        using qi::on_success;
        using qi::fail;
        using boost::phoenix::function;

        typedef function<glad::error_handler<Iterator> > error_handler_function;
        typedef function<glad::annotation<Iterator> > annotation_function;

        static qi::real_parser<double, qi::strict_real_policies<double> > const strict_double;

        ///////////////////////////////////////////////////////////////////////
        // Tokens
        logical_or_op.add
            ("||", ast::op_or)
            ;

        logical_and_op.add
            ("&&", ast::op_and)
            ;

        equality_op.add
            ("==", ast::op_equal)
            ("!=", ast::op_not_equal)
            ;

        relational_op.add
            ("<", ast::op_less)
            ("<=", ast::op_less_equal)
            (">", ast::op_greater)
            (">=", ast::op_greater_equal)
            ;

        additive_op.add
            ("+", ast::op_plus)
            ("-", ast::op_minus)
            ;

        multiplicative_op.add
            ("*", ast::op_times)
            ("/", ast::op_divide)
            ;

        unary_op.add
            ("+", ast::op_positive)
            ("-", ast::op_negative)
            ("!", ast::op_not)
            ;

        keywords.add
            ("true")
            ("false")
            ("if")
            ("else")
            ("while")
            ("int")
            ("void")
            ("return")
            ;

        ///////////////////////////////////////////////////////////////////////
        // Main expression grammar
        expr =
            logical_or_expr.alias()
            ;

        quoted_string %=
            '"'
            >> lexeme[*(char_ - '"')]
            >> '"'
            ;

        logical_or_expr =
                logical_and_expr
            >> *(logical_or_op > logical_and_expr)
            ;

        logical_and_expr =
                equality_expr
            >> *(logical_and_op > equality_expr)
            ;

        equality_expr =
                relational_expr
            >> *(equality_op > relational_expr)
            ;

        relational_expr =
                additive_expr
            >> *(relational_op > additive_expr)
            ;

        additive_expr =
                multiplicative_expr
            >> *(additive_op > multiplicative_expr)
            ;

        multiplicative_expr =
                unary_expr
            >> *(multiplicative_op > unary_expr)
            ;

        unary_expr =
                primary_expr
            |   (unary_op > unary_expr)
            ;

        primary_expr =
                strict_double
            |   uint_
            |   function_call
            |   identifier
            |   bool_
            |   '(' > expr > ')'
            |   quoted_string
            ;

        function_call =
                (identifier >> '(')
            >   argument_list
            >   ')'
            ;

        argument_list = -(expr % ',');

        identifier =
                !lexeme[keywords >> !(alnum | '_')]
            >>  raw[lexeme[(alpha | '_') >> *(alnum | '_')]]
            ;

        ///////////////////////////////////////////////////////////////////////
        // Debugging and error handling and reporting support.
        BOOST_SPIRIT_DEBUG_NODES(
            (expr)
            (logical_or_expr)
            (logical_and_expr)
            (equality_expr)
            (relational_expr)
            (additive_expr)
            (multiplicative_expr)
            (unary_expr)
            (primary_expr)
            (function_call)
            (argument_list)
            (identifier)
            (quoted_string)
        );

        ///////////////////////////////////////////////////////////////////////
        // Error handling: on error in expr, call error_handler.
        on_error<fail>(expr,
            error_handler_function(error_handler)(
                "Error! Expecting ", _4, _3));

        ///////////////////////////////////////////////////////////////////////
        // Annotation: on success in primary_expr, call annotation.
        // on_success(primary_expr,
        //     annotation_function(error_handler.iters)(_val, _1));
        debug(primary_expr);
        debug(identifier);
    }
}


