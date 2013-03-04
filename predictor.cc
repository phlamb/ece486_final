

#include "predictor.h"

#define PREDICTOR_LOCAL 0
#define PREDICTOR_GLOBAL 1

static bool get_branch_prediction(const branch_record_c* br, const op_state_c* os);
static void update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken);
static void get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);
static void update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);

struct AlphaPredictorStorage
{
    uint16_t    localHistory[1024];     //Size: 10b x 1024
    uint8_t     localPrediction[1024];  //Size: 3b x 1024
    uint8_t     globalPrediction[4096]; //Size: 2b x 4096
    uint8_t     choicePrediction[4096]; //Size: 2b x 4096
    //Total Size: 3.712x10^3 Bytes 
    // 2^(11.858) Bytes
    // < 4k Bytes
};

static AlphaPredictorStorage alpha;

static uint8_t choosePredictor()
{
    return PREDICTOR_LOCAL;
}

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
    int index = br->instruction_addr & 1024;
    alpha.localPrediction[alpha.localHistory[index]] <<= 1;
    if(!taken) //if not taken, << puts a 0 in lsb
    {
        alpha.localPrediction[alpha.localHistory[index]] |= 1;
    }
}


//======================================
// Branch Predictor Implementation
// ====================================
static bool get_branch_prediction(const branch_record_c* br, const op_state_c* os)
{

    //br->debug_print();
    bool prediction = true;
    if(!br->is_conditional)
        return true;

    if(choosePredictor() == PREDICTOR_LOCAL)
    {
        int index = br->instruction_addr & 1024;
        return (bool)alpha.localPrediction[alpha.localHistory[index]] & 0x1;
    }
    else //PREDICTOR_GLOBAL
    {
        return false; 
    }


    return prediction;   // true for taken, false for not taken
    
}

static void update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{

}
//======================================
// Target Predictor Implementation
// ====================================
static void get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address)
{

}

static void update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address)
{

}
