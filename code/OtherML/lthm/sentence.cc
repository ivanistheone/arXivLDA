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

#include "sentence.h"
#include "error.h"


using namespace std;

namespace htmm {

Sentence::Sentence(const Sentence &s) : data_(s.data_) {
}

Sentence::~Sentence() {
  // The vector destructor is called automatically
}

// Sets the number of words in the sentence (allocates space for them).
void Sentence::init(int words) {
  data_.resize(words);
}

// Adds the word 'word' in the location i.
void Sentence::AddWord(int i, int word) {
  data_[i] = word;
}

}  // namespace
