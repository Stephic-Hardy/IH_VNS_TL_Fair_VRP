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

class RelocateNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                     std::function<void(const Move&)> evaluator) const override;
};

class MoveVertexNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                     std::function<void(const Move&)> evaluator) const override;
};

class SwapAdjNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                     std::function<void(const Move&)> evaluator) const override;
};

class SwapNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                     std::function<void(const Move&)> evaluator) const override;
};

class TwoOptNeighborhood : public Neighborhood {
protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                     std::function<void(const Move&)> evaluator) const override;
};

class BlockMoveForwardNeighborhood : public Neighborhood {
public:
    BlockMoveForwardNeighborhood(size_t block_size = 3);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};


class BlockMoveBackwardNeighborhood : public Neighborhood {
public:
    BlockMoveBackwardNeighborhood(size_t block_size = 3);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route,
                       std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};

class ReorderBlockNeighborhood : public Neighborhood {
public:
    ReorderBlockNeighborhood(size_t k);

protected:
    void VisitEachMove(const RoutePack& sol, size_t route, std::function<void(const Move&)> evaluator) const override;

private:
    size_t k_;
};
