### from graehl import *
"""TODO:
 figure out python logging lib
"""

import sys,re,random,math,os,collections,subprocess

#from itertools import izip
from itertools import *

def out_dict(d,out=sys.stdout):
    out_tabsep(d.iteritems() if type(d)==dict else d)

def out_tabsep(pairs,out=sys.stdout):
    if type(out)==str: out=open(out,'w')
    for kv in pairs:
        out.write('\t'.join(map(str,kv))+'\n')

def callv(l):
    #subprocess.check_call(l)
    os.system(' '.join(l))

def is_iter(x):
    try: it = iter(x)
    except TypeError: return False
    return True

def is_nonstring_iter(x):
    return False if type(x)==str else is_iter(x)

def intern_tuple(seq):
    return tuple(intern(x) for x in seq)

def identity(x):
    return x

def firstn(l,head=None):
    return l if head is None else l[0:head]

def head_sorted(l,key=identity,reverse=False,head=None):
    return firstn(sorted(l,key=key,reverse=reverse),head=head)

def head_str(l,head=None):
    return ('All[\n' if head is None else 'First-%d\n'%head)+'\n'.join(map(str,l))+'\n'

def head_sorted_str(l,key=identity,reverse=False,head=None):
    return head_str(head_sorted(l,key=key,reverse=reverse,head=head),head=head)
#    return ('All[\n' if head is None else 'First-%d\n'%head)+'\n'.join(map(str,head_sorted(l,key=key,reverse=reverse,head=head)))+'\n'

def head_sorted_dict_val(d,reverse=False,key=identity,head=None):
    return d.iteritems() if head is None else head_sorted(d.iteritems(),reverse=reverse,key=lambda x:key(x[1]),head=head)

def head_sorted_dict_val_str(d,reverse=False,key=identity,head=None):
    return head_str(head_sorted_dict_val(d,reverse=reverse,key=key,head=head),head=head)

class RDict(dict):
    """perl's autovivification for dict of dict of ..."""
    def __getitem__(self, item):
        try:
            return dict.__getitem__(self, item)
        except KeyError:
            value = self[item] = type(self)()
            return value

class IntDict(collections.defaultdict):
    def __init__(self,*a,**kw):
        collections.defaultdict.__init__(self,int,*a,**kw)

class FloatDict(collections.defaultdict):
    def __init__(self,*a,**kw):
        collections.defaultdict.__init__(self,float,*a,**kw)


class Histogram(object):
    """dict counting occurrences of keys; will provide sorted list of (key,count) or output to gnuplot format"""
    def __init__(self):
        self.c=dict()
        self.N=0.0 # total number of counts
    def count(self,k,d=1):
        "by setting d to some arbitrary number, you can have any (x,y) to be graphed as a histogram or line"
        self.N+=d
        c=self.c
        if k in c:
            c[k]+=d
        else:
            c[k]=d
        assert(c[k]>=0)
        return c[k]
    def prob(self,k):
        return self.c.get(k,0.)/self.N
    def get_sorted(self):
        "return sorted list of (key,count)"
        return [(k,self.c[k]) for k in sorted(self.c.iterkeys())]
    def get_binned(self,binwidth=10):
        "assuming numeric keys, return sorted list ((min,max),count) grouped by [min,min+binwidth),[min+binwidth,min+binwidth*2) etc."
        assert False
    def text_gnuplot(self,binwidth=None,line=False):
        "returns a gnuplot program that uses bars if line is False, and bins if binwidth isn't None"
        assert binwidth is None
        assert False
    def __str__(self):
        return str(self.get_sorted())

class LengthDistribution(object):
    def __init__(self,geometric_backoff=1.0):
        self.hist=Histogram()
        self.sum_length=0 # for computing mean length
        #note: shifted geometric distribution - if p(STOP)=p, then E_p[length]=1/p. this is how we will fit the geometric backoff.
        self.bo=geometric_backoff
    def count(self,length,d=1):
        self.sum_length+=d*length
        self.hist.count(length,d)
    def fit_geometric(shifted=True): #
        "return p so that the mean value is the same as histogram - https://secure.wikimedia.org/wikipedia/en/wiki/Geometric_distribution - shifted means support is 1,2,... = avg. number of trials until success. unshifted means 0,... = avg. number of failures before success"
        return len(self.c)/self.sum

def mismatch_text(xs,ys,xname='xs',yname='ys',pre='mismatch: '):
    if xs==ys:
        return ''
    i=first_mismatch(xs,ys)
    if i<len(xs):
        if i<len(ys):
            return '%s%s[%d]!=%s[%d] ( %s != %s )'%(pre,xname,i,yname,i,xs[i],ys[i])
        else:
            return '%s%s longer than %s'%(pre,xname,yname)
    else:
        return '%s%s shorter than %s'%(pre,xname,yname)

def first_mismatch(xs,ys):
    "return first index where xs[i]!=ys[i], i.e. i may be len(xs) or len(ys) if no mismatch"
    l=min(len(xs),len(ys))
    for i in range(0,l):
        if xs[i]!=ys[i]:
            return i
    return l

def mapdictv(d,f):
    "copy of dict d but with keys v replaced by f(v), or deleted if None"
    d2=dict()
    for k,v in d.iteritems():
        v2=f(v)
        if v2 is not None:
            d2[k]=v2
    return d2

log_zero=-1e10
n_zeroprobs=0

epsilon=1e-5
neg_epsilon=-epsilon

def approx_zero(x,epsilon=epsilon):
    return abs(x)<=epsilon

# see knuth for something better, probably.  epsilon should be half of the allowable relative difference.
def approx_equal(x,a,epsilon=epsilon):
    return abs(x-a) <= (abs(x)+abs(a))*epsilon

def approx_leq(x,a,epsilon=epsilon):
    return x <= a+epsilon*(abs(x)+abs(a))

def approx_geq(x,a,epsilon=epsilon):
    return x+epsilon*(abs(x)+abs(a)) >= a

def definitely_gt(x,a,epsilon=epsilon):
    return x > a+epsilon*(abs(x)+abs(a))

def definitely_lt(x,a,epsilon=epsilon):
    return x+epsilon*(abs(x)+abs(a)) < a


# scala-like itertools extensions from 9.7.2. itertools Recipes

def take(n,gen):
    i=0
    for x in gen:
        if i>=n: break
        yield x
        i+=1

def drop(n,gen):
    i=0
    for x in gen:
        if i<n:
            i+=1
            continue
        yield x

def ireduce(func, iterable, init=None):
    if init is None:
        iterable = iter(iterable)
        curr = iterable.next()
    else:
        curr = init
    for x in iterable:
        curr = func(curr, x)
        yield curr

def default_generator(gen,default=None):
    'wrap gen - if gen is None, then yield (forever) None'
    if gen is None:
        while True: yield default
    else:
        for x in gen: yield x

def except_str():
    return sys.exc_type+':'+sys.exc_value

def close_file(f):
    if f is not sys.stdin and f is not sys.stderr and f is not sys.stdout:
        f.close()

def open_default_line(file,default=None):
    'return generator: either file lines if it can be opened, or infinitely many default'
    f=None
    try:
        f=open(file)
    except:
        while True: yield default
    for x in f: yield x

def tryelse(f,default=None):
    try:
        r=f()
    except:
        return default
    return r

def open_except(f,default=None):
    return tryelse(lambda:open(f),default)

def open_first(*fs):
    for f in fs:
        if f:
            try:
                r=open(f)
            except:
                continue
            return r
    return None

def interpolate(a,b,frac_b):
    f=float(frac_b)
    return b*f+a*(1.-f)

def anneal_temp(i,ni,t0,tf):
    return interpolate(t0,tf,1 if ni<=1 else float(i)/(ni-1.))

def anneal_power(i,ni,t0,tf):
    return 1./anneal_temp(i,ni,t0,tf)

def log_prob(p):
    global n_zeroprobs
    if p==0:
        n_zeroprobs+=1
        return log_zero
    return math.log(p)

n_warn=0
warncount=IntDict()

def warn(msg,post=None,pre="WARNING: ",max=None):
    global n_warn,warncount
    n_warn+=1
    p='\n'
    if post is not None:
        p=' '+str(post)+p
    warncount[msg]+=1
    if max is not None and warncount[msg]<=max:
        lastw=max is not None and warncount[msg]==max
        sys.stderr.write(pre+str(msg)+(" (max=%s shown; no more) "%max if lastw else "")+p)

def warncount_sorted():
    w=[(c,k) for (k,c) in warncount.iteritems()]
    return sorted(w,reverse=True)

def info_summary(pre='',warn_details=True):
    if n_warn and warn_details:
        log('N\twarning')
        for w in warncount_sorted():
            log("%s\t%s"%w)
    log("%s%d total warnings"%(pre,n_warn))

def report_zeroprobs():
    "print (and return) any zero probs since last call"
    global n_zeroprobs
    if n_zeroprobs>0:
        warn("encountered %d zeroprobs, used log(0)=%g"%(n_zeroprobs,log_zero))
    n=n_zeroprobs
    n_zeroprobs=0
    return n

def uniq_shuffled(xs):
    "doesn't preserve xs order, but removes duplicates.  fastest"
    return list(set(xs))

def uniq_stable(xs):
    'xs with duplicates removed; in original order'
    seen=set()
    r=[]
    for x in xs:
        if x not in seen:
            seen.add(x)
            r.append(x)
    return r

def uniq(xs):
    'xs must be sorted! removes duplicates'
    if len(xs)==0: return []
    p=xs[0]
    r=[p]
    for x in xs:
        if x!=p:
            r.append(x)
        p=x
    return r

def sort_uniq(xs):
    return uniq(sorted(xs))

def componentwise(xs,ys,s=sum):
    "return zs with zs[i]=s(xs[i],ys[i]); len(zs)==min(len(xs),len(ys))"
    return map(s,izip(xs,ys))

def set_agreement(test,gold):
    "return (true pos,false pos,false neg), i.e. (|test intersect gold|,|test - gold|,|gold - test|).  these can be vector_summed"
    if type(test) is not set:
        test=set(test)
    if type(gold) is not set:
        gold=set(gold)
    falsepos=len(test-gold)
    truepos=len(test)-falsepos
    falseneg=len(gold-test)
    return (truepos,falsepos,falseneg)

def divpos_else(q,d,e):
    return e if d==0 else float(q)/d

def pr_from_agreement(truepos,falsepos,falseneg):
    "given (true pos,false pos,false neg) return (precision,recall)"
    P=divpos_else(truepos,truepos+falsepos,1.)
    R=divpos_else(truepos,truepos+falseneg,1.)
    return (P,R)

def fmeasure(P,R,alpha_precision=.5):
    "given precision, recall, return weighted fmeasure"
    A=float(alpha_precision)
    return 0. if P==0 or R==0 else 1./(A/P+(1-A)/R)

def fmeasure_str(P,R,alpha_precision=.5):
    return 'P=%.3g R=%.3g F(%.3g)=%.3g'%(P,R,alpha_precision,fmeasure(P,R,alpha_precision))

class Alignment(object):
    apair=re.compile(r'(\d+)-(\d+)')
    def __init__(self,aline,ne,nf,fe_align=False):
        "aline is giza-format alignment: '0-0 0-1 ...' (e-f index with 0<=e<ne, 0<=f<nf).  if fe_align, then aline is f-e instead.  if ne or nf is not None, bound check (should catch backwards e-f vs f-e format often)"
        def intpair(stringpair):
            a,b=(int(stringpair[0]),int(stringpair[1]))
            e,f=(b,a) if fe_align else (a,b)
            if (ne is None or e<ne) and (nf is None or f<nf):
                return e,f
            raise Exception("alignment %d-%d >= bounds %d-%d - are you reading inverse or wrong alignments? aline=%s"%(e,f,ne,nf,aline))
        self.efpairs=list(set((intpair(Alignment.apair.match(a).group(1,2)) for a in aline.strip().split()))) if aline else []
        self.ne=ne
        self.nf=nf
    def inverse(self):
        r=Alignment(None,self.nf,self.ne)
        r.efpairs=[(b,a) for (a,b) in self.efpairs]
        return r
    def is_identity(self):
        return self.ne==self.nf and len(self.efpairs)==self.ne and all((a,a)==b for (a,b) in itertools.izip(itertools.count(0),sorted(self.efpairs)))
    def includes_identity(self):
        if self.ne!=self.nf: return False
        s=self.efpairs_set()
        return all((i,i) in s for i in range(0,self.nf))
    def copy_blank(self):
        "return a blank alignment of same dimensions"
        return Alignment(None,self.ne,self.nf)
    def corrupt(self,p,d):
        'corrupt e and f ends of an alignment link independently in output.a with probability p; move a distorted link within +-d'
        def rdistort(p,d):
            if random.random()<p:
                return random.randint(-d,d)
            return 0
        self.efpairs=list(set((bound(e+rdistort(p,d),self.ne),bound(f+rdistort(p,d),self.nf)) for e,f in self.efpairs))
    def str_agreement(self,gold,alpha_precision=.6):
        ag=self.agreement(gold)
        return Alignment.agreestr(ag)
    def agreement(self,gold):
        "returns (true pos,false pos,false neg) vs. gold"
        return set_agreement(self.efpairs,gold.efpairs)
    @staticmethod
    def agreestr(ag,alpha_precision=.6):
        p,r=pr_from_agreement(*ag)
        return Alignment.fstr(p,r,alpha_precision)
    @staticmethod
    def fstr(p,r,alpha_precision=.6):
        "alpha=.6 was best correlation w/ translation quality in Fraser+Marcu ISI-TR-616"
        return fmeasure_str(p,r,alpha_precision)
    def fully_connect(self,es,fs):
        "es and fs are lists of e and f indices, fully connect cross product"
        for e in es:
            for f in fs:
                self.efpairs.append((e,f))
    def efpairs_set(self):
        "set of (e,f) links"
        return set(self.efpairs)
    def adje(self):
        "for e word index, list of f words aligned to it"
        return adjlist(self.efpairs,self.ne)
    def adjf(self):
        "inverse of adje (indexed by f)"
        return adjlist([(f,e) for (e,f) in self.efpairs],self.nf)
    def spanadje(self):
        "minimal covering span (fa,fb) for e word index's aligned f words"
        return [span_cover_points(adj) for adj in self.adje()]
    def spanadjf(self):
        "inverse of spanadje"
        return [span_cover_points(adj) for adj in self.adjf()]
    def __str__(self):
        "warning: no roundtrip possible because we only print pairs and not e/f len"
        return " ".join(["%d-%d"%p for p in self.efpairs])

def clamp(x,a,b):
    'force x to lie on [a,b]'
    return min(b,max(a,x))

def bound(x,n):
    'force x to lie on [0,n)'
    return clamp(x,0,n-1)

def unordered_pairs(xs):
    "return list of (a,b) for all a,b in xs, such that a is before b in xs"
    n=len(xs)
    return [(xs[i],xs[j]) for i in range(0,n) for j in range(i+1,n)]
    # yuck, for x in X for y in f(x) is just like for x in X: for y in f(y): - should be backwards.

PYTHON26=sys.version >= '2.6'

# could be more concise; but hope that this is efficient.  note: on python 2.6 i saw no numerical difference when logadding 1e-15 and 1
if PYTHON26:
    def logadd(lhs,rhs):
        "return log(exp(a)+exp(b)), i.e. if a and b are logprobs, returns the logprob of their probs' sum"
        diff=lhs-rhs
        if diff > 36: return lhs # e^36 or more overflows double floats.
        if diff < 0: #rhs is bigger
            if diff < -36: return rhs
            return rhs+math.log1p(math.exp(diff))
        return lhs+math.log1p(math.exp(-diff))
else:
    def logadd(lhs,rhs):
        "return log(exp(a)+exp(b)), i.e. if a and b are logprobs, returns the logprob of their probs' sum"
        diff=lhs-rhs
        if diff > 36: return lhs # e^36 or more overflows double floats.
        if diff < 0: #rhs is bigger
            if diff < -36: return rhs
            return rhs+math.log(1.+math.exp(diff))
        return lhs+math.log(1.+math.exp(-diff))

def logadd_plus(a,b):
    return math.exp(logadd(math.log(a),math.log(b)))

def choosep(p_item_list):
    "given list of tuples (p,item), return a random item according to (possibly unnormalized) p, None if empty list input"
    if len(p_item_list)==0:
        return None
    z=sum(x[0] for x in p_item_list)
    c=random.random()*z
    for (p,i) in p_item_list:
        c-=p
        if c<=0:
            return i
    return p_item_list[-1][1]

def choosei(ps):
    "given list of possibly unnormalized probabilities, return a random index drawn from that multinomial; None if empty list input.  return with uniform prob if all ps=0 (underflow)"
    if len(ps)==0:
        return None
    z=sum(ps)
    if z<=0:
        return random.randint(0,len(ps)-1)
    c=random.random()*z
    for i in range(0,len(ps)):
        c-=ps[i]
        if c<=0:
            return i
    return len(ps)-1

log_of_10=math.log(10)

def normalize_logps(logps):
    logz=reduce(logadd,logps)
    return [lp-logz for lp in logps]

def ps_from_logps(logps):
    logz=reduce(logadd,logps)
    return [math.exp(lp-logz) for lp in logps]

def choosei_logps(logps,power=1.):
    if (power!=1.):
        logps=[power*l for l in logps]
    ps=ps_from_logps(logps)
    i=choosei(ps)
#    dump(callerstring(1),i,logps,ps)
    return i

def choosei_logp(ps):
    "given list of logprobs, return random index"
    if len(ps)==0:
        return None
    z=sum(ps)
    c=random.random()*z
    for i in range(0,len(ps)):
        c-=ps[i]
        if c<=0:
            return i
    return len(ps)-1

def withp(prob):
    return random.random()<prob

def filter2(list,p):
    "return tuple of two lists a,b: a is the subseq in list where p(a[i]) is True, b is everything else"
    a=[]
    b=[]
    for x in list:
        if p(x):
            a.append(x)
        else:
            b.append(x)
    return a,b

def func_args(func):
    "return dict func's arg=default (default=None if no default)"
    args, varargs, varkw, defaultvals = inspect.getargspec(func)
    nd=len(defaultvals)
    defaultvals = defaultvals or ()
    options = dict(zip(args[-nd:], defaultvals))
    options.pop('rest_', None)
    for a in args[0:-nd]:
        options[a]=None
    return options

class Record(object):
    def __init__(self):
        pass
    def update(self,d):
        """make fields of object for dict or object d.  FIXME: int keys in dict can't make int records (would have to hack Record to be indexable"""
        if (hasattr(d,'__dict__')):
            d=getattr(d,'__dict__')
        self.__dict__.update(d)
    def __str__(self):
        return attr_str(self)

def getlocals(up=0):
    """returns dict of locals of calling function (or up-parent) using frame"""
    f = sys._getframe(1+up)
    args = inspect.getargvalues(f)
    return args[3]

class Locals(Record):
    def __init__(self,up=0):
        self.update(getlocals(up+1))

def object_from_dict(d):
    "setattr(obj,key)=val for key,val in d and return obj"
    obj=Record()
    obj.update(d)

def log_start(s):
    sys.stderr.write("### "+s)
def log_continue(s):
    sys.stderr.write(s)
def log_finish(s):
    sys.stderr.write(s+"\n")
def log(s):
    sys.stderr.write("### "+s+"\n")
import time
def logtime(s=""):
    log(time.ctime()+(": "+s if s else ""))

def sec_per(sec,N=1):
    if N is None or N<=0: return "None finished yet"
    if sec<=0: return "Infinitely many per sec (time precision too small to measure)"
    return "%g sec per"%(float(sec)/N) if sec>N else "%g per sec"%(float(N)/sec)

time_keys=dict()
def log_time_since(key="",extra="",N=1):
    t=time.time()
    if key not in time_keys:
        time_keys[key]=t
        return
    last=time_keys[key]
    time_keys[key]=t
    d=t-last
    per=' (%s, N=%s)'%(sec_per(d,N),N) if N>0 else ''
    elapsed='%s sec since last '%d
    logtime('%s%s%s%s'%(elapsed,key,extra,per))

def test_since(key="test"):
    for i in range(0,10):
        log_time_since(key,i,i)
        for j in range(0,100000): a=1

def dict_slice(d,keys):
    return dict((k,d[k]) for k in keys)

def fold(f,z,list):
    for x in list:
        z=f(z,x)
    return z

def scan(f,z,list):
    result=[z]
    for x in list:
        z=f(z,x)
        result.append(z)
    return result

#reduce(lambda x, y: x+y, [1, 2, 3, 4, 5]) calculates ((((1+2)+3)+4)+5)
# If the optional initializer is present, it is placed before the items of the iterable in the calculation, and serves as a default when the iterable is empty
def fold(f,z,list):
    return reduce(f,list,z)


def cartesian_product(a,b):
    "return list of tuples"
    return [(x,y) for x in a for y in b]

def range_incl(a,b):
    return range(a,b+1)

import inspect

pod_types=[int,float,long,complex,str,unicode,bool]

def attr_pairlist(obj,names=None,types=pod_types,skip_callable=True,skip_private=True):
    """return a list of tuples (a1,v1)... if names is a list ["a1",...], or all the attributes if names is None.  if types is not None, then filter the tuples to those whose value's type is in types'"""
    if not names:
        names=[a for a in map(str,dir(obj)) if not (skip_private and a[0:2] == '__')]
    #    attrs,indices=filter2(names,lambda x:type(x) is str)
    avs=[(k,getattr(obj,k)) for k in names if hasattr(obj,k)]
    return [(a,b) for (a,b) in avs if not ((skip_callable and callable(b)) or (types is not None and type(b) not in types))]
    #+[(i,obj[i]) for i in indices]

import pprint

def attr_str(obj,names=None,types=pod_types,pretty=True):
    "return string: a1=v1 a2=v2 for attr_pairlist"
    return pairlist_str(attr_pairlist(obj,names,types,True,True),pretty)

def pairlist_str(pairlist,pretty=True):
    if pretty: pairlist=[(a,pprint.pformat(b,1,80,3)) for (a,b) in pairlist]
    return ' '.join(["%s=%s"%p for p in pairlist])

def obj2str(obj,names=None,types=pod_types,pretty=True):
    return '[%s %s]'%(obj.__class__.__name__,attr_str(obj,names,types,pretty))

def writeln(line,file=sys.stdout):
    f.write(f)
    f.write('\n')

def open_in(fname):
    "if fname is '-', return sys.stdin, else return open(fname,'r')"
    return sys.stdin if fname=='-' else open(fname,'r')

def open_out(fname):
    "if fname is '-', return sys.stdout, else return open(fname,'w').  not sure if it's ok to close stdout, so let GC close file please."
    if fname=='-':
        return sys.stdout
    return open(fname,'w')

def open_out_prefix(prefix,name):
    "open_out prefix+name, or stdout if prefix='-'"
    if prefix=='-':
        return sys.stdout
    if prefix=='-0' or prefix=='/dev/null':
        return os.devnull
    return open_out(prefix+name)

def adjlist(pairs,na):
    "return adjacency list indexed by [a]=[x,...,z] for pairs (a,x) ... (a,z)"
    adj=[[] for row in xrange(na)]
    for a,b in pairs:
        adj[a].append(b)
    return adj

"""spans: empty span is None, not (0,0) or similar.  s[0]<=i<s[1] are the i in a span s."""
def span_cover_points(points):
    "returns (a,b) for half-open [a,b) covering points"
    if (len(points)==0):
        return None
    return (min(points),max(points)+1)

def span_points(s):
    if s is None: return []
    return range(s[0],s[1])

def span_empty(s):
    return s is None

def span_size(s):
    if s is None: return 0
    return s[1]-s[0]

def span_cover(sa,sb):
    """return smallest span covering both sa and sb; if either is None other is returned; 0-length spans aren't allowed - use None instead"""
    if sa is None:
        return sb
    if sb is None:
        return sa
    return (min(sa[0],sb[0]),max(sa[1],sb[1])) # 0-length spans would confuse this formula, which is why

def span_in(a,b):
    "a contained in b"
    return a is None or (b is not None and a[0]>=b[0] and a[1]<=b[1])

def span_points_except(s,points):
    "return list of points in s=[a,b) but not in list points"
    if s is None: return []
    a=s[0]
    b=s[1]
    ss=range(a,b)
    for p in points:
        if a<p<=b:
            ss[i-a]=None
    return [x for x in ss if x is not None]

def unmarked_span(s):
    return [False for x in range(s[0],s[1])]

def fresh_mark(marks,i):
    if not marks[i]:
        marks[i]=True
        return True
    return False

def span_points_fresh(span,marks):
    "if marks[i] was false for s[0]<=i<s[1], set to true and include i in return list."
    return [i for i in range(span[0],span[1]) if fresh_mark(marks,i)]

def span_mark(span,marks):
    if span is not None:
        for i in range(span[0],span[1]):
            marks[i]=True

def span_str(s):
    if s is None:
        return "[]"
    return "[%d,%d]"%s

import string
d2at_trans=string.maketrans('0123456789','@@@@@@@@@@')
def digit2at(s):
    return s.translate(d2at_trans)


radu_drophead=re.compile(r'\(([^~]+)~(\d+)~(\d+)\s+(-?[.0123456789]+)')
#radu_lrb=re.compile(r'\((-LRB-(-\d+)?) \(\)')
#radu_rrb=re.compile(r'\((-RRB-(-\d+)?) \)\)')
sym_rrb=re.compile(r'\((\S+(-\d+)?) (\(|\)|[^) ]+)\)(?= |$)') #complicated, but necessary
rparen=re.compile(r'\)')
lparen=re.compile(r'\(')
def escape_paren(s):
    s=rparen.sub('-RRB-',s)
    return lparen.sub('-LRB-',s)
def rrb_repl(match):
    return '(%s %s)'%(match.group(1),escape_paren(match.group(3)))
def radu2ptb(t,strip_head=True):
    "radu format: all close parens that indicate tree structure are followed by space or end, so that () are legal within symbols -   also, we strip head info.  we escape them to -LRB- -RRB- for handling by tree.py."
    if strip_head: t=radu_drophead.sub(r'(\1',t)
    t=sym_rrb.sub(rrb_repl,t)
    return t

def shellquote(s):
    return "'" + s.replace("'", "'\\''") + "'"

def take(n, iterable):
    "Return first n items of the iterable as a list"
    return list(islice(iterable, n))

def tabulate(function, start=0):
    "Return function(0), function(1), ..."
    return imap(function, count(start))

def consume(iterator, n):
    "Advance the iterator n-steps ahead. If n is none, consume entirely."
    # Use functions that consume iterators at C speed.
    if n is None:
        # feed the entire iterator into a zero-length deque
        collections.deque(iterator, maxlen=0)
    else:
        # advance to the empty slice starting at position n
        next(islice(iterator, n, n), None)

def nth(iterable, n, default=None):
    "Returns the nth item or a default value"
    return next(islice(iterable, n, None), default)

def quantify(iterable, pred=bool):
    "Count how many times the predicate is true"
    return sum(imap(pred, iterable))

def padnone(iterable):
    """Returns the sequence elements and then returns None indefinitely.

    Useful for emulating the behavior of the built-in map() function.
    """
    return chain(iterable, repeat(None))

def ncycles(iterable, n):
    "Returns the sequence elements n times"
    return chain.from_iterable(repeat(tuple(iterable), n))

def dotproduct(vec1, vec2):
    return sum(imap(operator.mul, vec1, vec2))

def flatten(listOfLists):
    "Flatten one level of nesting"
    return chain.from_iterable(listOfLists)

def repeatfunc(func, times=None, *args):
    """Repeat calls to func with specified arguments.

    Example:  repeatfunc(random.random)
    """
    if times is None:
        return starmap(func, repeat(args))
    return starmap(func, repeat(args, times))

def pairwise(iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return izip(a, b)

def grouper(n, iterable, fillvalue=None):
    "grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx"
    args = [iter(iterable)] * n
    return izip_longest(fillvalue=fillvalue, *args)

def roundrobin(*iterables):
    "roundrobin('ABC', 'D', 'EF') --> A D E B F C"
    # Recipe credited to George Sakkis
    pending = len(iterables)
    nexts = cycle(iter(it).next for it in iterables)
    while pending:
        try:
            for next in nexts:
                yield next()
        except StopIteration:
            pending -= 1
            nexts = cycle(islice(nexts, pending))

def powerset(iterable):
    "powerset([1,2,3]) --> () (1,) (2,) (3,) (1,2) (1,3) (2,3) (1,2,3)"
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(len(s)+1))

def unique_everseen(iterable, key=None):
    "List unique elements, preserving order. Remember all elements ever seen."
    # unique_everseen('AAAABBBCCDAABBB') --> A B C D
    # unique_everseen('ABBCcAD', str.lower) --> A B C D
    seen = set()
    seen_add = seen.add
    if key is None:
        for element in ifilterfalse(seen.__contains__, iterable):
            seen_add(element)
            yield element
    else:
        for element in iterable:
            k = key(element)
            if k not in seen:
                seen_add(k)
                yield element

def unique_justseen(iterable, key=None):
    "List unique elements, preserving order. Remember only the element just seen."
    # unique_justseen('AAAABBBCCDAABBB') --> A B C D A B
    # unique_justseen('ABBCcAD', str.lower) --> A B C A D
    return imap(next, imap(itemgetter(1), groupby(iterable, key)))

def iter_except(func, exception, first=None):
    """ Call a function repeatedly until an exception is raised.

    Converts a call-until-exception interface to an iterator interface.
    Like __builtin__.iter(func, sentinel) but uses an exception instead
    of a sentinel to end the loop.

    Examples:
        bsddbiter = iter_except(db.next, bsddb.error, db.first)
        heapiter = iter_except(functools.partial(heappop, h), IndexError)
        dictiter = iter_except(d.popitem, KeyError)
        dequeiter = iter_except(d.popleft, IndexError)
        queueiter = iter_except(q.get_nowait, Queue.Empty)
        setiter = iter_except(s.pop, KeyError)

    """
    try:
        if first is not None:
            yield first()
        while 1:
            yield func()
    except exception:
        pass

def random_product(*args, **kwds):
    "Random selection from itertools.product(*args, **kwds)"
    pools = map(tuple, args) * kwds.get('repeat', 1)
    return tuple(random.choice(pool) for pool in pools)

def random_permutation(iterable, r=None):
    "Random selection from itertools.permutations(iterable, r)"
    pool = tuple(iterable)
    r = len(pool) if r is None else r
    return tuple(random.sample(pool, r))

def random_combination(iterable, r):
    "Random selection from itertools.combinations(iterable, r)"
    pool = tuple(iterable)
    n = len(pool)
    indices = sorted(random.sample(xrange(n), r))
    return tuple(pool[i] for i in indices)

def random_combination_with_replacement(iterable, r):
    "Random selection from itertools.combinations_with_replacement(iterable, r)"
    pool = tuple(iterable)
    n = len(pool)
    indices = sorted(random.randrange(n) for i in xrange(r))
    return tuple(pool[i] for i in indices)
