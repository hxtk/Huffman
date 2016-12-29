// Copyright: Peter Sanders. All rights reserved.
// Date: 2016-12-28

#include <cstdint>

#include <iostream>
#include <iomanip>
#include <string>

#include <gflags/gflags.h>

#include "base/bitstring.h"

using std::cout;
using std::endl;
using std::setw;
using std::string;

int main(int argc, char** argv) {
  gflags::SetUsageMessage(string(argv[0]) + " --v=<log level>");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  cout << "String of 25 `true`" << endl;
  base::BitString bs1;
  for (int i = 0; i < 25; ++i) {
    bs1.Append(true);
    cout << setw(2) << i << ": " << bs1 << endl;
  }
  cout << bs1 << endl;

  cout << "String of 25 `false`" << endl;
  base::BitString bs2;
  for (int i = 0; i < 25; ++i) {
    bs2.Append(false);
    cout << setw(2) << i << ": " << bs2 << endl;
  }
  cout << bs2 << endl;

  cout << "Join previous two strings with assignment" << endl;
  base::BitString bs3 = bs1;
  bs3.Append(bs2);
  cout << bs3 << endl;

  cout << "Empty first string" << endl;
  while (!bs1.empty()) {
    bs1.PopBack();
    cout << bs1 << endl;
  }

  cout << "Empty second string" << endl;
  while (!bs2.empty()) {
    bs2.PopBack();
    cout << bs2 << endl;
  }
  
  cout << "Serialize third string" << endl;
  void* buffer = nullptr;
  int size = -1;
  bs3.Serialize(&buffer, &size);

  cout << "Serialized.\nSize: " << size << " bytes. Unserializing." << endl;

  base::BitString bs4;
  bs4.Unserialize(buffer, size);

  cout << "original: " << bs3 << "\n"
       << "serial:   " << bs4 << endl;

  cout << "Empty third string" << endl;
  while (!bs3.empty()) {
    bs3.PopBack();
    cout << bs3 << endl;
  }

  return 0;
}
