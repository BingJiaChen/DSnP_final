/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include<string>
#include<map>
#include<sstream>
#include<algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}
bool CirMgr::mysort(CirGate const* p1,CirGate const* p2){
   return (p1->getID()<p2->getID());
}

bool collectInput(vector<CirGate*> &Inp,CirGate* p,int &num){
   if(p==0){return 0;}
   if(p->checkprint()){return 0;}
   if(p->getTypeStr()=="Pi"){
      Inp.push_back(p);
      return 0;
   }
   collectInput(Inp,p->getFanin0(),num);
   collectInput(Inp,p->getFanin1(),num);
   if(p->checkprint()){return 0;}
   p->setprint();
   if(p->getTypeStr()=="Aig"){num++;}
}
bool mysort(CirGate* a,CirGate* b){
   return a->getID()<b->getID();
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   const0=new CirConstGate();
   flAIG=0;
   ifstream f(fileName.c_str());
   if(!f.is_open()){cout<<"Cannot open design \""<<fileName.c_str()<<"\"!!"<<endl;return false;}
   string line;
   getline(f,line);
   readHeader(line);
   for (int i=0;i<M;++i){
      _undef.push_back(make_pair(false,false));
   }   
   for(int i=0;i<I;i++){
      getline(f,line);
      readInput(line,i+2);
   }

   for(int i=0;i<O;i++){
      getline(f,line);
      readOutput(line,i+2+I);
   }
   for(int i=0;i<A;i++){
      getline(f,line);
      readAig(line,i+2+I+O);
   }
   while(getline(f,line)){
      if(line=="c"){break;}
      readSymbol(line);
   }
   connect();
   _hashset.clear();
   return true;
}


/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout<<endl;
   cout<<"Circuit Statistics"<<endl;
   cout<<"=================="<<endl;
   cout<<"  PI "<<setw(11)<<_pilist.size()<<endl;
   cout<<"  PO "<<setw(11)<<_polist.size()<<endl;
   cout<<"  AIG"<<setw(11)<<_aiglist.size()<<endl;
   cout<<"------------------"<<endl;
   cout<<"  Total"<<setw(9)<<_pilist.size()+_polist.size()+_aiglist.size()<<endl;
}

void
CirMgr::printNetlist() const
{
   printList.clear();
   for(unsigned i=0;i<_polist.size();++i){
     DFS(_polist[i]);
   }
   resetprint();
   cout<<endl;
   for(unsigned i=0;i<printList.size();++i){
      cout<<"["<<i<<"] "<<printList[i]<<endl;
   }
   printList.clear();
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit: ";
   for(unsigned i=0;i<_pilist.size();i++){
      if(i!=_pilist.size()-1)cout<<_pilist[i]->getID()<<" ";
      else{cout<<_pilist[i]->getID();}
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit: ";
   for(unsigned i=0;i<_polist.size();i++){
      if(i!=_polist.size()-1)cout<<_polist[i]->getID()<<" ";
      else{cout<<_polist[i]->getID();}
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if(_floatlist.size()==0){}
   else{cout<<"Gates with floating fanin(s):";
   // sort(_floatlist.begin(),_floatlist.end(),mysort);
   for(unsigned i=0;i<_floatlist.size();++i){
      cout<<" ";
      cout<<_floatlist[i];
   }
   cout<<endl;
   }
   bool flag=false;
   map<int,CirGate*>:: iterator it;
   for(it=GateMap.begin();it!=GateMap.end();++it){
      if(it->second->getFanout().size()==0 && it->second->getTypeStr()!="Po" && it->second->getID()!=0){
         if(!flag){cout<<"Gates defined but not used  :";flag=true;}
         cout<<" ";
         cout<<it->second->getID();
         }
   }
   if(flag)cout<<endl;

}



void
CirMgr::printFECPairs() const
{
   //TODO
   // cout<<_FECgrps.size()<<endl;
   // SortFec(_FECgrps);
   for(unsigned i=0;i<_FECgrps.size();i++){
      cout<<"["<<i<<"]";
      for(unsigned j=0;j<_FECgrps[i].size();++j){
         if(!_FECgrps[i][0]->checkrev()){
            if(_FECgrps[i][j]->checkrev()){cout<<" !"<<_FECgrps[i][j]->getID();}
            else{cout<<" "<<_FECgrps[i][j]->getID();}
         }
         else{
            if(_FECgrps[i][j]->checkrev()){cout<<" "<<_FECgrps[i][j]->getID();}
            else{cout<<" !"<<_FECgrps[i][j]->getID();}
         }
      }
      cout<<endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   for(unsigned i=0;i<_polist.size();++i){
     DFS(_polist[i]);
   }
   resetprint();
   outfile<<"aag "<<M<<" "<<I<<" 0 "<<O<<" "<<A-flAIG<<endl;
   for(unsigned i=0;i<_pilist.size();++i){
      outfile<<_pilist[i]->getID()*2<<endl;
   }
   for(unsigned i=0;i<_polist.size();++i){
      if(_polist[i]->getInv0()){outfile<<_polist[i]->getFanin0()->getID()*2+1<<endl;}
      else{outfile<<_polist[i]->getFanin0()->getID()*2<<endl;}
   }
   for(unsigned i=0;i<_polist.size();++i){
      writeDFS(_polist[i],outfile);
   }
   resetprint();
   for(unsigned i=0;i<_pilist.size();++i){
      if(_pilist[i]->getSymbol()!=""){outfile<<"i"<<i<<" "<<_pilist[i]->getSymbol()<<endl;}
   }
   for(unsigned i=0;i<_polist.size();++i){
      if(_polist[i]->getSymbol()!=""){outfile<<"o"<<i<<" "<<_polist[i]->getSymbol()<<endl;}
   }
   outfile<<"c"<<endl;
   outfile<<"AAG file output by Bing-Jia Chen."<<endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   //TODO
   vector<CirGate*> input;
   int num_aig=0;
   collectInput(input,g,num_aig);
   resetprint();
   outfile<<"aag "<<g->getID()<<" "<<input.size()<<" 0 1 "<<num_aig<<'\n';
   sort(input.begin(),input.end(),mysort);
   for(unsigned i=0;i<input.size();++i){
      outfile<<input[i]->getID()*2<<'\n';
   }
   outfile<<g->getID()*2<<'\n';
   writeDFS(g,outfile);
   resetprint();
   for(unsigned i=0;i<input.size();++i){
      if(input[i]->getSymbol()!=""){outfile<<"i"<<i<<" "<<input[i]->getSymbol()<<'\n';}
   }
   outfile<<"o0 "<<g->getID()<<'\n';
   outfile<<"c"<<'\n';
   outfile<<"write gate ("<<g->getID()<<") "<< "by Bing-Jia Chen."<<'\n';
}

/**************************************************************/
/*   class CirMgr private member functions for circuit construction   */
/**************************************************************/
void CirMgr::readHeader(string header)
{
   istringstream iss(header);
   stringstream ss;
   string s;
   for (int i=0 ; i<6 ; i++){
      getline(iss,s,' ');
      if(i==1){
         ss<<s;
         ss>>M;
      }
      else if(i==2){
         ss<<s;
         ss>>I;
      }
      else if(i==4){
         ss<<s;
         ss>>O;
      }
      else if(i==5){
         ss<<s;
         ss>>A;
      }
      ss.str("");
      ss.clear();
   }
}
void CirMgr::readInput(string input,int l)
{
   stringstream ss;
   int ID=0;
   ss<<input;
   ss>>ID;
   CirGate* pi=new CirPiGate(ID/2,l);
   GateMap[ID/2]=pi;
   _undef[ID/2].first=true;
   _pilist.push_back(pi);
   ss.str("");
   ss.clear();
}
void CirMgr::readOutput(string output,int l)
{
   stringstream ss;
   int fanin=0;
   ss<<output;
   ss>>fanin;
   bool inv=(fanin%2==1);
   _undef[fanin/2].second=true;
   CirGate* po=new CirPoGate(fanin/2,M+l-I-1,inv,l);
   GateMap[M+l-I-1]=po;
   _polist.push_back(po);
   ss.str("");
   ss.clear();
}
void CirMgr::readAig(string Aig,int l)
{
   istringstream iss(Aig);
   stringstream ss;
   string s;
   int ID,fanin0,fanin1;
   getline(iss,s,' ');
   ss<<s;
   ss>>ID;
   ss.str("");
   ss.clear();
   getline(iss,s,' ');
   ss<<s;
   ss>>fanin0;
   ss.str("");
   ss.clear();
   getline(iss,s,' ');
   ss<<s;
   ss>>fanin1;
   ss.str("");
   ss.clear();
   bool i1=(fanin0%2==1);
   bool i2=(fanin1%2==1);
   _undef[ID/2].first=true;
   if(fanin0/2!=0){_undef[fanin0/2].second=true;}
   if(fanin1/2!=0){_undef[fanin1/2].second=true;}
   CirGate* aig=new CirAigGate(fanin0/2,fanin1/2,ID/2,i1,i2,l);
   GateMap[ID/2]=aig;
   _aiglist.push_back(aig);
}
void CirMgr::readSymbol(string symbol)
{
   istringstream iss(symbol);
   stringstream ss;
   string s;
   int index;
   getline(iss,s,' ');
   if(s[0]=='i'){
      string  s1=s1.assign(s,1,s.size()-1);
      ss<<s1;
      ss>>index;
      getline(iss,s,' ');
      _pilist[index]->setSymbol(s);  
   }
   else if(s[0]=='o'){
      string  s1=s1.assign(s,1,s.size()-1);
      ss<<s1;
      ss>>index;
      getline(iss,s,' ');
      _polist[index]->setSymbol(s);  
   }
   ss.str("");
   ss.clear();
}


void CirMgr::connect()
{
   map<int,CirGate*>::iterator it;
   for (it=GateMap.begin();it!=GateMap.end();++it){
      if(it->second->getTypeStr()=="Pi"){}
      else if(it->second->getTypeStr()=="Po"){
         pair<int,bool>check0(it->second->getPair0());
         if(check0.first==0){it->second->setFanin0(const0,check0.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check0.first]==0){
               CirGate* p=new CirUndefGate(check0.first);
                it->second->setFanin0(p,check0.second);
                p->pushFanout(it->second);
                GateMap[check0.first]=p;
               _floatlist.push_back(it->second->getID());}
            else{it->second->setFanin0(GateMap[check0.first],check0.second);GateMap[check0.first]->pushFanout(it->second);}
            }
      }
      else if(it->second->getTypeStr()=="Aig"){
         pair<int,bool>check0(it->second->getPair0());
         pair<int,bool>check1(it->second->getPair1());
         if(check0.first==0){it->second->setFanin0(const0,check0.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check0.first]==0){
               CirGate* p=new CirUndefGate(check0.first);
                it->second->setFanin0(p,check0.second);
                p->pushFanout(it->second);
                GateMap[check0.first]=p;
               _floatlist.push_back(it->second->getID());}
            else{
               if(GateMap[check0.first]->getTypeStr()=="UNDEF")_floatlist.push_back(it->second->getID());
               it->second->setFanin0(GateMap[check0.first],check0.second);GateMap[check0.first]->pushFanout(it->second);
               }
            }
         if(check1.first==0){it->second->setFanin1(const0,check1.second);const0->pushFanout(it->second);}
         else{
            if(GateMap[check1.first]==0){
               CirGate* p=new CirUndefGate(check1.first);
                it->second->setFanin1(p,check1.second);
                p->pushFanout(it->second);
                GateMap[check1.first]=p;
               if(GateMap[check0.first]!=0)
                  {_floatlist.push_back(it->second->getID());}
                  }
            else{
               if(GateMap[check1.first]->getTypeStr()=="UNDEF")_floatlist.push_back(it->second->getID());
               it->second->setFanin1(GateMap[check1.first],check1.second);GateMap[check1.first]->pushFanout(it->second);}
            }
         
      }
   }
   
}

CirGate* CirMgr::DFS(CirGate* p) const
{
   flAIG=0;
   stringstream ss;
   string ID;
   if(p==0){return 0;}
   if(p->checkprint()){return 0;}
   DFS(p->getFanin0());
   DFS(p->getFanin1());
   if(p->checkprint()){return 0;}
   if(p->getTypeStr()=="Pi"){
      string str;
      ss<<p->getID();
      ss>>ID;
      if(p->getSymbol()!=""){str="PI  "+ID+" ("+p->getSymbol()+")";printList.push_back(str);}
      else{str="PI  "+ID;printList.push_back("PI  "+ID);}
      p->setprint();
      ss.str("");
      ss.clear();
      // return 0;
   }
   if(p->getTypeStr()=="Aig"){
      string str="";
      string ID;
      ss<<p->getID();
      ss>>ID;
      ss.str("");
      ss.clear();
      str="AIG "+ID;
      ss<<p->getFanin0()->getID();
      ss>>ID;
      if(p->getFanin0()->getTypeStr()=="UNDEF" && p->getInv0()){str=str+" *!"+ID;}
      else if(p->getInv0()){str=str+" !"+ID;}
      else if(p->getFanin0()->getTypeStr()=="UNDEF"){str=str+" *"+ID;}
      else{str=str+" "+ID;}
      ss.str("");
      ss.clear();
      ss<<p->getFanin1()->getID();
      ss>>ID;
      if(p->getFanin1()->getTypeStr()=="UNDEF" && p->getInv1()){str=str+" *!"+ID;}
      else if(p->getInv1()){str=str+" !"+ID;}
      else if(p->getFanin1()->getTypeStr()=="UNDEF"){str=str+" *"+ID;}
      else{str=str+" "+ID;}
      ss.str("");
      ss.clear();
      // cout<<"["<<i<<"] "<<str<<endl;
      printList.push_back(str);
      p->setprint();
      flAIG++;
      // return 0;
   }
   if(p->getTypeStr()=="Po"){
      string str="";
      ss<<p->getID();
      ss>>ID;
      ss.str("");
      ss.clear();
      str="PO  "+ID;
      ss<<p->getFanin0()->getID();
      ss>>ID;
      if(p->getFanin0()->getTypeStr()=="UNDEF" && p->getInv0()){str=str+" *!"+ID;}
      else if(p->getInv0()){str=str+" !"+ID;}
      else if(p->getFanin0()->getTypeStr()=="UNDEF"){str=str+" *"+ID;}
      else{str=str+" "+ID;}
      if(p->getSymbol()!=""){str+=" ("+p->getSymbol()+")";}
      // cout<<"["<<i<<"] "<<str<<endl;
      printList.push_back(str);
      ss.str("");
      ss.clear();
      p->setprint();
      // return 0;
   }
   if(p->getTypeStr()=="const"){
      printList.push_back("CONST0");
      // cout<<"["<<i<<"] "<<"CONST0"<<endl;
      p->setprint();
      // return 0;
   }
   if(p->getTypeStr()=="UNDEF"){
      // return 0;
   }

   }

void CirMgr::resetprint()const
{
   map<int,CirGate*>::iterator it;
   for(auto it=GateMap.begin();it!=GateMap.end();++it){
      it->second->resetprint();
   }
   const0->resetprint();
}

CirGate* CirMgr::writeDFS(CirGate* p,ostream& outfile)const{
   if(p==0){return 0;}
   if(p->checkprint()){return 0;}
   writeDFS(p->getFanin0(),outfile);
   writeDFS(p->getFanin1(),outfile);
   if(p->getTypeStr()=="Aig"){
      int F0,F1;
      if(p->getInv0()){F0=p->getFanin0()->getID()*2+1;}
      else{F0=p->getFanin0()->getID()*2;}
      if(p->getInv1()){F1=p->getFanin1()->getID()*2+1;}
      else{F1=p->getFanin1()->getID()*2;}
      p->setprint();
      outfile<<p->getID()*2<<" "<<F0<<" "<<F1<<endl;
   }
}
