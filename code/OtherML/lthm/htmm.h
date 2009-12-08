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


#ifndef OPENHTMM_HTMM_H__
#define OPENHTMM_HTMM_H__

#include <vector>
#include <map>
#include "document.h"


using namespace std;

namespace htmm {

class HTMM {
 public:

  HTMM() {}

  virtual ~HTMM();

  virtual void Infer() = 0;  //  this can be EM inference,
                             //  Collapsed Gibbs sampling or whatever.
 protected:
  // init can now be called only by superclasses
  // fname is the name of the file that specifies the locations of
  // the train documents.
  void init(int topics, int words, double alpha, double beta,
            const char *fname, const char *data_dir,
	    const char *train_map_file);

  // Reads the train documents whose location is specified in the given file.
  virtual void ReadTrainDocuments(const char *fname, const char *data_dir,
				  const char *train_file);

  // Read the list of train files into a map.
  void ReadTrainMap(const char *train_file);

  // Data members:
  int topics_;                          // number of topic
  int words_;                           // number of words in vocabulary
  vector<Document *> docs_;             // the set of train documents
  double alpha_;                        // the symmetric dirichlet prior
  double beta_;                         // the symmetric dirichlet prior
  map<int, int> train_map_;		// the documents that belong
					// to the train set.

 private:

  // Copy constructor - never use it for this class!
  HTMM(const HTMM &);
};

}  // namespace

#endif  // OPENHTMM_HTMM_H__
