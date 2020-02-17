#include <cstdio>
#include "blockdb.hpp"
#include "blockdb_meta_data.hpp"

namespace prologcoin { namespace db {

void blockdb_meta_entry::print(std::ostream &out) const
{
    out << "{index=" << index_ << ",height=" << height_ << ",offset=" << offset_ << ",size=" << size_ << "}";
}

const char blockdb_bucket::VERSION[16] = "blockdb_1.0    ";
    
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
    uint8_t buffer[blockdb_params::MAX_META_BLOCK_SIZE];

    fstream *f = get_bucket_meta_stream();

    f->seekg(0, fstream::end);
    size_t max_offset = f->tellg();
    
    f->seekg(0);
    size_t p = 0;
    size_t meta_block_size = params_.meta_block_size();
    
    for (size_t offset = 0; offset < max_offset; offset += meta_block_size) {
        f->read(buffer, meta_block_size);

	if (!(*f)) {
	    break;
	}

	if (offset == 0) {
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
	}
      
	uint32_t num_entries = read_uint32(&buffer[p]);
	p += sizeof(uint32_t);
	p += static_cast<size_t>(num_entries)*blockdb_meta_entry::SERIALIZATION_SIZE;

	if (p > meta_block_size) {
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
	
	p = 0;
    }
}

void blockdb_bucket::write_meta_data()
{
    // Close read stream if there's one available
    if (fstream_meta_) {
        fstream_meta_->close();
	delete fstream_meta_;
	fstream_meta_ = nullptr;
    }
  
    uint8_t buffer[blockdb_params::MAX_BLOCK_SIZE];

    // Write meta data to a different file

    std::string new_path = file_path_meta_ + ".new";
  
    fstream f_new(new_path, fstream::out | fstream::binary | fstream::trunc);
    
    size_t meta_block_size = params_.meta_block_size();

    size_t p = 0;
    size_t offset = 0;
    uint32_t count = 0;
    size_t p_num_entries = 0;
    
    for (auto &v : entries_) {
        for (auto &e : v) {
	    if (p == 0) {
	        if (offset == 0) {
		    // Version info if first block
		    memcpy(buffer, VERSION, VERSION_SZ);
		    p += VERSION_SZ;
		    p_num_entries = p;
		}
		p += sizeof(uint32_t); // Number of entries in this block
	    }
	    if (p + blockdb_meta_entry::SERIALIZATION_SIZE > meta_block_size) {
	        // Update number of entries
	        write_uint32(&buffer[p_num_entries], count);
		f_new.write(buffer, meta_block_size);
		if (f_new.fail()) {
  		    throw blockdb_write_exception("Error while writing to " + new_path);
		}
		count = 0;
		p_num_entries = 0;
		p = sizeof(uint32_t); // Number of entries for next block
	    }
	    size_t n = 0;
	    e.write(&buffer[p], n);
	}
    }

    // Write remaining block
    if (count > 0) {
        write_uint32(&buffer[p_num_entries], count);
	f_new.write(buffer, meta_block_size);	
    }
    if (f_new.fail()) {
        throw blockdb_write_exception("Error while writing to " + new_path);
    }
    f_new.close();

    // Swap in the new file and remove the old
    std::string old_path = file_path_meta_ + ".old";

    auto do_rename = [&](const std::string &old_path, const std::string &new_path) {
        int status = std::rename(old_path.c_str(), new_path.c_str());
	if (status != 0) {
	    throw blockdb_write_exception("Error while renaming '" + old_path + "' to '" + new_path + "'");
        }
    };

    auto do_delete = [&](const std::string &path) {
        int status = std::remove(path.c_str());
	if (status != 0) {
	    throw blockdb_write_exception("Error while deleting '" + path + "'");
        }
    };

    
    do_rename(file_path_meta_, old_path);
    do_rename(new_path, file_path_meta_);
    do_delete(old_path);
}
    
}}
