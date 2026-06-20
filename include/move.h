#pragma once
#include <vector>

#include "route_pack.h"
#include "solution_metrics.h"

enum MoveType {
    N1_REMOVE_INSERT,
    N2_SWAP_ADJ,
    N3_SWAP,
    N4_2OPT,
    N5_MOVE_FWD_K,
    N6_MOVE_BWD_K,
    N7_REORDER_BLOCK,
    N_INTER_RELOCATE,
    N_INTER_SWAP,
    N_CROSS_EXCHANGE,
    N_TWO_OPT_STAR,
};

using TabuHash = uint64_t;

class Move : std::enable_shared_from_this<Move> {
public:
    Move(MoveType type, int route_idx = 0);

    virtual ~Move() = default;

    virtual RoutePack Apply(const RoutePack& sol) const = 0;

    virtual RouteMetricsUpdate EvaluateDelta(const RoutePack& sol,
                                             const InputData& input) const = 0;

    virtual TabuHash GetTabuHash() const = 0;

    MoveType Type() const;

    virtual std::unique_ptr<Move> Clone() const = 0;

    virtual std::vector<int> AffectedRoutes() const;

protected:
    int route_idx_;
    MoveType type_;
};

/**
 * Removes a vertex from @p remove_pos and inserts into @p insert_pos
 */
class RemoveInsertMove : public Move {
public:
    RemoveInsertMove(int r, size_t remove_pos, size_t insert_pos);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    int route_idx_;
    size_t remove_pos_, insert_pos_;
};

/**
 * Swaps vertices at @p pos1 and @p pos2
 */
class SwapMove : public Move {
public:
    SwapMove(int r, size_t pos1, size_t pos2);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    static MoveType DetermineType(size_t pos1, size_t pos2);

    int route_idx_;
    size_t pos1_, pos2_;
};

/**
 * Reverses subroute from @p start to @p end
 */
class TwoOptMove : public Move {
public:
    TwoOptMove(int r, size_t start, size_t end);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    int route_idx_;
    size_t start_pos_, end_pos_;
};

/**
 * Moves subroute of length @p size starting at the @p start to the @p end
 */
class BlockRelocateMove : public Move {
public:
    BlockRelocateMove(int r, size_t start, size_t size, size_t insert);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    static MoveType DetermineType(size_t start, size_t insert);

    int route_idx_;
    size_t start_pos_, length_, insert_pos_;
};

/**
 * Replaces a subroute of length @p order.size() starting at @p start with provided @p order
 */
class ReorderBlockMove : public Move {
public:
    ReorderBlockMove(int r, size_t start, std::vector<int> order);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

private:
    int route_idx_;
    size_t start_pos_;
    std::vector<int> new_order_;
};

class InterRelocateMove : public Move {
public:
    InterRelocateMove(int f_r, int t_r, size_t f_p, size_t t_p);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

    std::vector<int> AffectedRoutes() const override;

private:
    int from_route_, to_route_;
    size_t from_pos_, to_pos_;
};

class InterSwapMove : public Move {
public:
    InterSwapMove(int r1, int r2, size_t p1, size_t p2);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

    std::vector<int> AffectedRoutes() const override;

private:
    int route1_, route2_;
    size_t pos1_, pos2_;
};

class CrossExchangeMove : public Move {
public:
    CrossExchangeMove(int r1, int r2, size_t start1, size_t len1, size_t start2, size_t len2);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

    std::vector<int> AffectedRoutes() const override;

private:
    int route1_, route2_;
    size_t start1_, len1_, start2_, len2_;
};

class TwoOptStarMove : public Move {
public:
    TwoOptStarMove(int r1, int r2, size_t edge1_idx, size_t edge2_idx);

    RoutePack Apply(const RoutePack& sol) const override;

    RouteMetricsUpdate EvaluateDelta(const RoutePack& sol, const InputData& input) const override;

    TabuHash GetTabuHash() const override;

    std::unique_ptr<Move> Clone() const override;

    std::vector<int> AffectedRoutes() const override;

private:
    int route1_, route2_;
    size_t edge1_idx_, edge2_idx_;
};
