#include <boost/filesystem/operations.hpp>
#include "statedb.hpp"

namespace prologcoin { namespace statedb {

statedb::statedb(const std::string &dir_path) : dir_path_(dir_path), stream_closer_(), stream_cache_(CACHE_NUM_STREAMS, stream_closer_), block_flusher_(*this), block_cache_(DEFAULT_CACHE_NUM_BLOCKS, block_flusher_) {
}

bucket & statedb::get_bucket(size_t bucket_index) {
    if (bucket_index < buckets_.size() && !buckets_[bucket_index].empty()) {
        return buckets_[bucket_index];
    }
    get_bucket_stream(bucket_index);
    return buckets_[bucket_index];
}
    
fstream * statedb::get_bucket_stream(size_t bucket_index)
{
    if (bucket_index >= buckets_.size()) {
        buckets_.resize(bucket_index+1);
    }

    fstream *f = nullptr;
    auto r = stream_cache_.find(bucket_index);
    if (!r.is_initialized()) {
        auto path = bucket_file_path(bucket_index);
	bool exists = boost::filesystem::exists(path);
	f = new fstream(path.string(), fstream::binary);
	if (!exists) {
	    // Write meta-data for an empty bucket
	    bucket b;
	    write_meta_data(f, b);
	}
	stream_cache_.insert(bucket_index, f);
    } else {
        f = *r;
    }
    bucket &b = buckets_[bucket_index];
    if (b.empty()) {
        read_meta_data(f, b);
    }

    return f;
}

void statedb::read_meta_data(fstream *f, bucket &bucket)
{
    static const char VERSION[16] = "statedb_1.0    ";
    static const size_t VERSION_SZ = sizeof(VERSION);
  
    uint8_t buffer[MAX_BLOCK_SIZE];
  
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
        throw statedb_version_exception(msg);
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
	tot += static_cast<size_t>(num_entries)*meta_entry::SERIALIZATION_SIZE;

	if (tot > block_size_) {
	    throw statedb_meta_data_exception("Attempting to read meta data beyond block. num_entries=" + boost::lexical_cast<std::string>(num_entries));
	}

	size_t n = static_cast<size_t>(num_entries);

	for (size_t i = 0; i < n; i++) {
  	     meta_entry e;
	     size_t sz = 0;
	     e.read(&buffer[p], sz);
	     assert(sz == meta_entry::SERIALIZATION_SIZE);
	     p += meta_entry::SERIALIZATION_SIZE;
	     bucket.add_entry(e);
	}
	cont = next_offset != 0;

	if (next_offset < current_offset) {
	    throw statedb_meta_data_exception("Attempting to read meta data by going backwards in stream. current_offset=" + boost::lexical_cast<std::string>(current_offset) + "; next_offset=" + boost::lexical_cast<std::string>(next_offset));
	}
	
	if (next_offset >= max_offset) {
	    throw statedb_meta_data_exception("Attempting to read meta data outside file. max_offset=" + boost::lexical_cast<std::string>(max_offset) + "; next_offset=" + boost::lexical_cast<std::string>(next_offset));
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

void statedb::write_meta_data(fstream *f, const bucket &bucket)
{
}    


block * statedb::find_block(size_t index, size_t from_height)
{
    bucket &b = get_bucket(bucket_index(index));
    auto e = b.find_entry(index, from_height);
    if (!e.is_initialized()) {
        return nullptr;
    }
    return nullptr;
}
    
}}
