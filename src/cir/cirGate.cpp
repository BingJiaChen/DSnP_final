/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   printGate();
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   DFS((this),level,0,0);
   rprint(this,level);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   revDFS(this,level,0,0);
   revrprint(this,level);
}

CirGate* CirGate::DFS(const CirGate* p,int level,int i,bool inv) const{
   if(p==0 || level<0){return 0;}
   if(p->getTypeStr()=="Po"){
      cout<<"PO "<<p->getID()<<endl;
   }
   if(p->getTypeStr()=="Pi"){
      if(inv){cout<<setw(i)<<""<<"!PI "<<p->getID()<<endl;}
      else{cout<<setw(i)<<""<<"PI "<<p->getID()<<endl;}
   }
   if(p->getTypeStr()=="Aig"){
      if(inv && p->checkprint() && level!=0){cout<<setw(i)<<""<<"!AIG "<<p->getID()<<" (*)"<<endl;return 0;}
      else if(inv){cout<<setw(i)<<""<<"!AIG "<<p->getID()<<endl;}
      else if(p->checkprint() && level!=0){cout<<setw(i)<<""<<"AIG "<<p->getID()<<" (*)"<<endl;return 0;}
      else{cout<<setw(i)<<""<<"AIG "<<p->getID()<<endl;}
      if(level!=0)p->setprint();
   }
   if(p->getTypeStr()=="UNDEF"){
      if(inv){cout<<setw(i)<<""<<"!UNDEF "<<p->getID()<<endl;}
      else{cout<<setw(i)<<""<<"UNDEF "<<p->getID()<<endl;}
   }
   if(p->getTypeStr()=="const"){
      if(inv){cout<<setw(i)<<""<<"!CONST 0 "<<endl;}
      else{cout<<setw(i)<<""<<"CONST 0 "<<endl;}
   }
   DFS(p->getFanin0(),level-1,i+2,p->getInv0());
   DFS(p->getFanin1(),level-1,i+2,p->getInv1());

}

CirGate* CirGate::rprint(const CirGate* p,int level)const{
   if(p==0 || level<0){return 0;}
   p->resetprint();
   rprint(p->getFanin0(),level-1);
   rprint(p->getFanin1(),level-1);
}

CirGate* CirGate::revDFS(const CirGate* p,int level,int i,bool inv) const{
   if(level<0){return 0;}
   if(p->getTypeStr()=="Po"){
      if(inv){cout<<setw(i)<<""<<"!PO "<<p->getID()<<endl;}
      else{cout<<setw(i)<<""<<"PO "<<p->getID()<<endl;}
      return 0;
   }
   if(p->getTypeStr()=="Pi"){
      if(inv){cout<<"!PI "<<p->getID()<<endl;}
      else{cout<<"PI "<<p->getID()<<endl;}
   }
   if(p->getTypeStr()=="Aig"){
      if(inv && p->checkprint() && level!=0){cout<<setw(i)<<""<<"!AIG "<<p->getID()<<" (*)"<<endl;return 0;}
      else if(inv){cout<<setw(i)<<""<<"!AIG "<<p->getID()<<endl;}
      else if(p->checkprint() && level!=0){cout<<setw(i)<<""<<"AIG "<<p->getID()<<" (*)"<<endl;return 0;}
      else{cout<<setw(i)<<""<<"AIG "<<p->getID()<<endl;}
      if(level!=0 && p->getFanout().size()!=0)p->setprint();
   }
   if(p->getTypeStr()=="UNDEF"){
      if(inv){cout<<setw(i)<<""<<"!UNDEF "<<p->getID()<<endl;}
      else{cout<<setw(i)<<""<<"UNDEF "<<p->getID()<<endl;}
   }
   if(p->getTypeStr()=="const"){
      if(inv){cout<<setw(i)<<""<<"!CONST 0 "<<endl;}
      else{cout<<setw(i)<<""<<"CONST 0 "<<endl;}
   }
   for(unsigned j=0;j<p->getFanout().size();++j){
      bool _inv;
      if(p->getFanout()[j]->getFanin0()==p){_inv=p->getFanout()[j]->getInv0();}
      else if(p->getFanout()[j]->getFanin1()==p){_inv=p->getFanout()[j]->getInv1();}
      revDFS(p->getFanout()[j],level-1,i+2,_inv);
   }
}

CirGate* CirGate::revrprint(const CirGate* p,int level)const{
   if(p->getFanout().size()==0 || level<0){return 0;}
   p->resetprint();
   for(unsigned j=0;j<p->getFanout().size();++j){
      revrprint(p->getFanout()[j],level-1);
   }
}


