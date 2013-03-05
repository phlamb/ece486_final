/* Author: Mark Faust
 *
 * C version of predictor file
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <cstring>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class

#define PREDICTOR_LOCAL 0
#define PREDICTOR_GLOBAL 1

struct AlphaPredictorStorage
{
    uint16_t    localHistory[1024];     //Size: 10b x 1024
    uint8_t     localPrediction[1024];  //Size: 3b x 1024
    uint8_t     globalPrediction[4096]; //Size: 2b x 4096
    uint8_t     choicePrediction[4096]; //Size: 2b x 4096
    //Total Size: 3.712x10^3 Bytes 
    // 2^(11.858) Bytes
    // < 4k Bytes
    uint16_t    globalHistory;          //Size: 12b
};

class PREDICTOR
{
public:
    PREDICTOR();
    bool get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);

private:
    uint8_t choose_predictor();
    bool get_branch_prediction(const branch_record_c* br, const op_state_c* os);
    void update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken);
    void get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);
    void update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);


    //private variables
    AlphaPredictorStorage alpha;
};


#endif // PREDICTOR_H_SEEN

