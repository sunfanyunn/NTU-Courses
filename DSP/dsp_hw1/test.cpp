#include "hmm.h"
#include <cstdio>
#include <utility>
#include <string>
#include <cassert>
#include <algorithm>

static double delta[MAX_SEQ][MAX_STATE];

static double solve(HMM *model, std::string s) {
  int sn = model->state_num;
  int ln = (int)s.size();
  for(int i = 0; i < sn; i++)
    delta[0][i] = model->initial[i] * model->observation[s[0]-'A'][i];

  for(int t = 1; t < ln; t++) {
    for(int i = 0; i < sn; i++) {
      //solve for delta[t][i]
      double mx = 0;
      for(int j  =0; j < sn; j++)
        mx = std::max(mx, delta[t-1][j]*model->transition[j][i]);

      delta[t][i] = mx * model->observation[s[t]-'A'][i];
    }
  }
  return *std::max_element(delta[ln-1], delta[ln-1]+sn);
}
static std::pair<int, double> go(std::string test, HMM models[], int nmodels) {
  // returns the answer in the form (ans_id, prob)
  int ansid = -1;
  double mx = 0;
  for(int i = 0; i < nmodels; i++) {
    double prob = solve(models + i, test);
    //printf("%f", prob);
    if(prob > mx) 
      mx = prob, ansid = i;
  }
  assert(ansid != -1);
  return std::make_pair(ansid, mx);
}

int main(int argc, char *argv[]) {
  assert(argc == 4);
  constexpr int max_num = 10;
  HMM models[max_num];
  int nmodels = load_models(argv[1], models, max_num);
  char line[MAX_LINE];
  FILE *fin = fopen(argv[2], "r");
  FILE *fout = fopen(argv[3], "w");
  while(fscanf(fin, "%s", line) > 0) {
    std::pair<int, double> res = go(std::string(line), models, nmodels); 
    //fprintf(stdout, "%s %e\n", models[res.first].model_name, res.second);
    fprintf(fout, "%s %e\n", models[res.first].model_name, res.second);
  }
  fclose(fin);fclose(fout);
}
