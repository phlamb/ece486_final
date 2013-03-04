

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
    //Update branch history pattern
    int index = br->instruction_addr & (1023);
    //shift history
    alpha.localHistory[index] <<= 1;
    //mask off bits higher than 10 used bits
    alpha.localHistory[index] &= (1023);

    //If taken, put 1 (taken) in lsb, if not taken 0 already in lsb from <<
    if(taken)
        alpha.localHistory[index] |= 1;

    //Update prediction counter
    if(taken)
    {
        //If taken, increment counter, saturate at 7
        if(alpha.localPrediction[alpha.localHistory[index]] < 7)
            alpha.localPrediction[alpha.localHistory[index]] += 1;
    }
    else //not taken
    {
        //not taken, decrement counter, saturate at 0
        //if(alpha.localPrediction[alpha.localHistory[index]] > 0)
         //   alpha.localPrediction[alpha.localHistory[index]] -= 1;
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
        int index = br->instruction_addr & (1023);
        //mask and use MSB to determine whether to branch (0-3: not taken, 4-7: taken)
        return (bool)alpha.localPrediction[alpha.localHistory[index]] & 0x4;
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
