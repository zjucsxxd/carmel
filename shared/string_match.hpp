#ifndef GRAEHL__SHARED__STRING_MATCH_HPP
#define GRAEHL__SHARED__STRING_MATCH_HPP

#include <graehl/shared/function_macro.hpp>
#include <string>
#include <iterator>

//#define TOKENIZE_KEY_VAL_DEBUG

#ifdef TOKENIZE_KEY_VAL_DEBUG
# include <graehl/shared/debugprint.hpp>
# define TOKENIZE_KEY_VAL_IF_DBG(a) a;
#else
# define TOKENIZE_KEY_VAL_IF_DBG(a)
#endif

namespace graehl {

// returns true and writes pos,n for substring between left-right brackets.  or false if brackets not found.
template <class Str,class size_type> inline
bool
substring_inside_pos_n(const Str &s,const Str &leftbracket,const Str &rightbracket,size_type &pos,size_type &n)
{
    size_type rightpos;
    const size_type npos=Str::npos;
    if (npos==(pos=s.find(leftbracket,0))) return false;
    pos+=leftbracket.length();
    if (npos==(rightpos=s.find(rightbracket,pos))) return false;
    n=rightpos-pos;
    return true;
}

// first is first substring (left->right) between leftbracket and rightbracket in s.
// second is true if found, false if none found
template <class Str> inline
std::pair <Str,bool>
substring_inside(const Str &s,const Str &leftbracket,const Str &rightbracket)
{
    typedef std::pair <Str,bool> Ret;
    typename Str::size_type pos,n;
    if (substring_inside_pos_n(s,leftbracket,rightbracket,pos,n))
        return Ret(Str(s,pos,n),true);
    else
        return Ret(Str(),false);
}

// parse both streams as a sequence of ParseAs, comparing for equality
template <class ParseAs,class Istream> inline
bool
equal_streams_as_seq(Istream &i1,Istream &i2)
{
    /* could almost write as istream_iterator<ParseAs>, std::equal - except that
     doesn't check both iterators' end
    */
    ParseAs v1,v2;
    for (;;) {
        bool got1=i1>>v1;
        bool got2=i2>>v2;
        if (got1) {
            if (!got2) return false; //2 ended first
        } else {
            if (!got2) return true; // both ended together
            return false; // 1 ended first
        }
        if (!(v1==v2)) return false; // value mismatch
    }
    //unreachable!
    assert(0);
}

template <class ParseAs,class Ch,class Tr> inline
bool
equal_strings_as_seq(const std::basic_string<Ch,Tr> &s1,const std::basic_string<Ch,Tr> &s2)
{
    std::basic_stringstream<Ch,Tr> i1(s1),i2(s2);
    return equal_streams_as_seq<ParseAs>(i1,i2);
}

//std::equal can only be called if sequences are same length!
template <class I1,class I2,class Equal> inline
bool equal_safe(I1 b1,I1 e1,I2 b2,I2 e2,Equal eq)
{
    while (b1 != e1) {
        if (b2 == e2) return false;
        if (*b2++ != *e2++)
            return false;
    }
    // now b1 == e1
    return b2==e2;
}

template <class I1,class I2> inline
bool equal_safe(I1 b1,I1 e1,I2 b2,I2 e2)
{
    return equal_safe(b1,e1,b2,e2,equal_typeless());
}

//oops: didn't notice that I'd already implemented this before starts_with.  two implementations for testing anyway ;)
template <class Istr, class Isubstr> inline
bool match_begin(Istr bstr,Istr estr,Isubstr bsub,Isubstr esub) 
{
    while (bsub != esub) {
        if (bstr == estr)
            return false;
        if (*bsub++ != *bstr++)
            return false;
    }
    return true;
}

template <class Istr, class Isubstr> inline
bool match_end(Istr bstr,Istr estr,Isubstr bsub,Isubstr esub) 
{
    while (bsub != esub) {
        if (bstr == estr)
            return false;
        if (*--esub != *--estr)
            return false;
    }
    return true;
}

template <class It1,class It2,class Pred> inline
bool starts_with(It1 str,It1 str_end,It2 prefix,It2 prefix_end,Pred equals)
{
    for(;;) {
        if (prefix==prefix_end) return true;
        if (str==str_end) return false;
        if (!equals(*prefix,*str)) return false;
        ++prefix;++str;
    }
    //unreachable
    assert(0);
}

template <class It1,class It2> inline
bool starts_with(It1 str,It1 str_end,It2 prefix,It2 prefix_end)
{
    return starts_with(str,str_end,prefix,prefix_end,equal_typeless());
}


//FIXME: provide skip-first-whitespace or skip-no-whitespace iterators.
template <class Ch,class Tr,class CharIt> inline
bool expect_consuming(std::basic_istream<Ch,Tr> &i,CharIt begin,CharIt end) 
{
    typedef std::istream_iterator<Ch> II;
    II ibeg(i),iend;
    return match_begin(ibeg,iend,begin,end);
}

template <class Ch,class Tr,class CharIt> inline
bool expect_consuming(std::basic_istream<Ch,Tr> &i,CharIt begin,CharIt end,bool skip_first_ws=true) 
{
    if (begin==end) return true;
    Ch c;
    if (skip_first_ws)
        i>>c;
    else
        i.get(c);
    if (!i) return false;
    while (begin!=end) {
        if (!i.get(c))
            return false;
        if (c!=*begin)
            return false;
    }
    return true;  
/* //NOTE: whitespace will be ignored!  so don't include space in expectation ...    
    typedef std::istream_iterator<Ch> II;
    II ibeg(i),iend;
    return match_begin(ibeg,iend,begin,end);
*/
}

template <class Ch,class Tr,class Str> inline
bool expect_consuming(std::basic_istream<Ch,Tr> &i,const Str &str,bool skip_first_ws=true)
{
    return expect_consuming(i,str.begin(),str.end(),skip_first_ws);
}


template <class Str>
inline
bool starts_with(const Str &str,const Str &prefix) 
{
    return starts_with(str.begin(),str.end(),prefix.begin(),prefix.end());
}

template <class Str>
inline
bool ends_with(const Str &str,const Str &suffix) 
{
//        return starts_with(str.rbegin(),str.rend(),suffix.rbegin(),suffix.rend());
        return match_end(str.begin(),str.end(),suffix.begin(),suffix.end());

}

template <class Str>
inline
bool starts_with(const Str &str,char *prefix) 
{
    return starts_with(str,std::string(prefix));
}

template <class Str>
inline
bool ends_with(const Str &str,char *suffix) 
{
    return ends_with(str,std::string(suffix));
}


#if 0
inline
bool starts_with(const std::string &str,const std::string &prefix) 
{
    return match_begin(str.begin(),str.end(),prefix.begin(),prefix.end());
//    return (str.find(prefix)==0);
}

inline
bool ends_with(const std::string &str,const std::string &suffix) 
{
    return match_end(str.begin(),str.end(),suffix.begin(),suffix.end());
//        return starts_with(str.rbegin(),str.rend(),suffix.rbegin(),suffix.rend());
/*
  const std::string::size_type slen=str.length();
    return (str.rfind(suffix)==len-suffix.length());
*/    
}
#endif 


// func(const Func::argument_type &val) - assumes val can be parsed from string tokenization (no whitespace)
template <class In,class Func> inline
void parse_until(const std::string &term,In &in,Func func)
{
    std::string s;
    bool last=false;
    while(!last && (in>>s) ) {
        if (!term.empty() && ends_with(s,term)) {
            last=true;
            erase_end(s,term.length());
        }
        if (s.empty())
            break;
        typename Func::argument_type val;
        string_into(s,val);
        func(val);
    }
};


template <class Str,class Data> inline
void string_into(const Str &str,Data &data) 
{
    std::istringstream i(str);
    if (!(i>>data))
        throw std::runtime_error("Couldn't convert (string_into): "+str);
}


template <class Data,class Str> inline
Data string_to(const Str &str)
{
    Data ret;
    string_into(str,ret);
    return ret;
}
/*

template <class Str,class Data,class size_type> inline
void substring_into(const Str &str,size_type pos,size_type n,Data &data) 
{
//    std::istringstream i(str,pos,n); // doesn't exist!
    std::istringstream i(str.substr(pos,n));
    if (!(i>>*data))
        throw std::runtime_error("Couldn't convert (string_into): "+str);
}

template <class Data,class Str,class size_type> inline
Data string_to(const Str &str,size_type pos,size_type n)
{
    Data ret;
    substring_into(str,pos,n,ret);
    return ret;
}

*/

template <class Cont>
struct push_backer
{
    Cont *cont;
    typedef void result_type;
    typedef typename Cont::value_type argument_type;
    push_backer(Cont &container) : cont(&container) {}
    template <class V>
    void operator()(const V&v) const
    {
        cont->push_back(v);
    }
    void operator()() const
    {
        cont->push_back(argument_type());
    }
};

template <class Cont> inline
push_backer<Cont> make_push_backer(Cont &container) 
{
    return push_backer<Cont>(container);
}

template <class Str> inline
void erase_begin(Str &s,unsigned n) 
{
    s.erase(0,n);
}

template <class Str> inline
void erase_end(Str &s,unsigned n) 
{
    s.erase(s.length()-n,n);
}

template <class In,class Cont> inline
void push_back_until(const std::string &term,In &in,Cont & cont) 
{
    parse_until(term,in,make_push_backer(cont));
}

template <class F>
void tokenize_key_val_pairs(const std::string &s, F &f,char pair_sep=',',char key_val_sep=':') 
{
    typedef typename F::key_type Key;
    typedef typename F::data_type Data;
    using namespace std;    
    typedef pair<Key,Data> Component;
    typedef pair<Key,Data> Component;
    typedef string::size_type Pos;
    typedef string::const_iterator It;
    Component to_add;
    for (It i=s.begin(),e=s.end();;) {
        for(It key_beg=i; ;++i) {
            if (i==e) return;
            TOKENIZE_KEY_VAL_IF_DBG(DBP2(*i,i-s.begin()));
            if (*i == key_val_sep) { // [last,i) is key
                TOKENIZE_KEY_VAL_IF_DBG(DBPC2("key",string(key_beg,i)));
                string_into(string(key_beg,i),to_add.first);
                break; // done key, expect val
            }
        }
        for (It val_beg=++i; ;++i) {
            TOKENIZE_KEY_VAL_IF_DBG(
                if (i==e) DBPC2("<END>",i-s.begin());                    
                DBP2(*i,i-s.begin());
                );
            if (i == e || *i == pair_sep) {
                TOKENIZE_KEY_VAL_IF_DBG(DBPC2("val",string(val_beg,i)));
                string_into(string(val_beg,i),to_add.second);
                f(to_add);
                if (i==e) return;
                ++i;
                break; // next key/val
            }
        }
    }
}

#ifdef TEST
#include <graehl/shared/test.hpp>
#include <cctype>
#include <graehl/shared/debugprint.hpp>
#endif

#ifdef TEST
const char *TEST_starts_with[]={
    "s",
    "st",
    "str",
    "str1"
};

const char *TEST_ends_with[]={
    "1",
    "r1",
    "tr1",
    "str1"
};
// NOTE: could use substring but that's more bug-prone ;D

BOOST_AUTO_UNIT_TEST( TEST_FUNCS )
{
    using namespace std;
    string s1("str1"),emptystr;
    BOOST_CHECK(starts_with(s1,emptystr));
    BOOST_CHECK(starts_with(emptystr,emptystr));
    BOOST_CHECK(ends_with(s1,emptystr));
    BOOST_CHECK(ends_with(emptystr,emptystr));
    BOOST_CHECK(!starts_with(s1,string("str11")));
    BOOST_CHECK(!ends_with(s1,string("sstr1")));
    BOOST_CHECK(!starts_with(s1,string("str*")));
    BOOST_CHECK(!ends_with(s1,string("*tr1")));
    BOOST_CHECK(!ends_with(s1,string("str*")));
    BOOST_CHECK(!starts_with(s1,string("*tr1")));
    for (unsigned i=0;i<4;++i) {
        string starts(TEST_starts_with[i]),ends(TEST_ends_with[i]);        
        BOOST_CHECK(starts_with(s1,starts));
        BOOST_CHECK(ends_with(s1,ends));
        BOOST_CHECK(match_begin(s1.begin(),s1.end(),starts.begin(),starts.end()));
        BOOST_CHECK(match_end(s1.begin(),s1.end(),ends.begin(),ends.end()));
        if (i!=3) {            
            BOOST_CHECK(!starts_with(s1,ends));
            BOOST_CHECK(!ends_with(s1,starts));        
            BOOST_CHECK(!match_end(s1.begin(),s1.end(),starts.begin(),starts.end()));
            BOOST_CHECK(!match_begin(s1.begin(),s1.end(),ends.begin(),ends.end()));
        }        
    }
    string s2(" s t  r1");
    BOOST_CHECK(equal_strings_as_seq<char>(s1,s2));
    BOOST_CHECK(!equal_strings_as_seq<string>(s1,s2));
    string s3(" s \nt  \tr1 ");
    BOOST_CHECK(equal_strings_as_seq<char>(s2,s3));
    BOOST_CHECK(equal_strings_as_seq<string>(s2,s3));
    string s4("str1a");
    BOOST_CHECK(!equal_strings_as_seq<string>(s1,s4));
    BOOST_CHECK(!equal_strings_as_seq<char>(s1,s4));
    BOOST_CHECK(!equal_strings_as_seq<char>(s4,s1));    
}

#endif

} //graehl

#endif