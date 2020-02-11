/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include<vector>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  CirGate* tmp;
  map<int,CirGate*>::iterator i=GateMap.begin();
  for(i;i!=GateMap.end();++i){
    if(i->second->getFanout().size()==0 && !i->second->checkprint()){
      if(i->second->getTypeStr()=="Aig"){
        updateSweep(i->second);
      }
    }
     if(i->second->getTypeStr()=="UNDEF"){
      updateUndef(i->second);
    }
  }
  _floatlist.clear();
  map<int,CirGate*>::iterator it=GateMap.begin();
  vector<CirGate*>::iterator itv;
  while(it!=GateMap.end()){
    tmp=it->second;
    if(tmp->checkDel()){
      if(tmp->getTypeStr()=="Aig")
      {
        itv=find(_aiglist.begin(),_aiglist.end(),tmp);
          _aiglist.erase(itv);
          cout<<"Sweeping: AIG("<<tmp->getID()<<") removed..."<<endl;
          }
      else if(tmp->getTypeStr()=="UNDEF"){cout<<"Sweeping: UNDEF("<<tmp->getID()<<") removed..."<<endl;}
      GateMap.erase(it++);
      delete tmp;
      }
    else{++it;}
  }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  for(unsigned i=0;i<_polist.size();++i){
     optDFS(_polist[i]->getFanin0());
   }
   resetprint();
}



/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
CirGate*
CirMgr::updateSweep(CirGate* p)
{
    if(p->getTypeStr()=="Pi" || p->getTypeStr()=="UNDEF" || p->getTypeStr()=="const"){return 0;}
    p->setDel();
    p->getFanin0()->delFanout(p);
    p->getFanin1()->delFanout(p);
    if(p->getFanin0()->getFanout().size()==0){
      updateSweep(p->getFanin0());
    }
    if(p->getFanin1()->getFanout().size()==0){
      updateSweep(p->getFanin1());
    }
    return 0;
}

CirGate*
CirMgr::updateUndef(CirGate* p){
  if(p->getFanout().size()==0){return 0;}
  p->setDel();
  if(p->getFanin0()!=0)p->getFanin0()->delFanout(p);
  if(p->getFanin1()!=0)p->getFanin1()->delFanout(p);
  for(unsigned i=0;i<p->getFanout().size();++i){
    updateUndef(p->getFanout()[i]);
  }
}

CirGate*
CirMgr::optDFS(CirGate* p){
  if(p==0){return 0;}
  auto it=GateMap.find(p->getID());
  if(it==GateMap.end()){return 0;}
  if(p->getTypeStr()=="Pi" || p->getTypeStr()=="const" || p->getTypeStr()=="UNDEF"){return 0;}
  if(p->checkprint()){return 0;}
  optDFS(p->getFanin0());
  optDFS(p->getFanin1());
  if(p->checkprint()){return 0;}
  if(p==0){return 0;}
  p->setprint();


  //const 0
  if(((p->getFanin0()->getID()==0 && p->getInv0()==false) || (p->getFanin1()->getID()==0 && p->getInv1()==false))&& p->getTypeStr()=="Aig"){
    for(unsigned i=0;i<p->getFanout().size();++i){
      if(p->getFanout()[i]->getFanin0()->getID()==p->getID()){
        p->getFanout()[i]->setFanin0(const0,p->getFanout()[i]->getInv0());
        const0->pushFanout(p->getFanout()[i]);
      }
      else if(p->getFanout()[i]->getFanin1()->getID()==p->getID()){
        p->getFanout()[i]->setFanin1(const0,p->getFanout()[i]->getInv1());
        const0->pushFanout(p->getFanout()[i]);
      }
    }
    cout<<"Simplifying: 0 merging "<<p->getID()<<"..."<<endl;
    vector<CirGate*>::iterator itv;
    itv=find(_aiglist.begin(),_aiglist.end(),p);
    _aiglist.erase(itv);
    auto it=GateMap.find(p->getID());
    if(it!=GateMap.end())GateMap.erase(it);
    p->getFanin0()->delFanout(p);
    p->getFanin1()->delFanout(p);
    // delete p;
    return 0;
  }


  //const 1
  else if(((p->getFanin0()->getID()==0 && p->getInv0()==true) || (p->getFanin1()->getID()==0 && p->getInv1()==true))&& p->getTypeStr()=="Aig"){
    bool flag;
    for(unsigned i=0;i<p->getFanout().size();++i){
      if(p->getFanout()[i]->getFanin0()->getID()==p->getID()){
        if(p->getFanin1()->getID()!=0){
          flag=1;
          p->getFanout()[i]->setFanin0(p->getFanin1(),p->getFanout()[i]->getInv0()^p->getInv1());
          p->getFanin1()->pushFanout(p->getFanout()[i]);
          }
        else if(p->getFanin0()->getID()!=0){
          flag=0;
          p->getFanout()[i]->setFanin0(p->getFanin0(),p->getFanout()[i]->getInv0()^p->getInv0());
          p->getFanin0()->pushFanout(p->getFanout()[i]);
          }
          else{
            flag=0;
            p->getFanout()[i]->setFanin0(const0,p->getFanout()[i]->getInv0()^p->getInv1());
            p->getFanout()[i]->setFanin1(const0,p->getFanout()[i]->getInv1()^p->getInv1());
            p->getFanin0()->pushFanout(p->getFanout()[i]);
          }
      }
       else if(p->getFanout()[i]->getFanin1()->getID()==p->getID()){
        if(p->getFanin1()->getID()!=0){
          flag=1;
          p->getFanout()[i]->setFanin1(p->getFanin1(),p->getFanout()[i]->getInv1()^p->getInv1());
          p->getFanin1()->pushFanout(p->getFanout()[i]);
          }
        else if(p->getFanin0()->getID()!=0){
          flag=0;
          p->getFanout()[i]->setFanin1(p->getFanin0(),p->getFanout()[i]->getInv1()^p->getInv0());
          p->getFanin0()->pushFanout(p->getFanout()[i]);
          }
          else{
            flag=0;
            p->getFanout()[i]->setFanin0(const0,p->getFanout()[i]->getInv0()^p->getInv1());
            p->getFanout()[i]->setFanin1(const0,p->getFanout()[i]->getInv1()^p->getInv1());
            p->getFanin0()->pushFanout(p->getFanout()[i]);}
      }
    }
    if(flag){
      if(!p->getInv1())cout<<"Simplifying: "<<p->getFanin1()->getID()<<" merging "<<p->getID()<<"..."<<endl;
      else if(p->getInv1())cout<<"Simplifying: "<<p->getFanin1()->getID()<<" merging !"<<p->getID()<<"..."<<endl;
    }
    else{
      if(!p->getInv0())cout<<"Simplifying: "<<p->getFanin0()->getID()<<" merging "<<p->getID()<<"..."<<endl;
      else if(p->getInv0())cout<<"Simplifying: "<<p->getFanin0()->getID()<<" merging !"<<p->getID()<<"..."<<endl;
    }
    vector<CirGate*>::iterator itv;
    itv=find(_aiglist.begin(),_aiglist.end(),p);
    _aiglist.erase(itv);
    auto it=GateMap.find(p->getID());
    if(it!=GateMap.end())GateMap.erase(it);
    p->getFanin0()->delFanout(p);
    p->getFanin1()->delFanout(p);
    // delete p;
    return 0;
  }



  //both fanin are the same
  else if(p->getFanin0()->getID()==p->getFanin1()->getID() && p->getInv0()==p->getInv1()){
    for(unsigned i=0;i<p->getFanout().size();++i){
      if(p->getFanout()[i]->getTypeStr()=="Po"){
        p->getFanout()[i]->setFanin0(p->getFanin0(),p->getInv0()^p->getFanout()[i]->getInv0());
        p->getFanin0()->pushFanout(p->getFanout()[i]);
      }
      else if(p->getFanout()[i]->getFanin0()->getID()!=p->getID()){
        p->getFanout()[i]->setFanin1(p->getFanin1(),p->getInv1()^p->getFanout()[i]->getInv1());
        p->getFanin0()->pushFanout(p->getFanout()[i]);
      }
     else if (p->getFanout()[i]->getFanin1()->getID()!=p->getID()){
        p->getFanout()[i]->setFanin0(p->getFanin0(),p->getInv0()^p->getFanout()[i]->getInv0());
        p->getFanin0()->pushFanout(p->getFanout()[i]);
      }
    
    else{
      p->getFanout()[i]->setFanin0(p->getFanin0(),p->getInv0()^p->getFanout()[i]->getInv0());
      p->getFanout()[i]->setFanin1(p->getFanin1(),p->getInv1()^p->getFanout()[i]->getInv1());
      p->getFanin0()->pushFanout(p->getFanout()[i]);
    }
    }
    if(!p->getInv0())cout<<"Simplifying: "<<p->getFanin1()->getID()<<" merging "<<p->getID()<<"..."<<endl;
    else if(p->getInv0())cout<<"Simplifying: "<<p->getFanin1()->getID()<<" merging !"<<p->getID()<<"..."<<endl;
    vector<CirGate*>::iterator itv;
    itv=find(_aiglist.begin(),_aiglist.end(),p);
    _aiglist.erase(itv);
    auto it=GateMap.find(p->getID());
    if(it!=GateMap.end())GateMap.erase(it);
    p->getFanin0()->delFanout(p);
    p->getFanin1()->delFanout(p);
    // delete p;
    return 0;
  }


  //the one fanin is the inverse of the other
  else if(p->getFanin0()->getID()==p->getFanin1()->getID() && p->getInv0()!=p->getInv1()){
    for(unsigned i=0;i<p->getFanout().size();++i){
      if(p->getFanout()[i]->getFanin0()->getID()==p->getID()){
        p->getFanout()[i]->setFanin0(const0,p->getFanout()[i]->getInv0());
        const0->pushFanout(p->getFanout()[i]);
      }
      else if (p->getFanout()[i]->getFanin1()->getID()==p->getID()){
        p->getFanout()[i]->setFanin1(const0,p->getFanout()[i]->getInv1());
        const0->pushFanout(p->getFanout()[i]);
      }
    }
    cout<<"Simplifying: 0 merging "<<p->getID()<<"..."<<endl;
     vector<CirGate*>::iterator itv;
    itv=find(_aiglist.begin(),_aiglist.end(),p);
    _aiglist.erase(itv);
    auto it=GateMap.find(p->getID());
    GateMap.erase(it);
    p->getFanin0()->delFanout(p);
    p->getFanin1()->delFanout(p);
    // delete p;
    return 0;
  }
}