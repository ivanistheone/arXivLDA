/**
   This file is a part of

   DF-LDA - Implementation of the Dirichlet Forest (DF) prior
   for the Latent Dirichlet Allocation (LDA) model

   Copyright (C) 2009 David Andrzejewski (andrzeje@cs.wisc.edu)
 
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <typeinfo>

using namespace std;

#ifndef H_NODE
#define H_NODE

// Uniform rand between [0,1] (inclusive)
#define unif() ((double) rand()) / ((double) RAND_MAX)

class node
{
 protected:
  vector<node*> ichildren;
  vector<int> maxind;
  int leafstart;

 public:
  static double unif01();
  static int mult_sample(double* vals, double sum);
  static int mult_sample(vector<double> vals, double sum);

  static int log_mult_sample(vector<double> vals);

  static void set_randseed(uint randseed);
  vector<node*> get_multinodes();
  virtual void modify_count(double val, int leaf);
  virtual double calc_wordterm(double val, int leaf);  
  virtual ~node();
};
#endif
