

#include "predictor.h"

PREDICTOR::PREDICTOR()
{
#if EXTRACT_TRACE
   tracefp = fopen("extract.trace", "w"); 
   printf("Creating trace file extract.trace\n");
#endif
    //Initialize alpha predictor storage to 0
    memset(&alpha, 0, sizeof(AlphaPredictorStorage));
}

PREDICTOR::~PREDICTOR()
{
#if EXTRACT_TRACE
    fclose(tracefp);
    printf("Closing trace file\n");

#endif 
}

uint8_t PREDICTOR::choose_predictor()
{
    return PREDICTOR_LOCAL;
}

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{
#if EXTRACT_TRACE
    extract_trace(br, os);
    return true;
#endif

    bool prediction = get_branch_prediction(br, os);

    get_target_prediction(br, os, predicted_target_address);

    return prediction;
}

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{
#if EXTRACT_TRACE
    extract_trace_update(br, os, taken, actual_target_address);
    return;
#endif
    //update branch predictor
    update_branch_predictor(br, os, taken);
}


//======================================
// Branch Predictor Implementation
// ====================================
bool PREDICTOR::get_branch_prediction(const branch_record_c* br, const op_state_c* os)
{
    bool prediction;
    if(!br->is_conditional)
        return true;

    if(choose_predictor() == PREDICTOR_LOCAL)
    {
        int index = br->instruction_addr & (1023);
        //mask and use MSB to determine whether to branch (0-3: not taken, 4-7: taken)
        prediction = (bool)alpha.localPrediction[alpha.localHistory[index]] & 0x4;
        return prediction;
    }
    else //PREDICTOR_GLOBAL
    {
        return false; 
    }
}

void PREDICTOR::update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
    //Update branch history pattern
    int index = br->instruction_addr & (1023);

    //Update prediction (BEFORE! updating history (index into prediction))
    if(taken)
    {
        //If taken, increment counter, saturate at 7
        if(alpha.localPrediction[alpha.localHistory[index]] < 7)
            alpha.localPrediction[alpha.localHistory[index]] += 1;
    }
    else //not taken
    {
        //not taken, decrement counter, saturate at 0
        if(alpha.localPrediction[alpha.localHistory[index]] > 0)
            alpha.localPrediction[alpha.localHistory[index]] -= 1;
    }

    //Update local history
    //shift history
    alpha.localHistory[index] <<= 1;
    //mask off bits higher than 10 used bits
    alpha.localHistory[index] &= (1023);
    //If taken, put 1 (taken) in lsb, if not taken 0 already in lsb from <<
    if(taken)
        alpha.localHistory[index] |= 1;

    //Update prediction counter
}
//======================================
// Target Predictor Implementation
// ====================================
void PREDICTOR::get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{

}

void PREDICTOR::update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{

}

//=====================================
// Trace Extraction
//=====================================
void PREDICTOR::extract_trace(const branch_record_c* br, const op_state_c* os)
{
#if EXTRACT_TRACE
    fprintf(tracefp, "%x %d %d %d", br->instruction_addr, (int)br->is_conditional, (int)br->is_call, (int)br->is_return);
#endif
}

void PREDICTOR::extract_trace_update(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{
#if EXTRACT_TRACE
    fprintf(tracefp, " %d %x\n", (int)taken, actual_target_address);
#endif
}
