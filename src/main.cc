// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include <cstdlib>
#include <cstdint>

#include <iostream>
#include <fstream>
#include <string>

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

DEFINE_string(f, "archive.huf", "A .huf archive");
DEFINE_bool(c, false, "Create an archive");
DEFINE_bool(x, false, "Extract an archive");


void usage(char** argv) {
  cout << "Huffman File Compression\n\n"
       << "\t-h\t\tPrints this usage information\n"
       << "\t-x <archive>\tArchive file to be extracted\n"
       << "\t-x <archive>\tArchive file to be created\n\n"
       << "Exactly one of <c|x> must be used. "
       << "Additionally, a filename is required to specify "
       << "the file being compressed for the `c` flag or "
       << "the location to write the decompressed data "
       << "for the `x` flag." << endl;
  exit(1);
}

void create(char* data_file_name) {
  ifstream data_file(data_file_name, std::ios::binary);
  ofstream archive_file(FLAGS_f, std::ios::binary);
  if (!archive_file.is_open()) {
    cerr << "Could not open output stream." << endl;
    exit(1);
  }else if (!data_file.is_open()) {
    cerr << "Data file not found." << endl;
    exit(1);
  }
  
  int in_size = data_file.tellg();
  data_file.seekg(0, ios::end);
  in_size = data_file.tellg() - in_size;
  data_file.seekg(0, ios::beg);

  cout << "Creating input buffer" << endl;
  
  char* in_buffer = new char[in_size];
  data_file.read(in_buffer, in_size);
  data_file.close();

  cout << "Input buffer complete; buiding encoding tree" << endl;
  
  huffman::Huffman huf;
  huf.BuildTree(reinterpret_cast<unsigned char*>(in_buffer), in_size);

  cout << "Encoding tree built; serializing histogram." << endl;
  
  char* out_buffer = nullptr;
  int out_size = -1;
  huf.Serialize(reinterpret_cast<uint8_t**>(&out_buffer), &out_size);
  archive_file.write(out_buffer, out_size);
  delete[] out_buffer;

  cout << "Histogram serialized and written to output; encoding file." << endl;

  base::BitString encoded;
  huf.BuildMap();

  cout << "Map built... ";
  
  huf.Encode(in_buffer, in_size, &encoded);
  delete[] in_buffer;

  cout << "File encoded. Serializing bitstring." << endl;

  out_buffer = nullptr;
  out_size = -1;
  encoded.Serialize(reinterpret_cast<uint8_t**>(&out_buffer), &out_size);

  archive_file.write(out_buffer, out_size);
  delete[] out_buffer;

  archive_file.flush();
  archive_file.close();
}

void extract(char* data_file_name);

int main(int argc, char** argv) {
  gflags::SetUsageMessage(string(argv[0]) + " -<x|c> -f <archive.huf> <file>");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Only one of `x` and `c` may be used
  // There must be exactly one argument remaining
  if (FLAGS_x == FLAGS_c)
    usage(argv);

  cout << argc << endl;

  if (FLAGS_x) {
    //    extract(argv[1]);
    cout << "unimplemented" << endl;
  } else {
    create(argv[1]);
  }

  gflags::ShutDownCommandLineFlags();
  return 0;
}
