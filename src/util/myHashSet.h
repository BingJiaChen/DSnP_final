/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>
#include "math.h"

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      iterator(){_node=0;index=0;}
      iterator(Data* p,vector<Data>* b,size_t in,size_t num){_node=p;index=in;_buckets=b;numBuckets=num;}
      ~iterator(){}
      const Data& operator * () const { return *_node; }
      iterator& operator ++ () {
         _node=_node+1;
         typename vector<Data>::iterator p(_node);
         if(p==_buckets[index].end() && index!=numBuckets-1){
            index+=1;
            while(true){
               if(!_buckets[index].empty()){break;}
               if(index==numBuckets){index=numBuckets-1;break;}
               index+=1;
            }
            _node=&_buckets[index][0];
         }
         return (*this); }
      bool operator != (const iterator& i) const { return (_node!=i._node); }
      void operator =(const iterator& i) const{_node=i._node;index=i._index;_buckets=i._buckets;numBuckets=i.numBuckets;}
   private:
      Data* _node;
      size_t         index;
      size_t         numBuckets;
      vector<Data>* _buckets;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { 
      size_t index=0;
      while(true){
         if(!_buckets[index].empty())break;
         index+=1;
      }
      iterator it(&_buckets[index][0],_buckets,index,_numBuckets);
      return it; }
   // Pass the end
   iterator end() const { 
      if(empty()){return begin();}
      int index=0;
      Data* p=&_buckets[_numBuckets-1][index];
      while(true){
         typename vector<Data>::iterator it(p);
         if(it==_buckets[_numBuckets-1].end()){break;}
         index+=1;
         p=&_buckets[_numBuckets-1][index];
      }
      return iterator(p,_buckets,_numBuckets-1,_numBuckets); }
   // return true if no valid data
   bool empty() const { 
      size_t index=0;
      for(index=0;index<numBuckets();++index){
         if(!_buckets[index].empty())return false;
      }
      return true; }
   // number of valid data
   size_t size_Buckets() const { 
     return _numBuckets;}

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const { 
      size_t index=bucketNum(d);
      if(_buckets[index].empty()){return false;}
      return true;
      }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const { 
      size_t index=bucketNum(d);
      typename vector<Data>::iterator it;
      for(it=_buckets[index].begin();it!=_buckets[index].end();++it){
         if(*it==d){d=*it;return true;}
      }
      return false; }
   Data find(Data& d){
      size_t index=bucketNum(d);
      return _buckets[index][0];
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) { 
      size_t index=bucketNum(d);
      typename vector<Data>::iterator it;
      for(it=_buckets[index].begin();it!=_buckets[index].end();++it){
         if(*it==d){*it=d;return true;}
      }
      return false; }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) { 
      size_t index=bucketNum(d);
      cout<<"["<<d.getNode()->getID()<<"] "<<index<<endl;
      // cout<<bucketNum(d)<<endl;
      if(check(d)){return false;}
      _buckets[index].push_back(d);
      
      return true; }

   bool insertFEC(const Data& d,bool rev){
      if(!rev){
      size_t index=bucketNum(d);
      _buckets[index].push_back(d);
      // cout<<"["<<d.getNode()->getID()<<"] "<<index<<endl;
      }
      else{
         size_t index=(d() ^ ~size_t(0)) % _numBuckets;
         _buckets[index].push_back(d);
         // cout<<"["<<d.getNode()->getID()<<"] "<<index<<endl;
      }
      return true;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) { 
      size_t index=bucketNum(d);
      typename vector<Data>::iterator it;
      for(it=_buckets[index].begin();it!=_buckets[index].end();++it){
         if(*it==d){
            *it=*(_buckets[index].end()-1);
            _buckets[index].pop_back();
            return true;}
      }
      return false; }

   vector<Data>* getBucket(){return _buckets;}

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
