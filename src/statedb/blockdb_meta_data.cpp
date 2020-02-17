#include "blockdb.hpp"
#include "blockdb_meta_data.hpp"

namespace prologcoin { namespace statedb {

void blockdb_meta_entry::print(std::ostream &out) const
{
    out << "{index=" << index_ << ",height=" << height_ << ",offset=" << offset_ << ",size=" << size_ << "}";
}
    
void blockdb_bucket::print(std::ostream &out) const
{
    size_t num_entries = 0;
    for (auto &v : entries_) {
        num_entries += v.size();
    }
    out << "bucket{num_entries=" << num_entries << ",[";
    bool first = true;
    for (auto &v : entries_) {
        for (auto &e : v) {
	    if (!first) out << ", ";
	    e.print(out);
	    first = false;
        }
    }
    out << "]}";
}

void blockdb_bucket::read_meta_data()
{
    static const char VERSION[16] = "blockdb_1.0    ";
    static const size_t VERSION_SZ = sizeof(VERSION);
  
    uint8_t buffer[blockdb_params::MAX_BLOCK_SIZE];

    fstream *f = get_bucket_stream();
  
    f->seekg(0);
    f->read(buffer, sizeof(buffer));

    size_t p = 0;
    size_t current_offset = 0;

    if (memcmp(buffer, VERSION, VERSION_SZ) != 0) {
        buffer[15] = '\0';
        std::string msg = "Unrecognized version: ";
	msg += reinterpret_cast<char *>(&buffer[0]);
	msg += "; expected: '";
	msg += VERSION;
	msg += "'";
        throw blockdb_version_exception(msg);
    }

    p += VERSION_SZ;

    size_t tot = VERSION_SZ;
    bool cont = true;

    f->seekg(0, fstream::end);
    size_t max_offset = f->tellg();
    
    while (cont) {
        uint32_t next_offset = read_uint32(&buffer[p]);
	p += sizeof(uint32_t);
	uint32_t num_entries = read_uint32(&buffer[p]);

	tot += 2*sizeof(uint32_t);
	tot += static_cast<size_t>(num_entries)*blockdb_meta_entry::SERIALIZATION_SIZE;

	if (tot > params_.block_size()) {
	    throw blockdb_meta_data_exception("Attempting to read meta data beyond block. num_entries=" + boost::lexical_cast<std::string>(num_entries));
	}

	size_t n = static_cast<size_t>(num_entries);

	for (size_t i = 0; i < n; i++) {
  	     blockdb_meta_entry e;
	     size_t sz = 0;
	     e.read(&buffer[p], sz);
	     assert(sz == blockdb_meta_entry::SERIALIZATION_SIZE);
	     p += blockdb_meta_entry::SERIALIZATION_SIZE;
	     add_entry(e);
	}
	cont = next_offset != 0;

	if (next_offset < current_offset) {
	    throw blockdb_meta_data_exception("Attempting to read meta data by going backwards in stream. current_offset=" + boost::lexical_cast<std::string>(current_offset) + "; next_offset=" + boost::lexical_cast<std::string>(next_offset));
	}
	
	if (next_offset >= max_offset) {
	    throw blockdb_meta_data_exception("Attempting to read meta data outside file. max_offset=" + boost::lexical_cast<std::string>(max_offset) + "; next_offset=" + boost::lexical_cast<std::string>(next_offset));
	}
	
	p = 0;
	tot = 0;
	if (cont) {
	    p = 0;
	    f->seekg(next_offset);
	    f->read(buffer, sizeof(buffer));
	}
    }
}
    
}}
