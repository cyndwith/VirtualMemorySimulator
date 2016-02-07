#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED
#include "virtualProgram.h"
#include "randomFunction.h"
#include "statistics.h"
#include "L2Cache.h"

/*
"iL1Cache.h - file contains the following functions/Classes: VPT, iTLB, iL1_Cache.

1. VPT (Virtual Page Table Class) : Contains the simulates Page Table using Array of Size (1024)
        VPT class has  following internal function :
            a) lookup : Search VPT for Match -> If not (use random variable to Indicate Main Memory/Disk fetch) generate a Match
            b) print : Prints the contents Virtual Address and Physical Address entries of the VPT

2. iTLB (Instruction Table Look aside Buffer Class) : Implements iTLB of size 4x32 blocks : each block holds 4 - 128 bit instructions
        VPT class has  following internal function :
            a) lookup : Search iTLB at Index across 4 Banks for a Match -> If not found  -> Search TLB for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iTLB Block Allocated as a match

3. iL1_Cache (Instruction Level 1 Cache Class) : Implements iL1 Cache of size 4x128 blocks : each block holds 4 - 128 bit instructions
        iL1 Cache Class has  following internal function :
            a) lookup : Search iL1 Cache at Index (0-127) across 4 Banks for a Match -> If not found  -> Search L2 Cache for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iL1 Cache Block Allocated as a match
*/


int noVPTEntry = 0;     //  Used to store the number of Valid entries in the VPT Table

class VPT{              // Virtual Page Table (VPT) as Array of Size 1024
    public:
        long long int VPT[1024];
        long long int virAdd;   // Virtual Address Stored in Table
        long long int phyAdd;   // Corresponding Physical Address Stored in Table
        int LRU[1024];
    long long int lookup(long long int PC)  //  Search the VPT with PC -> Virtual Address Tag (36bit)
    {
        cout<<" ==> Lookup in VPT!";
        int i=0;                            //  Used to Index VPT Array -> we start from the start of the array
        virAdd = bitMask(PC,48,13);         //  Take the 36 bit Virtual Address
        while(i<noVPTEntry && i<1024)       //  loops for all valid entries and size of the array
        {
                if( virAdd == bitMask(VPT[i],60,25))    //  check for a match in Virtual Address and Tag field in VPT Entry
                {
                    cout<<"\n\t Match Found in VPT!\n\t Vir. Add. : "<<(bitMask(VPT[i],60,25));
                    cout<<" | Phy. Add. :"<<(bitMask(VPT[i],24,1))<<endl;   // Prints the Physical Address  Tag of the match
                    cycle+=2;   // Access time of the VPT - 2 Clocks
                    return VPT[i]; //   Terminates and Returns the VPT block with a match
                }
                i++; // No Match -> We move to the next element
        }
        cout<<"\n\tVir. Add.(31bit Miss): "<<virAdd;
        int rVPMiss=rand()%2;   // Random variable assigned '0' or '1' to check if its a Page Fault
        if(rVPMiss) // Checks the random variable
        {
            cout<<"==> PageFault!";
            page_Fault++;
            //cycle+=16;
        }
        int newEntry;       // Used to decide if we need replacement in the VPT (if VPT Array is full)
        if(noVPTEntry<1024) // Checks if the VPT Array is Full
        {
            newEntry = noVPTEntry;  // If not Use the next index as new entry
            noVPTEntry++;           // Increment the number of Valid Entires
        }
        else
            newEntry = rand()%1024;                     //  Select a random entry to replace in the VPT
        VPT[newEntry] = ((virAdd<<24)|randomBits(24));  //  Generate a random virtual -> Physical memory match
        cout<<"==> Match (Generated) VPT!";             //  printing to the console for user
        //cycle+=2;                                     //  Clocks for VPT Access
        return VPT[newEntry];                           //  Return the matched/Generated VPT Entry
    }
    void print()                                        //  Prints the Virtual Address and Physical Address of Entries in VPT Array
    {
         cout <<"\n\tPrinting contents of VPT!"<<endl;
         for(int i=0;i<noVPTEntry;i++)                  //  Loops for all the valid entries in the VPT Array
         {
                cout<<"\t "<<i<<"| Virtual Address:"<<(bitMask(VPT[i],60,25));      // Takes the first 36 bit of the entry as Virtual Address
                cout<<"\t| Phy. Add.:"<<(bitMask(VPT[i],24,1))<<endl;               // Takes the last 24 bits of the entry as Physical Address
         }
    }
};

/*
2. iTLB (Instruction Table Look aside Buffer Class) : Implements iTLB of size 4x32 blocks : each block holds Virtual to Physical Address Translation
        VPT class has  following internal function :
            a) lookup : Search iTLB at Index across 4 Banks for a Match -> If not found  -> Search TLB for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iTLB Block Allocated as a match
*/

class iTLB{
    public:
        long long int index;
        long long int tag;
        long long int result;
        long long int iTLB[4][32];  //      Implements 4x32 iTLB with 4 - Banks and 32 - Sets
        int LRU[32][4];
    long int lookup(long long int PC)       //      Lookup Function used to check for block match using PC
    {
             long long int match_tag;
             index = bitMask(PC,48,44);     //      Takes the first 5 bits of 36 bits Virtual Page Number as Index
             tag = bitMask(PC,43,13);       //      Takes the remaining 31 bits of 36 bit Virtual Page Number as Match Tag
             cout<<"\tSearching (iTLB)\n\tIndex: "<<index<<" & Add.Tag :"<<tag; // Prints the Index and Address Tag to Console output
             for(int i=0;i<4;i++) // Searching the 4 banks for match based on the Index
             {
                 match_tag = bitMask(iTLB[i][index],55,25);     // Extracts the match tag from the iTLB Block  for a given Index
                 if (match_tag==tag && (iTLB[i][index]&(1ll<<55)))                            // Check for the tag match from PC and iTLB Block
                 {
                    cout<<bitMask(iTLB[i][index],55,25)<<endl;  // Prints the matched Tag to the Console
                    result = iTLB[i][index];                    // Results stores the matched block value
                    cout<<"\n\t==> Match Found (31bit) Vir. Add.Tag: "<<match_tag<<"| Phy. Add.: "<<bitMask(result,24,1)<<endl;
                    // Above statements prints the match found and Tag to the console
                    //cycle++;
                    for(int j=0;j<4;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                            LRU[index][j]++;
                    LRU[index][i]=0;
                    return result; // Terminates the process by returning the match
                 }
                 //LRU[index][i]++;
             }
             for(int j=0;j<4;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                LRU[index][j]++;

             iTLB_Miss++; // we get here if a match is not found -> we increment iTLB Misses count
             cout<<" ==> iTLB Miss Registered!"; // print the declaration to console
             //cycle++;
             return -1; // Return a sentinel value (i.e. -1) to indicate a Miss
    }
    // Takes input as x - TLB Block and PC - the Program Counter
    long long int allocBlock(long long int x,long long int PC) //Allocates a location within iTLB and returns the newly allocated block
    {
        cout<<"\n\tBlock Alloc.(iTLB) |";           //  print data
        long long int virAdd = bitMask(x,60,25);    //  Take the first 36 bits from the TLB Block (Virtual Address)
        int index = bitMask(PC,48,44);              //  first 5 bits of virtual address
        for(int i=0;i<4;i++)                        //  Searching all the 4 banks at given Index -> for Invalid blocks to replace
        {
            if(!(iTLB[i][index]&(1ll<<55)))         // Checks if the Valid bit is '0' or '1'
            {
                iTLB[i][index]=  (bitMask(PC,43,13)<<24)| bitMask(x,24,1);  //  Allocates 31 bit virtual address as Tag and 24 bit physical address from TLB Block
                iTLB[i][index] =  iTLB[i][index]|(1ll<<55);                 //  Makes the Valid bit of allocated block -> '1'
                cout<<" Block(Invalid) | Bank:"<<i<<" | Index:"<<index<<"|"<<endl;      //   prints data to console
                cout<<"\tMatch Found Vir. Add.Tag: "<<bitMask(iTLB[i][index],55,25);    //   prints data to console
                cout<<"| Phy. Add.: "<<bitMask(iTLB[i][index],24,1);                    //   prints data to console
                return iTLB[i][index];                                                  //   terminates and returns the allocated block
            }
        }
        // If we do not find any invalid block
        //int block = rand()%4;
        int block = lruBlock(LRU[index],index,4);                          //  Selects a rand bank ( or LRU/Pseudo LRU) to select the block to be replaced
        iTLB[block][index]= (bitMask(PC,43,13)<<24)| bitMask(x,24,1);   //  Allocates 31 bit virtual address as Tag and 24 bit physical address from TLB Block
        iTLB[block][index] =  iTLB[block][index]|(1ll<<55);             //  Makes the Valid bit of allocated block -> '1'
        cout<<"\tBlock (Reallocated) | Block:"<<block<<" | Index:"<<index<<"|"<<endl;       //  prints data to console
        cout<<"\tMatch Found (31bit) Vir. Add. Tag: "<<bitMask(iTLB[block][index],55,25);   //  prints data to console
        cout<<"| Phy. Add. : "<<bitMask(iTLB[block][index],24,1)<<endl;                     //  prints data to console
        return iTLB[block][index];                                                          //  Terminates and Returns the allocated block
    }
};

/*
3. iL1_Cache (Instruction Level 1 Cache Class) : Implements iL1 Cache of size 4x128 blocks : each block holds 4 - 128 bit instructions
        iL1 Cache Class has  following internal function :
            a) lookup : Search iL1 Cache at Index (0-127) across 4 Banks for a Match -> If not found  -> Search L2 Cache for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iL1 Cache Block Allocated as a match
*/

class iL1Cache{
    public :
            long long int block;
            long long int offset;
            int LRU[128][4];
            class iL1Block{                 // subClass used to store Instruction in iL1Cache - contains: Tag field and four instruction (Objects)
                public:
                    long long int iL1Tag;
                    inst instL1[4];
                    void print()            // Prints the values of Tag of the block
                    {
                        cout<<"\t | Physical Address (tag):"<<bitMask(iL1Tag,23,1);
                        for(int i=0;i<4;i++)
                            instL1[i].print();
                    }
                    void setValues(long long int tag,inst instList[]) //    Set the values of the tag field
                    {
                        iL1Tag = tag;
                        for(int i=0;i<4;i++)                          //    Initializes the object with input instruction sets
                                instL1[i]=instList[i];
                    }
            }iL1Cache[4][128];                                        //    We implement a iL1 Cache with 4 - banks and 128 - Sets
    //Function used for searching of blocks within iL1 Cache
    inst lookup(long long int PC, long long int iTLB)                 //    Takes the PC and iTLB_Block -> with physical address (24bit) as Inputs
    {

        inst instDummy;                                               //    Dummy or sentinel instruction used to return of Miss
        long long int oprAdd[3];                                      //    Operands for the Dummy Instruction
        for(int i=0;i<3;i++)
            oprAdd[i]=-1;                                             //    Initializing the values of Operand Address to sentinel values
        instDummy.setValues(-1ll,-1ll,-1ll,oprAdd);                   //    Using setValues to assign sentinel values to the Dummy Instruction.
        block = bitMask(PC,13,7);                                     //    Block from first 7 bits of last 13bits of virtual address
        offset = bitMask(PC,6,1);                                     //    last 6 bits of the virtual offset - within the block
        cout<<"\tSearching(iL1Cache)\n\t| Block:"<< block;            //    Printing Block values console
        cout<<"| Phy.Add.Tag(iTLB): "<<bitMask(iTLB,24,2);            //    Print Tag field to console
        long long int blockAdd;
        for(int i=0;i<4;i++)                                          //    Searching 4 banks of the iL1 Cache
        {
            blockAdd = iL1Cache[i][block].iL1Tag;                     //    Block Address
            if (bitMask(blockAdd,23,1) == bitMask(iTLB,24,2) && (iL1Cache[i][block].iL1Tag&(1ll<<24)))          //    Match the 21 bit tag field with 21 bit of physical address
            {   // Need to check for valid bit
                cout<<"\n\t ==> Match Found ! Returning one 128 bit instruction !"<<endl;
                for(int j=0;j<4;j++)                                  //    Used to update other  block LRU values 0 - if used - +1 if not used
                        LRU[block][j]++;                              //
                LRU[block][i]=0;
                iL1Cache[i][block].instL1[(offset/16)].print();       //    Selects the Instruction -> floors the offset to have aligned access to memory
                iL1_Access++;                                         //    Increments the iL1 Cache Access variable
                cycle+=4;                                             //    Hit time of the iL1 Cache + Added to the clock cycle
                return iL1Cache[i][block].instL1[(offset/16)];        //    Terminates and returns the instructions
            }
            //LRU[block][i]++;
         }
         for(int j=0;j<4;j++)                                  //    Used to update other  block LRU values 0 - if used - +1 if not used
                LRU[block][j]++;
         cout<<" ==> iL1 Cache Miss!";      //      We enter this section -> if we do not have a match
         iL1Cache_Misses++;                 //      Increment the iL1 Cache Misses
         iL1_Access++;                      //      Increment iL1 Access
         cycle+=4;                          //      Hit time of the iL1 Cache
         return instDummy;                  //      Terminates and Returns Dummy Instruction for a Miss
    }
    // It uses the B - Block from VL3 Cache , PC and 36bit Physical Address
    inst allocBlock(L2Cache::L2Block B, long long int PC,long long int phyAdd) // Allocates a new block in the memory and return the allocated block
    {
        cout<<"\n\tBlock Alloc.(iL1 Cache)|";
        long long int block = bitMask(PC,13,7);     //      first 7 bits of last 13 bit page offset
        long long int offset = bitMask(PC,6,1);     //      Last 6 bits of 13 bit page offset
        int bank;                                   //      Used to select the bank
        for(int i=0;i<4;i++)                        //      Check all the banks
        {
            if(!(iL1Cache[i][block].iL1Tag&(1ll<<24))) // Check for a invalid bit - for invalid block
            {
                iL1Cache[i][block].setValues(B.L2Tag,B.instL2);                     //      Take the values from Tag field and Instructions List (4 instructions)
                iL1Cache[i][block].iL1Tag = bitMask(phyAdd,36,14);                  //      Contains only first 23 bits of physical address as tag
                iL1Cache[i][block].iL1Tag =  iL1Cache[i][block].iL1Tag|(1ll<<24);   //      Valid bit is made one
                cout<<" | Bank:"<<i<<" | Block(Invalid):"<<block;                   //      Displays the Block assigned
                cout<<"\n\tMatch Found (23 bit) Add.Tag: "<<bitMask(iL1Cache[i][block].iL1Tag,23,1)<<endl;
                iL1Cache[i][block].instL1[offset/16].print();                       //      Floors offset to select Instruction with aligned access
                return iL1Cache[i][block].instL1[(offset/16)];                      //      Terminates and returns the matched Instruction
            }
        }
        //bank = rand()%4;
        bank = lruBlock(LRU[block],block,4);        //      We get here if we did not find a invalid block -> we replace a block using LRU/Pseudo LRU/Random
        iL1Cache[bank][block].setValues(B.L2Tag,B.instL2);                      //      Take the values from Tag field and Instructions List (4 instructions)
        iL1Cache[bank][block].iL1Tag = bitMask(phyAdd,36,14);                   //      Contains only first 23 bits of physical address as tag
        iL1Cache[bank][block].iL1Tag = iL1Cache[bank][block].iL1Tag|(1ll<<24);  //      Valid bit is made one
        cout<<"\tBlock(Reallocated) | Bank:"<<bank<<" | Block:"<<block;         //      Prints the allocated block ID
        cout<<"\n\tMatch Found(23bit) Add.Tag: "<<bitMask(iL1Cache[bank][block].iL1Tag,23,1)<<endl; // Prints the address tag of the allocated block
        iL1Cache[bank][block].instL1[offset/16].print();
        return iL1Cache[bank][block].instL1[(offset/16)];                       //      Terminates and returns the allocated block
    }
};

#endif // MEMORY_H_INCLUDED
