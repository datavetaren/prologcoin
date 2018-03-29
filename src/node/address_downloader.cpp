#include "../common/term_match.hpp"
#include "self_node.hpp"
#include "address_downloader.hpp"
#include "local_interpreter.hpp"

using namespace prologcoin::common;

namespace prologcoin { namespace node {

address_downloader::address_downloader(out_connection &out)
    : out_task(out, &address_downloader::process_fn)
{ }

void address_downloader::process_fn(out_task &task)
{
    reinterpret_cast<address_downloader &>(task).process();
}

void address_downloader::process()
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
	// peers(10, X)
	//
	int64_t num_to_download
	    = static_cast<int64_t>(self().get_num_download_addresses());
        set_query(e.new_term(local_interpreter::COLON,
	         {local_interpreter::ME,
		 e.new_term(con_cell("peers", 2),
			    {int_cell(num_to_download),e.new_ref()})}));
    } else if (get_state() == RECEIVED) {
	auto const me = local_interpreter::ME;
	auto const colon = local_interpreter::COLON;
	auto const result_3 = con_cell("result",3);
	auto const peers_2 = con_cell("peers",2);
	term peers;

	//
	// pattern: result(me:peers(10,X),_,_)
	//
	pattern p(e);
	auto const pat = p.str(result_3,
			       p.str(colon,
				     p.con(me),
				     p.str(peers_2,
					   p.ignore(),
					   p.any(peers))));

	if (!pat(e, get_result())) {
	    fail(ERROR_UNRECOGNIZED);
	    return;
	}

	//
	// Answer accepted. Move unverified entry to verified.
	//
	auto book = self().book();

	while (e.is_dotted_pair(peers)) {
	    address_entry entry;
	    term term_entry = e.arg(peers, 0);
	    if (entry.from_term(e, term_entry)) {
		entry.set_source(connection().ip());
		entry.set_score(0);
		entry.set_time(utime::now_seconds());
		book().add(entry);
	    }
	}
    }
}

}}
