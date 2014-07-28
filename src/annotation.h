/*=============================================================================
    Copyright (c) 2014 Lingyun Yang
=============================================================================*/
#ifndef GLAD_ANNOTATION_H_
#define GLAD_ANNOTATION_H_

#include <map>
#include <boost/variant/apply_visitor.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/bool.hpp>
#include "ast.h"

namespace glad
{
    ////////////////////////////////////////////////////////////////////////////
    //  The annotation handler links the AST to a map of iterator positions
    //  for the purpose of subsequent semantic error handling when the
    //  program is being compiled.
    ////////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    struct annotation
    {
        template <typename, typename>
        struct result { typedef void type; };

        std::vector<Iterator>& iters;
        annotation(std::vector<Iterator>& iters)
          : iters(iters) {}

        struct set_id
        {
            typedef void result_type;

            int id;
            set_id(int id) : id(id) {}

            void operator()(ast::function_call& x) const
            {
                x.function_name.id = id;
            }

            void operator()(ast::identifier& x) const
            {
                x.id = id;
            }

            template <typename T>
            void operator()(T& x) const
            {
                // no-op
            }
        };

        void operator()(ast::operand& ast, Iterator pos) const
        {
            int id = iters.size();
            iters.push_back(pos);
            boost::apply_visitor(set_id(id), ast);
        }

        void operator()(ast::assignment& ast, Iterator pos) const
        {
            int id = iters.size();
            iters.push_back(pos);
            ast.lhs.id = id;
        }

        void operator()(ast::identifier& ast, Iterator pos) const
        {
            int id = iters.size();
            iters.push_back(pos);
            ast.id = id;
        }
    };
}

#endif

