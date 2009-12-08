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

#ifndef OPENHTMM_DOCUMENT_H__
#define OPENHTMM_DOCUMENT_H__

#include <vector>
#include "sentence.h"


using namespace std;

namespace htmm {

class Document {
 public:
  Document() {}

  // Copy Ctor.
  //  Document(const Document &d);

  virtual ~Document();

  // Initialization: reads a sequence of sentences from a file.
  virtual void init(const char *fname);

  // Retrieves the i sentence of the document.
  const Sentence &GetSentence(int i) const { return *(data_[i]); }

  // Returns the number of sentences in the document.
  int size() const { return data_.size(); }
 protected:
  vector<Sentence *> data_;  // the basic unit is a sentence,
                             // it could also consist of a single word

 private:
  // Never use copy constructor with this class !
  Document(const Document &doc);

  // not to be use in subclasses
  void ReadSentence(ifstream *in, Sentence *s);
};

}  // namespace

#endif  // OPENHTMM_DOCUMENT_H__
