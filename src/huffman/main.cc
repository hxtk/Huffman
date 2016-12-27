// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-11-30

#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <unistd.h>

#include "huffman.h"

using std::cout;
using std::cerr;
using std::endl;

using std::ofstream;
using std::ifstream;
using std::ios;

using std::string;
using std::vector;

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

void create(char* archive_file_name, char* data_file_name) {
  ifstream data_file(data_file_name, std::ios::binary);
  ofstream archive_file(archive_file_name, std::ios::binary);
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

  char* in_buffer = new char[in_size];
  data_file.read(in_buffer, in_size);
  data_file.close();

  Huffman huf;
  huf.BuildTree(reinterpret_cast<unsigned char*>(in_buffer), in_size);
  
  char* out_buffer = nullptr;
  int out_size = -1;
  huf.Serialize(&out_buffer, &out_size);
  archive_file.write(out_buffer, out_size);
  delete[] out_buffer;

  vector<bool> encoded = {};
  huf.BuildMap();
  huf.Encode(in_buffer, in_size, &encoded);
  delete[] in_buffer;

  archive_file.write(
      reinterpret_cast<const char*>(&encoded[0]), encoded.size());
  archive_file.flush();
  archive_file.close();
}

void extract(char* archive_file_name, char* data_file_name);

int main(int argc, char** argv) {
  bool extract_flag = false;
  bool create_flag = false;
  int character = 0;
  char* archive_file = nullptr;

  opterr = 0;

  while (character = getopt(argc, argv, "c:x:h") != -1) {
    switch (character) {
      case 'x':
        extract_flag = true;
        break;
      case 'c':
        create_flag = true;
        break;
      case 'h':
      default:
        usage(argv);
    }
  }
  // There must be only one non-consumed input string, which shall be
  // the input file.
  //
  // Further, one may only create or extract an archive.
  // Both cannot be done at once.
  if (optind + 1 != argc || (!extract_flag == !create_flag)) {
    usage(argv);
  }

  if (extract_flag) {
    extract(optarg, argv[optind]);
  } else {
    create(optarg, argv[optind]);
  }
  
  return 0;
}
