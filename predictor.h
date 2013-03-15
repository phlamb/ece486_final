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


#define EXTRACT_TRACE 0

#if EXTRACT_TRACE
    #include <stdio.h>
    #include <stdlib.h>
#endif

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

#define TP_INDEX_BITS           10 
#define TP_OFFSET_MASK          ((1 << 16) - 1)
#define TP_INDEX_MASK           (((1 << TP_INDEX_BITS) - 1) & ((1 << 16) - 1))
#define TP_INDEX_SHIFT_BITS     0
#define TP_STACK_SIZE           16

struct TargetPredictorStorage
{
    //History cache
    uint32_t    history[(1 << TP_INDEX_BITS)];

    //Call location stack
    uint32_t    stack[TP_STACK_SIZE];
    uint32_t    stack_top;
    uint32_t    stack_bottom;
};

class PREDICTOR
{
public:
    PREDICTOR();
    ~PREDICTOR();
    bool get_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);

private:
    uint8_t choose_predictor();
    bool get_branch_prediction(const branch_record_c* br, const op_state_c* os);
    void update_branch_predictor(const branch_record_c* br, const op_state_c* os, bool taken);
    void get_target_prediction(const branch_record_c* br, const op_state_c* os, uint *predicted_target_address);
    void update_target_predictor(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);
    void update_global_history(bool taken);
    void update_choose_predictor(bool taken);
    void update_global_prediction(bool taken);
    
#ifdef EXTRACT_TRACE
    //Functions to create ascii trace files
    void extract_trace(const branch_record_c* br, const op_state_c* os);
    void extract_trace_update(const branch_record_c* br, const op_state_c* os, bool taken, uint actual_target_address);
#endif //EXTRACT_TRACE

    //private variables
    AlphaPredictorStorage alpha;
    TargetPredictorStorage tgtpred;
    bool last_local_prediction;
    bool last_global_prediction;

#if EXTRACT_TRACE
    FILE *tracefp;
#endif
};


#endif //PREDICTOR_H_SEEN

