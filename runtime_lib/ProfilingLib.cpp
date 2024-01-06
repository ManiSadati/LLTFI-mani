#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <string>

struct layerProfCycle {
  int layerNo;
  std::string layerName;
  long long unsigned cycleStart;
  long long unsigned cycleEnd;

  layerProfCycle(int layerNo, std::string layerName) {
    this->layerNo = layerNo;
    this->layerName = layerName;
    this->cycleStart = -1;
    this->cycleEnd = -1;
  }

  void registerCycle (long long unsigned cycle) {
    if (this->cycleStart == -1) {
      this->cycleStart = cycle;
    }

    this->cycleEnd = cycle;
  }
};

static std::vector<layerProfCycle> layerProfileInfo;
static std::vector<std::vector<int> > patternInfoConv, patternInfoFC;
static std::vector<float* > ptrInfo;
static std::vector<std::vector<int> > layerInfo;
static int64_t globalLayerNo = 0;
static layerProfCycle *currentLayer = NULL;


// Export these functions in C dilect.
extern "C" {
#include "Utils.h"

static long long unsigned opcodecount[OPCODE_CYCLE_ARRAY_LEN] = {0};
static long long unsigned globalCycle = 0;

void lltfiMLLayer(int64_t layerName, int64_t start) {

  assert(start == 1 || start == 2 && "Layer start is denoted by 1 and end by 2");

  int64_t *layerNamePtr = &layerName;
  char* layerNameStr = (char*)layerNamePtr;

  if (start == 1) { /* Layer started. */
    globalLayerNo++;
    currentLayer = new layerProfCycle(globalLayerNo, std::string(layerNameStr));
  }
  else {

    layerProfileInfo.push_back(*currentLayer);
    delete currentLayer;
    currentLayer = NULL;
  }
}

void doProfiling(int opcode) {
  assert(opcodecount[opcode] >= 0 &&
         "dynamic instruction number too large to be handled by llfi");
  opcodecount[opcode]++;
  globalCycle++;
  if (currentLayer != NULL)
    currentLayer->registerCycle(globalCycle);
}

void YYYYYYYYYYY(float x) {
}

// int64_t* ptr,
void doProfiling_PatternConv( float* ptr1, float* ptr2 ,int64_t opcode1, int64_t opcode2, int64_t opcode3, int64_t opcode4, int64_t opcode5, int64_t opcode6 , int64_t opcode7, int64_t opcode8, int64_t opcode9) {
  fprintf(stderr,"Pattern opcode \n");
  std::vector<int> v;
  int b = opcode2;
  int c = opcode3;
  int h = opcode4;
  int w = opcode5;
  int sumb = opcode6;
  int sumc = opcode7;
  int sumh = opcode8;
  int sumw = opcode9;

  fprintf(stderr," output dimensions: b = %d c = %d h = %d w = %d\n",b, c, h, w);
  for(int i_b = 0 ; i_b < b; i_b++){
    fprintf(stderr,"data (%d)\n", i_b);
    for(int i_c = 0 ; i_c < c; i_c++){
      fprintf(stderr,"[[\n");
      for(int i_h = 0 ; i_h < h; i_h++){
        fprintf(stderr,"[");
        for(int i_w = 0 ; i_w < w; i_w++){
          int index = i_b * sumb + i_c * sumc + i_h * sumh + i_w * sumw;
          float out = ptr2[index];
          // if(i_c == 0 && i_b == 0){
          //   ptr2[index] = 1000.;
          // }
          fprintf(stderr,"%f (%f), ", out, ptr2[index]);
        }
        fprintf(stderr,"]\n");
      }
      fprintf(stderr,"]]\n");
    }
    fprintf(stderr,"\n\n");
  }


  v.push_back(ptr1[0]);
  v.push_back(ptr2[0]);
  ptrInfo.push_back(ptr2);
  v.push_back(opcode1);
  v.push_back(opcode2);
  v.push_back(opcode3);
  v.push_back(opcode4);
  v.push_back(opcode5);
  v.push_back(opcode6);
  v.push_back(opcode7);
  v.push_back(opcode8);
  v.push_back(opcode9);
  patternInfoConv.push_back(v);
  // fprintf(stderr,"Pattern opcode = %ld , %ld %ld %ld %ld , %ld  \n",opcode1, opcode2, opcode3, opcode4, opcode5, opcode6);
}


// int64_t* ptr,
void doProfiling_PatternFC( float* ptr1, float* ptr2 ,int64_t opcode1, int64_t opcode2, int64_t opcode3, int64_t opcode4, int64_t opcode5) {
  fprintf(stderr,"Pattern opcode \n");
  std::vector<int> v;
  int b = opcode2;
  int n = opcode3;
  int sumb = opcode4;
  int sumn = opcode5;

  fprintf(stderr," output dimensions: b = %d n = %d\n",b, n);
  for(int i_b = 0 ; i_b < b; i_b++){
    fprintf(stderr,"data (%d)\n", i_b);
    for(int i_n = 0 ; i_n < n; i_n++){
      int index = i_b * sumb + i_n * sumn;
      float out = ptr2[index];
      if(i_n == 5){
        ptr2[index] = 1000.;
      }
      fprintf(stderr,"%f (%f), ", out, ptr2[index]);
    }
    fprintf(stderr,"\n\n");
  }


  v.push_back(ptr1[0]);
  v.push_back(ptr2[0]);
  ptrInfo.push_back(ptr2);
  v.push_back(opcode1);
  v.push_back(opcode2);
  v.push_back(opcode3);
  v.push_back(opcode4);
  v.push_back(opcode5);
  patternInfoFC.push_back(v);
  // fprintf(stderr,"Pattern opcode = %ld , %ld %ld %ld %ld , %ld  \n",opcode1, opcode2, opcode3, opcode4, opcode5, opcode6);
}


void endProfiling() {
  FILE *profileFile;
  char profilefilename[80] = "llfi.stat.prof.txt";
  profileFile = fopen(profilefilename, "w");
  if (profileFile == NULL) {
    fprintf(stderr, "ERROR: Unable to open profiling result file %s\n",
            profilefilename);
    exit(1);
  }

  int opcode_cycle_arr[OPCODE_CYCLE_ARRAY_LEN];
  getOpcodeExecCycleArray(OPCODE_CYCLE_ARRAY_LEN, opcode_cycle_arr);

  unsigned i = 0;
  long long unsigned total_cycle = 0;
  for (i = 0; i < 100; ++i) {
    assert(total_cycle >= 0 &&
            "total dynamic instruction cycle too large to be handled by llfi");
    if (opcodecount[i] > 0) {
      assert(opcode_cycle_arr[i] >= 0 &&
          "opcode does not exist, need to update instructions.def");
      total_cycle += opcodecount[i] * opcode_cycle_arr[i];
    }
  }

  fprintf(profileFile, "# do not edit\n");
  fprintf(profileFile,
          "# cycle considered the execution cycle of each instruction type\n");
  fprintf(profileFile, "total_cycle=%lld\n", total_cycle);

  for (auto layer : layerProfileInfo) {
    fprintf(profileFile, "ml_layer=%d,%s,%lld,%lld\n", layer.layerNo,
            layer.layerName.c_str(), layer.cycleStart, layer.cycleEnd);
  }

  for (auto vec : patternInfoConv) {
    fprintf(profileFile, "new pattern Conv layer=" );
    for (auto op: vec){
      fprintf(profileFile, "%d ",op );
    }
    fprintf(profileFile, "\n" );
  }

  for (auto vec : patternInfoFC) {
    fprintf(profileFile, "new pattern FC layer=" );
    for (auto op: vec){
      fprintf(profileFile, "%d ",op );
    }
    fprintf(profileFile, "\n" );
  }

	fclose(profileFile);
}
} // End of extern "C"
