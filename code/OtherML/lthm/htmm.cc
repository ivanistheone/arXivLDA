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

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "htmm.h"
#include "document.h"
#include "error.h"


using namespace std;

namespace htmm {

// Initializes data members and reads the train documents.
void HTMM::init(int topics, int words, double alpha, double beta,
                const char *fname, const char *data_dir, 
		const char *train_map_file) {
  topics_ = topics;
  words_ = words;
  alpha_ = alpha;
  beta_ = beta;
  ReadTrainDocuments(fname, data_dir, train_map_file);
}

// Reads the train documents whose location is specified in the given file.
void HTMM::ReadTrainDocuments(const char *fname, const char *data_dir,
			      const char *train_map_file) {
  ReadTrainMap(train_map_file);
  string full_path = fname;
  ifstream in(full_path.c_str());
  if (!in || in.fail()) {
    ERROR("can't open file " << full_path << " for reading");
  }
  std::string str;
  int counter = 0;
  for (in >> str; !in.eof(); in >> str, counter++) {
    // read only train documents
    if (train_map_.find(counter) != train_map_.end()) {
      Document *d = new Document;
      str = std::string(data_dir) + str;
      d->init(str.c_str());
      docs_.push_back(d);
    }
  }
  in.close();
}

// Read the list of train files into a map.
void HTMM::ReadTrainMap(const char *train_file) {
  ifstream in(train_file);
  if (!in || in.fail()) {
    ERROR("can't open train map file " << train_file);
  }
  int doc_num, N;
  int counter = 0;
  in >> N;  // Read number of docs in corpus (more than there are in the train)
  for (in >> doc_num; !in.eof(); in >> doc_num, counter++) {
    train_map_[doc_num-1] = counter;
  }
  in.close();
}

HTMM::~HTMM() {
  for (int d = 0; d < docs_.size(); d++) {
    delete docs_[d];
  }
}

}  // namespace
