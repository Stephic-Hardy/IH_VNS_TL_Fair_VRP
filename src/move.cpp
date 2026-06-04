#include "move.h"

#include <algorithm>

namespace {
// the following implementation was taken from
// https://github.com/HowardHinnant/hash_append/issues/7
// which is a repo for the N3980 proposal
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html
inline void HashCombine(uint64_t& seed, uint64_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 12) + (seed >> 4);
}

template <typename... Args>
uint64_t ComputeHash(Args... args) {
    uint64_t seed = 0;
    (HashCombine(seed, static_cast<uint64_t>(args)), ...);
    return seed;
}
}  // namespace

Move::Move(MoveTypes type, int route_idx) : route_idx_(route_idx), type_(type) {
}

MoveTypes Move::Type() const {
    return type_;
}

std::vector<int> Move::AffectedRoutes() const {
    return std::vector<int>{route_idx_};
}

RemoveInsertMove::RemoveInsertMove(int r, size_t remove_pos, size_t insert_pos)
    : Move(N1_REMOVE_INSERT), route_idx_(r), remove_pos_(remove_pos), insert_pos_(insert_pos) {
}

RoutePack RemoveInsertMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    new_sol.MutateRoute(route_idx_).Update([this](auto& vertices) {
        int vertex = vertices[remove_pos_];
        if (remove_pos_ < insert_pos_) {
            std::copy(vertices.begin() + remove_pos_ + 1, vertices.begin() + insert_pos_ + 1,
                      vertices.begin() + remove_pos_);
            vertices[insert_pos_] = vertex;
        } else {
            std::copy(vertices.begin() + insert_pos_, vertices.begin() + remove_pos_,
                      vertices.begin() + insert_pos_ + 1);
            vertices[insert_pos_] = vertex;
        }
    });

    return new_sol;
}

TabuHash RemoveInsertMove::GetTabuHash() const {
    return ComputeHash(type_, route_idx_, remove_pos_, insert_pos_);
}

std::unique_ptr<Move> RemoveInsertMove::Clone() const {
    return std::make_unique<RemoveInsertMove>(*this);
}

MoveTypes SwapMove::DetermineType(size_t pos1, size_t pos2) {
    const auto diff = std::abs(static_cast<ssize_t>(pos1) - static_cast<ssize_t>(pos2));
    if (diff == 1) {
        return N2_SWAP_ADJ;
    }
    return N3_SWAP;
}

SwapMove::SwapMove(int r, size_t pos1, size_t pos2)
    : Move(DetermineType(pos1, pos2)), route_idx_(r), pos1_(pos1), pos2_(pos2) {
}

RoutePack SwapMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    new_sol.MutateRoute(route_idx_).Update([this](auto& vertices) {
        std::swap(vertices[pos1_], vertices[pos2_]);
    });

    return new_sol;
}

TabuHash SwapMove::GetTabuHash() const {
    return ComputeHash(type_, route_idx_, std::min(pos1_, pos2_), std::max(pos1_, pos2_));
}

std::unique_ptr<Move> SwapMove::Clone() const {
    return std::make_unique<SwapMove>(*this);
}

TwoOptMove::TwoOptMove(int r, size_t start, size_t end)
    : Move(N4_2OPT), route_idx_(r), start_pos_(start), end_pos_(end) {
}

RoutePack TwoOptMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    new_sol.MutateRoute(route_idx_).Update([this](auto& vertices) {
        std::reverse(vertices.begin() + start_pos_, vertices.begin() + end_pos_ + 1);
    });

    return new_sol;
}

TabuHash TwoOptMove::GetTabuHash() const {
    return ComputeHash(type_, route_idx_, start_pos_, end_pos_);
}

std::unique_ptr<Move> TwoOptMove::Clone() const {
    return std::make_unique<TwoOptMove>(*this);
}

BlockRelocateMove::BlockRelocateMove(int r, size_t start, size_t size, size_t insert)
    : Move(DetermineType(start, insert)),
      route_idx_(r),
      start_pos_(start),
      length_(size),
      insert_pos_(insert) {
}

RoutePack BlockRelocateMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    new_sol.MutateRoute(route_idx_).Update([this](auto& vertices) {
        auto begin_it = vertices.begin() + start_pos_;
        auto end_it = begin_it + length_;
        std::vector<int> block(begin_it, end_it);

        vertices.erase(begin_it, end_it);
        vertices.insert(vertices.begin() + insert_pos_, block.begin(), block.end());
    });

    return new_sol;
}

TabuHash BlockRelocateMove::GetTabuHash() const {
    return ComputeHash(type_, route_idx_, start_pos_, length_, insert_pos_);
}

std::unique_ptr<Move> BlockRelocateMove::Clone() const {
    return std::make_unique<BlockRelocateMove>(*this);
}

MoveTypes BlockRelocateMove::DetermineType(size_t start, size_t insert) {
    if (insert >= start) {
        return N5_MOVE_FWD_K;
    }
    return N6_MOVE_BWD_K;
}

ReorderBlockMove::ReorderBlockMove(int r, size_t start, std::vector<int> order)
    : Move(N7_REORDER_BLOCK), route_idx_(r), start_pos_(start), new_order_(std::move(order)) {
}

RoutePack ReorderBlockMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    new_sol.MutateRoute(route_idx_).Update([this](auto& vertices) {
        std::copy(new_order_.begin(), new_order_.end(), vertices.begin() + start_pos_);
    });

    return new_sol;
}

TabuHash ReorderBlockMove::GetTabuHash() const {
    uint64_t hash = ComputeHash(type_, route_idx_, start_pos_, new_order_.size());
    for (int v : new_order_) {
        HashCombine(hash, v);
    }
    return hash;
}

std::unique_ptr<Move> ReorderBlockMove::Clone() const {
    return std::make_unique<ReorderBlockMove>(*this);
}

InterRelocateMove::InterRelocateMove(int f_r, int t_r, size_t f_p, size_t t_p)
    : Move(N_INTER_RELOCATE), from_route_(f_r), to_route_(t_r), from_pos_(f_p), to_pos_(t_p) {
}

RoutePack InterRelocateMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    int vertex = sol.GetRoute(from_route_).Vertices()[from_pos_];

    new_sol.MutateRoute(from_route_).Update([&](auto& v) { v.erase(v.begin() + from_pos_); });

    new_sol.MutateRoute(to_route_).Update([&](auto& v) { v.insert(v.begin() + to_pos_, vertex); });

    return new_sol;
}

TabuHash InterRelocateMove::GetTabuHash() const {
    return ComputeHash(type_, from_route_, to_route_, from_pos_, to_pos_);
}

std::unique_ptr<Move> InterRelocateMove::Clone() const {
    return std::make_unique<InterRelocateMove>(*this);
}

std::vector<int> InterRelocateMove::AffectedRoutes() const {
    return {from_route_, to_route_};
}

InterSwapMove::InterSwapMove(int r1, int r2, size_t p1, size_t p2)
    : Move(N_INTER_SWAP), route1_(r1), route2_(r2), pos1_(p1), pos2_(p2) {
}

RoutePack InterSwapMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    int v1 = sol.GetRoute(route1_).Vertices()[pos1_];
    int v2 = sol.GetRoute(route2_).Vertices()[pos2_];

    new_sol.MutateRoute(route1_).Update([&](auto& v) { v[pos1_] = v2; });
    new_sol.MutateRoute(route2_).Update([&](auto& v) { v[pos2_] = v1; });

    return new_sol;
}

TabuHash InterSwapMove::GetTabuHash() const {
    int r1 = route1_, r2 = route2_;
    size_t p1 = pos1_, p2 = pos2_;

    if (r1 > r2) {
        std::swap(r1, r2);
        std::swap(p1, p2);
    }
    return ComputeHash(type_, r1, r2, p1, p2);
}

std::unique_ptr<Move> InterSwapMove::Clone() const {
    return std::make_unique<InterSwapMove>(*this);
}

std::vector<int> InterSwapMove::AffectedRoutes() const {
    return {route1_, route2_};
}

CrossExchangeMove::CrossExchangeMove(int r1, int r2, size_t start1, size_t len1, size_t start2,
                                     size_t len2)
    : Move(N_CROSS_EXCHANGE),
      route1_(r1),
      route2_(r2),
      start1_(start1),
      len1_(len1),
      start2_(start2),
      len2_(len2) {
}

RoutePack CrossExchangeMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    const auto& route1_verts = sol.GetRoute(route1_).Vertices();
    const auto& route2_verts = sol.GetRoute(route2_).Vertices();

    std::vector<int> seg1(route1_verts.begin() + start1_, route1_verts.begin() + start1_ + len1_);
    std::vector<int> seg2(route2_verts.begin() + start2_, route2_verts.begin() + start2_ + len2_);

    new_sol.MutateRoute(route1_).Update([&](std::vector<int>& v) {
        v.erase(v.begin() + start1_, v.begin() + start1_ + len1_);
        v.insert(v.begin() + start1_, seg2.begin(), seg2.end());
    });

    new_sol.MutateRoute(route2_).Update([&](std::vector<int>& v) {
        v.erase(v.begin() + start2_, v.begin() + start2_ + len2_);
        v.insert(v.begin() + start2_, seg1.begin(), seg1.end());
    });

    return new_sol;
}

TabuHash CrossExchangeMove::GetTabuHash() const {
    int r1 = route1_, r2 = route2_;
    size_t s1 = start1_, l1 = len1_, s2 = start2_, l2 = len2_;

    if (r1 > r2) {
        std::swap(r1, r2);
        std::swap(s1, s2);
        std::swap(l1, l2);
    }
    return ComputeHash(type_, r1, r2, s1, l1, s2, l2);
}

std::unique_ptr<Move> CrossExchangeMove::Clone() const {
    return std::make_unique<CrossExchangeMove>(*this);
}

std::vector<int> CrossExchangeMove::AffectedRoutes() const {
    return {route1_, route2_};
}

TwoOptStarMove::TwoOptStarMove(int r1, int r2, size_t edge1_idx, size_t edge2_idx)
    : Move(N_TWO_OPT_STAR), route1_(r1), route2_(r2), edge1_idx_(edge1_idx), edge2_idx_(edge2_idx) {
}

RoutePack TwoOptStarMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;

    const auto& route1_verts = sol.GetRoute(route1_).Vertices();
    const auto& route2_verts = sol.GetRoute(route2_).Vertices();

    std::vector<int> prefix1(route1_verts.begin(), route1_verts.begin() + edge1_idx_ + 1);
    std::vector<int> suffix1(route1_verts.begin() + edge1_idx_ + 1, route1_verts.end());

    std::vector<int> prefix2(route2_verts.begin(), route2_verts.begin() + edge2_idx_ + 1);
    std::vector<int> suffix2(route2_verts.begin() + edge2_idx_ + 1, route2_verts.end());

    std::vector<int> new_route1 = prefix1;
    new_route1.insert(new_route1.end(), suffix2.begin(), suffix2.end());

    std::vector<int> new_route2 = prefix2;
    new_route2.insert(new_route2.end(), suffix1.begin(), suffix1.end());

    new_sol.MutateRoute(route1_).Update([&](std::vector<int>& v) { v = new_route1; });
    new_sol.MutateRoute(route2_).Update([&](std::vector<int>& v) { v = new_route2; });

    return new_sol;
}

TabuHash TwoOptStarMove::GetTabuHash() const {
    int r1 = route1_, r2 = route2_;
    size_t e1 = edge1_idx_, e2 = edge2_idx_;

    if (r1 > r2) {
        std::swap(r1, r2);
        std::swap(e1, e2);
    }
    return ComputeHash(type_, r1, r2, e1, e2);
}

std::unique_ptr<Move> TwoOptStarMove::Clone() const {
    return std::make_unique<TwoOptStarMove>(*this);
}

std::vector<int> TwoOptStarMove::AffectedRoutes() const {
    return {route1_, route2_};
}
