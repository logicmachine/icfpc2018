#include <bits/stdc++.h>
#include "nbtasm.hpp"
#include "read_model.hpp"

using namespace std;

#define rep(i, N) for (int i = 0; i < N; i++)
#define Rep(i, N) for (int i = 1; i < N; i++)
#define For(i, a, b) for (int i = (a); i < (b); i++)
#define pb push_back
#define eb emplace_back
#define mp make_pair
#define i_i pair<int, int>
#define vi vector<int>
#define vvi vector<vi >
#define vb vector<bool>
#define vvb vector<vb >
#define vp vector< i_i >
#define all(a) (a).begin(), (a).end()
#define Int(x) int x; cin >> x;
#define int2(x, y) Int(x); Int(y);
#define int3(x, y, z) Int(x); int2(y, z);
#define fir first
#define sec second
#define ffir first.first
#define fsec first.second
#define sfir second.first
#define ssec second.second
#define Decimal fixed << setprecision(10)

//int dxy[5] = {0, 1, 0, -1, 0};
// cmd

VoxelGrid targ, now;

void solve()
{
    int r = targ.r();
    int xl = r-1, xr = 0, yl = r-1, yr = 0, zl = r-1, zr = 0;

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < r; j++) {
            for (int k = 0; k < r; k++) {
                if (targ(i, j, k)) {
                    xl = min(xl, i); xr = max(xr, i);
                    yl = min(yl, j); yr = max(yr, j);
                    zl = min(zl, k); zr = max(zr, k);
                }
            }
        }
    }

    cout << " 探索範囲 : (" << xl << "," << xr << "), (" << 
        yl << "," << yr << "), (" << 
        zl << "," << zr << ")" << endl;
}


signed main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    string fn; cin >> fn;
    targ = read_data(fn);

    solve();        // 簡単のため、4 * 4 より小さいものは受け取らない

    return 0;
}

