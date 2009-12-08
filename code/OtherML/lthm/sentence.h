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

#ifndef OPENHTMM_SENTENCE_H__
#define OPENHTMM_SENTENCE_H__

#include <vector>


using namespace std;

namespace htmm {

class Sentence {
 public:

  Sentence() {}

  // Copy constructor
  Sentence(const Sentence &s);

  ~Sentence();

  // Sets the number of words in the sentence (allocates space for them).
  void init(int words);

  // Returns the number of words in the sentence.
  int size() const { return data_.size(); }

  // Returns the i word of the sentence.
  int GetWord(int i) const { return data_[i]; }

  // Adds the word 'word' as the i'th word of the sentence.
  void AddWord(int i, int word);

 private:
  vector<int> data_;                    // The words of the sentence.
};

}  // namespace

#endif  // OPENHTMM_SENTENCE_H__
