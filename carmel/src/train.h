/*******************************************************************************
* This software ("Carmel") is licensed for research use only, as described in  *
* the LICENSE file in this distribution.  The software can be downloaded from  *
* http://www.isi.edu/natural-language/licenses/carmel-license.html.  Please    *
* contact Yaser Al-Onaizan (yaser@isi.edu) or Kevin Knight (knight@isi.edu)    *
* with questions about the software or commercial licensing.  All software is  *
* copyrighted C 2000 by the University of Southern California.                 *
*******************************************************************************/

#ifndef TRAIN_H
#define TRAIN_H 1
#include "assert.h"
#include "2hash.h"
#include "weight.h"
#include "list.h"
#include "2hash.h"
#include "Arc.h"

struct IntKey {
  int i;
  int hash() const { return i * 2654435767U; }
  IntKey() {}
  IntKey(int a) : i(a) {}
  operator int() const { return i; }
};


struct State {
  List<Arc> arcs;
  int size;
#ifdef BIDIRECTIONAL
  int hitcount;			// how many times index is used, negative for index on input, positive for index on output
#endif
  HashTable<IntKey, List<HalfArc> > *index;
  State() : arcs(), size(0), 
#ifdef BIDIRECTIONAL
hitcount(0), 
#endif
index(NULL) { }
  State(const State &s): arcs(s.arcs),size(s.size) {
#ifdef BIDIRECTIONAL
    hitcount = s.hitcount;
#endif
    if (s.index == NULL)
      index = (HashTable<IntKey, List<HalfArc> > *) NULL ;
    else
      index = new HashTable<IntKey, List<HalfArc> >(*s.index);
     } 
  ~State() { flush(); }
  void indexBy(int output = 0) {
    List<HalfArc> *list;
    if ( output ) {
#ifdef BIDIRECTIONAL
      if ( hitcount++ > 0 )
	return;
      hitcount = 1;
      delete index;
#else
      if ( index )
	return;
#endif
      index = new HashTable<IntKey, List<HalfArc> >(size);
      for ( List<Arc>::iterator l=arcs.begin() ; 
	    l != arcs.end() ; 
	    ++l ) {
	if ( !(list = index->find(l->out)) )
	  index->add(l->out, 
		     List<HalfArc>(static_cast<size_t>(1),&(*l)));
	else
	  list->push(&(*l));
      }
      return;
    }
#ifdef BIDIRECTIONAL
    if ( hitcount-- < 0 )
      return;
    hitcount = -1;
    delete index;
#else
    if ( index )
      return;
#endif
    index = new HashTable<IntKey, List<HalfArc> >(size);
    for ( List<Arc>::iterator l=arcs.begin() ; l != arcs.end() ; ++l ) {
      if ( !(list = index->find(l->in)) )
	index->add(l->in, List<HalfArc>(1,&(*l)));
      else
	list->push(&(*l));
    }
  }
  void flush() {
    delete index;
    index = NULL;
#ifdef BIDIRECTIONAL
    hitcount = 0;
#endif
  }
  void addArc(const Arc &arc)
  {
    arcs.push(arc);
    ++size;
    Assert(!index);
#ifdef DEBUG
    if (index) {
      cerr << "Warning: adding arc to indexed state.\n";
      delete index;
      index = NULL;
    }
#endif
  }
  void reduce() {		// consolidate all duplicate arcs
    HashTable<UnArc, Weight *> hWeights;
    UnArc un;
    Weight **ppWt;
  for ( List<Arc>::iterator l=arcs.begin() ; l != arcs.end() ; ) {
      if ( l->weight == 0 ) {
	// remove as originally implemented by John in ListIter deletes current and jumbs to next
	 List<Arc>::iterator temp = l++ ;	 
	 arcs.erase(temp);
	continue;
      }
      un.in = l->in;
      un.out = l->out;
      un.dest = l->dest;
      if ( (ppWt = hWeights.find(un)) ) {
	**ppWt += l->weight;
	if ( **ppWt > 1 )
	  **ppWt = Weight((float)1.);
	 List<Arc>::iterator temp = l++ ;
	 arcs.erase(temp);
      } else {
	hWeights.add(un, &l->weight);
	++l;
      }
    }
  }
  void prune(Weight thresh) {
      List<Arc>::iterator end = arcs.end();      
    for ( List<Arc>::iterator l=arcs.begin() ; l!= end; ){
      if ( l->weight < thresh ){
	List<Arc>::iterator temp = l++ ;
	arcs.erase(temp);
      }
      else
      	++l;
    }
  }
  void renumberDestinations(int *oldToNew) { // negative means remove transition
    Assert(!index);
    List<Arc>::iterator l=arcs.begin();
    while ( l != arcs.end() ) {
      int &dest = (int &)l->dest;
      if ( oldToNew[dest] < 0 ) {
	--size;
	arcs.erase(l++); 
      } else {
	dest = oldToNew[dest];
	++l;
      }
    }
  }
};

ostream& operator << (ostream &out, struct State &s); // Yaser 7-20-2000

struct IOPair {
  int in;
  int out;
  int hash() const
  {
    return (67913 * out + in) * 2654435767U;
  }
};

ostream & operator << (ostream &o, IOPair p);

int operator == (IOPair l, IOPair r);

struct DWPair {
  int dest;
  Arc *arc;
  Weight scratch;
  Weight counts;
  Weight &weight() { return arc->weight; }
};

ostream & operator << (ostream &o, DWPair p);


ostream & hashPrint(HashTable<IOPair, List<DWPair> > &h, ostream &o);

struct symSeq {
  int n;
  int *let;
  int *rLet;
};

ostream & operator << (ostream & out , const symSeq & s);

struct IOSymSeq {
  symSeq i;
  symSeq o;
  float weight;
  void init(List<int> &inSeq, List<int> &outSeq, float w) {
    i.n = inSeq.length();
    o.n = outSeq.length();
    i.let = new int[i.n];
    o.let = new int[o.n];
    i.rLet = new int[i.n];
    o.rLet = new int[o.n];
    int *pi, *rpi;
    pi = i.let;
    rpi = i.rLet + i.n;
    for ( List<int>::const_iterator inL=inSeq.begin() ; inL != inSeq.end() ; ++inL )
      *pi++ = *--rpi = *inL;
    pi = o.let;
    rpi = o.rLet + o.n;
    for ( List<int>::const_iterator outL=outSeq.begin() ; outL != outSeq.end() ; ++outL )
      *pi++ = *--rpi = *outL;
    weight = w;
  }
  void kill() {
    delete[] o.let;
    delete[] o.rLet;
    delete[] i.let;
    delete[] i.rLet;
  }
};

ostream & operator << (ostream & out , const IOSymSeq & s);   // Yaser 7-21-2000

class trainInfo {
 public:
  HashTable<IOPair, List<DWPair> > *forArcs;
  HashTable<IOPair, List<DWPair> > *revArcs;
  List<int> *forETopo;
  List<int> *revETopo;
  Weight ***f;
  Weight ***b;
  int maxIn, maxOut;
  List <IOSymSeq> examples;
  Weight smoothFloor;
  int nStates; // Yaser added this . number of States 
#ifdef N_E_REPS // Yaser : the following variables need to be taken care of in the copy constructor
  Weight *wNew;
  Weight *wOld;
#endif

  trainInfo(){};
  trainInfo(const trainInfo& a){
    if (a.forArcs == NULL)
      forArcs = NULL ;
    else 
      forArcs=new HashTable<IOPair, List<DWPair> >(*a.forArcs);
    revArcs=new HashTable<IOPair, List<DWPair> >(*a.revArcs);
    forETopo=new List<int>(*a.forETopo);
    revETopo=new List<int>(*a.revETopo);
    maxIn=(a.maxIn);
    maxOut=(a.maxOut);
    examples=(a.examples);
    smoothFloor =(a.smoothFloor);
    nStates=a.nStates;
    f = new Weight **[maxIn+1];
    b = new Weight **[maxIn+1];
    for ( int i = 0 ; i <= maxIn ; ++i ) {
      f[i] = new Weight *[maxOut+1];
      b[i] = new Weight *[maxOut+1];
      for ( int o = 0 ; o <= maxOut ; ++o ) {
	f[i][o] = new Weight [nStates];
	b[i][o] = new Weight [nStates];
	for (int s = 0 ; s < nStates ; s++){
	  f[i][o][s] = a.f[i][o][s] ;
	  b[i][o][s] = a.b[i][o][s] ;
	}
      }
    }    
  };
};

#endif
