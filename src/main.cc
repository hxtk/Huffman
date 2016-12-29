// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include <cstdlib>
#include <cstdint>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "base/bitstring.h"
#include "huffman/huffman.h"

using std::cout;
using std::cerr;
using std::endl;

using std::ofstream;
using std::ifstream;
using std::ios;

using std::string;
using std::vector;

DEFINE_string(f, "archive.huf", "A .huf archive");
DEFINE_bool(c, false, "Create an archive");
DEFINE_bool(x, false, "Extract an archive");

void create(char* data_file_name) {
  // Open files
  ifstream data_file(data_file_name, std::ios::binary);
  ofstream archive_file(FLAGS_f, std::ios::binary);

  // Ensure that both files are opened
  if (!archive_file.is_open()) {
    cerr << "Could not open output stream." << endl;
    exit(1);
  }else if (!data_file.is_open()) {
    cerr << "Data file not found." << endl;
    exit(1);
  }

  // Find size of uncompressed file and create a buffer to hold it
  int in_size = data_file.tellg();
  data_file.seekg(0, ios::end);
  in_size = data_file.tellg() - in_size;
  data_file.seekg(0, ios::beg);

  // Read the uncompressed file into the buffer and close the file stream
  void* in_buffer = new char[in_size];
  data_file.read(in_buffer, in_size);
  data_file.close();

  // Create a Huffman Coding object and initialize the histogram
  // from the file buffer.
  huffman::Huffman huf;
  huf.BuildTree(in_buffer, in_size);

  // Create and write the header of the oputput file
  void* out_buffer = nullptr;
  int out_size = -1;
  huf.Serialize(&out_buffer, &out_size);
  archive_file.write(out_buffer, out_size);
  delete[] out_buffer;

  // Encode the uncompressed file into a packed bitstring and delete the buffer
  huf.BuildMap();
  base::BitString encoded;
  huf.Encode(in_buffer, in_size, &encoded);
  delete[] in_buffer;

  // Serialize the bitstring and write it to the archive.
  out_buffer = nullptr;
  out_size = -1;
  encoded.Serialize(reinterpret_cast<uint8_t**>(&out_buffer), &out_size);
  archive_file.write(out_buffer, out_size);
  delete[] out_buffer;

  // Flush and close the archive file
  archive_file.flush();
  archive_file.close();
}

void extract(char* data_file_name) {
  // Open files.
  ifstream archive(FLAGS_f, std::ios::binary);
  ofstream decompressed(data_file_name, std::ios::binary);

  // Ensure that both files are opened
  if (!archive.is_open()) {
    cerr << "Archive not found." << endl;
    exit(1);
  }else if (!decompressed.is_open()) {
    cerr << "Could not open output stream." << endl;
    exit(1);
  }

  // Find size of archive file and create a buffer to hold it
  int in_size = archive.tellg();
  archive.seekg(0, ios::end);
  in_size = archive.tellg() - in_size;
  archive.seekg(0, ios::beg);

  // Read the archive into the buffer and close the file stream
  char* in_buffer = new char[in_size];
  archive.read(in_buffer, in_size);
  archive.close();

  // Unserialize histogram
  huffman::Huffman huf;
  huf.Unserialize(reinterpret_cast<uint8_t*>(in_buffer), in_size);

  // Unserialize data segment
  int bitstring_size = in_size - huffman::Huffman::get_header_size(
      reinterpret_cast<uint8_t*>(in_buffer));

  cout << "Archive has\nHeader: " << (in_size - bitstring_size)
       << "\nData: " << bitstring_size
       << "\nTotal: " << in_size << endl;
  base::BitString bits;
  if (!bits.Unserialize(huffman::Huffman::get_data_segment(
	   reinterpret_cast<uint8_t*>(in_buffer)), bitstring_size)) {
    cout << "Failed to unserialize bitstring" << endl;
    delete[] in_buffer;
    exit(1);
  }
  delete[] in_buffer;

  // Decode BitString
  vector<uint8_t> out_buffer = huf.Decode(bits);
  const char* out_ptr = reinterpret_cast<char*>(out_buffer.data());
  decompressed.write(out_ptr, out_buffer.size());

  // Flush and close file
  decompressed.flush();
  decompressed.close();
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(string(argv[0]) + " -<x|c> -f <archive.huf> <file>");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Only one of `x` and `c` may be used
  // There must be exactly one argument remaining
  if (FLAGS_x == FLAGS_c || argc != 2) {
    cout << "See `" << argv[0] << " --help` for usage information." << endl;
    exit(1);
  }

  if (FLAGS_x) {
    extract(argv[1]);
  } else {
    create(argv[1]);
  }

  gflags::ShutDownCommandLineFlags();
  return 0;
}
