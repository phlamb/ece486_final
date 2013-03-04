

#include "predictor.h"

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{
    bool prediction = get_branch_prediction(br, os);

    get_target_prediction(br, os, predicted_target_address);

    return prediction;
}

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{

}


//======================================
// Branch Predictor Implementation
// ====================================
bool PREDICTOR::get_branch_prediction(const branch_record_c* br, const op_state_c* os)
{

    bool prediction = true;
    if(!br->is_conditional)
        return true;

    if(chosePredictor == PREDICTOR_LOCAL)
    {
        int index = br->instruction_addr & 1024;
        return (bool)alpha.localPrediction[localHistory[index]] & 0x1;
    }
    else //PREDICTOR_GLOBAL
    {
        return false; 
    }


    return prediction;   // true for taken, false for not taken
    
}

void PREDICTOR::update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{

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
