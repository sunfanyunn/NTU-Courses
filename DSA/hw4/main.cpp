#include <bits/stdc++.h>
using namespace std;
#define fi first
#define se second
#define mp make_pair
#define pb push_back
#define maxn
struct buy{
    int id, client, price;
    bool operator > (const buy& cmp) const{
        if( price != cmp.price) return price > cmp.price;
        return id < cmp.id;
    }
    bool operator < (const buy& cmp) const{
        return !(*this > cmp);
    }
};
struct sell{
    int id, client, price;
    bool operator > (const sell& cmp) const{
        if( price != cmp.price) return price < cmp.price;
        return id < cmp.id;
    }
    bool operator < (const sell& cmp) const{
        return !(*this > cmp);
    }
};

vector<int> sharecnt;
priority_queue<sell> Sell;
priority_queue<buy> Buy;
vector<bool> cancelled;
int main() {
    int transid = 0;
    int bidid, c, action, p, s;
    while( scanf("%d%d%d%d%d", &bidid, &c, &action, &p, &s) != EOF) {
        sharecnt.push_back(s), cancelled.push_back(false);
        assert( (int)sharecnt.size()-1 == bidid );
        //1 for sell
        if( action == 1) Sell.push( (sell){bidid, c, p} );
        //0 for buy
        else if( action == 0) Buy.push( (buy){bidid, c, p} );
        //2 for cancel
        else {
            // p is bidid in this situation
            cancelled[p] = true;
        }
        int deal_price;
        while( !Sell.empty() && (cancelled[Sell.top().id] || sharecnt[Sell.top().id]==0) ) Sell.pop();
        while( !Buy.empty() && (cancelled[Buy.top().id] || sharecnt[Buy.top().id]==0) ) Buy.pop();
        if( Buy.empty() || Sell.empty() ) continue;
        while( Buy.top().price >= (deal_price = Sell.top().price) ) {

            int buy_bidid = Buy.top().id, sell_bidid = Sell.top().id;
            int cnt = min(sharecnt[buy_bidid], sharecnt[sell_bidid]);
            sharecnt[buy_bidid] -= cnt, sharecnt[sell_bidid] -= cnt;
            printf("%d\t%d\t%d\t%d\t%d\n", transid++, Buy.top().client, Sell.top().client, deal_price, cnt );

            while( !Sell.empty() && (cancelled[Sell.top().id] || sharecnt[Sell.top().id]==0) ) Sell.pop();
            while( !Buy.empty() && (cancelled[Buy.top().id] || sharecnt[Buy.top().id]==0) ) Buy.pop();
            if( Buy.empty() || Sell.empty() ) break;
        }
    }
    return 0;
}
