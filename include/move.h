#pragma once
#include <algorithm>
#include <string>

#include "route_pack.h"

enum MoveTypes {
    N1_REMOVE_INSERT,
    N2_SWAP_ADJ,
    N3_SWAP,
    N4_2OPT,
    N5_MOVE_FWD_K,
    N6_MOVE_BWD_K,
};

class Move : std::enable_shared_from_this<Move> {
public:
    Move(MoveTypes type);

    virtual ~Move() = default;

    virtual RoutePack Apply(const RoutePack& sol) const = 0;

    virtual std::string GetTabuHash() const = 0;

    MoveTypes Type() const;

    virtual std::unique_ptr<Move> Clone() const = 0;

private:
    MoveTypes type_;
};


class RemoveInsertMove : public Move {
public:
    RemoveInsertMove(int r, size_t pos);

    RoutePack Apply(const RoutePack& sol) const override;

    std::string GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    int route_idx_;
    size_t pos_;
};

class SwapMove : public Move {
public:
    SwapMove(int r, size_t p1, size_t p2);

    RoutePack Apply(const RoutePack& sol) const override;

    std::string GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    static MoveTypes DetermineType(size_t pos1, size_t pos2);

    int route_idx_;
    size_t pos1_, pos2_;
};

class TwoOptMove : public Move {
public:
    TwoOptMove(int r, size_t start, size_t end);

    RoutePack Apply(const RoutePack& sol) const override;

    std::string GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    int route_idx_;
    size_t start_pos_, end_pos_;
};

class BlockRelocateMove : public Move {
public:
    BlockRelocateMove(int r, size_t start, size_t size, size_t insert);

    RoutePack Apply(const RoutePack& sol) const override;

    std::string GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    static MoveTypes DetermineType(size_t start, size_t insert);

    int route_idx_;
    size_t start_pos_, length_, insert_pos_;
};