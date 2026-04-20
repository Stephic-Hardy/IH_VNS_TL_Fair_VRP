#include "move.h"

Move::Move(MoveTypes type) : type_(type) {
}

MoveTypes Move::Type() const {
    return type_;
}

RemoveInsertMove::RemoveInsertMove(int r, size_t pos)
    : Move(N1_REMOVE_INSERT), route_idx_(r), pos_(pos) {
}

RoutePack RemoveInsertMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    new_sol.DeepCopyRoute(route_idx_);

    auto& vec = new_sol.routes[route_idx_]->vertices;
    int vertex = vec[pos_];
    vec.erase(vec.begin() + pos_);
    vec.push_back(vertex);

    new_sol.routes[route_idx_]->InvalidateCache();
    return new_sol;
}

std::string RemoveInsertMove::GetTabuHash() const {
    return "RELOC_" + std::to_string(route_idx_) + "_" +
           std::to_string(pos_);
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

SwapMove::SwapMove(int r, size_t pos1, size_t pos2) : Move(DetermineType(pos1, pos2)),
                                                      route_idx_(r), pos1_(pos1), pos2_(pos2) {
}

RoutePack SwapMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    new_sol.DeepCopyRoute(route_idx_);
    std::swap(new_sol.routes[route_idx_]->vertices[pos1_],
              new_sol.routes[route_idx_]->vertices[pos2_]);

    new_sol.routes[route_idx_]->InvalidateCache();
    return new_sol;
}

std::string SwapMove::GetTabuHash() const {
    return "SWAP_" + std::to_string(route_idx_) + "_" +
           std::to_string(std::min(pos1_, pos2_)) + "_" + std::to_string(std::max(pos1_, pos2_));
}


std::unique_ptr<Move> SwapMove::Clone() const {
    return std::make_unique<SwapMove>(*this);
}

TwoOptMove::TwoOptMove(int r, size_t start, size_t end) : Move(N4_2OPT), route_idx_(r),
                                                          start_pos_(start),
                                                          end_pos_(end) {
}

RoutePack TwoOptMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    new_sol.DeepCopyRoute(route_idx_);
    auto& vec = new_sol.routes[route_idx_]->vertices;
    std::reverse(vec.begin() + start_pos_, vec.begin() + end_pos_ + 1);

    new_sol.routes[route_idx_]->InvalidateCache();
    return new_sol;
}

std::string TwoOptMove::GetTabuHash() const {
    return "2OPT_" + std::to_string(route_idx_) + "_" +
           std::to_string(start_pos_) + "_" + std::to_string(end_pos_);
}

std::unique_ptr<Move> TwoOptMove::Clone() const {
    return std::make_unique<TwoOptMove>(*this);
}

BlockRelocateMove::BlockRelocateMove(int r, size_t start, size_t size, size_t insert)
    : Move(DetermineType(start, insert)), route_idx_(r), start_pos_(start), length_(size),
      insert_pos_(insert) {
}

RoutePack BlockRelocateMove::Apply(const RoutePack& sol) const {
    RoutePack new_sol = sol;
    new_sol.DeepCopyRoute(route_idx_);
    auto& vec = new_sol.routes[route_idx_]->vertices;

    auto begin_it = vec.begin() + start_pos_;
    auto end_it = begin_it + length_;
    std::vector<int> block(begin_it, end_it);

    vec.erase(begin_it, end_it);
    vec.insert(vec.begin() + insert_pos_, block.begin(), block.end());

    new_sol.routes[route_idx_]->InvalidateCache();
    return new_sol;
}

std::string BlockRelocateMove::GetTabuHash() const {
    return "BLKREL_" + std::to_string(route_idx_) + "_" +
           std::to_string(start_pos_) + "_" + std::to_string(length_) + "_" +
           std::to_string(insert_pos_);
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