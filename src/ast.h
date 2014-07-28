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
#include <map>

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
        op_or,
        op_concat
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
    struct action_statement;
    struct beamline_statement;

    typedef boost::variant<
        assignment,
        element_statement,
        action_statement,
        boost::recursive_wrapper<beamline_statement>,
        boost::recursive_wrapper<statement_list>
        >
    statement;

    struct statement_list : std::list<statement> {};

    struct property_list: std::list<property> {};

    struct element_statement {
        identifier element_name;
        identifier element_type;
        boost::optional<property_list> properties;
    };

    struct action_statement {
        identifier name;
        property_list properties;
    };

    struct beamline_statement {
        identifier name;
        expression line;
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
        std::vector<double> stack_;
        std::vector<double>::iterator stack_ptr;
        std::map<std::string, double> local_;

        printer() 
        :stack_(1024), stack_ptr(stack_.begin()) {}

        void operator()(nil) { std::cout << "(nil)"; }
        void operator()(bool v) { *stack_ptr++ = v; }
        void operator()(double v) { *stack_ptr++ = v; }
        void operator()(unsigned int v) { *stack_ptr++ = v; }
        void operator()(const identifier& id) {
            if (local_.find(id.name) != local_.end()) {
                *stack_ptr++ = local_[id.name];
            } else if (id.name.find(".") != std::string::npos) {
                // an element/action/beamline
            } else {
                // not defined
            }
        }
        void operator()(const quoted_string& s) {
            std::cout << '"' << s.s << '"'; }
        void operator()(const unary& s) {
            boost::apply_visitor(*this, s.operand_);
            switch(s.operator_) {
            case ast::op_negative:
                stack_ptr[-1] = -stack_ptr[-1];
                break;
            case ast::op_positive:
                break;
            }
        }
        void operator()(const function_call& s) {}
        void operator()(const expression& ex) {
            // operand and a list of operation
            // std::cout << "(";
            boost::apply_visitor(*this, ex.first);
            // std::cout << "Stack: " << (stack_ptr - stack_.begin()) << " "
            //           << *stack_ptr << std::endl;
            BOOST_FOREACH(operation const& op, ex.rest)
            {
                std::cout << ' ';
                //boost::apply_visitor(*this, op);
                (*this)(op);
            }
            // std::cout << ")";
        }

        void operator()(const property& p) {
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
        void operator()(const operation& opr) {
            //std::cout << "( ";
            // not (*this)(opr.operand_); 
            boost::apply_visitor(*this, opr.operand_);
            switch(opr.operator_) {
            case ast::op_plus: 
                stack_ptr[-2] += stack_ptr[-1];
                --stack_ptr;
                break;
            case ast::op_minus: 
                stack_ptr[-2] -= stack_ptr[-1];
                --stack_ptr;
                break;
            case ast::op_times: 
                stack_ptr[-2] *= stack_ptr[-1];
                --stack_ptr;
                break;
            case ast::op_divide: 
                stack_ptr[-2] /= stack_ptr[-1];
                --stack_ptr;
                break;
            default:
                std::cout << "op:" << opr.operator_ << " not defined" << std::endl;
            }
            std::cout << opr.operator_ << " :" << stack_ptr[-1];
            //std::cout << " )";
        }
        void operator()(const statement_list& stl) {
            BOOST_FOREACH(statement const& st, stl)
            {
                std::cout << ' ';
                boost::apply_visitor(*this, st);
            }
        }
        void operator()(const assignment& st) {
            (*this)(st.rhs);
            local_[st.lhs.name] = stack_ptr[-1];
        }            
        void operator()(const element_statement& es) {
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
        void operator()(const action_statement& es) {
            std::cout << "define: " << es.name << ": ";
            BOOST_FOREACH(property const& p, es.properties)
            {
                std::cout << ", ";
                //boost::apply_visitor(*this, op);
                (*this)(p);
            }
            std::cout << ";" << std::endl;
        }
        void operator()(const beamline_statement& st) {
        }

        void print_locals() const {
            std::cout << std::endl
                      << "# local variables:" << std::endl;
            std::map<std::string, double>::const_iterator it;
            for ( it = local_.begin(); it != local_.end(); ++it) {
                std::cout << it->first << "= " << it->second << std::endl;
            }
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
    glad::ast::action_statement,
    (glad::ast::identifier, name)
    (glad::ast::property_list, properties)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::beamline_statement,
    (glad::ast::identifier, name)
    (glad::ast::expression, line)
)

BOOST_FUSION_ADAPT_STRUCT(
    glad::ast::quoted_string,
    (std::string, s)
)

#endif
