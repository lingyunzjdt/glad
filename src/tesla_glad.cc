/**
  Copyright (c) 2014 Lingyun Yang
=============================================================================*/

#include "statement.h"
#include "skipper.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>
#include <map>

using namespace std;
using namespace glad::ast;

class Element;
class TransportLine;

int parse_glad(glad::ast::statement_list& ast, const string& fname);

typedef boost::variant<int, double, vector<double>, string,
                       Element*, TransportLine*> symb_expr;
typedef map<string, symb_expr> symbol_table;

// evaluate the double
struct eval
{
    typedef double result_type;
    double operator()(nil) const { return 0.0; }
    double operator()(int n) const { return n; }
    double operator()(double x) const { return x; }
    double operator()(unsigned int x) const { return x; }
    double operator()(const identifier& x) const { return 0.0; }
    double operator()(const quoted_string& x) const { return 0.0; }
    double operator()(const unary& x) const { return 0.0; }
    double operator()(const function_call& x) const { return 0.0; }
    double operator()(const expression& x) const {
        double lhs = boost::apply_visitor(*this, x.first);
        BOOST_FOREACH(const operation &op, x.rest)
        {
            lhs = (*this)(op, lhs);
        }
        return lhs;
    }
    double operator()(operation const& x, double lhs) const {
        double rhs = boost::apply_visitor(*this, x.operand_);
        switch(x.operator_) {
        case op_plus:   return lhs + rhs;
        case op_minus:  return lhs - rhs;
        case op_times:  return lhs * rhs;
        case op_divide: return lhs / rhs;
        default:
            std::cout << "undefined operation " << x.operator_ << std::endl;
        }
        return 0.0;
    }
    
};

struct tesla_vm
{
    symbol_table symbtbl_;
    tesla_vm() {
        symbtbl_["pi"] = M_PI;
    }
    
    typedef void result_type;
    void operator()(nil) { std::cout << "(nil)"; }
    void operator()(bool v) { }
    void operator()(double v) { }
    void operator()(int v) { }
    void operator()(unsigned int v) { std::cout << v; }
    void operator()(vector<double>& v) { }
    void operator()(const identifier& id) {
        if (symbtbl_.count(id.name) == 1) 
            boost::apply_visitor(*this, symbtbl_[id.name]);
        else std::cout << "ERROR: " << id.name << " not defined" << std::endl;
    }
    void operator()(const quoted_string& s) {
        std::cout << '"' << s.s << '"'; }
    void operator()(const unary& s) {}
    void operator()(const function_call& s) {}
    void operator()(const property& p) {
        std::cout << p.name << "= ";
        std::list<expression>::const_iterator it = p.value.begin();
        eval e;
        if (p.value.size() > 1) {
            std::cout << "( ";
            e(*it);
            ++it;
            while (it != p.value.end()) {
                std::cout << ", ";
                e(*it);
                ++it;
            }
            std::cout << " )";
        } else {
            e(*it);
        }
    }
    void operator()(const statement_list& stl) {
        BOOST_FOREACH(const statement& st, stl)
        {
            std::cout << ' ';
            boost::apply_visitor(*this, st);
        }
    }
    void operator()(const assignment& st) {
        eval e;
        double r = e(st.rhs);
        std::cout << st.lhs.name << "=" << r << std::endl;
    }            
    void operator()(const element_statement& es){
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
    void operator()(const action_statement& es){
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
    void operator()(const include_statement& inc) {
        std::cout << "include " << inc.filename.s << std::endl;
    }
};

int main(int argc, char** argv)
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

    tesla_vm tpl;
    tpl(ast);
    //eval e;
    //e(ast)
}
