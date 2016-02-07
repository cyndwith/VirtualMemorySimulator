#ifndef MAINRUN_H_INCLUDED
#define MAINRUN_H_INCLUDED
#include "virtualProgram.h"
#include "iL1Cache.h"
#include "dL1Cache.h"
#include "L2Cache.h"
#include "statistics.h"
#include "randomFunction.h"

int run()
{
  // Initialize the PC counter to random 48 bit virtual address + offset
  long long int PC;
  long long int iTLB_Block;
  long long int dTLB_Block;
  long long int TLB_Block;
  long long int currOpr;
  int branchTest;
  //Bottom Up approach
  inst currInst;
  iTLB readiTLB;
  VPT readVPT;
  iL1Cache iL1Block;
  VL3Cache readVL3Cache;
  VL3Cache::VL3Block readVL3;
  //Data Blocks
  dTLB readdTLB;
  TLB readTLB;
  //VPT common for Instruction and Data
  dL1Cache dL1Block;
  L2Cache readL2Cache;
  L2Cache::L2Block readL2;

  PC = randomBits(48);
  for(int i=0;i<1000;i++)
  {
    //PC = randomBits(48);//PC = (PC>>12);
    cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;;
    cout<<"% | CLK:"<<cycle<<"\t\t | iL1_Misses:"<<iL1Cache_Misses<<"\t\t | dL1_Misses:"<<dL1Cache_Misses<<endl;
    cout<<"% | PF:"<<page_Fault<<"\t\t | L2_Misses:"<<L2Cache_Misses<<"\t\t | L3_Misses:"<<L3Cache_Misses<<endl;
    cout<<"% | CPI:"<<CPI<<"\t\t | No. Instruction :"<<noInstruction<<"\t\t | iL1_Miss_rate:"<<iL1_Miss_Rate<<endl;
    cout<<"% | dL1_Miss_rate:"<<dL1_Miss_Rate<<"\t\t | L2_Miss_rate:"<<L2_Miss_Rate<<"\t\t | L3_Miss_rate:"<<L3_Miss_Rate<<endl;
    cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%<<iTLB>>%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    //iTLB_Block = readiTLB.lookup(PC);
    iTLB_Block = readiTLB.lookup(PC);
    if(iTLB_Block == -1)
    {
        TLB_Block = readTLB.lookup(PC);
        if(TLB_Block==-1)
            TLB_Block = readTLB.allocBlock(readVPT.lookup(PC),PC);//cout<<"\nAllocated Block in iTLB!"<<endl;
        iTLB_Block = readiTLB.allocBlock(TLB_Block,PC);
        //iTLB_Block = readiTLB.allocBlock(readVPT.lookup(PC)); //cout<<"\nAllocated Block in iTLB!"<<endl;
    }
    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;;
    cout<<"\n\t%%%%%%%%%%%%%%%%%%<<iL1 Cache>>%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    currInst = iL1Block.lookup(PC,iTLB_Block);
    if(currInst.opCode == -1)
    {
        long long int phyAdd = (((iTLB_Block&0xFFFFFF)<<12)|(PC & 0xFFF));
        readL2 = readL2Cache.lookup(phyAdd);
        if(readL2.L2Tag == -1)
        {
            readVL3 = readVL3Cache.lookup(phyAdd);
            readL2 = readL2Cache.allocBlock(readVL3,phyAdd);
        }
        currInst=iL1Block.allocBlock(readL2,PC,phyAdd);//iTLB_Block);
        /*
        readVL3 = readVL3Cache.lookup(phyAdd);
        currInst = iL1Block.allocBlock(readVL3,PC);
        */
    }
    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    for(int i=0;i<currInst.oprCount;i++)
    {
            cout<<"\n\t%%%%%%%%%%%%%%%<<dTLB Cache>>%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            cout<<"\n\tEntered Operands fetching loop!"<<endl;
            dTLB_Block = readdTLB.lookup(currInst.oprAdd[i]);
            if(dTLB_Block == -1)
            {
                //dTLB_Block = readdTLB.allocBlock(readVPT.lookup(currInst.oprAdd[i]),PC); //cout<<"\nAllocated Block in iTLB!"<<endl;
                long long int oprVA = currInst.oprAdd[i];
                TLB_Block = readTLB.lookup(oprVA);
                if(TLB_Block==-1)
                    TLB_Block = readTLB.allocBlock(readVPT.lookup(oprVA),oprVA);//cout<<"\nAllocated Block in iTLB!"<<endl;
                dTLB_Block = readdTLB.allocBlock(TLB_Block,oprVA);
            }
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%<<dL1 Cache>>%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            currOpr = dL1Block.lookup(currInst.oprAdd[i],dTLB_Block);
            if(currOpr==-1)
            {
                long long int oprVA = currInst.oprAdd[i];
                long long int phyAdd = (((dTLB_Block&0xFFFFFF)<<12)|(PC & 0xFFF));
                readL2 = readL2Cache.lookup(phyAdd);
                if(readL2.L2Tag == -1)
                {
                    readVL3 = readVL3Cache.lookup(phyAdd);
                    readL2 = readL2Cache.allocBlock(readVL3,phyAdd);
                }
                currOpr=dL1Block.allocBlock(readL2,oprVA,phyAdd);//dTLB_Block);
                /*
                long long int phyAdd = (((dTLB_Block&0xFFFFFF)<<12)|(currInst.oprAdd[i] & 0xFFF));
                readVL3 = readVL3Cache.lookup(phyAdd);
                currOpr = dL1Block.allocBlock(readVL3,PC);
                */
            }
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            //system("pause");
    }
    //system("pause");
    //PC = randomBits(48);
    branchTest = rand()%2;
    //cout<<"branchTest="<<branchTest;
    //cout<<"|opCode :"<<currInst.opCode<<endl;
    long long int nextInst;
    if((currInst.opCode==2)&&(branchTest==1))
    {
        cout<<"Branch Taken!"<<endl;
        //system("pause");
        nextInst = currInst.brnAdd;
    }
    else
        nextInst = 16;
    stats();
    PC = PC+nextInst;
  }
  return clock;
}





#endif // MAINRUN_H_INCLUDED
