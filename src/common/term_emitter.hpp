#pragma once

#ifndef _common_term_emitter_hpp
#define _common_term_emitter_hpp

//
// term_emitter
//
// This class emits a term into a sequence of ASCII characters.
//
class term_emitter {
public:
    inline term_emitter(std::ostream &out) : out_(out) { }

private:
    std::ostream &out_;
};

#endif
