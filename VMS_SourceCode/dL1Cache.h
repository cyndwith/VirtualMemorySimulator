#ifndef DL1CACHE_H_INCLUDED
#define DL1CACHE_H_INCLUDED
#include "statistics.h"
#include "L2Cache.h"

/*
"dL1Cache.h - file contains the following functions/Classes: TLB, dTLB, dL1_Cache.

1. TLB (Table Look aside Buffer Class) : Implements TLB with 4x128
        TLB class has  following internal function :
            a) lookup : Search TLB for Match -> If match not found -> we look into VPT
            b) print : Prints the contents Virtual Address and Physical Address entries of the TLB

2. dTLB (Instruction Table Look aside Buffer Class) : Implements dTLB of size 4x32 blocks : each block holds 4 - 128 bit instructions
        VPT class has  following internal function :
            a) lookup : Search iTLB at Index across 4 Banks for a Match -> If not found  -> Search TLB for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iTLB Block Allocated as a match

3. dL1_Cache (Instruction Level 1 Cache Class) : Implements dL1 Cache of size 4x128 blocks : each block holds 4 - 128 bit instructions
        iL1 Cache Class has  following internal function :
            a) lookup : Search iL1 Cache at Index (0-127) across 4 Banks for a Match -> If not found  -> Search L2 Cache for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iL1 Cache Block Allocated as a match
*/

/*
TLB (Table Look aside Buffer Class) : Implements TLB with 4x128 - 4 Banks and 128 Sets
        TLB class has  following internal function :
            a) lookup : Search TLB for Match -> If match not found -> we look into VPT
            b) print : Prints the contents Virtual Address and Physical Address entries of the TLB
*/
class TLB{
    public:
        long long int index;
        long long int tag;
        long long int result;
        long long int TLB[4][128];
        int LRU[128][4];
    //Function within iTLB
    long int lookup(long long int PC)       //  Search the TLB with PC -> Virtual Address Tag (36bit)
    {
             long long int match_tag;       //  Used to store the Tag field on the block in different banks
             index = bitMask(PC,48,42);     //  first 7bits of Physical Address Space
             tag = bitMask(PC,41,13);       //  next 29 bit Tag - Physical Address Space
             cout<<"\n\tSearching (TLB)\n\tIndex:"<<index<<" & Add.Tag:"<< tag ;
             for(int i=0;i<4;i++)
             {
                 LRU[index][i]=0;
                 match_tag = bitMask(TLB[i][index],53,25);  //  check for 29bit Tag field of the block with PC
                 if (match_tag==tag && (TLB[i][index]&(1ll<<53)))                        //  checks for the match of Tag field
                 {
                    cout<<bitMask(TLB[i][index],55,25)<<endl;
                    result = TLB[i][index];                 //  Stores the matched block
                    cout<<" ==> Match Found (29bit) Vir. Add.Tag : "<<match_tag<<"| Phy. Add. : "<<bitMask(result,24,1)<<endl;//((result<<40)>>40) <<endl;
                    return result;                          //  Terminates and Returns the match TLB Block
                 }
                 LRU[index][i]++;
             }
             TLB_Miss++;                //  We enter the section if the match is not found
             cout<<" ==> TLB Miss Registered!";
             return -1;                 //  Returns -1 - sentinel values to indicate a miss in TLB
    }
    // allocBlock( VPT Block,PC) - It takes VPT block and PC as inputs
    long long int allocBlock(long long int x,long long int PC)  //   Allocates block in TLB and returns the newly allocated block
    {
        cout<<"\n\tBlock Alloc. (TLB) |";                       //   print the data to console
        long long int virAdd = bitMask(x,60,25);                //   36 bit virtual address from VPT Block
        int index = bitMask(PC,48,42);                          //   first 7 bits of virtual address
        for(int i=0;i<4;i++)                                    //   Check all the four banks
        {
            if(!(TLB[i][index]&(1ll<<53)))                      //   Checks for Invalid block in the four banks
            {
                TLB[i][index] =  (bitMask(PC,41,13)<<24)| bitMask(x,24,1);  //  Assigning 29bit Virtual Address (7bits used for indexing/36bits) & 24bit Phy. Address bits
                TLB[i][index] =  TLB[i][index]|(1ll<<53);                   //  Set the Valid bit in the block
                cout<<" Block (Invalid) | Bank:"<<i<<" | Index:"<<index<<"|"<<endl;
                cout<<"\tMatch Found (29bit) Vir. Add.Tag: "<<bitMask(TLB[i][index],53,25);
                cout<<"| Phy. Add. : "<<bitMask(TLB[i][index],24,1)<<endl;
                return TLB[i][index];                                       //  Terminate and return the matched block
            }
        }
        //int bank = rand()%4;
        int bank = lruBlock(LRU[index],index,4);                                            //  No Invalid block was found - replace existing blocks using LRU/Pseudo LRU Algorithm
        TLB[bank][index] =  (bitMask(PC,41,13)<<24)| bitMask(x,24,1);   //  Taking only 55 (31 v +21 phy.) bits left after taking 5 bits for index
        TLB[bank][index] =  TLB[bank][index]|(1ll<<53);                 //  Sets the valid bit one
        cout<<"\tBlock (Reallocated) | Bank:"<<bank<<" | Index:"<<index<<"|"<<endl;
        cout<<"\tMatch Found (29bit) Vir. Add.Tag: "<<bitMask(TLB[bank][index],53,25);
        cout<<"| Phy. Add. : "<<bitMask(TLB[bank][index],24,1)<<endl;
        return TLB[bank][index];
    }
};

class dTLB{
    public:
        long long int index;
        long long int tag;
        long long int result;
        int LRU[16][4];
        long long int dTLB[4][16];          //      Implements 4x16 iTLB with 4 - Banks and 16 - Sets
    //Function within iTLB
    long int lookup(long long int PC)       //      Lookup Function used to check for block match using PC
    {
             long long int match_tag;
             index = bitMask(PC,47,44);     //      Takes the  first  5 bits (first bit set to '0') of 36 bits Virtual Page Number as Index
             tag = bitMask(PC,43,13);       //      Takes the remaining 31 bits of 36 bit Virtual Page Number as Match Tag
             cout<<"\tSearching(dTLB) Index :"<<index<<" & Add. Tag :"<< tag; // Prints the Index and Address Tag to Console output
             for(int i=0;i<4;i++)           //      Searching the 4 banks for match based on the Index
             {
                 match_tag = bitMask(dTLB[i][index],55,25);     // Extracts the match tag from the iTLB Block  for a given Index
                 if (match_tag==tag && (dTLB[i][index]&(1ll<<55)))                            // Check for the tag match from PC and iTLB Block
                 {
                     for(int j=0;j<4;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                            LRU[index][j]++;
                    LRU[index][i]=0;
                    cout<<bitMask(dTLB[i][index],55,25)<<endl;   // Prints the matched Tag to the Console
                    result = dTLB[i][index];                     // Results stores the matched block value
                    cout<<"\tMatch Found Vir. Add.Tag : "<<match_tag<<"| Phy. Add. : "<<bitMask(result,24,1)<<endl;
                    return result;                              //  Terminates the process by returning the match
                 }
                 //LRU[index][i]++;
             }
             for(int j=0;j<4;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                     LRU[index][j]++;
             dTLB_Miss++;   // we get here if a match is not found -> we increment dTLB Misses count
             cout<<" ==> dTLB Miss!";
             return -1;     // Return a sentinel value (i.e. -1) to indicate a Miss
    }
    // Takes input as x - TLB Block and PC - the Program Counter
    long long int allocBlock(long long int x,long long int PC)  //  Allocates a location within iTLB and returns the newly allocated block
    {
        cout<<"\n\tBlock Alloc. (dTLB) |";                      //  print data
        long long int virAdd = bitMask(x,60,25);                //  Take the first 36 bits from the TLB Block (Virtual Address)
        int index = bitMask(PC,47,44);                          //  first 5 bits of virtual address (first bit being '0')
        for(int i=0;i<4;i++)
        {
            if(!(dTLB[i][index]&(1ll<<55)))                     // Checks if the Valid bit is '0' or '1'
            {
                dTLB[i][index]= (bitMask(PC,43,13)<<24)| bitMask(x,24,1);   //  Allocates 31 bit virtual address as Tag and 24 bit physical address from TLB Block
                dTLB[i][index] =  dTLB[i][index]|(1ll<<55);                 //  Makes the Valid bit of allocated block -> '1'
                cout<<" Block (Invalid) | Bank:"<<i<<" | Index:"<<index<<"|"<<endl;
                cout<<"\n\tMatch Found (31bit) Vir. Add.Tag: "<<bitMask(dTLB[i][index],55,25);
                cout<<"| Phy. Add. : "<<bitMask(dTLB[i][index],24,1)<<endl;
                return dTLB[i][index];                                      //   terminates and returns the allocated block
            }
        }
        // If we do not find any invalid block
        //int bank = rand()%4;
        int bank = lruBlock(LRU[index],index,4);                                            //  Selects a rand bank ( or LRU/Pseudo LRU) to select the block to be replaced
        dTLB[bank][index]= (bitMask(PC,43,13)<<24)| bitMask(x,24,1);    //  Allocates 31 bit virtual address as Tag and 24 bit physical address from TLB Block
        dTLB[bank][index] =  dTLB[bank][index]|(1ll<<55);                //  Makes the Valid bit of allocated block -> '1'
        cout<<"\tBlock (Reallocated) | Bank:"<<bank<<" | Index:"<<index<<"|"<<endl;
        cout<<"\n\tMatch Found (31bit) Vir. Add.Tag: "<<bitMask(dTLB[bank][index],55,25);
        cout<<"| Phy. Add. : "<<bitMask(dTLB[bank][index],24,1)<<endl;
        return dTLB[bank][index];                                        //  Terminates and Returns the allocated block
    }
};


/*
3. dL1_Cache (Instruction Level 1 Cache Class) : Implements dL1 Cache of size 4x64 blocks : each block holds 8 - 64 bit operands
        dL1 Cache Class has  following internal function :
            a) lookup : Search dL1 Cache at Index (0-127) across 4 Banks for a Match -> If not found  -> Search L2 Cache for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the dL1 Cache Block Allocated as a match
*/
class dL1Cache{
    public :
            long long int block;
            long long int offset;
            int LRU[64][8];
            class dL1Block{                   // subClass used to store Operands in dL1Cache - contains: Tag field and 8 Operands
                public:
                    long long int dL1Tag;
                    inst operands[8];
                    //Functions
                    void print()                 // Prints the values of Tag of the block
                    {
                        cout<<"\t | Physical Address(tag): "<<bitMask(dL1Tag,24,1);
                        for(int i=0;i<8;i++)
                            operands[i].print();
                    }
                    void setValues(long long int tag,inst oprList[])        //    Set the values of the tag field
                    {
                        dL1Tag = tag;
                        for(int i=0;i<8;i++)                                //    Initializes the object with operands data sets
                                operands[i]=oprList[i];
                    }
            }dL1Cache[8][64];                                                //    We implement a dL1 Cache with 4 - banks and 64 - Sets
     //Function used for searching of blocks within dL1 Cache
    long long int lookup(long long int currOpr, long long int dTLB)      //    Takes current Operand Address and dTLB Block -> with physical address (24bit) as Inputs
    {
        block = bitMask(currOpr,12,7);                                   //    Block from first 6 bits of last 12bits of virtual address
        offset = bitMask(currOpr,6,1);                                   //    last 6 bits of the virtual offset - within the block
        cout<<"\n\tSearching(dL1 Cache)\n\tBlock: "<< block;             //    Printing Block values console
        cout<<" | Phy. Add.Tag(dTLB): "<<bitMask(dTLB,24,1) <<endl;
        long long int blockAdd;
        for(int i=0;i<8;i++)                                             //    Searching 4 banks of the iL1 Cache
        {
            blockAdd = dL1Cache[i][block].dL1Tag;                        //    Block Address Tag
            if ((bitMask(blockAdd,24,1) == bitMask(dTLB,24,1)) && (dL1Cache[i][block].dL1Tag&(1ll<<25)) )      //    Match the 21 bit tag field with 21 bit of physical address and Checks for Valid bit
            {
                for(int j=0;j<8;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                    LRU[block][j]++;
                LRU[block][i]=0;
                if(storeInst==1)
                {
                    dL1Cache[i][block].dL1Tag =  dL1Cache[i][block].dL1Tag|(1ll<<24);   //      Dirty bit is its a store instructions made one
                    storeInst = 0;
                }
                cout<<"\t Match Found ! Returning one operand !"<<endl;  //    Selects one operand out of 8 -> floors the offset to have aligned access to memory
                cycle+=4;                                                //    Hit time of the iL1 Cache + Added to the clock cycle
                dL1_Access++;                                            //    Increments the dL1 Cache Access variable
                return 0;                     //    Terminates and returns zero to indicate a hit -> we do not concern with values of operand as execution is not performed
            }
            //LRU[block][i]++;
         }
         for(int j=0;j<8;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
               LRU[block][j]++;
         cout<<"\n\tdL1 Cache Miss Registered!";         //      We enter this section -> if we do not have a match
         dL1Cache_Misses++;                              //      Increment the iL1 Cache Misses
         cycle+=4;                                      //      Hit time of the iL1 Cache
         dL1_Access++;                                    //      Increment dL1 Cache Access
         return -1;                                          //      Terminates and Returns -1 - sentinel value for a Miss
    }
     // It uses the B - Block from VL3Cache , PC and 36bit Physical Address
    long long int allocBlock(L2Cache::L2Block B, long long int currOpr,long long int phyAdd) // Allocates a new block in the memory and return the allocated block
    {
        cout<<"\n\tBlock Alloc.(dL1 Cache)|";
        long long int block = bitMask(currOpr,12,7);            //      first 7 bits of last 13 bit page offset
        long long int offset = bitMask(currOpr,6,1);            //      Last 6 bits of 13 bit page offset
        for(int i=0;i<8;i++)                                    //      Searching 8 - banks
        {
            if(!(dL1Cache[i][block].dL1Tag&(1ll<<25)))          //      Check for the invalid block using valid bit
            {
                dL1Cache[i][block].dL1Tag =  bitMask(phyAdd,36,13);      //      Take the values from Tag field
                if(storeInst==1)
                {
                    dL1Cache[i][block].dL1Tag =  dL1Cache[i][block].dL1Tag|(1ll<<24);   //      Dirty bit is its a store instructions made one
                    storeInst = 0;
                }
                dL1Cache[i][block].dL1Tag =  dL1Cache[i][block].dL1Tag|(1ll<<25);   //      Valid bit is made one
                cout<<"\n\tBlock (Invalid) | Bank:"<<i<<" | Block:"<<block;         //      Displays the Block assigned
                cout<<"\n\tMatch Found (24bit) Add.Tag: "<<bitMask(dL1Cache[i][block].dL1Tag,24,1)<<endl;
                return 0;                                               //      Terminates and returns 0 indicating a match
            }
        }
        //int bank = rand()%4;
        int bank = lruBlock(LRU[block],block,8);                                                    //      We get here if we did not find a invalid block -> we replace a block using LRU/Pseudo LRU/Random
        if(dL1Cache[bank][block].dL1Tag&(1ll<<24)) //   Check if the Dirty Bit
        {
            cycle=cycle+8+16+100; // Add L2 + VL3 + Main Memory
            cout<<"Write Back!!"<<endl;
            WB++;
            //system("pause");
        }
        dL1Cache[bank][block].dL1Tag =  bitMask(phyAdd,36,13);                     //      Contains only first 23 bits of physical address as tag
        if(storeInst==1)
        {
                    dL1Cache[bank][block].dL1Tag =  dL1Cache[bank][block].dL1Tag|(1ll<<24);   //      Dirty bit is its a store instructions made one
                    storeInst = 0;
        }
        dL1Cache[bank][block].dL1Tag = dL1Cache[bank][block].dL1Tag|(1ll<<25);     //      Valid bit is made one
        cout<<"\n\tBlock (Reallocated) | Bank:"<<bank<<" | Block:"<<block;         //      Prints the allocated block ID
        cout<<"\n\tMatch Found with (24bit) Add.Tag: "<<bitMask(dL1Cache[bank][block].dL1Tag,24,1)<<endl;
        return 0;
    }                                                                  //      Terminates and returns '0' - indicating a match   }
};

#endif // DL1CACHE_H_INCLUDED
