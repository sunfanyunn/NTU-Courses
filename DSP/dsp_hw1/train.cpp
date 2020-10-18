#include "hmm.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <string>
#include <numeric>

static std::vector<std::string> load_training_data(char *filename) {
  std::vector<std::string> ret;
  char line[MAX_LINE];
  FILE *f = fopen(filename, "r");
  while(fscanf(f, "%s", line) > 0) ret.push_back(line);
  fclose(f);
  return ret;
}


static double alpha[MAX_SEQ][MAX_STATE];
static double beta[MAX_SEQ][MAX_STATE];
// be careful, gamma sum's dimension
static double gamma_sum[MAX_STATE][MAX_OBSERV];
// !! update rule for transition[i][j] needs to subtract tails
static double gamma_sum_end[MAX_STATE];
static double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];
static double epsilon_sum[MAX_STATE][MAX_STATE];
static double initial[MAX_STATE];

static void init() {
  memset(gamma_sum, 0, sizeof(gamma_sum));
  memset(gamma_sum_end, 0, sizeof(gamma_sum_end));
  memset(epsilon_sum, 0, sizeof(epsilon_sum));
  memset(initial, 0, sizeof(initial));
}
static void init_alpha(HMM *model, std::string sample) {
  int sn = model->state_num;
  for(int i = 0; i < sn; i++) {
    alpha[0][i] = model->initial[i] * model->observation[sample[0]-'A'][i];
  }
   

  for(int t = 1; t < (int)sample.size(); t++) {
    for(int i = 0; i < sn; i++) {
      double sum = 0;
      for(int j = 0; j< sn; j++)
        sum += alpha[t-1][j] * model->transition[j][i];
      alpha[t][i] = sum * model->observation[sample[t]-'A'][i];
    }
  }
}
static void init_beta(HMM *model, std::string sample) {
  int sn = model->state_num;
  int ln = (int)sample.size();
  for(int i = 0; i < sn; i++) beta[ln-1][i] = 1;
  for(int t = ln-2; t >= 0; t--) {
    for(int i = 0; i < sn; i++) {
      // solving for beta[t][i]
      beta[t][i] = 0;
      for(int j = 0; j < sn; j++) {
        beta[t][i] += model->transition[i][j] * beta[t+1][j] * model->observation[sample[t+1]-'A'][j];
      }
    }
  }
}
// Add initial as well
static void add_gamma(HMM *model, std::string sample) {

  int ln = (int)sample.size();
  int sn = model->state_num;
  for(int t = 0; t < ln; t++) {
    double sum = 0;
    for(int i = 0; i < sn; i++) sum += alpha[t][i] * beta[t][i];

    for(int i = 0; i < sn; i++) {
      //gamma[t][i]
      double gamma = alpha[t][i] * beta[t][i] / sum;
      if (!t)
        initial[i] += gamma;
      if (t==ln-1)
        gamma_sum_end[i] += gamma;
      // adding gamma[t][i] to gamma_sum[t][i]
      gamma_sum[i][sample[t]-'A'] += gamma;
    }
  }
}
static void add_epsilon(HMM *model, std::string sample) {
  int ln = (int)sample.size();
  int sn = model->state_num;
  for(int t = 0; t < ln-1; t++) {
    double sum = 0;
    for(int i = 0; i < sn; i++) 
      for(int j = 0; j < sn; j++) {
        epsilon[t][i][j] =
          alpha[t][i] * 
          model->transition[i][j] * 
          model->observation[sample[t+1]-'A'][j] *
          beta[t+1][j];
        sum += epsilon[t][i][j];
      }
  
    for(int i = 0; i < sn; i++)
      for(int j = 0; j < sn; j++)
        epsilon[t][i][j] /= sum,
        epsilon_sum[i][j] += epsilon[t][i][j];
  }
}
static void update_parameters(HMM *model, int nsamples) {
  int sn = model->state_num;
  int obn = model->observ_num;
  // update initial
  for(int i = 0; i < sn; i++) model->initial[i] = initial[i]/nsamples;
  //update transition[i][j]
  for(int i = 0; i < sn; i++) {
    for(int j = 0; j < sn; j++) {
      model->transition[i][j] = 
        epsilon_sum[i][j] / 
        (std::accumulate(gamma_sum[i], gamma_sum[i]+obn, 0.) - gamma_sum_end[i]);
    }
  }

  for(int i = 0; i < obn; i++) {
    for(int j = 0; j < sn; j++) {
      //observing i in state j
      model->observation[i][j] = gamma_sum[j][i] / std::accumulate(gamma_sum[j], gamma_sum[j]+obn, 0.);
    }
  }
}
static void update_iteration(HMM *model, std::vector<std::string>& data) {
  init();
  // do an parameter update with the given data
  for(std::string s : data) {
    init_alpha(model, s);
    init_beta(model, s);
    //calculate eps and gamma and add them to sum_*
    add_gamma(model, s);
    add_epsilon(model, s);
  }
  update_parameters(model, (int)data.size());
}

int main(int argc, char *argv[]) {
  // parse from
  assert(argc == 5);
  int iterations = strtol(argv[1], NULL, 10);
  HMM model; loadHMM(&model, argv[2]);
  std::vector<std::string> data = load_training_data(argv[3]);
  for(int _ = 0; _ < iterations; _++) 
    update_iteration(&model, data);
  // write result to file
  FILE *fout = fopen(argv[4], "w");
  dumpHMM(fout, &model);
  fclose(fout);
}
