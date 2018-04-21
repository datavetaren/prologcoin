#include "../common/term_match.hpp"
#include "../common/checked_cast.hpp"
#include "self_node.hpp"
#include "address_verifier.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

task_address_verifier::task_address_verifier(out_connection &out)
    : out_task("address_verifier", out)
{ }

void task_address_verifier::process()
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
	// me:id(X), me:version(Y), me:comment(Z)
	//
        set_query(e.new_term(con_cell(",",2),
	     {e.new_term(local_interpreter::COLON,
		         {local_interpreter::ME,
				 e.new_term(con_cell("id", 1),{e.new_ref()})}),
	      e.new_term(con_cell(",",2),
			 {e.new_term(local_interpreter::COLON,
				     {local_interpreter::ME,
					     e.new_term(con_cell("version", 1),
							{e.new_ref()})}),
			  e.new_term(local_interpreter::COLON,
				     {local_interpreter::ME,
					     e.new_term(con_cell("comment", 1),
							{e.new_ref()})})
		        })
	     }));
    } else if (get_state() == RECEIVED) {
	pattern p(e);

	auto const me = local_interpreter::ME;
	auto const colon = local_interpreter::COLON;
	auto const comma = local_interpreter::COMMA;
	auto const result_5 = con_cell("result",5);
	auto const id_1 = con_cell("id",1);
	auto const version = con_cell("version",1);
	auto const comment_1 = con_cell("comment",1);
	auto const ver = con_cell("ver",2);
	con_cell id;
	int64_t major_ver0 = 0, minor_ver0 = 0;
	term comment;

	//
	// pattern: result(me:id(Id), me:version(Major,Minor),
        //                 me:comment(Comment),_,_)
	//
	auto const pat = p.str(result_5,
	       p.str( comma,
		      p.str(colon,
			    p.con(me),
			    p.str(id_1, p.any_atom(id))),
		      p.str(comma,
			    p.str(colon,
				  p.con(me),
				  p.str(version,
					p.str(ver,
					      p.any_int(major_ver0),
					      p.any_int(minor_ver0)))),
			    p.str(colon,
				  p.con(me),
				  p.str(comment_1, p.any(comment)))),
		      p.ignore(),
		      p.ignore(),
		      p.ignore(),
		      p.ignore()));

	if (!pat(e, get_result())) {
	    error(reason_t::ERROR_UNRECOGNIZED);
	    return;
	}

	// Have we connected to ourselves via another address?
	if (e.atom_name(id) == self().id()) {
	    self().add_self(connection().ip());
	    self().book()().remove(connection().ip());
	    error(reason_t::ERROR_SELF);
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

	//
	// Answer accepted. Move unverified entry to verified.
	//

	auto book = self().book();
	book().remove(ip());

	// Now add a verified entry with neutral score
	address_entry verified(ip());
	verified.set_score(address_entry::VERIFIED_INITIAL_SCORE);
	verified.set_version(major_ver, minor_ver);
	verified.set_comment(comment, e);

	book().add(verified);

	connection().stop();
    }
}

}}
