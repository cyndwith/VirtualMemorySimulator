#include "virtualProgram.h"
#include "iL1Cache.h"
#include "dL1Cache.h"
#include "L2Cache.h"
#include "statistics.h"
#include "randomFunction.h"

/*
The header files brief summary is presented here: (look into each header file for more details)
1. "virtualProgram.h"   :    Contains functions related to virtual instruction generation (VirtualProgram function that generates Instructions)
2. "iL1_Cache.h"        :    Contains functions for iTLB , Virtual Page Table (VPT), iL1_Cache
3. "dL1_Cache.h"        :    Functions for dTLB, dL1_Cache, TLB
4. "L2_Cache.h"         :    Function for L2 Cache, VL3 Cache
5. "statistics.h"       :    Function for statistics of parameters ( calculates miss rate, CPI, Average Access time)
6. "randomFunction.h"   :    Functions to random bit generation (rondomBits, bitMask, convertToString)
*/

int main()
{
  srand(time(0));
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
  for(int i=0;i<100;i++)
  {
    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%<<iTLB>>%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    iTLB_Block = readiTLB.lookup(PC); // Search for the block in iTLB
    if(iTLB_Block == -1) // Checks if its a iTLB Miss
    {
        //After iTLB Miss -> We look for data in TLB
        TLB_Block = readTLB.lookup(PC); // lookup in TLB for the Block
        if(TLB_Block==-1) // Checks if its a TLB Miss
            TLB_Block = readTLB.allocBlock(readVPT.lookup(PC),PC); //lookup in VPT for the Block and allocate the block in TLB
        iTLB_Block = readiTLB.allocBlock(TLB_Block,PC); // allocate the TLB block found to iTLB
    } // After this "if" statement iTLB -> TLB - > VPT : we have the required iTLB Block with 24 bit physical address

    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;;
    cout<<"\n\t%%%%%%%%%%%%%%%%%%<<iL1 Cache>>%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    currInst = iL1Block.lookup(PC,iTLB_Block); // lookup/Search in iL1 Cache with Index from PC and physical address Tag from the iTLB
    if(currInst.opCode == -1) // Checks if its a iL1 Cache Miss
    {
        long long int phyAdd = (((iTLB_Block&0xFFFFFF)<<12)|(PC & 0xFFF));
        //Gets last 24 bit Phy. Address from the iL1 Cache and add appends 12bit page offset from PC
        readL2 = readL2Cache.lookup(phyAdd); //lookup/Search in L2 Cache with the 36 bit Physical address
        if(readL2.L2Tag == -1) // checks if its a L2 Cache Miss
        {
            readVL3 = readVL3Cache.lookup(phyAdd); // Searches Virtual L3 Cache (VL3) with physical address (If its a Miss -> Match is generated from Main Memory/Disk)
            readL2 = readL2Cache.allocBlock(readVL3,phyAdd); // Allocates the Block to L2 Cache
        }
        currInst=iL1Block.allocBlock(readL2,PC,phyAdd); // Allocates the L2 Cache block to iL1 Cache (Using Virtual Index and Physical Tag)
        if(currInst.opCode == 1)                        // Makes the Store Instruction Flag - one : Used for setting the dirty bit
            storeInst=1;
    }

    cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    for(int i=0;i<currInst.oprCount;i++) //Loops for each operand
    {
            cout<<"\n\t%%%%%%%%%%%%%%%<<dTLB Cache>>%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            cout<<"\n\tEntered Operands fetching loop!"<<endl;
            dTLB_Block = readdTLB.lookup(currInst.oprAdd[i]); //We use the operand Virtual address  (48bit) to search dTLB
            if(dTLB_Block == -1) //Checks if its a dTLB Miss
            {
                long long int oprVA = currInst.oprAdd[i]; //Virtual Address of Operand  used multiple time in following sections
                TLB_Block = readTLB.lookup(oprVA); // Search/Lookup in TLB through (readdTLB Object)
                if(TLB_Block==-1) // Checks if its TLB Miss
                    TLB_Block = readTLB.allocBlock(readVPT.lookup(oprVA),oprVA);//Check VPT for Operand Virtual Address & Allocate it in TLB (If a Miss - > Match generated from Disk PT)
                    //cout<<"Debug:\n\tAllocated Block in iTLB!"<<endl;
                dTLB_Block = readdTLB.allocBlock(TLB_Block,oprVA); //Allocate the block in dTLB with TLB Block
            }
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%<<dL1 Cache>>%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            currOpr = dL1Block.lookup(currInst.oprAdd[i],dTLB_Block); //Search/Lookup dL1 Cache
            if(currOpr==-1) // Checks if dL1 Block Miss
            {
                long long int oprVA = currInst.oprAdd[i]; // Get the virtual address of the operand
                long long int phyAdd = (((dTLB_Block&0xFFFFFF)<<12)|(PC & 0xFFF)); // Get the 36 bit physical address
                readL2 = readL2Cache.lookup(phyAdd); // Search/Lookup in L2 Cache
                if(readL2.L2Tag == -1) // Checks for L2 Cache Miss
                {
                    readVL3 = readVL3Cache.lookup(phyAdd); // Search/Lookup Virtual L3 Cache (VL3)  -> (if a Miss - > Match is Generated through Main Memory/Disk)
                    readL2 = readL2Cache.allocBlock(readVL3,phyAdd); // Allocate the block in Virtual L3 Cache
                }
                currOpr=dL1Block.allocBlock(readL2,oprVA,phyAdd);// Allocate the block in L2 Cache into dL1 Cache
            }
            cout<<"\n\t%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
            //cout<<"\n\tDebug system Pause : "<<system("pause");
    }
    //cout<<"\n\tDebug system Pause : "<<system("pause");
    branchTest = rand()%2; //Generate a random '0' or '1'
    long long int nextInst; // Stores the next PC counter values
    if((currInst.opCode==2)&&(branchTest==1)) // Checks if its a branch instruction and if its branch is taken (or true)
    {
        cout<<"Branch Taken!"<<endl;
        branch_Taken++;
        nextInst = currInst.brnAdd; // Makes the Next Instruction based on branch address
    }
    else
        nextInst = PC+16;   //  Makes the Next Instruction PC+16 if we do not see
    stats();                //  Calculates the statistics -> refer to "statistics.h" for more details
    PC = nextInst;          //  Updating PC with next Instruction location

    // Printing all the statistics of misses, miss rates , CPI and Avg. access time calculations
    cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    cout<<"% | iTLB_Miss: "<<iTLB_Miss<<"\t| dTLB_Miss: "<<dTLB_Miss<<"\t | TLB_Miss: "<<TLB_Miss<<endl;
    cout<<"% | CLK: "<<cycle<<"\t | iL1_Misses: "<<iL1Cache_Misses<<"\t | dL1_Misses: "<<dL1Cache_Misses<<endl;
    cout<<"% | PF: "<<page_Fault<<"\t | L2_Misses: "<<L2Cache_Misses<<"\t | L3_Misses: "<<L3Cache_Misses<<endl;
    cout<<"% | CPI: "<<CPI<<"\t | No. Instruction: "<<noInstruction<<"\t | iL1_MissRate: "<<iL1_Miss_Rate<<endl;
    cout<<"% | dL1_MRate: "<<dL1_Miss_Rate<<"\t | L2_MRate: "<<L2_Miss_Rate<<"\t | L3_MRate: "<<L3_Miss_Rate<<endl;
    cout<<"% | Branch_Taken: "<<branch_Taken<<"\t | Avg.DataTime: "<<avgDT<<"\t | Avg.InstTime: "<<avgIT<<endl;
    cout<<"% | Write_Backs: "<<WB<<"\t | No Data: "<<0<<"\t | No Data: "<<0<<endl;
    cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
    //system("pause");
  }
  return 0;
}


