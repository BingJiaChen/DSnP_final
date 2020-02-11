/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include<map>
#include<unordered_map>
#include"../util/myHashSet.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class STRNode
{
public:
   STRNode(){}
   STRNode(CirGate* x){p=x;};
   ~STRNode() {}
   size_t operator () ()const;
   CirGate* getNode() const{return p;}
   bool operator == (const STRNode& n) const;

   friend ostream& operator << (ostream& os, const STRNode& n);

private:
   CirGate* p;
};

class FECNode
{
public:
   FECNode(){};
   FECNode(CirGate* x){p=x;};
   ~FECNode() {}
   size_t operator () ()const;
   CirGate* getNode() const{return p;}
   bool operator == (const FECNode& n) const;

   friend ostream& operator << (ostream& os, const FECNode& n);

private:
   CirGate* p;
};

class CirMgr
{
public:
   CirMgr():flAIG(0){
      _undef.push_back(pair<bool,bool>(true,true));_FECgrps.clear();_fecset.clear();issim=0;_hashset.clear();_DFSlist.clear();
      _pilist.clear();_polist.clear();_aiglist.clear();GateMap.clear();
      }
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
    int s=gid;
    if(s==0){return const0;}
    auto it=GateMap.find(s);
    if(it==GateMap.end()){return 0;}
    return it->second;
    }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void connect();
   // Member functions about circuit optimization
   void sweep();
   CirGate* updateSweep(CirGate*);
   CirGate* updateUndef(CirGate*);
   CirGate* optDFS(CirGate*);
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   size_t rand64();
   bool getIssim(){return issim;}
   void setIssim(){issim=true;}
   // Member functions about fraig
   void strash();
   CirGate* DFSstr(CirGate*);
   void printFEC() const;
   void fraig();
   void genProofModel(SatSolver& s);
   void refreshProofModel(SatSolver& s);
   int genResult(SatSolver&,CirGate*,CirGate*,size_t[],int&);
   void merge(CirGate* , CirGate*,bool);
   CirGate* simDFS(CirGate* );
   void simrand();
   void simfile(size_t [] ,size_t&);
   void simFraig(size_t[]);
   void writeLog(ostream&,size_t&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   CirGate* DFS(CirGate* p) const;
   CirGate* writeDFS(CirGate* p,ostream& outfile) const;
    

private:
   ofstream           *_simLog;
   vector<CirGate*>           _pilist;
   vector<CirGate*>           _polist;
   vector<CirGate*>          _aiglist;
   vector<int>                      _floatlist;
   vector<pair<bool,bool>>                       _undef;
   mutable vector< string>                printList;
   int                             M;
   int                             I;
   int                             O;
   int                             A;
   mutable int                             flAIG;
   CirGate*                            const0;
   mutable map<int,CirGate*>       GateMap;
   bool                                      issim;
   void readHeader(string header);
   void readInput(string input,int l);
   void readOutput(string output,int l);
   void readAig(string Aig,int l);
   void readSymbol(string symbol);
   void resetprint() const;
   static bool mysort(CirGate const *p1,CirGate const* p2);
   unordered_map<size_t,STRNode>        _hashset;
   mutable vector<unordered_map<size_t,vector<CirGate*>>>              _fecset;
   mutable vector<vector<CirGate*>>  _FECgrps;
   vector<CirGate*>                                      _DFSlist;
};



#endif // CIR_MGR_H
