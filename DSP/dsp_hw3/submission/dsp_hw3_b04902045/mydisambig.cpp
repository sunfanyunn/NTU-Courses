#include <cassert>
#include <algorithm>
#include <vector>

#include "File.h"
#include "Prob.h"
#include "Ngram.h"
#include "Vocab.h"
#include "VocabMap.h"

#define DEBUG 0

#define MAX_WORD 200
#define NUM_CANDIDATE 1000

// Get P(W2 | W1) -- bigram
double getBigramProb(VocabIndex &wid1, VocabIndex &wid2, Vocab &voc, Ngram &lm) {
    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

bool in_voc(Vocab &voc, VocabString word) {
  return (voc.getIndex(word) != Vocab_None);
}
void solve(char *line, Ngram &lm, VocabMap &map, Vocab &voc, Vocab&v1, Vocab &v2) {
  VocabString sent[MAX_WORD];
  sent[0] = Vocab_SentStart;
  int cnt = Vocab::parseWords(line, sent+1, MAX_WORD);
  sent[cnt+1] = Vocab_SentEnd;
  cnt += 2;

#if DEBUG
  fprintf(stderr, "cnt = %d\n", cnt);
#endif

  std::vector<std::vector<LogP>> dp(cnt);
  // stores the VocabIndex(for voc)
  std::vector<std::vector<VocabString>> pos(cnt);
  // for backtracking
  std::vector<std::vector<int>> backtrack(cnt);

  for(int k = 0; k < cnt; k++) {
#if DEBUG
//    fprintf(stderr, "k = %d\n", k);
#endif
    if (sent[k] == Vocab_SentStart) {
      assert (k == 0);
      dp[k].push_back(0);
      pos[k].push_back(sent[k]);
      backtrack[k].push_back(-1);
    }
    else {

      VocabMapIter it(map, v1.getIndex(sent[k])); it.init();
      VocabIndex v2_idx;
      Prob p;
      // p is not useful here
      while(it.next(v2_idx, p)) {
        // vocabindex in voc
        VocabIndex candidate = in_voc(voc, v2.getWord(v2_idx))?
            voc.getIndex(v2.getWord(v2_idx)) : voc.getIndex(Vocab_Unknown);

        LogP mxp = LogP_Zero;
        int idx = -1;
        for(int i = 0; i < (int)dp[k-1].size(); i++) {
          VocabIndex vi = in_voc(voc, pos[k-1][i])?
            voc.getIndex(pos[k-1][i]) : voc.getIndex(Vocab_Unknown);
          LogP logp = dp[k-1][i] + getBigramProb(vi, candidate, voc, lm);
          if (logp > mxp)
              mxp = logp, idx = i;
        }
        assert (idx != -1);
        
        dp[k].push_back(mxp);
        pos[k].push_back(v2.getWord(v2_idx));
        backtrack[k].push_back(idx);
#if DEBUG
  //      fprintf(stderr, "dp[%d][%d] = %f\n", k, (int)dp[k].size(), dp[k].back());
#endif
      }
    }
  }

#if DEBUG
  fprintf(stderr, "start backtracking ...\n");
#endif
  int pp = 0;
  VocabString ans[cnt];
  for(int i = cnt-1; i >= 0; i--) {
    ans[i] = pos[i][pp];
    pp = backtrack[i][pp];
  }
  assert (pp == -1);
  for(int i = 0; i < cnt; i++) 
    printf("%s%c", ans[i], (i+1==cnt)?'\n':' ');

}

int main (int argc, char* argv[]) {
  assert(argc == 9);

  Vocab voc;
  Vocab zhuyin, big5;
  VocabMap zhu2big(zhuyin, big5);
  // order
  Ngram lm(voc, atoi(argv[8]));
  // load language model
  File lmfile(argv[6], "r");
  lm.read(lmfile);
  lmfile.close();
  // actually mapping from zhuyin/big5 -> big5
  File mapfile(argv[4], "r");
  zhu2big.read(mapfile);
  mapfile.close();
  //read testing data
  File testfile(argv[2], "r");
  char *line;
  while((line = testfile.getline())) {
      solve(line, lm, zhu2big, voc, zhuyin, big5);
  }
  testfile.close();
}
      
