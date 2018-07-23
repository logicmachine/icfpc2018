#include <bits/stdc++.h>
#include <cassert>

#define STRONG_VALIDATION

#include "nbtasm.hpp"
#include "read_model.hpp"
#include "backward.hpp"

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
#define BOT_COUNT 40
#define BOT_X 8
#define BOT_Z 5
//int dxy[5] = {0, 1, 0, -1, 0};
// cmd

VoxelGrid targ;

void dump(State& s){
    s.dump_pending_commands(std::cerr);
	std::cout << std::endl;
}

bool move_once(State &s, int bnum, const Vec3 &from, const Vec3 &to)
{
    if (from == to) return true;
    if (abs(from.x - to.x) >= 10 || (from.y == to.y && from.z == to.z)) { 
        if (from.x < to.x) {
            s.bots(bnum).smove(Vec3(min(15, to.x-from.x), 0, 0));
        } else {
            s.bots(bnum).smove(Vec3(max(-15, to.x-from.x), 0, 0));
        }
    } else if (abs(from.y - to.y) >= 10 || (from.x == to.x && from.z == to.z)) { 
        if (from.y < to.y) {
            s.bots(bnum).smove(Vec3(0, min(15, to.y-from.y), 0));
        } else {
            s.bots(bnum).smove(Vec3(0, max(-15, to.y-from.y), 0));
        }
    } else if (abs(from.z - to.z) >= 10 || (from.y == to.y && from.x == to.x)) { 
        if (from.z < to.z) {
            s.bots(bnum).smove(Vec3(0, 0, min(15, to.z-from.z)));
        } else {
            s.bots(bnum).smove(Vec3(0, 0, max(-15, to.z-from.z)));
        }
    } else if (from.x == to.x) {
        int dy = (to.y - from.y), dz = (to.z - from.z);
        dy = ((abs(dy) > 5) ? (5 * ((dy >= 0) ? 1 : -1)) : dy);
        dz = ((abs(dz) > 5) ? (5 * ((dz >= 0) ? 1 : -1)) : dz);
        s.bots(bnum).lmove(Vec3(0, dy, 0), Vec3(0, 0, dz));
    } else if (from.y == to.y) {
        int dx = (to.x - from.x), dz = (to.z - from.z);
        dx = ((abs(dx) > 5) ? (5 * ((dx >= 0) ? 1 : -1)) : dx);
        dz = ((abs(dz) > 5) ? (5 * ((dz >= 0) ? 1 : -1)) : dz);
        s.bots(bnum).lmove(Vec3(dx, 0, 0), Vec3(0, 0, dz));
    } else if (from.z == to.z) {
        int dy = (to.y - from.y), dx = (to.x - from.x);
        dx = ((abs(dx) > 5) ? (5 * ((dx >= 0) ? 1 : -1)) : dx);
        dy = ((abs(dy) > 5) ? (5 * ((dy >= 0) ? 1 : -1)) : dy);
        s.bots(bnum).lmove(Vec3(0, dy, 0), Vec3(dx, 0, 0));
    } else {
        int dy = (to.y - from.y), dz = (to.z - from.z);
        dy = ((abs(dy) > 5) ? (5 * ((dy >= 0) ? 1 : -1)) : dy);
        dz = ((abs(dz) > 5) ? (5 * ((dz >= 0) ? 1 : -1)) : dz);
        s.bots(bnum).lmove(Vec3(0, dy, 0), Vec3(0, 0, dz));
    }
    return false;
}

bool clear_table(State &s, Vec3 &p1, Vec3 &p2, int h)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i)) return false;
        }
    }
    return true;
}

bool clear_table_outer(State &s, Vec3 &p1, Vec3 &p2, int h)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i) && (h != 0 && !s.matrix(j, h-1, i))) return false;
        }
    }
    return true;
}

bool clear_table_inner(State &s, Vec3 &p1, Vec3 &p2, int h)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i) && (h == 0 || s.matrix(j, h-1, i))) return false;
        }
    }
    return true;
}

bool clear_once(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int dx[5] = {0, -1, 0, 1, 0};
    int dz[5] = {0, 0, -1, 0, 1};
    for (int i = 0; i < 5; i++) {
        Vec3 des(dx[i], -1, dz[i]);
        if (min(p1.z, p2.z) <= s.bots(bnum).pos().z + des.z && max(p1.z, p2.z) >= s.bots(bnum).pos().z + des.z &&
            min(p1.x, p2.x) <= s.bots(bnum).pos().x + des.x && max(p1.x, p2.x) >= s.bots(bnum).pos().x + des.x &&
                s.matrix()(s.bots(bnum).pos().z + des.z, s.bots(bnum).pos().y + des.y, s.bots(bnum).pos().x + des.x)) {
            s.bots(bnum).empty(des);
            return true;
        }
    }
    return false;
}

bool clear_once_outer(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int dx[5] = {0, -1, 0, 1, 0};
    int dz[5] = {0, 0, -1, 0, 1};
    for (int i = 0; i < 5; i++) {
        Vec3 des(dx[i], -1, dz[i]);
        if (min(p1.z, p2.z) <= s.bots(bnum).pos().z + des.z && max(p1.z, p2.z) >= s.bots(bnum).pos().z + des.z &&
            min(p1.x, p2.x) <= s.bots(bnum).pos().x + des.x && max(p1.x, p2.x) >= s.bots(bnum).pos().x + des.x &&
                s.matrix()(s.bots(bnum).pos().z + des.z, s.bots(bnum).pos().y + des.y, s.bots(bnum).pos().x + des.x) &&
                (s.bots(bnum).pos().y + des.y != 0 && 
                 !s.matrix()(s.bots(bnum).pos().z + des.z, s.bots(bnum).pos().y + des.y - 1, s.bots(bnum).pos().x + des.x))) {
            s.bots(bnum).empty(des);
            return true;
        }
    }
    return false;
}

bool clear_once_inner(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int dx[5] = {0, -1, 0, 1, 0};
    int dz[5] = {0, 0, -1, 0, 1};
    for (int i = 0; i < 5; i++) {
        Vec3 des(dx[i], -1, dz[i]);
        if (min(p1.z, p2.z) <= s.bots(bnum).pos().z + des.z && max(p1.z, p2.z) >= s.bots(bnum).pos().z + des.z &&
            min(p1.x, p2.x) <= s.bots(bnum).pos().x + des.x && max(p1.x, p2.x) >= s.bots(bnum).pos().x + des.x &&
                s.matrix()(s.bots(bnum).pos().z + des.z, s.bots(bnum).pos().y + des.y, s.bots(bnum).pos().x + des.x) &&
                (s.bots(bnum).pos().y + des.y == 0 ||
                 s.matrix()(s.bots(bnum).pos().z + des.z, s.bots(bnum).pos().y + des.y - 1, s.bots(bnum).pos().x + des.x))) {
            s.bots(bnum).empty(des);
            return true;
        }
    }
    return false;
}

void move_near(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    int h = s.bots(bnum).pos().y-1;
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i)) {
                move_once(s, bnum, s.bots(bnum).pos(), Vec3(i,h+1,j));
                return;
            }
        }
    }
}

void move_near_outer(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    int h = s.bots(bnum).pos().y-1;
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i) &&
                    (h != 0 && !s.matrix(j, h-1, i))) {
                move_once(s, bnum, s.bots(bnum).pos(), Vec3(i,h+1,j));
                return;
            }
        }
    }
}

void move_near_inner(State &s, int bnum, Vec3 &p1, Vec3 &p2)
{
    int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
        lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
    Vec3 popos = s.bots(bnum).pos();
    if (!(lx <= popos.x && popos.x <= rx && lz <= popos.z && popos.z <= rz)) {
        cerr << popos << endl;
        cerr << lx << " " << rx << " , " << lz << " " << rz << endl;
        dump(s);
        assert(0);
    }
    int h = s.bots(bnum).pos().y-1;
    for (int i = lx; i <= rx; i++) {
        for (int j = lz; j <= rz; j++) {
            if (s.matrix(j, h, i) &&
                    (h == 0 || s.matrix(j, h-1, i))) {
                move_once(s, bnum, s.bots(bnum).pos(), Vec3(i,h+1,j));
                return;
            }
        }
    }
}

void region(State &s, vector<pair<Vec3, Vec3>> &list)
{
    if (s.num_bots() != list.size()) {
        cout << "differeint size" << endl;
        assert(0);
        return;
    }
    rep(i, list.size()) {
        Vec3 popos = s.bots(i).pos();
        Vec3 p1 = list[i].fir, p2 = list[i].sec;
        int lx = min(p1.x, p2.x), rx = max(p1.x, p2.x),
            lz = min(p1.z, p2.z), rz = max(p1.z, p2.z);
        if (!(lx <= popos.x && popos.x <= rx && lz <= popos.z && popos.z <= rz)) {
            cerr << popos << endl;
            cerr << lx << " " << rx << " , " << lz << " " << rz << endl;
            dump(s);
            assert(0);
        }
    }
}

bool div_cmp(const Vec3 &p1, const Vec3 &p2, vector<pair<Vec3, Vec3>> &data)
{
    int p1x = -1000, p2x = -1000, p1z = -1000, p2z = -1000;
    Rep(i, data.size()) {
        int lx = min(data[i].fir.x, data[i].sec.x), rx = max(data[i].fir.x, data[i].sec.x),
            lz = min(data[i].fir.z, data[i].sec.z), rz = max(data[i].fir.z, data[i].sec.z);
        if (lx <= p1.x && p1.x <= rx && lz <= p1.z && p1.z <= rz) {
            p1x = (data.size()-i) / BOT_Z;
            p1z = (data.size()-i) % BOT_Z;
    //cout << p1 << " " << p1x << " " << p1z << " : " << i << endl;
        }
        if (lx <= p2.x && p2.x <= rx && lz <= p2.z && p2.z <= rz) {
            p2x = (data.size()-i) / BOT_Z;
            p2z = (data.size()-i) % BOT_Z;
    //cout << p2 << " " << p2x << " " << p2z << " : " << i << endl;
        }
    }
    if (p1x == -1000) p1x = p1z = 0;
    if (p2x == -1000) p2x = p2z = 0;
    //cout << make_tuple(p1.x/dx, p1.y, p1.z/dz) << " , " << 
     //   make_tuple(p2.x/dx, p2.y, p2.z/dz) << endl;
    //cout << p1.x/dx << " " << p1.x << " , " << p2.x/dx << " " << p2.x << endl;
    return make_tuple(p1x, p1.y, p1z) < make_tuple(p2x, p2.y, p2z);
}


void solve(const char *fname)
{
    State state(targ, BOT_COUNT);

    int r = targ.r();
    int xl = r-1, xr = 0, yl = r-1, yr = 0, zl = r-1, zr = 0;

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < r; j++) {
            for (int k = 0; k < r; k++) {
                if (targ(k, j, i)) {
                    xl = min(xl, i); xr = max(xr, i);
                    yl = min(yl, j); yr = max(yr, j);
                    zl = min(zl, k); zr = max(zr, k);
                }
            }
        }
    }
      /* 
    cerr << " 探索範囲 : (" << xl << "," << xr << "), (" << 
        yl << "," << yr << "), (" << 
        zl << "," << zr << ")" << endl;
    */
    if (xr - xl < BOT_X || zr - zl < BOT_Z) {
        cerr << "small case" << endl;
        cerr << " 探索範囲 : (" << xl << "," << xr << "), (" << 
            yl << "," << yr << "), (" << 
            zl << "," << zr << ")" << endl;
        cerr << " 処理幅 " << xr - xl << " , " << zr - zl << endl;
        return ;
    }
          
    while (!move_once(state,0,state.bots(0).pos(),Vec3(0, yr+1, 0))) {
    dump(state);
        state.commit();
    }
    dump(state);

    int dx = (xr - xl + 1) / BOT_X, ex = (xr - xl + 1) % BOT_X, 
        dz = (zr - zl + 1) / BOT_Z;

    vector<pair<Vec3, Vec3>> data(BOT_COUNT);
    int pos = 1;
    for (int i = xr; i >= xl; i -= dx) {
        int ez = (zr - zl + 1) % BOT_Z;
        for (int j = zr; j >= zl; j -= dz) {
            data[pos++] = mp(Vec3(i, yr+1, j), 
                    Vec3(i-dx+1-!!(ex), yr+1, j-dz+1-!!(ez)));
            if (ez) {
                ez--;
                j--;
            }
            if (pos == data.size()) pos = 0;
        }
        if (ex) {
            ex--;
            i--;
        }
    }
    /*   
    for (int i = 0; i < data.size(); i++) {
        cout << data[i].fir << " -> " << data[i].sec << endl;
    }
    */
    for (int i = 1; i < data.size(); i++) {
        for (int j = 1; j < i; j++) {
            move_once(state,j,state.bots(j).pos(),data[j].fir);
        }
        state.commit();
        for (int j = 1; j < i; j++) {
            move_once(state,j,state.bots(j).pos(),data[j].fir);
        }
        state.bots(0).fission(Vec3(1, 0, 0), 0);
        state.commit();
    }

    for (int j = 1; j < data.size(); j++) {
        move_once(state,j,state.bots(j).pos(),data[j].fir);
    }
    state.commit();
    for (int j = 1; j < data.size(); j++) {
        move_once(state,j,state.bots(j).pos(),data[j].fir);
    }
    state.commit();

    while (!move_once(state,0,state.bots(0).pos(),data[0].fir)) {
        for (int j = 1; j < data.size(); j++) {
            move_once(state,j,state.bots(j).pos(),data[j].fir);
        }
        state.commit();
    }

    for (int i = yr+1; i > 0; i--) {
        while (1) {
            bool finish = true;
            for (int j = 0; j < state.num_bots(); j++) {
                bool j_clear = 
                    clear_table_outer(state, data[j].fir, data[j].sec, i-1);
                finish &= j_clear;
                if (!j_clear && !clear_once_outer(state, j, data[j].fir, data[j].sec)) {
                    move_near_outer(state, j, data[j].fir, data[j].sec);
                }
            }
            if (finish) break;
            state.commit();
        }
        while (1) {
            bool finish = true;
            for (int j = 0; j < state.num_bots(); j++) {
                bool j_clear = 
                    clear_table_inner(state, data[j].fir, data[j].sec, i-1);
                finish &= j_clear;
                if (!j_clear && !clear_once_inner(state, j, data[j].fir, data[j].sec)) {
                    move_near_inner(state, j, data[j].fir, data[j].sec);
                }
            }
            if (finish) break;
            state.commit();
        }
        for (int j = 0; j < state.num_bots(); j++) {
            state.bots(j).smove(Vec3(0, -1, 0));
        }
        state.commit();
    }

    Vec3 merge_dz(1, 0, 0);
    while (state.num_bots() > BOT_Z) {
        vi pos_bots(state.num_bots());  //  ここからバブルソート
        rep(i, pos_bots.size()) pos_bots[i] = i;
        rep(i, pos_bots.size()) { // Z 
            for (int j = pos_bots.size()-1; j > i; j--) {
                if (div_cmp(state.bots(pos_bots[j]).pos(), state.bots(pos_bots[j-1]).pos(), data))  // <
                    swap(pos_bots[j], pos_bots[j-1]);
            }
        }
        //rep(i, pos_bots.size()) cout << pos_bots[i] << " "; cout << endl;
        while (1) {
            bool adj = true;
            for (int i = 1; i <= BOT_Z; i++) {
                adj &= move_once(state, pos_bots[state.num_bots()-i], 
                        state.bots(pos_bots[state.num_bots()-i]).pos(), 
                        state.bots(pos_bots[state.num_bots()-i-BOT_Z]).pos()
                        + merge_dz);
            }
            if (adj) break;
            state.commit();
        }
        for (int i = 1; i <= BOT_Z; i++) {
            state.bots(pos_bots[state.num_bots()-i]).fusion_s(Vec3(-1, 0, 0));
            state.bots(pos_bots[state.num_bots()-i-BOT_Z]).fusion_p(Vec3(1, 0, 0));
        }
        state.commit();
    }

    while (!move_once(state,0,state.bots(0).pos(),Vec3(0, 0, 0))) {
        state.commit();
    }

    rep(i, BOT_Z-1) {
        while (!move_once(state,state.num_bots()-1,state.bots(state.num_bots()-1).pos(),Vec3(1, 0, 0))) {
            state.commit();
        }
        state.bots(0).fusion_p(Vec3{1, 0, 0});
        state.bots(state.num_bots()-1).fusion_s(Vec3{-1, 0, 0});
        state.commit();
    }

    state.bots(0).halt();
    state.commit();

    export_backward_trace(fname, state);

}

signed main(int argc, char *argv[])
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    if (argc != 3) {
        cout << "input error" << endl;
        return 0;
    }

    targ = read_data(string(argv[1]));
    solve(argv[2]);        // 簡単のため、5 * 8 より小さいものは受け取らない

    return 0;
}

