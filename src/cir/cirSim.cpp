/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include<unordered_map>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "math.h"
#include<string>
#include<bitset>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
size_t
CirGate::cal(){
  size_t result=0;
  if(this->getInv0() && !this->getInv1()){
    result=(~this->getFanin0()->getValue()) & (this->getFanin1()->getValue());
  }
  else if(!this->getInv0() && this->getInv1()){
    result=(~this->getFanin1()->getValue()) & (this->getFanin0()->getValue());
  }
  else if(this->getInv0() && this->getInv1()){
    result=(~this->getFanin0()->getValue()) & (~this->getFanin1()->getValue());
  }
  else{
    result=(this->getFanin0()->getValue()) & (this->getFanin1()->getValue());
  }
  this->value=result;
  return result;
}

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

bool FECgrpSort(const vector<CirGate*> &a,const vector<CirGate*> &b){
   return a[0]->getID() < b[0]->getID();}

bool FECsort(CirGate* a,CirGate* b){
  return a->getID()<b->getID();
}

void insertNode(unordered_map<size_t,vector<CirGate*>>  & _FECgrp,size_t key,CirGate* tmp){
  if(_FECgrp.find(key)==_FECgrp.end()){
    vector<CirGate*> n;
    n.push_back(tmp);
    _FECgrp.insert({{key,n}});
  }
  else{
    _FECgrp.find(key)->second.push_back(tmp);
  }
}

bool initFEC(unordered_map<size_t,vector<CirGate*>>& _FECgrp,CirGate* p){
  if(p==0){return 0;}
  if(p->getTypeStr()!="Aig" ){return 0;}
  if(p->checkprint()){return 0;}
  initFEC(_FECgrp,p->getFanin0());
  initFEC(_FECgrp,p->getFanin1());
  if(p==0){return 0;}
  if(p->checkprint()){return 0;}
  p->setprint();
  insertNode(_FECgrp,p->getValue(),p);
}
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  int flag=0;
  int pat_num=0;
  int grp_num=0;
  _DFSlist.clear();
  for(unsigned i=0;i<_polist.size();++i){
    simDFS(_polist[i]->getFanin0());
  }
  resetprint();
  unordered_map<size_t,vector<CirGate*>>  _FECgrp;
  for(unsigned i=0;i<_polist.size();++i){
    initFEC(_FECgrp,_polist[i]->getFanin0());
  }
  insertNode(_FECgrp,0,const0);
  resetprint();
  _fecset.push_back(_FECgrp);
  while(true){
    simrand();
    unsigned s=_fecset.size();
    for(unsigned i=0;i<s;++i){
      for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
        if(it->second.size()>1){
          unordered_map<size_t,vector<CirGate*>>          _FECgrp;
          // for(auto itv=rec[j].begin();itv!=rec[j].end();++itv){cout<<itv->getNode()->getID()<<" ";}
          // cout<<endl;
          grp_num+=1;
          for(unsigned i=0;i<it->second.size();++i){
            if(it->second[i]->getValue()>~size_t(0)/2){insertNode(_FECgrp,~it->second[i]->getValue(),it->second[i]);if(!getIssim())it->second[i]->setrev();}
            else{insertNode(_FECgrp,it->second[i]->getValue(),it->second[i]);}
          }
          _fecset.push_back(_FECgrp);
        }
      }
    }
    for(unsigned k=0;k<s;k++){_fecset[k].clear();}
    _fecset.erase(_fecset.begin(),_fecset.begin()+s);
    if(_fecset.size()==s){flag+=1;}
    else{flag=0;}
    if(flag>=5 || pat_num>grp_num*200){break;}
    grp_num=0;
    pat_num+=64;
    setIssim();
  }
  cout<<pat_num<<" patterns simulated."<<endl;
   _FECgrps.clear();
   for(unsigned i=0;i<_fecset.size();i++){
      for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
         if(it->second.size()>1){
            sort(it->second.begin(),it->second.end(),FECsort);
            for(unsigned z=0;z<it->second.size();++z){it->second[z]->setFECpair(it->second);}
            _FECgrps.push_back(it->second);
         }
      }
   }
   sort(_FECgrps.begin(),_FECgrps.end(),FECgrpSort);
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  string signal="";
  bool flag=0;
  size_t len=_pilist.size();
  size_t input[len+1]={0};
  size_t count=0;
  size_t _=64;
  _DFSlist.clear();
  for(unsigned i=0;i<_polist.size();++i){
    simDFS(_polist[i]->getFanin0());
  }
  resetprint();
  unordered_map<size_t,vector<CirGate*>>  _FECgrp;
  for(unsigned i=0;i<_polist.size();++i){
    initFEC(_FECgrp,_polist[i]->getFanin0());
  }
  insertNode(_FECgrp,0,const0);
  resetprint();
  _fecset.push_back(_FECgrp);
  while(patternFile>>signal){
    if(flag){break;}
    if(signal.size()==0){}
    else if(signal.size()!=len){
      cout<<endl;
      cout<<"Error: Pattern("<<signal<<") length("<<signal.size()<<") does not match the number of inputs("<<len<<") in a circuit!!"<<endl;
      break;
    }
    for(unsigned i=0;i<signal.size();++i){
      if(count%64==0){
        if(signal[i]=='0'){input[i]=size_t(0);}
        else if(signal[i]=='1'){input[i]=size_t(1);}
        else{cout<<endl;cout<<"Error: Pattern("<<signal<<") contains a non-0/1 character(‘"<<signal[i]<<"’)."<<endl;flag=1; break;}
      }
      else{
        if(signal[i]=='0'){input[i]=size_t(input[i]<<1)+0;}
        else if(signal[i]=='1'){input[i]=size_t(input[i]<<1)+1;}
        else{cout<<endl;cout<<"Error: Pattern("<<signal<<") contains a non-0/1 character(‘"<<signal[i]<<"’)."<<endl;flag=1; break;}
      }
    }
    count++;
    if(count%64==0){
      simfile(input,count);
      input[len+1]={0};
      if(_simLog!=0){writeLog(*_simLog,count);}
    }
  }
  if(!flag && count%64!=0){
    for(size_t i=0;i<64-count%64;++i){
      for(size_t j=0;j<len;++j){
        input[j]=input[j]<<1;
      }
    }
    if(count<64){simfile(input,_);if(_simLog!=0){writeLog(*_simLog,count);}}
    else{simfile(input,count);if(_simLog!=0){writeLog(*_simLog,count);}}
  }
  if(!flag){
    cout<<count<<" patterns simulated."<<endl;
    _FECgrps.clear();
    for(unsigned i=0;i<_fecset.size();i++){
      for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
         if(it->second.size()>1){
            sort(it->second.begin(),it->second.end(),FECsort);
            for(unsigned z=0;z<it->second.size();++z){it->second[z]->setFECpair(it->second);}
            _FECgrps.push_back(it->second);
         }
      }
   }
   sort(_FECgrps.begin(),_FECgrps.end(),FECgrpSort);
    }
  else{cout<<"0 patterns simulated."<<endl;}
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
size_t
CirMgr::rand64(){
  size_t r=rnGen(256);
  for(int i=0;i<7;++i){
    r=r<<8;
    r+=rnGen(256);
  }
  return r;
}

size_t rand8(){
  return rnGen(256);
}

CirGate* 
CirMgr::simDFS(CirGate* p){
  if(p==0){return 0;}
  if(p->getTypeStr()!="Aig"){return 0;}
  if(p->checkprint()){return 0;}
  simDFS(p->getFanin0());
  simDFS(p->getFanin1());
  if(p==0){return 0;}
  if(p->getTypeStr()!="Aig"){return 0;}
  if(p->checkprint()){return 0;}
  p->setprint();
  _DFSlist.push_back(p);
}

void
CirMgr::simrand(){
  for(unsigned i=0;i<_pilist.size();++i){
    _pilist[i]->setValue(rand64()); //change 8 to 64
  }
  
  for(unsigned i=0;i<_DFSlist.size();++i){
    _DFSlist[i]->cal();
  }
  for(unsigned i=0;i<_polist.size();i++){
    _polist[i]->setValue(_polist[i]->getFanin0()->getValue());
  }
  // for(unsigned i=0;i<_aiglist.size();++i){
  //   cout<<"["<<_aiglist[i]->getID()<<"] "<<_aiglist[i]->getValue()<<" ("<<_aiglist[i]->getFanin0()->getID()<<") "<<_aiglist[i]->getFanin0()->getValue()<<" ("<<_aiglist[i]->getFanin1()->getID()<<") "<<_aiglist[i]->getFanin1()->getValue()<<endl;
  // }
}

void
CirMgr::simfile(size_t a[],size_t &count){
  for(unsigned i=0;i<_pilist.size();++i){
    _pilist[i]->setValue(a[i]); //change 8 to 64
  }
  for(unsigned i=0;i<_DFSlist.size();++i){
    _DFSlist[i]->cal();
  }
  for(unsigned i=0;i<_polist.size();i++){
    _polist[i]->setValue(_polist[i]->getFanin0()->getValue());
  }
  // for(unsigned i=0;i<_aiglist.size();++i){
  //   cout<<"["<<_aiglist[i]->getID()<<"] "<<_aiglist[i]->getValue()<<" ("<<_aiglist[i]->getFanin0()->getID()<<") "<<_aiglist[i]->getFanin0()->getValue()<<" ("<<_aiglist[i]->getFanin1()->getID()<<") "<<_aiglist[i]->getFanin1()->getValue()<<endl;
  // }
  unsigned s=_fecset.size();
  for(unsigned i=0;i<s;++i){
      for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
        if(it->second.size()>1){
          unordered_map<size_t,vector<CirGate*>>  _FECgrp;
          // for(auto itv=rec[j].begin();itv!=rec[j].end();++itv){cout<<itv->getNode()->getID()<<" ";}
          // cout<<endl;
          for(unsigned i=0;i<it->second.size();++i){
            if(it->second[i]->getValue()>~size_t(0)/2){insertNode(_FECgrp,~it->second[i]->getValue(),it->second[i]);if(!getIssim())it->second[i]->setrev();}
            else{insertNode(_FECgrp,it->second[i]->getValue(),it->second[i]);}
          }
          _fecset.push_back(_FECgrp);
        }
      }

    }
  for(unsigned k=0;k<s;k++){_fecset[k].clear();}
  _fecset.erase(_fecset.begin(),_fecset.begin()+s);
  setIssim();
}

void
CirMgr::writeLog(ostream &Log,size_t &count){
  vector<string> _piValue;
  vector<string> _poValue;
  for(unsigned i=0;i<_pilist.size();++i){
    string bin=bitset<64>(_pilist[i]->getValue()).to_string();
    _piValue.push_back(bin);
  }
  for(unsigned i=0;i<_polist.size();++i){
    string bin=bitset<64>(_polist[i]->getValue()).to_string();
    _poValue.push_back(bin);
  }
  if(count%64==0){
    for(int _=0;_<64;_++){
      for(unsigned i=0;i<_piValue.size();++i){
        Log<<_piValue[i][_];
      }
      Log<<" ";
      for(unsigned i=0;i<_poValue.size();++i){
        Log<<_poValue[i][_];
      }
      Log<<'\n';
    }
  }
  else{
    for(int _=0;_<count%64;_++){
      for(unsigned i=0;i<_piValue.size();++i){
        Log<<_piValue[i][_];
      }
      Log<<" ";
      for(unsigned i=0;i<_poValue.size();++i){
        Log<<_poValue[i][_];
      }
      Log<<'\n';
    }
  }
}

void
CirMgr::simFraig(size_t a[]){
  int num=0;
  _DFSlist.clear();
  for(unsigned i=0;i<_polist.size();++i){
    simDFS(_polist[i]->getFanin0());
  }
  resetprint();
  for(unsigned i=0;i<_pilist.size();++i){
    _pilist[i]->setValue(a[i]); 
  }
  for(unsigned i=0;i<_DFSlist.size();++i){
    _DFSlist[i]->cal();
  }
  for(unsigned i=0;i<_polist.size();i++){
    _polist[i]->setValue(_polist[i]->getFanin0()->getValue());
  }
  unsigned s=_fecset.size();
  for(unsigned i=0;i<s;++i){
      for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
        if(it->second.size()>1){
          unordered_map<size_t,vector<CirGate*>>  _FECgrp;
          // for(auto itv=rec[j].begin();itv!=rec[j].end();++itv){cout<<itv->getNode()->getID()<<" ";}
          // cout<<endl;
          for(unsigned i=0;i<it->second.size();++i){
            if(it->second[i]->getValue()>~size_t(0)/2){insertNode(_FECgrp,~it->second[i]->getValue(),it->second[i]);}
            else{insertNode(_FECgrp,it->second[i]->getValue(),it->second[i]);}
          }
          _fecset.push_back(_FECgrp);
        }
      }

    }
  for(unsigned k=0;k<s;k++){_fecset[k].clear();}
  _fecset.erase(_fecset.begin(),_fecset.begin()+s);
  for(unsigned i=0;i<_fecset.size();++i){
    for(auto it=_fecset[i].begin();it!=_fecset[i].end();++it){
      if(it->second.size()>1){
        num++;
        for(unsigned z=0;z<it->second.size();z++){it->second[z]->setFECpair(it->second);}
      }
    }
  }
  cout<<"Updating by SAT... #Toltal group: "<<num<<endl;
}

