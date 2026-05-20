#pragma once
#include <functional>
#include <vector>

#include "route_pack.h"
#include "move.h"

class Neighborhood {
public:
    virtual ~Neighborhood() = default;

    std::pair<RoutePack, std::unique_ptr<Move>> FindBestNeighbor(
        const RoutePack& sol, const InputData& input_data) const;

    std::pair<RoutePack, std::unique_ptr<Move>> FindBestNeighbor(
        const RoutePack& sol, const InputData& input_data, size_t route) const;

protected:
    virtual void VisitEachMove(const RoutePack& sol, size_t route,
                               std::function<void(const Move&)> evaluate) const = 0;

private:
    void VisitEachMove(const RoutePack& sol, std::function<void(const Move&)> evaluate) const;
};

/**
 * Represents all moves that affect the @p route by removing a vertex from the inside
 * and push it to the end
 */
class RemovePushBackNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;
};

/**
 * Represents all moves that affect the @p route by removing a vertex from one place
 * and inserting it into another
 */
class MoveVertexNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;
};

/**
 * Represents all moves that affect the @p route by swapping two adjacent vertices
 */
class SwapAdjNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;
};

/**
 * Represents all moves that affect the @p route by swapping two arbitrary vertices
 */
class SwapNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;
};

/**
 * Represents all moves that affect the @p route by reversing a subroute of length at least 3
 */
class TwoOptNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;
};

/**
 * Represents all moves that affect the @p route by advancing a subroute of length @p block_size
 * forward by one
 */
class BlockMoveForwardNeighborhood : public Neighborhood {
public:
    BlockMoveForwardNeighborhood(size_t block_size = 3);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};

/**
 * Represents all moves that affect the @p route by moving a subroute of length @p block_size
 * backwards by one
 */
class BlockMoveBackwardNeighborhood : public Neighborhood {
public:
    BlockMoveBackwardNeighborhood(size_t block_size = 3);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};

/**
 * Represents all moves that affect the @p route by reordering a subroute of length @p k
 */
class ReorderBlockNeighborhood : public Neighborhood {
public:
    ReorderBlockNeighborhood(size_t k);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};