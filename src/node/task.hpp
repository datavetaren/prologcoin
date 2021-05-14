#pragma once

#ifndef _node_out_task_hpp
#define _node_out_task_hpp

#include "../common/term_env.hpp"
#include "../common/utime.hpp"
#include "ip_service.hpp"

namespace prologcoin { namespace node {

class out_connection;
class self_node;
class reason_t;

class out_task {
public:
    using term_env = prologcoin::common::term_env;
    using term = prologcoin::common::term;
    using utime = prologcoin::common::utime;

    enum state_t {
	IDLE = 0,
	SEND = 1,
	RECEIVED = 2,
	WAIT = 3,
	KILLED = 4
    };

    enum type_t {
	TYPE_NONE = 0,
	TYPE_ADDRESS_DOWNLOADER = 1,
	TYPE_ADDRESS_VERIFIER = 2,
	TYPE_INIT_CONNECTION = 3,
	TYPE_HEARTBEAT = 4,
	TYPE_INFO = 5,
	TYPE_PUBLISH = 6,
	TYPE_RESET = 7,
	TYPE_EXECUTE_QUERY = 8
    };

    out_task(const char *description, type_t type, out_connection *out);
    virtual ~out_task();

    static bool comparator(const out_task *t1, const out_task *t2);

    inline const char * description() const { return description_; }

    inline type_t get_type() const { return type_; }

    virtual void process() = 0;

    void stop();

    inline void set_query(const term t, bool silent)
    {
	auto silent_con = silent ? common::con_cell("true",0)
	                         : common::con_cell("false",0);
	set_term(env_->new_term(common::con_cell("query",2), {t, silent_con}));
    }
    inline void set_command(const term t)
    { set_term(env_->new_term(common::con_cell("command",1),{t})); }

    bool is_error() const;
    bool is_exception();
    std::string get_exception();
    term get_result() const;
    term get_result_goal() const;
    std::string get_standard_out();
    uint64_t get_cost() const;
    bool has_more() const;
    bool at_end() const;

    inline term_env & env() { return *env_; }
    inline const term_env & env() const { return *env_; }

    const ip_service & ip() const;

    inline bool has_connection() const { return out_ != nullptr; }
    inline out_connection & connection() { return *out_; }
    inline const out_connection & connection() const { return *out_; }

    self_node & self();

    inline bool expiring() const { return utime::now() >= get_when(); }

    inline state_t get_state() const { return state_; }
    inline void set_state(state_t st) { state_ = st; }
   
    std::string get_state_name() const;

    inline utime get_when() const { return when_; }
    inline void set_when(utime when) { when_ = when; }

    inline term get_term() const { return term_; }
    inline void set_term(term t) { term_ = t; }

    inline bool operator < (const out_task &other) const {
	return get_when() < other.get_when();
    }

    inline bool operator > (const out_task &other) const {
	return get_when() > other.get_when();
    }

    inline bool operator == (const out_task &other) const {
	return get_when() == other.get_when();
    }

    bool is_connected() const;

    template<uint64_t C> void reschedule(utime::dt<C> dt)
    { return reschedule(utime::now() + dt); }

    void reschedule(utime t);
    void reschedule_last();
    void reschedule_next();
    void trigger_now();

protected:
    void error(const reason_t &reason);
    void error(const reason_t &reason, const std::string &msg);

private:
    type_t type_;
    const char *description_;
    out_connection *out_;
    term_env *env_;
    state_t state_;
    utime when_;
    term term_;
};

}}

#endif
