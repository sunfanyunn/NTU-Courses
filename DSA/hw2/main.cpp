#include <fstream>
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <utility>
#include <map>
#define fi first
#define se second
#define pb push_back
#define mp make_pair
#define all(x) x.begin(), x.end()
#define maxn 2500000
#define num_of_users 1392873
using namespace std;
typedef pair<int,int> pii;
const int MAX_USERID = 2422000;

struct L1 {
	int item, res, t;
	bool operator == (const L1& cmp) const {
		return item == cmp.item && res == cmp.res && t == cmp.t;
	}
	bool operator < (const L1& cmp) const{
		if( item != cmp.item ) return item < cmp.item;
		if( t != cmp.t ) return t < cmp.t;
		return res < cmp.res;
	}
};
struct L2 {
	int user, res, t;
	bool operator == (const L2& cmp) const {
		return user == cmp.user && res == cmp.res && t == cmp.t;
	}
	bool operator < (const L2& cmp) const{
		if( user != cmp.user ) return user < cmp.user;
		if( t != cmp.t ) return t < cmp.t;
		return res < cmp.res;
	}
};
vector<L1> ui[maxn];
vector<L2> iu[maxn];
bool I[maxn];
vector< pii > R;

void init_U( int u) {
	 sort( all( ui[u] ) );
	 ui[u].resize( unique(all(ui[u])) - ui[u].begin());
}
void init_I( int i) {
	I[i] = true;
	sort( all(iu[i]) );
	iu[i].resize( unique(all(iu[i])) - iu[i].begin());
}

int accept(int u, int i, int t);
void items(int u1, int u2);
void users(int i1, int i2, int t1, int t2);
void ratio(int i, int threshold);
void findtime_item(int i, vector<int> Us);

int main() {
	FILE* fp;
	fp = fopen("/tmp2/KDDCUP2012/track1/rec_log_train.txt", "r");
	int userid, itemid, r, t, cnt = 0;
	while( fscanf(fp, "%d%d%d%d", &userid, &itemid, &r, &t) != EOF) {
		ui[userid].pb( (L1){ itemid, r, t} );
		iu[itemid].pb( (L2){ userid, r, t} );
#ifdef DEBUG
		if(++cnt % 100000 == 0) printf("[%d]\n", cnt);
#endif
	}
	fclose( fp );
	memset(I, false, sizeof(I));
	//preprocessing for ratio
	for(int u = 100000; u <= 1+MAX_USERID; u++) {
		if( ui[u].empty() ) continue;
		init_U( u );
		R.pb( mp((int)ui[u].size(), u) );
	}
	sort( all(R) );
#ifdef DEBUG
	assert( R.size() == num_of_users);
#endif
	int T ; scanf("%d", &T);
	char operation[20];
	while(T--) {
		scanf("%s", operation);
		if( operation[0] == 'a') {
			//puts("start accept");
			int u, i, t; scanf("%d%d%d", &u, &i, &t);
			printf("%d\n", accept(u, i, t));
			//puts("end accept");
		}
		else if(operation[0] == 'i') {
			//puts("start items");
			int u1, u2; scanf("%d%d", &u1, &u2);
			items(u1, u2);
			//puts("end items");
		}
		else if(operation[0] == 'u') {
			//puts("start users");
			int i1, i2, t1, t2; scanf("%d%d%d%d", &i1, &i2, &t1, &t2);
			users(i1, i2, t1, t2);
			//puts("end users");
		}
		else if(operation[0] == 'r') {
			//puts("start ratio");
			int i, threshold; scanf("%d%d", &i ,&threshold);
			ratio(i, threshold);
			//puts("end ratio");
		}else {
			//puts("start findtime");
			vector<int> vec; vec.clear();
			int i, u; char tmp;
			scanf("%d", &i);
			while( scanf("%d", &u) == 1)
				vec.pb(u);
			findtime_item(i, vec);
			//puts("end findtime");
		}
	}
	return 0;
}
int accept(int u, int i, int t) {
	vector< L1 >::iterator it = lower_bound( all(ui[u]), (L1){i, -10, t});
	if(it == ui[u].end() || it->item != i || it->t != t) return 0;
	return it->res;
}
void items(int u1, int u2) {
	bool flag = false;
	int tmp;
	auto first1 = ui[u1].begin(), last1 = ui[u1].end();
	auto first2 = ui[u2].begin(), last2 = ui[u2].end();
	while (first1 != last1 && first2 != last2) {
		if ( first1->item < first2->item )  first1++;
		else if ( first2->item < first1->item ) first2++;
		else {
			flag = true;
			printf("%d\n", tmp = first1->item);
		 	while( first1->item == tmp && first1 != last1) first1++;
		 	while( first2->item == tmp && first2 != last2) first2++;
		}
	}
	if(!flag) puts("EMPTY");
}
void users(int i1, int i2, int t1, int t2) {
	if( !I[i1] ) init_I( i1 );
	if( !I[i2] ) init_I( i2 );
	int tmp;
	bool flag = false;
	auto first1 = iu[i1].begin(), last1 = iu[i1].end();
	auto first2 = iu[i2].begin(), last2 = iu[i2].end();
	while (first1 != last1 && first2 != last2) {
		if ( first1->user < first2->user ) first1++;
		else if ( first2->user < first1->user )  first2++;
		else {
			bool flag1 = false, flag2 = false;
			tmp = first1->user;
		 	while( first1->user == tmp && first1 != last1) {
				if( first1->t >= t1 && first1->t <= t2) flag1 = true;
				first1++;
			}
		 	while( first2->user == tmp && first2 != last2) {
				if(first2->t >= t1 && first2->t <= t2) flag2 = true;
				first2++;
			}
			if( flag1 && flag2) flag = true,  printf("%d\n", tmp);
		}
	}
	if( !flag ) puts("EMPTY");
}

void ratio(int i, int threshold) {
	int up = 0, down;
	vector< pii >::iterator it;
	down = R.end() - (it = upper_bound( all(R), mp(threshold, MAX_USERID+5)));
#ifdef DEBUG
	assert(it == lower_bound( all(R), mp(threshold+1, 0)));
#endif
	vector< L1 >::iterator itr;
	for(; it != R.end(); it++) {
		int user = it->se;
		itr = lower_bound( all(ui[user]), (L1){i,-1,-1});
		while(  itr->item == i && itr != ui[user].end()  ) {
			if( itr->res == 1) { up++; break; }
			itr++;
		}
	}
	printf("%d/%d\n", up, down);

}
void findtime_item(int i, vector<int> Us) {
	vector<int> t; t.clear();
	sort( all(Us) );
	int tmp;
	if( !I[i] ) init_I( i );
	auto first1 = Us.begin(), last1 = Us.end();
	auto first2 = iu[i].begin(), last2 = iu[i].end();
	while (first1 != last1 && first2 != last2) {
		if ( *first1 < first2->user ) ++first1;
		else if ( first2->user < *first1) ++first2;
		else {
			tmp = *first1;
			while( first2 != last2 && first2->user == tmp) t.pb( first2->t ), first2++;
			++first1;
		}
	}
	sort( all(t) );
	t.resize( unique( all(t) ) - t.begin() );
	if( t.empty() ) puts("EMPTY");
	else {
		for(int T : t)
			printf("%d\n", T);
	}
}

