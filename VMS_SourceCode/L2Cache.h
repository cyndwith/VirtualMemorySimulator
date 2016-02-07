#ifndef L2CACHE_H_INCLUDED
#define L2CACHE_H_INCLUDED
#include "iL1Cache.h"

/*
"dL1Cache.h - file contains the following functions/Classes: TLB, dTLB, dL1_Cache.

1. VL3 Cache (Table Look aside Buffer Class) : Implements TLB with 4x128
        TLB class has  following internal function :
            a) lookup : Search TLB for Match -> If match not found -> we look into VPT
            b) print : Prints the contents Virtual Address and Physical Address entries of the TLB

2. L2 Cache (Instruction Table Look aside Buffer Class) : Implements dTLB of size 4x32 blocks : each block holds 4 - 128 bit instructions
        VPT class has  following internal function :
            a) lookup : Search iTLB at Index across 4 Banks for a Match -> If not found  -> Search TLB for a match
            b) allocBlock : Allocates a block in iTLB using LRU/Psuedo LRU Algorithm  and returns the iTLB Block Allocated as a match

*/

int noVL3Entry = 0;

class VL3Cache{
    public:
        long long int offset;
        long long int tag;
        class VL3Block{
            public:
            long long int VL3Tag;
            inst instVL3[4];
            //Functions
            void print()
            {
                cout<<"\t | Phy. Add. (tag) : "<<bitMask(VL3Tag,36,7);
                for(int i=0;i<4;i++)
                    instVL3[i].print();
            }
        }B[1024];
    //Functions long long int
    VL3Block lookup(long long int phyAdd)
    {

        cout<<"\n\t ==> Lookup in VL3 Cache!";//tag = (phyAdd&phyAddMask)>>6;//Taking only 23 bits of 24bits masked
        tag = bitMask(phyAdd,36,7);//(phyAdd<<28)>>34;
        offset = bitMask(phyAdd,12,1);//(phyAdd&offsetAddMask);//Taking only 23 bits of 24bits masked
        int i=0; //cout<<"\tSearching VL3 Cache"<<endl;
        while(i<noVL3Entry && i<1024)
        {
                //cout<<"tag : "<<tag<<endl;//cout<<"phy. Add: "<<((VL3Cache[i]<<41)>>41);
                if( tag == bitMask(B[i].VL3Tag,36,7))//((VL3Cache[i]<<28)>>34) )
                {
                    //Need to check for valid bit as well
                    cout<<"\n\tMatch Found in VL3 Cache!";// Physical Address (tag): "<<((VL3Cache[i]&phyAddMask)>>13);
                    //cout<<" | Physical Address (tag) :"<<bitMask(B[i].VL3Tag,36,7)<<endl;//((VL3Cache[i]<<28)>>34)<<endl;
                    B[i].print();
                    //for(int j=0;j<3;j++)//Generating four random instructions
                    //    B[i].instVL3[j]=virtualProgram();
                    cycle+=16;
                    L3_Access++;
                    cout<<"\ncycle:"<<cycle;
                    return B[i];
                }
                i++;
        }
        cout<<"\n\tPhy. Add.Tag (30bit Miss): "<<tag;
        int rVL3Miss;
        if(rVL3Miss=rand()%2)
        {
            cout<<"\n\tL3 Cache Miss ==> Main Memory!";
            L3Cache_Misses++;
            cycle+=100;
            //cout<<"\ncycle:"<<cycle;

        }
        int newEntry;
        if(noVL3Entry<1024)
        {
            newEntry = noVL3Entry;
            noVL3Entry++;
        }
        else
            newEntry = rand()%1024;

        B[newEntry].VL3Tag = ((1ll<<37)|(1ll<<36)|phyAdd); //Adding Valid and Dirty Bits
        for(int j=0;j<4;j++)//Generating four random instructions
                B[newEntry].instVL3[j]=virtualProgram();
        cout<<"==> Match(Generated!) in VL3 Cache!";// Physical Address (23 bit tag): "<<((VL3Cache[noVPTEntry-1]&phyAddMask)>>13);
        //cout<<"\n\t | Physical Address (30 bit tag) :"<<((VL3Cache[noVPTEntry-1]<<28)>>34)<<endl;
        //print();
        cycle+=16;
        L3_Access++;
        //cout<<"\ncycle:"<<cycle;
        return B[newEntry];
    }
    void print()
    {
         cout <<"\n\tPrinting contents (Phy. Add) of VL3 !"<<endl;
         for(int i=0;i<noVL3Entry;i++)
         {
                //cout<<i<<"| Bit String : " <<convertToBitString(VPT[i],64)<<endl;//cout<<i<<"| Virtual Address:"<<B[i].VL3Tag;
                cout<<"\t "<<i;
                B[i].print();
                //"| Physical Address (tag) :"<<bitMask(B[i].VL3Tag,36,7)<<endl;//((VL3Cache[i]<<28)>>34)<<endl;
         }
    }
};

class L2Cache{
    public :
            long long int block;
            long long int offset;
            int LRU[512][8];
            class L2Block{
                public:
                    long long int L2Tag;
                    inst instL2[4];
                    //Functions
                    void print()
                    {
                        cout<<"\t | Physical Address (tag) : "<<bitMask(L2Tag,23,1);
                        for(int i=0;i<4;i++)
                            instL2[i].print();
                    }
                    void setValues(long long int tag,inst instList[])
                    {
                        L2Tag = tag;
                        for(int i=0;i<4;i++)
                                instL2[i]=instList[i];
                    }
            }L2[8][512];
            L2Block L2Dummy;
    //Function used for searching and allocation of blocks
    L2Block lookup(long long int phyAdd)
    {

        long long int oprAdd[3];
        for(int i=0;i<3;i++)
            oprAdd[i]=-1;
        inst instList[4];
        for(int i=0;i<3;i++)
            instList[i].setValues(-1,-1,-1ll,oprAdd);
        //for(int i=0;i<3;i++)
        //  L2Dummy.instL2[i].setValues(-1ll,-1ll,-1ll,oprAdd);
        L2Dummy.setValues(-1ll,instList);

        block = bitMask(phyAdd,15,7);//first 9 bits other than 6 bits last phyAdd.
        offset = bitMask(phyAdd,6,1);//(PC & iL1_Offset_Mask); // last 6 bits of PC
        //cout<<"\n \t Searching iL1 Cache (13bit - Page Offset ) "<< (PC & iL1Mask) <<endl;convertToBitString(offset,64)
        cout<<"\n\tSearching(L2 Cache)\n\t| Block: "<< block ;//<<" | Offset: "<<offset;
        cout<<"| Phy.Add.Tag(i/dTLB): "<< bitMask(phyAdd,36,16);// <<endl;//((phyAdd<<40)>>41) <<endl;
        long long int blockAdd;
        //cout<<"\t";
        for(int i=0;i<8;i++)
        {
            //cout<<"|Bank "<<i<<":";
            blockAdd = L2[i][block].L2Tag;
            if (bitMask(blockAdd,21,1) == bitMask(phyAdd,36,16) && (L2[i][block].L2Tag&(1ll<<22)))//((phyAdd<<40)>>41))
            {   // Need to check for valid bit
                cout<<"\t Match Found !"<<endl;// Returning one 128 bit instruction !"<<endl;
                //L2[i][block];//.instL2[(offset/16)].print();
                for(int j=0;j<8;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                            LRU[block][j]++;
                LRU[block][i]=0;
                cycle+=8;
                L2_Access++;
                //cout<<"\ncycle:"<<cycle;
                return L2[i][block];
            }
         }
         for(int j=0;j<8;j++)                       //    Used to update other  block LRU values 0 - if used - +1 if not used
                LRU[block][j]++;
         cout<<" ==> L2 Cache Miss!";
         L2Cache_Misses++;
         cycle+=8;
         L2_Access++;
         //cout<<"\ncycle:"<<cycle;
         return L2Dummy;
    }
    //Allocating a block to new entry //inst allocBlock(long long int VDTag, long long int PC) //returns the newly allocated block
    L2Block allocBlock(VL3Cache::VL3Block B, long long int phyAdd)//PC) //returns the newly allocated block
    {
        cout<<"\n\tBlock Alloc.(L2 Cache)|";
        long long int block = bitMask(phyAdd,15,7);//bitMask(PC,13,7);//(PC&blockMask)>>6; bitMask(phyAdd,36,28);
        long long int offset = bitMask(phyAdd,6,1);//bitMask(PC,6,1);//(PC&offsetMask);
        int bank;
        for(int i=0;i<8;i++)
        {
            if(!(L2[i][block].L2Tag&(1ll<<21))) // Check for the valid bit
            {
                L2[i][block].setValues(B.VL3Tag,B.instVL3);
                L2[i][block].L2Tag = bitMask(phyAdd,36,16);//contains only first 21 bits of physical address as tag
                L2[i][block].L2Tag =  L2[i][block].L2Tag|(1ll<<22);
                cout<<"| Bank:"<<i<<" | Block(Invalid):"<<block;//<<"| offset:"<<offset<<endl;
                cout<<"\n\tMatch Found(21bit) Add.Tag: "<<bitMask(L2[i][block].L2Tag,21,1);//((iTLB[i][index]&iTLB_Tag_Block)>>24);
                //for(int j=0;j<4;j++)
                //L2[i][block].instL2[offset/16].print();
                //cycle+=8;
                return L2[i][block];//.instL2[(offset/16)];
            }
        }
        //bank = rand()%8;
        bank = lruBlock(LRU[block],block,8);
        L2[bank][block].setValues(B.VL3Tag,B.instVL3);
        L2[bank][block].L2Tag = bitMask(phyAdd,36,16);//(B.VL3Tag)>>13;
        L2[bank][block].L2Tag = L2[bank][block].L2Tag|(1ll<<22);
        cout<<"\tBlock(Reallocated) | Bank:"<<bank<<" | Block:"<<block<<"| offset:"<<offset<<endl;
        cout<<"\n\tMatch Found (21bit) Add.Tag: "<<bitMask(L2[bank][block].L2Tag,21,1)<<endl;//((iTLB[i][index]&iTLB_Tag_Block)>>24);
        return L2[bank][block];//.instL2[(offset/16)];
    }
};

#endif // L2CACHE_H_INCLUDED
