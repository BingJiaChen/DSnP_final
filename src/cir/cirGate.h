/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include<iomanip>
#include<sstream>
#include <algorithm>
#include "cirDef.h"
#include "sat.h"
#include<bitset>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
  public:
   CirGate():_fanin0(0),_fanin1(0),_inv0(0),_inv1(0),lineNo(0),symbol(""),isprint(0),mustDel(0),value(0),isrev(0),candicate(0) {_fanout.clear();FECpair.clear();}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return type; }
   bool isAig(){return type=="Aig";}
   unsigned getLineNo() const { return lineNo; }
   void setSymbol(string str){symbol=str;}
   string getSymbol()const{return symbol;}
   void setLineNo(int No){lineNo=No;}
   void setType(string str){type=str;}
   CirGate* getFanin0()const{return _fanin0;}
   CirGate* getFanin1()const{return _fanin1;}
   void setFanin0(CirGate* p,bool i){_fanin0=p;_inv0=i;}
   void setFanin1(CirGate* p,bool i){_fanin1=p;_inv1=i;}
   bool getInv0()const{return _inv0;}
   bool getInv1()const{return _inv1;}
   void setprint()const{isprint=1;}
   void resetprint()const{isprint=0;}
   bool checkprint()const{return isprint;}
   bool checkrev()const{return isrev;}
   void setrev(){isrev=true;}
   void setDel(){mustDel=1;}
   bool checkDel(){return mustDel;}
   size_t getValue()const{return value;}
   void setValue(const size_t &p){value=p;};
   void setVar(Var &v){var=v;}
   void setCan(){candicate=true;}
   bool checkCan(){return candicate;}
   Var getVar(){return var;}
   size_t cal();
   // Printing functions
   virtual void printGate() const{} ;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   CirGate* DFS(const CirGate* p,int level,int i,bool inv) const;
   CirGate* revDFS(const CirGate* p,int level,int i,bool inv) const;
   CirGate* rprint(const CirGate* p,int level)const;
   CirGate* revrprint(const CirGate*,int level)const;
   void pushFanout(CirGate* p){
     auto it=find(_fanout.begin(),_fanout.end(),p);
     if(it==_fanout.end())_fanout.push_back(p);
     }
    void setFECpair(vector<CirGate*>& a){FECpair=a;};
    vector<CirGate*> getFECpair()const{return FECpair;}
    void delFECpair(CirGate*p){
      auto it=find(FECpair.begin(),FECpair.end(),p);
      if(it!=FECpair.end())FECpair.erase(it);
    }
   vector<CirGate*> getFanout()const{return _fanout;}
   void delFanout(CirGate* p){auto it=find(_fanout.begin(),_fanout.end(),p);if(it!=_fanout.end())_fanout.erase(it);}
   //helper function
   virtual int getID() const{};
   virtual pair<int,bool> getPair0() const{};
   virtual pair<int,bool> getPair1() const{};
private:
string                             type;
string                             symbol;
unsigned                     lineNo;
vector<CirGate*>      _fanout;
CirGate*                       _fanin0;
CirGate*                      _fanin1;
bool                              _inv0;
bool                              _inv1;
mutable bool                              isprint;
bool                              mustDel;
size_t                            value;
bool                              isrev;
bool                              candicate;
Var                                 var;
vector<CirGate*>    FECpair;
protected:

};


class CirPiGate:public CirGate
{
  public:
      CirPiGate(int ID,int No):gateID(ID){setLineNo(No);setType("Pi");}
      ~CirPiGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"==============================================================================="<<endl;
        if(getSymbol()==""){str="= PI(" + ID + "), line "+No;}
        else{str="= PI(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<str;
        cout<<endl;
        cout<<"= FECs:"<<endl;
        cout<<"= Value: ";
        string bin=bitset<64>(getValue()).to_string();
        for(unsigned i=0;i<bin.size();++i){
          if(i%8==0 && i!=0){cout<<"_";}
          cout<<bin[i];
        }
        cout<<endl;
        cout<<"==============================================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};

class CirPoGate:public CirGate
{
  public:
      CirPoGate(int f,int ID,bool i,int No):gateID(ID){
        setLineNo(No);
        setType("Po");
        _fanin0.first=f;
        _fanin0.second=i;
        }
      ~CirPoGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"==============================================================================="<<endl;
        if(getSymbol()==""){str="= PO(" + ID + "), line "+No;}
        else{str="= PO(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<str;
        cout<<endl;
        cout<<"= FECs:"<<endl;
        cout<<"= Value: ";
        string bin=bitset<64>(getValue()).to_string();
        for(unsigned i=0;i<bin.size();++i){
          if(i%8==0 && i!=0){cout<<"_";}
          cout<<bin[i];
        }
        cout<<endl;
        cout<<"==============================================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{ return _fanin0;}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}
  private:
  int               gateID;
  pair<int,bool>        _fanin0;

};

class CirAigGate:public CirGate
{
  public:
      CirAigGate(int f0,int f1,int ID,bool i1,bool i2,int No):gateID(ID){
        setLineNo(No);
        setType("Aig");
        _fanin0.first=f0;
        _fanin0.second=i1;
        _fanin1.first=f1;
        _fanin1.second=i2;
        }
      ~CirAigGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"==============================================================================="<<endl;
        if(getSymbol()==""){str="= AIG(" + ID + "), line "+No;}
        else{str="= AIG(" + ID + ")\""+getSymbol()+"\", line "+No;}
        cout<<str;
        cout<<endl;
        cout<<"= FECs:";
        for(unsigned i=0;i<getFECpair().size();++i){
          if(getFECpair()[i]->getID()!=getID()){
            if(!getFECpair()[i]->checkrev() ^ checkrev()){cout<<" "<< getFECpair()[i]->getID();}
            else{cout<<" !"<<getFECpair()[i]->getID();}
          }
        }
        cout<<endl;
        cout<<"= Value: ";
        string bin=bitset<64>(getValue()).to_string();
        for(unsigned i=0;i<bin.size();++i){
          if(i%8==0 && i!=0){cout<<"_";}
          cout<<bin[i];
        }
        cout<<endl;
        cout<<"==============================================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return _fanin0;}
      virtual pair<int,bool> getPair1() const{return _fanin1;}
  private:
  pair<int,bool>                 _fanin0;
  pair<int,bool>                 _fanin1;
  int                 gateID;
};

class CirUndefGate:public CirGate
{
  public:
      CirUndefGate(int ID):gateID(ID){setLineNo(0);setType("UNDEF");}
      ~CirUndefGate(){}
      virtual void printGate()const{
        stringstream ss;
        string ID,No;
        ss<<gateID;
        ss>>ID;
        ss.str("");
        ss.clear();
        ss<<getLineNo();
        ss>>No;
        string str="";
        cout<<"==============================================================================="<<endl;
        if(getSymbol()==""){str="= UNDEF(" + ID + "), line "+"0";}
        else{str="= UNDEF(" + ID + ")\""+getSymbol()+"\", line "+"0";}
        cout<<str;
        cout<<endl;
        cout<<"= FECs:"<<endl;
        cout<<"= Value: ";
        string bin=bitset<64>(getValue()).to_string();
        for(unsigned i=0;i<bin.size();++i){
          if(i%8==0 && i!=0){cout<<"_";}
          cout<<bin[i];
        }
        cout<<endl;
        cout<<"==============================================================================="<<endl;
        ss.str("");
        ss.clear();
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};

class CirConstGate:public CirGate
{
  public:
      CirConstGate():gateID(0){setType("const");}
      ~CirConstGate(){}
      virtual void printGate()const{
        string str;
        cout<<"==============================================================================="<<endl;
        if(getSymbol()==""){str="= CONST(0), line 0";}
        cout<<str;
        cout<<endl;
        cout<<"= FECs:";
        for(unsigned i=0;i<getFECpair().size();++i){
          if(getFECpair()[i]->getID()!=getID()){
            if(!getFECpair()[i]->checkrev() ^ checkrev()){cout<<" "<< getFECpair()[i]->getID();}
            else{cout<<" !"<<getFECpair()[i]->getID();}
          }
        }
        cout<<endl;
        cout<<"= Value: ";
        string bin=bitset<64>(getValue()).to_string();
        for(unsigned i=0;i<bin.size();++i){
          if(i%8==0 && i!=0){cout<<"_";}
          cout<<bin[i];
        }
        cout<<endl;
        cout<<"==============================================================================="<<endl;
      }
      virtual int getID()const{return gateID;}
      virtual pair<int,bool> getPair0() const{return pair<int,bool>(0,0);}
      virtual pair<int,bool> getPair1() const{return pair<int,bool>(0,0);}

  private:
  int                 gateID;

};
#endif // CIR_GATE_H
