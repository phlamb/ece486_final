#include "predictor.h"

//-----------------------------------------------
// PREDICTOR
//
// PREDICTOR constructor.  Initialize branch
// and branch target predictor data structures.
//-----------------------------------------------
PREDICTOR::PREDICTOR()
{
    //Initialize alpha predictor storage to 0
    memset(&alpha, 0, sizeof(AlphaPredictorStorage));
    
    //Initialize counters to weakly taken
    memset(&alpha.localHistory, 0, sizeof(alpha.localHistory));
    memset(&alpha.localPrediction, 2, sizeof(alpha.localPrediction));
    memset(&alpha.globalPrediction, 1, sizeof(alpha.globalPrediction));
    memset(&alpha.choicePrediction, 1, sizeof(alpha.choicePrediction));
    //Initialize target predictor data structure
    memset(&tgtpred, 0, sizeof(TargetPredictorStorage));
}

//-----------------------------------------------
// ~PREDICTOR
//
// PREDICTOR destructor
//-----------------------------------------------
PREDICTOR::~PREDICTOR()
{
}

//-----------------------------------------------
// get_prediction
//
// Predict branch taken/not taken and predict
// branch target.
//-----------------------------------------------
bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{
    bool prediction = get_branch_prediction(br, os);
    
    get_target_prediction(br, os, predicted_target_address);

    return prediction;
}

//-----------------------------------------------
// update_predictor
//
// Update predictors based on feedback after
// branch is taken or not taken.
//-----------------------------------------------
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{
    //update local branch predictor
    update_local_predictor(br, os, taken);
    //update global branch predictor
    update_global_prediction(taken);
    //update tournament chooser
    update_choose_predictor(taken);
    //update global path history (This must be done AFTER the global branch
    //predictor counters and predictor chooser are updated.
    update_global_history(taken);
    
    //update branch target predictor
    update_target_predictor(br,os,taken,actual_target_address);
}

//======================================
// Branch Predictor Implementation
// ====================================

//-----------------------------------------------
// get_branch_prediction
//
// Get result of branch prediction (taken/not
// taken).  
//-----------------------------------------------
bool PREDICTOR::get_branch_prediction(const branch_record_c* br, const op_state_c* os)
{
    bool prediction;
    if(!br->is_conditional)
        return true;


    //mask and use MSB to determine whether to branch (0-3: not taken, 4-7: taken)
    int index = br->instruction_addr & (1023);
    last_local_prediction = (bool)(alpha.localPrediction[alpha.localHistory[index]] & 0x4);

    last_global_prediction = (bool)(alpha.globalPrediction[alpha.globalHistory] & 0x2);


    if(choose_predictor() == PREDICTOR_LOCAL)
    {
        return last_local_prediction;
    }
    else //PREDICTOR_GLOBAL
    {
        return last_global_prediction;
    }
}

//-----------------------------------------------
// update_local_predictor
//
// Updates branch predictor local history and 
//  local prediction counters
//-----------------------------------------------
void PREDICTOR::update_local_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
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
}

//======================================
// Global Predictor implementation
// =====================================

//-----------------------------------------------
// update_global_history
//
// Update global path history, used to index
// into global target predictor counter table.
//-----------------------------------------------
void PREDICTOR::update_global_history(bool taken)
{
    if(taken)
    {
        alpha.globalHistory <<= 1;
        alpha.globalHistory += 1;        
        alpha.globalHistory &= (4095); // increment path history and mask upper 4 bits
    }
    else //not taken
    {
        (alpha.globalHistory <<= 1);
         alpha.globalHistory &= (4095);// left shift global history and mask upper 4 bits
    }
}

//-----------------------------------------------
// update_global_prediction
//
// Update global prediction counters.  This MUST
// be done BEFORE the global history is updated.
//-----------------------------------------------
void PREDICTOR::update_global_prediction(bool taken)
{
    if(taken)
    {
        // If Taken, increment counter and saturate at 3
        if(alpha.globalPrediction[alpha.globalHistory] < 3)
            alpha.globalPrediction[alpha.globalHistory] += 1;
    }
    else //not taken
    {
        // If not taken, decrement counter and saturate at 0
        if (alpha.globalPrediction[alpha.globalHistory] > 0)
            alpha.globalPrediction[alpha.globalHistory] -= 1;
    }
}

//======================================
// Predictor Chooser Implementation
// ====================================

//-----------------------------------------------
// choose_predictor
//
// Returns which predictor to use (global or 
// local) based on the value of the chooser 
// counter for the current global history
// pattern.
//-----------------------------------------------
uint8_t PREDICTOR::choose_predictor()
{
    // If counter is 2 or 3 predict local. 1 or 2 predict global
    if(alpha.choicePrediction[alpha.globalHistory] & 2)
    {
       // printf("L");
        return PREDICTOR_LOCAL;
    }
    else
    {
       // printf("G");
        return PREDICTOR_GLOBAL;
    }
    
}

//-----------------------------------------------
// update_choose_predictor
//
// Update predictor chooser (chooses whether to 
// use global predictor or local predictor).  If
// both predictors predict the same, whether 
// correct or incorrect, don't update chooser.
// If one predictor chooses correctly and the
// other incorrectly, adjust counter towards
// the predictor which was correct.
// 
// This must be updated BEFORE global path 
// history is updated.
//-----------------------------------------------
void PREDICTOR::update_choose_predictor(bool taken)
{
    //If both predicted correctly, or both predicted incorrectly,
    //don't update chooser.
    if(last_local_prediction == last_global_prediction)
    {
        return;
    }

    //If global predicts correctly and local predicts incorrectly, 
    //update chooser towards global
    if(taken == last_global_prediction && alpha.choicePrediction[alpha.globalHistory] > 0)
    {
        alpha.choicePrediction[alpha.globalHistory] -= 1;
    }

    //If local predicts correctly and local predicts incorrectly, 
    //update chooser towards local
    if(taken == last_local_prediction && alpha.choicePrediction[alpha.globalHistory] < 3)
    {
        alpha.choicePrediction[alpha.globalHistory] += 1;
    }
}

//==========================================
// Target Predictor Implementation 
// =========================================

//-----------------------------------------------
// get_target_prediction
//
// Gets predicted branch target address.
//-----------------------------------------------
void PREDICTOR::get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{
    //If this is a return, pop from call address stack
    if(br->is_return)
    {
        *predicted_target_address = tgtpred.stack[tgtpred.stack_top];
        tgtpred.stack_top = (tgtpred.stack_top - 1) % TP_STACK_SIZE;
        return;
    }

    //Fetch prediction from branch target cache
    uint index = br->instruction_addr >> TP_INDEX_SHIFT_BITS;
    *predicted_target_address = tgtpred.history[(index & TP_INDEX_MASK)];
}

//-----------------------------------------------
// update_target_predictor
//
// Updates branch target predictor once result
// of branch is known.
//-----------------------------------------------
void PREDICTOR::update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{
    //if the branch was a call, push next address following branch instruction onto call address stack
    // if stack is full, wrap around and overwrite oldest value on stack
    if(br->is_call)
    {
        tgtpred.stack_top = tgtpred.stack_top + 1;
        if(tgtpred.stack_top > TP_STACK_SIZE - 1)
        {
            tgtpred.stack_top %= TP_STACK_SIZE;
            tgtpred.stack_bottom = (tgtpred.stack_bottom + 1) % TP_STACK_SIZE;
        }
        tgtpred.stack[tgtpred.stack_top] = br->instruction_next_addr;
    }

    uint index = br->instruction_addr >> TP_INDEX_SHIFT_BITS;
    tgtpred.history[(index & TP_INDEX_MASK)] = actual_target_address;
}
