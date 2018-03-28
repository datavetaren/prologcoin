#pragma once

#ifndef _common_term_match_hpp
#define _common_term_match_hpp

#include "term.hpp"
#include "term_env.hpp"

namespace prologcoin { namespace common {

template<size_t N, size_t K, typename Arg, typename...Args> struct pattern_args {
    inline pattern_args(Arg arg, Args... args)
	: arg_(arg), args_(args...) { }

    inline bool operator () (term_env &env, const term t) const
    {
	return arg_(env, env.arg(t, K)) && args_(env,t);
    }

    Arg arg_;
    pattern_args<N-1, K+1, Args...> args_;
};

template<size_t K, typename Arg> struct pattern_args<1,K,Arg> {
    inline pattern_args(Arg arg) : arg_(arg)
    { }

    inline bool operator () (term_env &env, const term t) const
    {
	return arg_(env, env.arg(t, K));
    }
    Arg arg_;
};

struct pattern_con {
    inline pattern_con(con_cell c) : c_(c) { }

    inline bool operator () (term_env &, const term t) const {
	return t.tag() == tag_t::CON
	       && reinterpret_cast<const con_cell &>(t) == c_;
    }
    con_cell c_;
};

struct pattern_any {
    inline pattern_any(term &any)  : any_(any) { }

    inline bool operator () (term_env &, const term t) const {
	any_ = t;
	return true;
    }
    term &any_;
};

struct pattern_any_int {
    inline pattern_any_int(int64_t &i) : i_(i) { }

    inline bool operator () (term_env &, const term t) const {
	if (t.tag() != tag_t::INT) {
	    return false;
	}
	auto const &ic = reinterpret_cast<const int_cell &>(t);
	i_ = ic.value();
	return true;
    }
    int64_t &i_;
};

struct pattern_any_atom {
    inline pattern_any_atom(con_cell &c) : cc_(c) { }

    inline bool operator () (term_env &env, const term t) const {
	if (!env.is_atom(t)) {
	    return false;
	}
	auto const cc = env.functor(t);
	cc_ = cc;
	return true;
    }
    con_cell &cc_;
};

struct pattern_ignore {
    inline pattern_ignore() { }

    inline bool operator () (term_env &, const term) const {
	return true;
    }
};

template<typename...Args> struct pattern_str {
    inline pattern_str(con_cell f, Args... args)
	: f_(f), args_(args...) { }

    inline bool operator () (term_env &env, const term t) const
    { return t.tag() == tag_t::STR && env.functor(t) == f_ && args_(env,t); }

    con_cell f_;
    pattern_args<sizeof...(Args), 0, Args...> args_;
};

struct pattern {
    inline pattern(term_env &env) : env_(env) { }

    inline pattern_con con(con_cell c)
    {
	return pattern_con(c);
    }

    inline pattern_con con(const std::string &name, size_t arity)
    {
	return pattern_con(env_.functor(name,arity));
    }

    inline pattern_any any(term &a)
    {
	return pattern_any(a);
    }

    inline pattern_any_int any_int(int64_t &i)
    {
	return pattern_any_int(i);
    }

    inline pattern_any_atom any_atom(con_cell &c)
    {
	return pattern_any_atom(c);
    }

    inline pattern_ignore ignore()
    {
	return pattern_ignore();
    }

    template<typename...Args> inline pattern_str<Args...> str(con_cell f,
							      Args... args)
    {
	return pattern_str<Args...>(f, args...);
    }

    term_env &env_;
};


}}

#endif
