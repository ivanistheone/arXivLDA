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

#include <iostream>
#include <fstream>
#include "document.h"
#include "sentence.h"
#include "error.h"


using namespace std;

namespace htmm {

Document::~Document() {
  for (int i = 0; i < data_.size(); i++) {
    delete data_[i];
  }
}

// Initialization: Reads a sequence of sentences from a file.
void Document::init(const char *fname) {
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file " << fname << " for reading");
  }

  Sentence *s = new Sentence;
  for (ReadSentence(&in, s); !in.eof(); ReadSentence(&in, s)) {
    data_.push_back(s);
    s = new Sentence;
  }
  // The last sentence object corresponds to eof and was not added to the
  // document.
  delete s;

  in.close();
}

void Document::ReadSentence(ifstream *in, Sentence *s) {
  int words, w;
  (*in) >> words;
  if (in->eof()) {
    return;
  }
  s->init(words);
  for (int i = 0; i < words; i++) {
    (*in) >> w;
    s->AddWord(i, w-1);  // We subtract one due to the range of input in
                         // data (1..N)
  }
}

}  // namespace
