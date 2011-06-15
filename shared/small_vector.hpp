#ifndef GRAEHL_SHARED___SMALL_VECTOR
#define GRAEHL_SHARED___SMALL_VECTOR

/* REQUIRES that T is POD (can be memcpy).  won't work (yet) due to union with SMALL_VECTOR_POD==0 - may be possible to handle movable types that have ctor/dtor, by using  explicit allocation, ctor/dtor calls.  but for now JUST USE THIS FOR no-meaningful ctor/dtor POD types.

   TODO: need to implement SMALL_VECTOR_POD=0 (will guarantee ctor/dtor calls in that case, incl copy ctor when changing between heap and inline). right now things are assigned, or memcpyd, without ctor/dtor care at all

   stores small element (<=SV_MAX items) vectors inline.  recommend SV_MAX=sizeof(T)/sizeof(T*)>1?sizeof(T)/sizeof(T*):1.  may not work if SV_MAX==0.
 */

#define SMALL_VECTOR_POD 1

#include <algorithm>  // std::max
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <new>
#include <graehl/shared/swap_pod.hpp>
#include <boost/functional/hash.hpp>
#include <graehl/shared/io.hpp>
#include <graehl/shared/genio.h>

//sizeof(T)/sizeof(T*)>1?sizeof(T)/sizeof(T*):1

namespace graehl {

template <class T,int SV_MAX=2>
class small_vector {
//  typedef unsigned short uint16_t;
  void Alloc(size_t s) { // doesn't free old; for ctor. sets size_
    size_=s;
    assert(s <= size_max);
    if (s>SV_MAX) {
      capacity_ = s;
      Alloc_heap();
    }
  }
  void Alloc_heap() {
    data_.ptr=new T[capacity_];  // TODO: replace this with allocator or ::operator new(sizeof(T)*s) everywhere
  }
  void Free_heap() {
    delete[] data_.ptr;
  }
  T *Alloc_heap(size_t s) {
    return new T[s];
  }
  void Free_heap(T *a) {
    delete[] a;
  }

 public:
  typedef small_vector<T,SV_MAX> Self;
  small_vector() : size_(0) {}

  typedef T const* const_iterator;
  typedef T* iterator;
  typedef T value_type;
  typedef T &reference;
  typedef T const& const_reference;
  typedef uint16_t size_type;
  static const size_type size_max=
#ifdef NDEBUG
    (size_type)-1;
#else
  ((size_type)-1)/2;
#endif
  T *begin() { return size_>SV_MAX?data_.ptr:data_.vals; }
  T const* begin() const { return const_cast<Self*>(this)->begin(); }
  T *end() { return begin()+size_; }
  T const* end() const { return begin()+size_; }

  explicit small_vector(size_t s) {
    assert(s<=size_max);
    Alloc(s);
    if (s <= SV_MAX) {
      for (size_type i = 0; i < s; ++i) new(&data_.vals[i]) T();
    } //TODO: if alloc were raw space, construct here.
  }

  small_vector(size_t s, T const& v) {
    assert(s<=size_max);
    Alloc(s);
    if (s <= SV_MAX) {
      for (size_type i = 0; i < s; ++i) data_.vals[i] = v;
    } else {
      for (size_type i = 0; i < size_; ++i) data_.ptr[i] = v;
    }
  }

  //TODO: figure out iterator traits to allow this to be selcted for any iterator range.
  template <class I>
  small_vector(I const* begin,I const* end) {
    int s=end-begin;
    assert(s<=size_max);
    Alloc(s);
    if (s <= SV_MAX) {
      for (size_type i = 0; i < s; ++i,++begin) data_.vals[i] = *begin;
    } else
      for (size_type i = 0; i < s; ++i,++begin) data_.ptr[i] = *begin;
  }
  small_vector(T const* i,T const* end) {
    int s=end-i;
    assert(s<=size_max);
    Alloc(s);
    std::memcpy(this->begin(),i,s*sizeof(T));
  }

  small_vector(const Self& o) : size_(o.size_) {
    if (size_ <= SV_MAX) {
      std::memcpy(data_.vals,o.data_.vals,size_*sizeof(T));
//      for (int i = 0; i < size_; ++i) data_.vals[i] = o.data_.vals[i];
    } else {
      capacity_ = size_;
      Alloc_heap();
      memcpy_heap(o.data_.ptr);
    }
  }

  //TODO: test.  this invalidates more iterators than std::vector since resize may move from ptr to vals.
  T *erase(T *b) {
    return erase(b,b+1);
  }
  T *erase(T *b,T* e) {
    T *tb=begin(),*te=end();
    int nbefore=b-tb;
    if (e==te) {
      resize(nbefore);
    } else {
      int nafter=te-e;
      std::memmove(b,e,nafter*sizeof(T));
      resize(nbefore+nafter);
    }
    return begin()+nbefore;
  }
  void memcpy_heap(T *p) {
    assert(size_<=size_max);
    assert(capacity_>=size_);
    std::memcpy(data_.ptr,p,size_*sizeof(T));
  }
  const Self& operator=(const Self& o) {
    if (size_ <= SV_MAX) {
      if (o.size_ <= SV_MAX) {
        size_ = o.size_;
        for (int i = 0; i < SV_MAX; ++i) data_.vals[i] = o.data_.vals[i];
      } else {
        capacity_ = size_ = o.size_;
        Alloc_heap();
        memcpy_heap(o.data_.ptr);
      }
    } else {
      if (o.size_ <= SV_MAX) {
        Free_heap();
        size_ = o.size_;
        for (int i = 0; i < size_; ++i) data_.vals[i] = o.data_.vals[i];
      } else {
        if (capacity_ < o.size_) {
          Free_heap();
          capacity_ = o.size_;
          Alloc_heap();
        }
        size_ = o.size_;
        memcpy_heap(o.data_ptr);
        for (int i = 0; i < size_; ++i)
          data_.ptr[i] = o.data_.ptr[i];
      }
    }
    return *this;
  }

  ~small_vector() {
    if (size_ <= SV_MAX) {
      // skip if pod?  yes, we required pod anyway.  no need to destruct
    } else
      Free_heap();
  }

  void clear() {
    if (size_ > SV_MAX)
      Free_heap();
    size_ = 0;
  }

  bool empty() const { return size_ == 0; }
  size_t size() const { return size_; }

  inline void reserve(size_type sz) {
    if (size_<=SV_MAX) return;
    ensure_capacity(sz);
  }

  inline void ensure_capacity(size_type min_size) {
    assert(min_size > SV_MAX);
    if (min_size < capacity_) return;
    size_type new_cap = std::max(static_cast<size_type>(capacity_ << 1), min_size);
    T* tmp = Alloc_heap(new_cap);
    std::memcpy(tmp, data_.ptr, capacity_ * sizeof(T));
    Free_heap();
    data_.ptr = tmp;
    capacity_ = new_cap;
  }

private:
  inline void copy_vals_to_ptr() {
    capacity_ = SV_MAX * 2;
    T* tmp = new T[capacity_];
    for (int i = 0; i < SV_MAX; ++i) tmp[i] = data_.vals[i];
    data_.ptr = tmp;
  }
  inline void ptr_to_small() {
    assert(size_<=SV_MAX);
    T *tmp=data_.ptr; // should be no problem with memory access order (strict aliasing) because of safe access through union
    for (size_type i=0;i<size_;++i) // no need to memcpy for small size
      data_.vals[i]=tmp[i];
    Free_heap(tmp);
  }

public:

  template <class I>
  inline void append(I i,I end) {
    for (;i!=end;++i)
      push_back(*i);
  }

  template <class I>
  inline void append_ra(I i,I end) {
    reserve(size_+(end-i));
  }

  inline void push_back(T const& v) {
    if (size_ < SV_MAX) {
      data_.vals[size_] = v;
      ++size_;
      return;
    } else if (size_ == SV_MAX) {
      copy_vals_to_ptr();
    } else if (size_ == capacity_) {
      ensure_capacity(size_ + 1);
    }
    data_.ptr[size_] = v;
    ++size_;
  }

  T& back() { return this->operator[](size_ - 1); }
  const T& back() const { return this->operator[](size_ - 1); }
  T& front() { return this->operator[](0); }
  const T& front() const { return this->operator[](0); }

  void pop_back() {
    assert(size_>0);
    --size_;
    if (size_==SV_MAX)
      ptr_to_small();
  }

  void compact() {
    compact(size_);
  }

  // size must be <= size_ - TODO: test
  void compact(size_type newsz) {
    assert(newsz<=size_);
    if (size_>SV_MAX) { // was heap
      size_=newsz;
      if (newsz<=SV_MAX) // now small
        ptr_to_small();
    } else // was small already
      size_=newsz;
  }

  void resize(size_t s, int v = 0) {
    assert(s<=size_max);
    assert(size_<=size_max);
    if (s <= SV_MAX) {
      if (size_ > SV_MAX) {
        T *tmp=data_.ptr;
        for (size_type i = 0; i < s; ++i) data_.vals[i] = tmp[i];
        delete[] tmp;
        size_ = s;
        return;
      }
      if (s <= size_) {
        size_ = s;
        return;
      } else {
        for (size_type i = size_; i < s; ++i)
          data_.vals[i] = v;
        size_ = s;
        return;
      }
    } else {
      if (size_ <= SV_MAX)
        copy_vals_to_ptr();
      if (s > capacity_)
        ensure_capacity(s);
      if (s > size_) {
        for (size_type i = size_; i < s; ++i)
          data_.ptr[i] = v;
      }
      size_ = s;
    }
  }

  T& operator[](size_t i) {
    if (size_ <= SV_MAX) return data_.vals[i];
    return data_.ptr[i];
  }

  const T& operator[](size_t i) const {
    if (size_ <= SV_MAX) return data_.vals[i];
    return data_.ptr[i];
  }

  bool operator==(const Self& o) const {
    if (size_ != o.size_) return false;
    if (size_ <= SV_MAX) {
      for (size_t i = 0; i < size_; ++i)
        if (data_.vals[i] != o.data_.vals[i]) return false;
      return true;
    } else {
      for (size_t i = 0; i < size_; ++i)
        if (data_.ptr[i] != o.data_.ptr[i]) return false;
      return true;
    }
  }

  friend bool operator!=(const Self& a, const Self& b) {
    return !(a==b);
  }

  void swap(Self& o) {
    swap_pod(*this,o);
  }

  inline std::size_t hash_impl() const {
    using namespace boost;
    if (size_==0) return 0;
    if (size_==1) return hash_value(data_.vals[0]);
    if (size_ <= SV_MAX)
      return hash_range(data_.vals,data_.vals+size_);
    return hash_range(data_.ptr,data_.ptr+size_);
  }
//template <class O> friend inline O& operator<<(O &o,small_vector const& x) { return o; }
  typedef void has_print_writer;
  template <class charT, class Traits>
  std::basic_ostream<charT,Traits>& print(std::basic_ostream<charT,Traits>& o,bool multiline=false) const
  {
    return range_print(o,begin(),end(),DefaultWriter(),multiline);
  }
  template <class charT, class Traits, class Writer >
  std::basic_ostream<charT,Traits>& print(std::basic_ostream<charT,Traits>& o,Writer w,bool multiline=false) const
  {
    return range_print(o,begin(),end(),w,multiline);
  }
  typedef small_vector<T,SV_MAX> self_type;
  TO_OSTREAM_PRINT

 private:
  union StorageType { // should be safe from strict-aliasing optimizations
    T vals[SV_MAX];
    T* ptr;
  };
  StorageType data_;
  size_type size_;
  size_type capacity_;  // only defined when size_ > SV_MAX
};

template <class T,int M>
inline std::size_t hash_value(small_vector<T,M> const& x) {
  return x.hash_impl();
}

template <class T,int M>
inline void swap(small_vector<T,M> &a,small_vector<T,M> &b) {
  a.swap(b);
}

template <class T,int M>
void memcpy(void *out,small_vector<T,M> const& v) {
  std::memcpy(out,v.begin(),v.size()*sizeof(T));
}

}//ns

#if defined(TEST) && 0
#include <graehl/shared/test.hpp>

typedef graehl::small_vector<int,2> SmallVectorInt;

BOOST_AUTO_TEST_CASE( test_small_vector_larger_than_2 ) {
  using namespace std;
  SmallVectorInt v;
  SmallVectorInt v2;
  v.push_back(0);
  v.push_back(1);
  v.push_back(2);
  assert(v.size() == 3);
  assert(v[2] == 2);
  assert(v[1] == 1);
  assert(v[0] == 0);
  v2 = v;
  SmallVectorInt copy(v);
  assert(copy.size() == 3);
  assert(copy[0] == 0);
  assert(copy[1] == 1);
  assert(copy[2] == 2);
  assert(copy == v2);
  copy[1] = 99;
  assert(copy != v2);
  assert(v2.size() == 3);
  assert(v2[2] == 2);
  assert(v2[1] == 1);
  assert(v2[0] == 0);
  v2[0] = -2;
  v2[1] = -1;
  v2[2] = 0;
  assert(v2[2] == 0);
  assert(v2[1] == -1);
  assert(v2[0] == -2);
  SmallVectorInt v3(1,1);
  assert(v3[0] == 1);
  v2 = v3;
  assert(v2.size() == 1);
  assert(v2[0] == 1);
  SmallVectorInt v4(10, 1);
  assert(v4.size() == 10);
  assert(v4[5] == 1);
  assert(v4[9] == 1);
  v4 = v;
  assert(v4.size() == 3);
  assert(v4[2] == 2);
  assert(v4[1] == 1);
  assert(v4[0] == 0);
  SmallVectorInt v5(10, 2);
  assert(v5.size() == 10);
  assert(v5[7] == 2);
  assert(v5[0] == 2);
  assert(v.size() == 3);
  v = v5;
  assert(v.size() == 10);
  assert(v[2] == 2);
  assert(v[9] == 2);
  SmallVectorInt cc;
  for (int i = 0; i < 33; ++i)
    cc.push_back(i);
  for (int i = 0; i < 33; ++i)
    assert(cc[i] == i);
  cc.resize(20);
  assert(cc.size() == 20);
  for (int i = 0; i < 20; ++i)
    assert(cc[i] == i);
  cc[0]=-1;
  cc.resize(1, 999);
  assert(cc.size() == 1);
  assert(cc[0] == -1);
  cc.resize(99, 99);
  for (int i = 1; i < 99; ++i) {
    //cerr << i << " " << cc[i] << endl;
    assert(cc[i] == 99);
  }
  cc.clear();
  assert(cc.size() == 0);
}


BOOST_AUTO_TEST_CASE( test_small_vector_small )
{
  using namespace std;
  SmallVectorInt v;
  SmallVectorInt v1(1,0);
  SmallVectorInt v2(2,10);
  SmallVectorInt v1a(2,0);
  EXPECT_TRUE(v1 != v1a);
  EXPECT_TRUE(v1 == v1);
  EXPECT_EQ(v1[0], 0);
  EXPECT_EQ(v2[1], 10);
  EXPECT_EQ(v2[0], 10);
  ++v2[1];
  --v2[0];
  EXPECT_EQ(v2[0], 9);
  EXPECT_EQ(v2[1], 11);
  SmallVectorInt v3(v2);
  assert(v3[0] == 9);
  assert(v3[1] == 11);
  assert(!v3.empty());
  assert(v3.size() == 2);
  v3.clear();
  assert(v3.empty());
  assert(v3.size() == 0);
  assert(v3 != v2);
  assert(v2 != v3);
  v3 = v2;
  assert(v3 == v2);
  assert(v2 == v3);
  assert(v3[0] == 9);
  assert(v3[1] == 11);
  assert(!v3.empty());
  assert(v3.size() == 2);
  //cerr << sizeof(SmallVectorInt) << endl;
  //cerr << sizeof(vector<int>) << endl;
}
#endif

#endif
