#include "../common/term_match.hpp"
#include "self_node.hpp"
#include "address_verifier.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

address_verifier::address_verifier(out_connection &out)
    : out_task(out, &address_verifier::check_fn)
{ }

void address_verifier::check_fn(out_task &task)
{
    reinterpret_cast<address_verifier &>(task).check();
}

void address_verifier::check()
{
    if (!is_connected()) {
	reschedule_last();
	set_state(IDLE);
	return;
    }

    auto &e = env();

    if (get_state() == SEND) {
	//
	// Construct the query:
	//
	// version(X), comment(Y)
	//
        set_query(e.new_term(con_cell(",",2),
	     {e.new_term(local_interpreter::COLON,
		         {local_interpreter::ME,
			 e.new_term(con_cell("version", 1),{e.new_ref()})}),
	      e.new_term(local_interpreter::COLON,
			 {local_interpreter::ME,
			 e.new_term(con_cell("comment", 1),{e.new_ref()})})
	}));
    } else if (get_state() == RECEIVED) {
	pattern p(e);

	auto const me = local_interpreter::ME;
	auto const colon = local_interpreter::COLON;
	auto const comma = local_interpreter::COMMA;
	auto const result_3 = con_cell("result",3);
	auto const version = con_cell("version",1);
	auto const comment_1 = con_cell("comment",1);
	auto const ver = con_cell("ver",2);
	int64_t major_ver = 0, minor_ver = 0;
	term comment;

	//
	// pattern: result(me:version(Major,Minor), me:comment(Comment),_,_)
	//
	auto const pat = p.str(result_3,
			       p.str( comma,
				      p.str(colon,
					    p.con(me),
					    p.str(version,
						  p.str(ver,
							p.any(major_ver),
							p.any(minor_ver)))),
				      p.str(colon,
					    p.con(me),
					    p.str(comment_1,
						  p.any(comment)))),
			       p.ignore(),
			       p.ignore());

	if (!pat(e, get_result())) {
	    fail(ERROR_UNRECOGNIZED);
	    return;
	}

	//
	// Answer accepted. Move unverified entry to verified.
	//

	auto book = self().book();
	book().remove(ip());

	// Now add a verified entry with neutral score
	address_entry verified(ip());
	verified.set_score(address_entry::VERIFIED_INITIAL_SCORE);
	verified.set_comment(comment, e);
	
	book().add(verified);
    }
}

void address_verifier::fail(address_verifier::fail_t reason)
{
    std::cout << "address_verifier::fail(): reason=" << reason << std::endl;
}

}}
