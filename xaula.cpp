#include <bits/stdc++.h>
using namespace std;
#define int long long int
#define pb push_back
#define I insert
#define B begin()
#define E end()
#define sz size()
#define endl '\n'
#define inf (int)1e18
#define left p<<1
#define right (p<<1)|1
#define amem cin.tie(0)->sync_with_stdio(0);
const int mod=1e9+7;

//We seek the perfect algorithm, but life's best answers just don't compile.
//By Xaulin_Du_Grau. ;-;

struct pt{
    long long x, y;
    pt(long long x_ = 0, long long y_ = 0) : x(x_), y(y_) {}
    pt operator -(pt fds) const{
        return pt(x-fds.x, y-fds.y);
    }
    pt operator +(pt fds) const{
        return pt(x+fds.x, y+fds.y);
    }
    long long operator ^(pt fds) const{//produto vetorial cross
        return (x*fds.y - y*fds.x);
    }
    long long operator *(pt fds) const{//produto escalar(projeçao)
        return (x*fds.x + y*fds.y);
    }
};

istream &operator >>(istream &in, pt &p){
    in>>p.x>>p.y;
    return in;
}

signed main(){
    amem;
    
    exit (0);
}