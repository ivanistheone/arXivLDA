// Copyright 2007 Google Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: agdevhtmm@gmail.com (Amit Gruber)

#include <stdlib.h>
#include <iostream>
#include <string>
#include <new>
#include <fstream>
#include "web_em.h"
#include "error.h"

// Driver for EM approximate inference for HTMM
// Analyses train data and saves the parameters and posterior distributions


using namespace std;

using namespace htmm;

void my_handler() {
  ERROR("memory allocation failed");
}

void SaveCmdLine(int argc, char *argv[], const string &fname) {
  ofstream out(fname.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << fname << " for writing");
  }
  for (int i = 0; i < argc; i++) {
    out << argv[i] << " ";
  }
  out << endl;
  out.close();
}

int main(int argc, char *argv[]) {
  if (argc != 10) {
    cerr << "Usage: " << argv[0]
	 << " topics words alpha beta iters train_dir out_file working_dir" <<
      " map_file\n";
      //         << " topics words alpha beta iters train_file out_file working_dir "
      //	 << " links_file gamma_file\n";
    exit(1);
  }
  set_new_handler(my_handler);
  int topics = atoi(argv[1]);           // number of topics
  int words = atoi(argv[2]);            // number of words in vocabulary
  double alpha = atof(argv[3]);         // Dirichlet prior
  double beta = atof(argv[4]);          // Dirichlet prior
  int iters = atoi(argv[5]);            // number of EM iterations
  const char *train_dir = argv[6];
  //  const char *train = argv[6];          // file with a list of train docs
  const char *out_file = argv[7];       // base name for output files
  const char *working_dir = argv[8];
  //  const char *links_file = argv[9];
  //  const char *gamma_file = argv[10];
  string train = string(train_dir) + "/list";
  string gamma_file = string(train_dir) + "/gamma.init";
  string links_file = string(train_dir) + "/links";
  const char  *map_file = argv[9];
  WebEM em;
  // Use clock to initialise random seed.
  int seed = 0; //1198585985; //1198245638;
  SaveCmdLine(argc, argv, string(out_file)+string(".cmd"));
  em.init(topics, words, alpha, beta, iters, train.c_str(), working_dir, seed,
	  links_file.c_str(), gamma_file.c_str(), map_file);
	  //	  "/cs/vis/amitg/webLDA/data/gamma.init");
  cerr << "after init\n";
  em.Infer();
  cerr << "after infer\n";
  em.SaveAll(out_file);
  return 0;
}
