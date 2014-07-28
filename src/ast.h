/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/

#ifndef GLAD_AST_H_
#define GLAD_AST_H_

#include <boost/config/warning_disable.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/optional.hpp>
#include <list>

namespace glad { namespace ast 
{
    ///////////////////////////////////////////////////////////////////////////
    //  The AST
    ///////////////////////////////////////////////////////////////////////////
    struct tagged {
        int id; 
    };

    struct nil {};
    struct unary;
    struct function_call;
    struct expression;
    struct quoted_string;

    struct identifier : tagged {
        identifier(std::string const& name = "") : name(name) {}
        std::string name;
    };

    typedef boost::variant<
            nil
          , bool
          , unsigned int
          , double 
          , identifier
          , quoted_string
          , boost::recursive_wrapper<unary>
          , boost::recursive_wrapper<function_call>
          , boost::recursive_wrapper<expression>
        >
    operand;

    enum optoken {
        op_plus,
        op_minus,
        op_times,
        op_divide,
        op_positive,
        op_negative,
        op_not,
        op_equal,
        op_not_equal,
        op_less,
        op_less_equal,
        op_greater,
        op_greater_equal,
        op_and,
        op_or
    };

    struct quoted_string {
        //quoted_string(std::string const& val = "") : s(val) {}
        std::string s;
    };

    struct unary {
        optoken operator_;
        operand operand_;
    };

    struct operation {
        optoken operator_;
        operand operand_;
    };

    struct function_call {
        identifier function_name;
        std::list<expression> args;
    };

    struct expression {
        operand first;
        std::list<operation> rest;
    };

    struct assignment {
        identifier lhs;
        expression rhs;
    };

    struct property {
        identifier name;
        std::list<expression> value;
    };

    struct statement_list;
    struct element_statement;

    typedef boost::variant<
            assignment
          , element_statement
          , boost::recursive_wrapper<statement_list>
        >
    statement;

    struct statement_list : std::list<statement> {};

    struct property_list: std::list<property> {};

    struct element_statement {
        identifier element_name;
        identifier element_type;
        boost::optional<property_list> properties;
    };

    // print functions for debugging
    inline std::ostream& operator<<(std::ostream& out, nil)
    {
        out << "nil"; return out;
    }

    inline std::ostream& operator<<(std::ostream& out, identifier const& id)
    {
        out << id.name; return out;
    }

    struct printer
    {
        typedef void result_type;
        void operator()(nil) const { std::cout << "(nil)"; }
        void operator()(bool v) const { std::cout << v; }
        void operator()(double v) const { std::cout << v; }
        void operator()(unsigned int v) const { std::cout << v; }
        void operator()(const identifier& id) const {
            std::cout << id.name; }
        void operator()(const quoted_string& s) const {
            std::cout << '"' << s.s << '"'; }
        void operator()(const unary& s) const {}
        void operator()(const function_call& s) const {}
        void operator()(const expression& ex) const {
            // std::cout << "(";
            boost::apply_visitor(*this, ex.first);
            BOOST_FOREACH(operation const& op, ex.rest)
            {
                std::cout << ' ';
                //boost::apply_visitor(*this, op);
                (*this)(op);
            }
            // std::cout << ")";
        }

        void operator()(const property& p) const {
            std::cout << p.name << "= ";
            std::list<expression>::const_iterator it = p.value.begin();
            if (p.value.size() > 1) {
                std::cout << "( ";
                (*this)(*it);
                ++it;
                while (it != p.value.end()) {
                    std::cout << ", ";
                    (*this)(*it);
                    ++it;
                }
                std::cout << " )";
            } else {
                (*this)(*it);
            }
        }
        void operator()(const operation& opr) const {
            //std::cout << "( ";
            // not (*this)(opr.operand_); 
            boost::apply_visitor(*this, opr.operand_);
            std::cout << ' ' << opr.operator_;
            //std::cout << " )";
        }
        void operator()(const statement_list& stl) const {
            BOOST_FOREACH(statement const& st, stl)
            {
                std::cout << ' ';
                boost::apply_visitor(*this, st);
            }
        }
        void operator()(const assignment& st) const {
            (*this)(st.rhs);
            std::cout << " -> " << st.lhs.name << std::endl;
        }            
        void operator()(const element_statement& es) const{
            std::cout << "define: " << es.element_name << ": "
                      << es.element_type;
            BOOST_FOREACH(property const& p, *es.properties)
            {
                std::cout << ", ";
                //boost::apply_visitor(*this, op);
                (*this)(p);
            }
            std::cout << ";" << std::endl;
        }
    };

}}

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::unary,
    (glad::ast::optoken, operator_)
    (glad::ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::operation,
    (glad::ast::optoken, operator_)
    (glad::ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::function_call,
    (glad::ast::identifier, function_name)
    (std::list<glad::ast::expression>, args)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::expression,
    (glad::ast::operand, first)
    (std::list<glad::ast::operation>, rest)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::assignment,
    (glad::ast::identifier, lhs)
    (glad::ast::expression, rhs)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::property,
    (glad::ast::identifier, name)
    (std::list<glad::ast::expression>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::element_statement,
    (glad::ast::identifier, element_name)
    (glad::ast::identifier, element_type)
    (boost::optional<glad::ast::property_list>, properties)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::quoted_string,
    (std::string, s)
)

#endif
