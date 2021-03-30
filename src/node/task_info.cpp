#include "../common/term_match.hpp"
#include "../common/checked_cast.hpp"
#include "self_node.hpp"
#include "task_info.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_info::task_info(out_connection *out) : out_task("info", out_task::TYPE_INFO, out)
{ }


//
// Grab version and comment info. Update address book.
//
void task_info::process()
{
    using namespace prologcoin::common;

    static const con_cell colon(":", 2);
    static const con_cell comma(",", 2);
    static const con_cell me("me",0);
    static const con_cell version_1("version",1);
    static const con_cell comment_1("comment",1);
    static const con_cell ver_2("ver",2);

    auto &e = env();

    switch (get_state()) {
    case IDLE:
	break;
    case RECEIVED: {
	// This is a one time event only.
	auto &e = env();

	pattern p(e);
	int64_t major_ver0 = 0, minor_ver0 = 0;
	term comment;
	auto const pat = p.str(comma,
			       p.str(colon,
				     p.con(me),
				     p.str(version_1,
					   p.str(ver_2,
						 p.any_int(major_ver0),
						 p.any_int(minor_ver0)))),
			       p.str(colon,
				     p.con(me),
				     p.str(comment_1, p.any(comment))));
	if (!pat(e, get_result_goal())) {
	    error(reason_t::ERROR_UNRECOGNIZED);
	    return;
	}

	int32_t major_ver = 0, minor_ver = 0;
	try {
	    major_ver = checked_cast<int32_t>(major_ver0, 0, 1000);
	    minor_ver = checked_cast<int32_t>(minor_ver0, 0, 1000);
	} catch (checked_cast_exception &ex) {
	    error(reason_t::ERROR_UNRECOGNIZED);
	    return;
	}

	// Successful. So update address book etc.
	address_entry entry;
	if (self().book()().exists(ip(), &entry)) {
	    entry.set_version(major_ver, minor_ver);
	    entry.set_comment(comment, e);
	    self().book()().update(entry);
	}
	break;
        }
    case SEND:
	set_term(
          e.new_term(con_cell("query",2),
	    {e.new_term(comma, {
			   e.new_term(colon,
				 {me, e.new_term(version_1,{e.new_ref()})
					 }),
			   e.new_term(colon,
				 {me, e.new_term(comment_1,
						 {e.new_ref()})
				 })}),
	     con_cell("false",0) // Silent = false
	    }));
	break;
    case KILLED:
    case WAIT:
	break;
    }
}

}}
