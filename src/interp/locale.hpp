#pragma once

#ifndef _interp_locale_hpp
#define _interp_locale_hpp

#include "../common/term_env.hpp"
#include <vector>
#include <locale.h>

namespace prologcoin { namespace interp {

class interpreter_base;

//
// This class should not be used in consensus rules.
//
class locale {
    using con_cell = prologcoin::common::con_cell;

public:
    locale(interpreter_base &intp);
    locale(interpreter_base &intp, const std::string &name);

    void total_reset();

    void set_from_platform();

    void set_decimal_point(con_cell decp) { decimal_point_ = decp; }
    void set_thousands_sep(con_cell sep) { thousands_sep_ = sep; }
    void set_grouping(const std::vector<int> &g) { grouping_ = g; }

    inline con_cell decimal_point() const { return decimal_point_; }
    inline con_cell thousands_sep() const { return thousands_sep_; }
    inline const std::vector<int> & grouping() const { return grouping_; }

private:
    void init();

    interpreter_base &interp_;
    con_cell alias_;
    con_cell decimal_point_;
    con_cell thousands_sep_;
    std::vector<int> grouping_;
    
};

}}

#endif
