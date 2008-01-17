#ifndef CARMEL_CONFIG_H
#define CARMEL_CONFIG_H

#define USE_GRAEHL_HASH_MAP
// with stdext::hash_map, copies may be made of values (not tested lately)

#ifdef _MSC_VER
#ifndef USE_GRAEHL_HASH_MAP
#define USE_GRAEHL_HASH_MAP
#endif
#endif

// use singly linked list - recommended (less space) - but FIXME: double free!
#define USE_SLIST

#ifndef SINGLE_PRECISION
#define DOUBLE_PRECISION
#endif

//#define CARMEL_DEBUG_PRINTS

#if defined(DEBUG) && defined(CARMEL_DEBUG_PRINTS)
//# define DEBUG_STRINGPOOL
#define DEBUGLEAK
#define DEBUG_ESTIMATE_PP
#define DEBUGNAN
//#define DEBUGTRAIN
//#define DEBUGTRAINDETAIL
#define DEBUGNORMALIZE
#define DEBUGKBEST
#define DEBUG_RANDOM_GENERATE
#define DEBUGPRUNE
//#define DEBUGFB
#define DEBUGCOMPOSE
#define DEBUG_ADAPTIVE_EM
#define ALLOWED_FORWARD_OVER_BACKWARD_EPSILON 1e-3
#endif

#ifdef DEBUG
//#define DEBUGNORMALIZE
#endif

//#define DEBUGNAN


#ifdef DEBUGNORMALIZE
#define CHECKNORMALIZE
#endif


#include <iostream>
namespace Config {
  inline std::ostream &err() {
    return std::cerr;
  }
  inline std::ostream &message() {
    return std::cerr;
  }
  inline std::ostream &log() {
    return std::cerr;
  }
  inline std::ostream &debug() {
    return std::cerr;
  }
  inline std::ostream &warn() {
    return std::cerr;
  }
}

#ifndef FLOAT_TYPE
#ifdef DOUBLE_PRECISION
# define FLOAT_TYPE double
#else
# define FLOAT_TYPE float
#endif
#endif

#include <graehl/shared/memleak.hpp>

#define TREE_SINGLETON_OPT
typedef short unsigned var_type; // 0 = no var, 1,2,3,... = var index
typedef short rank_type; // (rank=#children) -1 = any rank, -2 = any tree ... (can't be unsigned type)

#define COPYRIGHT_YEAR 2007

//do this in Makefile for consistency with boost test lib src that don't include me:
//#define BOOST_DISABLE_THREADS
//#define BOOST_NO_MT


//#define UNORDERED_MAP

#define STATIC_HASH_EQUAL
#define STATIC_HASHER

// if not STRINGPOOL, then same string -> different address (but no global hashtable needed)
#if !defined(DEBUG)
#define STRINGPOOL
#endif
#ifndef STRINGPOOLCLASS
#define STRINGPOOLCLASS StringPool
#endif

// reference counts of alphabet symbols/state names - might save a little memory and could hurt or help performance





// use old, slower string hash
//#define OLD_HASH

// for meaningful compose state names
#define MAX_STATENAME_LEN 15000

#ifdef _MSC_VER
#pragma warning( disable : 4355 )
#endif


#define MAX_LEARNING_RATE_EXP 20

#define WEIGHT_FLOAT_TYPE FLOAT_TYPE
// unless defined, Weight(0) will may give bad results when computed with, depending on math library behavior
#define WEIGHT_CORRECT_ZERO
// however, carmel checks for zero weight before multiplying in a bad way.  if you get #INDETERMINATE results, define this
// definitely needs to be defined for Microsoft (debug or release) now

// allows WFST to be indexed in either direction?  not recommended. or tested lately.
//#define BIDIRECTIONAL


#endif //guard
