/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
size_t
STRNode::operator () () const{
  size_t k=0;
  if(p->getFanin0()->getID()<p->getFanin1()->getID()){
    k=k^size_t(2*p->getFanin0()->getID()+(size_t(p->getInv0())));
    k=k^size_t(2*p->getFanin1()->getID()+(size_t(p->getInv1()))<<24);
  }
  else{
    k=k^size_t(2*p->getFanin1()->getID()+(size_t(p->getInv1())));
    k=k^size_t(2*p->getFanin0()->getID()+(size_t(p->getInv0()))<<24);
  }
  return k;
}

bool
STRNode::operator == (const STRNode& n)const{
   return p->getID()==n.getNode()->getID();
}


/**************************************/
/*   Static varaibles and functions   */
/**************************************/



/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  for(unsigned i=0;i<_polist.size();++i){
    DFSstr(_polist[i]);
  }
  resetprint();
}

void
CirMgr::fraig()
{
  SatSolver solver;
  solver.initialize();
  _DFSlist.clear();
  for(unsigned i=0;i<_polist.size();++i){
    simDFS(_polist[i]->getFanin0());
  }
  resetprint();
  genProofModel(solver);
  size_t     input[_pilist.size()]={0};
  int count=0;
  const0->setCan();
  auto it=_DFSlist.begin();
  while(true){
        for(unsigned z=0;z<(*it)->getFECpair().size();++z){
          if((*it)->getFECpair()[z]->checkCan() && (*it)->getFECpair()[z]->getID()!=(*it)->getID()){
            int result=genResult(solver,(*it)->getFECpair()[z],(*it),input,count);
            if(result==1 && count==64){
              it=find(_DFSlist.begin(),_DFSlist.end(),*it);
              count=0;
              }
            else if(result==0){
              if(!(*it)->getFECpair()[z]->checkrev()^(*it)->checkrev()){
                cout<<"Fraig: "<<(*it)->getFECpair()[z]->getID()<<" merging "<<(*it)->getID()<<"..."<<endl;
                merge((*it)->getFECpair()[z],(*it),false);
                break;
              }
              else{
                cout<<"Fraig: "<<(*it)->getFECpair()[z]->getID()<<" merging !"<<(*it)->getID()<<"..."<<endl;
                merge((*it)->getFECpair()[z],(*it),true);
                break;
              }
            }
          }
        (*it)->setCan();
        }
        ++it;
        if(it==_DFSlist.end()){break;}
  }
  _FECgrps.clear();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
CirGate* 
CirMgr::DFSstr(CirGate* p){
  if(p==0){return 0;}
  if(p->getTypeStr()=="Pi" || p->getTypeStr()=="const" || p->getTypeStr()=="UNDEF"){return 0;}
  if(p->checkprint()){return 0;}
  DFSstr(p->getFanin0());
  DFSstr(p->getFanin1());
  if(p==0){return 0;}
  if(p->getTypeStr()!="Aig"){return 0;}
  if(p->checkprint()){return 0;}
  p->setprint();

  STRNode tmp(p);
  // cout<<"....."<<endl;
  // cout<<tmp->getNode()->getID()<<" "<<tmp->getNode()->getFanin0()->getID()<<" "<<tmp->getNode()->getFanin1()->getID()<<endl;
  if(_hashset.find(tmp())==_hashset.end()){_hashset.insert({{tmp(),tmp}});}
  else{
    cout<<"Strashing: "<<_hashset.find(tmp())->second.getNode()->getID()<<" merging "<<tmp.getNode()->getID()<<"..."<<endl;
    merge(_hashset.find(tmp())->second.getNode(),tmp.getNode(),false);
    
    }
}

void 
CirMgr::merge(CirGate *p1,CirGate *p2,bool rev){
  p2->getFanin0()->delFanout(p2);
  p2->getFanin1()->delFanout(p2);
  for (unsigned i=0;i<p2->getFanout().size();++i){
    p1->pushFanout(p2->getFanout()[i]);
    if(p2->getFanout()[i]->getFanin0()->getID()==p2->getID()){
      if(!rev)p2->getFanout()[i]->setFanin0(p1,p2->getFanout()[i]->getInv0());
      else{p2->getFanout()[i]->setFanin0(p1,!p2->getFanout()[i]->getInv0());}
  }
  if(p2->getFanout()[i]->getTypeStr()!="Po"){
   if(p2->getFanout()[i]->getFanin1()->getID()==p2->getID()){
      if(!rev)p2->getFanout()[i]->setFanin1(p1,p2->getFanout()[i]->getInv1());
      else{p2->getFanout()[i]->setFanin1(p1,!p2->getFanout()[i]->getInv1());}
    }}
}
auto it=GateMap.find(p2->getID());
GateMap.erase(it);
if(p2->getFanin0()->getTypeStr()=="UNDEF" || p2->getFanin1()->getTypeStr()=="UNDEF" ){
  auto itv=find(_floatlist.begin(),_floatlist.end(),p2->getID());
  _floatlist.erase(itv);
}
for(unsigned i=0;i<p2->getFECpair().size();++i){
  p2->getFECpair()[i]->delFECpair(p2);
}
auto itv=find(_aiglist.begin(),_aiglist.end(),p2);
_aiglist.erase(itv);
// delete p2;
}


void 
CirMgr::genProofModel(SatSolver& s){
  for(unsigned i=0;i<_DFSlist.size();++i){
    if(_DFSlist[i]->getFanin0()->getTypeStr()=="Pi"){
      Var v=s.newVar();
      _DFSlist[i]->getFanin0()->setVar(v);
    }
    if(_DFSlist[i]->getFanin1()->getTypeStr()=="Pi"){
      Var v=s.newVar();
      _DFSlist[i]->getFanin1()->setVar(v);
    }
    Var v=s.newVar();
    _DFSlist[i]->setVar(v);
    v=s.newVar();
    const0->setVar(v);
  }
  for(unsigned i=0;i<_DFSlist.size();++i){
    s.addAigCNF(_DFSlist[i]->getVar(),_DFSlist[i]->getFanin0()->getVar(),_DFSlist[i]->getInv0(),_DFSlist[i]->getFanin1()->getVar(),_DFSlist[i]->getInv1());
  }
  s.addAigCNF(const0->getVar(),const0->getVar(),true,const0->getVar(),false);
}


int
CirMgr::genResult(SatSolver& s,CirGate* p1,CirGate* p2,size_t a[],int &count){
  Var newV=s.newVar();
  bool result;
  if(!p1->checkrev()^p2->checkrev()){
    s.addXorCNF(newV,p1->getVar(),false,p2->getVar(),false);
    s.assumeRelease();
    s.assumeProperty(newV,true);
    result=s.assumpSolve();
  }
  else{
    s.addXorCNF(newV,p1->getVar(),true,p2->getVar(),false);
    s.assumeRelease();
    s.assumeProperty(newV,true);
    result=s.assumpSolve();
  }
  if(result){
    count++;
    for(unsigned i=0;i<_pilist.size();++i){
      if(count==1){a[i]=size_t(s.getValue(_pilist[i]->getVar()));}
      else{a[i]=size_t(a[i]<<1)+size_t(s.getValue(_pilist[i]->getVar()));}
    }
    if(count==64){
      simFraig(a);
      a={0};
    }
  }
  
  return result;
}
