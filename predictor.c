//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Mahima Rathore";
const char *studentID   = "A53297810";
const char *email       = "mrathore@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//GSHARE
uint32_t gs_ghr;
uint8_t *gs_bht;
//TOURNAMENT
uint32_t global_ghr;
uint32_t *local_pht;
uint8_t *local_bht;
uint8_t *choice_bht;
uint8_t *global_bht;
uint32_t gs_bht_idx;
uint8_t gs_prediction;
uint32_t local_pht_idx,local_bht_idx,global_bht_idx,choice_prediction;
uint8_t  local_outcome, global_outcome;

//CUSTOM - TOURNAMENT
#define custom_ghistoryBits 13
#define custom_lhistoryBits 11
#define custom_pcIndexBits 11
uint32_t custom_gs_ghr;
uint8_t *custom_gs_bht;
uint32_t *custom_local_pht;
uint8_t *custom_local_bht;
uint8_t *custom_choice_bht;
uint32_t custom_gs_bht_idx;
uint8_t custom_gs_prediction;
uint32_t custom_local_pht_idx,custom_local_bht_idx,custom_global_bht_idx,custom_choice_prediction;
uint8_t  custom_local_outcome, custom_gs_outcome;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
void update_ghr_bht(uint8_t *cntr, uint8_t outcome) {
    if (outcome == NOTTAKEN) {
        if (*cntr != SN) {
            (*cntr)--;
        }
    } else {
        if (*cntr != ST) {
            (*cntr)++;
        }
    }
}

void update_custom_ghr_bht(uint8_t *cntr, uint8_t outcome) {
    if (outcome == NOTTAKEN) {
        if (*cntr != 0) {
            (*cntr)--;
        }
    } else {
        if (*cntr != 7) {
            (*cntr)++;
        }
    }
}

void custom_init() {
  //gshare
  custom_gs_ghr = 0 ; //Initialized to 00000
  custom_gs_bht = malloc(3*(1<<custom_ghistoryBits));
  for(int i=0;i<(1<<custom_ghistoryBits);i++)
     custom_gs_bht[i] = WN; //Initialized to Weakly Not Taken State
  //local_bht
  custom_local_bht = malloc(3*(1<<custom_lhistoryBits));
  for(int i=0;i<(1<<custom_lhistoryBits);i++)
    custom_local_bht[i] = WN;

  custom_local_pht = malloc(custom_lhistoryBits*(1 << custom_pcIndexBits));
  for(int i=0;i<(1 << custom_pcIndexBits);i++)
    custom_local_pht[i] = 0;

  custom_choice_bht = malloc(3*(1<<custom_ghistoryBits));
  for(int i=0;i<(1<<custom_ghistoryBits);i++)
    custom_choice_bht[i] = WN;
}

uint8_t custom_predict(uint32_t pc){
  //gshare
  custom_gs_bht_idx = ((pc)^custom_gs_ghr) & ((1 << custom_ghistoryBits) - 1);
  custom_gs_prediction = custom_gs_bht[custom_gs_bht_idx];
  custom_gs_outcome = (custom_gs_prediction == WN || custom_gs_prediction == SN) ? NOTTAKEN : TAKEN;
  //return custom_gs_outcome;
  //local predictor
  custom_local_pht_idx = (pc) & ((1 << custom_pcIndexBits) - 1);
  custom_local_bht_idx = custom_local_pht[custom_local_pht_idx];
  custom_local_outcome = ((custom_local_bht[custom_local_bht_idx] == WN || custom_local_bht[custom_local_bht_idx] == SN) ? NOTTAKEN : TAKEN);
  //chooser
  custom_choice_prediction = custom_choice_bht[custom_gs_bht_idx];

  return (custom_choice_prediction == WN || custom_choice_prediction == SN) ? custom_gs_outcome : custom_local_outcome;

}

void custom_train(uint32_t pc, uint8_t outcome){
  //train gshare
  update_custom_ghr_bht(&custom_gs_bht[custom_gs_bht_idx], outcome);
  custom_gs_ghr = ((custom_gs_ghr<<1) & (1 << custom_ghistoryBits) - 1) | outcome;
  //Train chooser: choice bht
  if (custom_local_outcome != custom_gs_outcome)
      update_custom_ghr_bht(&custom_choice_bht[custom_gs_bht_idx], (custom_gs_outcome == outcome) ? NOTTAKEN : TAKEN);
  //Train local bht & local pht
  custom_local_pht_idx = (pc) & ((1 << custom_pcIndexBits) - 1);
  custom_local_bht_idx = custom_local_pht[custom_local_pht_idx];
  update_custom_ghr_bht(&(custom_local_bht[custom_local_bht_idx]), outcome);
  custom_local_pht[custom_local_pht_idx] = ((custom_local_pht[custom_local_pht_idx] << 1) & ((1 << custom_lhistoryBits) - 1)) | outcome;
}



// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  switch(bpType) {
      case STATIC:
        break;

      case GSHARE:
        gs_ghr = 0 ; //Initialized to 00000
        gs_bht = (uint8_t*) malloc(sizeof(uint8_t)*(1<<ghistoryBits));
        for(int i=0;i<(1<<ghistoryBits);i++)
           gs_bht[i] = WN; //Initialized to Weakly Not Taken State
        break;

      case TOURNAMENT:
        global_ghr = 0;

        local_bht = (uint8_t*) malloc(sizeof(uint8_t)*(1<<lhistoryBits));
        for(int i=0;i<(1<<lhistoryBits);i++)
          local_bht[i] = WN;

        local_pht = (uint32_t*) malloc(sizeof(uint32_t)*(1 << pcIndexBits));
        for(int i=0;i<(1 << pcIndexBits);i++)
          local_pht[i] = 0;

        global_bht = (uint8_t*) malloc(sizeof(uint8_t)*(1<<ghistoryBits));
        for(int i=0;i<(1<<ghistoryBits);i++)
          global_bht[i] = WN;

        choice_bht = (uint8_t*) malloc(sizeof(uint8_t)*(1<<ghistoryBits));
        for(int i=0;i<(1<<ghistoryBits);i++)
          choice_bht[i] = WN;

        break;

      case CUSTOM:
        custom_init();
        break;
    }

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  //uint32_t gs_bht_idx = ((pc>>3)^gs_ghr) & ((1 << ghistoryBits) - 1);
  //uint8_t gs_prediction = gs_bht[gs_bht_idx];

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;

    case GSHARE:
      gs_bht_idx = ((pc)^gs_ghr) & ((1 << ghistoryBits) - 1);
      gs_prediction = gs_bht[gs_bht_idx];
      return (gs_prediction == WN || gs_prediction == SN) ? NOTTAKEN : TAKEN;
      break;

    case TOURNAMENT:
      //local predictor
      local_pht_idx = (pc) & ((1 << pcIndexBits) - 1);
      local_bht_idx = local_pht[local_pht_idx];
      local_outcome = ((local_bht[local_bht_idx] == WN || local_bht[local_bht_idx] == SN) ? NOTTAKEN : TAKEN);
      //global predictor
      global_bht_idx = (global_ghr) & ((1 << ghistoryBits) - 1);
      global_outcome = ((global_bht[global_bht_idx] == WN || global_bht[global_bht_idx] == SN) ? NOTTAKEN : TAKEN);
      //chooser
      choice_prediction = choice_bht[global_bht_idx];
      // Negative means global predictor, positive means local predictor
      return (choice_prediction == WN || choice_prediction == SN) ? global_outcome : local_outcome;

    case CUSTOM:
      return custom_predict(pc);

    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  switch (bpType) {
    case STATIC:
        break;

    case GSHARE:
        update_ghr_bht(&gs_bht[((pc)^gs_ghr) & ((1 << ghistoryBits) - 1)], outcome);
        gs_ghr = ((gs_ghr<<1) & (1 << ghistoryBits) - 1) | outcome;
        break;

    case TOURNAMENT:
        //Train chooser: choice bht
        if (local_outcome != global_outcome)
            update_ghr_bht(&choice_bht[global_ghr], (global_outcome == outcome) ? NOTTAKEN : TAKEN);
        //Train local bht & local pht
        local_pht_idx = (pc) & ((1 << pcIndexBits) - 1);
        local_bht_idx = local_pht[local_pht_idx];
        update_ghr_bht(&(local_bht[local_bht_idx]), outcome);
        local_pht[local_pht_idx] = ((local_pht[local_pht_idx] << 1) & ((1 << lhistoryBits) - 1)) | outcome;
        //Train global bht
        update_ghr_bht(&global_bht[global_ghr], outcome);
        global_ghr = ((global_ghr << 1) & ((1 << ghistoryBits) - 1)) | outcome;
        break;

    case CUSTOM:
        custom_train(pc,outcome);
        break;

    default:
        break;
    }


}
