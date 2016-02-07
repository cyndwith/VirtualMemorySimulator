#ifndef STATISTICS_H_INCLUDED
#define STATISTICS_H_INCLUDED


int cycle;
int page_Fault;
int iL1_Access;
int dL1_Access;
int L2_Access;
int L3_Access;
int storeInst;
int WB;

int noInstruction;
int iTLB_Miss;
int dTLB_Miss;
int TLB_Miss;
int branch_Taken;
int iL1Cache_Misses;
int dL1Cache_Misses;
int L2Cache_Misses;
int L3Cache_Misses;
int MM_Misses;
float CPI;
float iL1_Miss_Rate;
float dL1_Miss_Rate;
float L2_Miss_Rate;
float L3_Miss_Rate;
float avgDT,avgIT;

// Calculates the Statistics of the System : Cache Miss rate, CPI etc.,
void stats()
{
    noInstruction++;
    CPI = (cycle/noInstruction);
    iL1_Miss_Rate = (float)((float)iL1Cache_Misses/(float)iL1_Access);
    dL1_Miss_Rate = (float)dL1Cache_Misses/(float)dL1_Access;
    L2_Miss_Rate = (float)L2Cache_Misses/(float)L2_Access;
    L3_Miss_Rate = (float)L3Cache_Misses/(float)L3_Access;
    avgDT = 4+dL1_Miss_Rate*(8+L2_Miss_Rate*(16+L3_Miss_Rate*(100+0.01*10000)));
    avgIT = 1+dL1_Miss_Rate*(2+L2_Miss_Rate*(4+L3_Miss_Rate*(100+0.01*10000)));
}

void resetStats()
{
    cycle=0;
    page_Fault=0;
    iL1_Access=0;
    dL1_Access=0;
    L2_Access=0;
    L3_Access=0;

    iTLB_Miss=0;
    dTLB_Miss=0;
    noInstruction=0;
    iL1Cache_Misses=0;
    dL1Cache_Misses=0;
    L2Cache_Misses=0;
    L3Cache_Misses=0;
    MM_Misses=0;
    CPI=0;
    iL1_Miss_Rate=0;
    dL1_Miss_Rate=0;
    L2_Miss_Rate=0;
    L3_Miss_Rate=0;

}

#endif // STATISTICS_H_INCLUDED
