#include <gtest/gtest.h>
#include <vector>

#include "neighborhood.h"

namespace {
struct MoveInterceptor {
    std::vector<std::unique_ptr<Move>> moves;
    std::function<void(const Move&)> evaluate = [this](const Move& m) {
        this->moves.push_back(m.Clone());
    };
};

RoutePack CreateMockPack(std::vector<int> vertices) {
    RoutePack pack;
    pack.AddRoute(Route(std::move(vertices)));
    return pack;
}
}

TEST(Neighborhoods, RelocateBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4});

    class MockRelocate : public RemovePushBackNeighborhood {
    public:
        using RemovePushBackNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockRelocate().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 3 moves: moving 1 / 2 / 3 to the end
    EXPECT_EQ(interceptor.moves.size(), 3);
    for (const auto& m : interceptor.moves) {
        EXPECT_EQ(m->Type(), N1_REMOVE_INSERT);
    }
}

TEST(Neighborhoods, SwapAdjBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5});

    class MockSwapAdj : public SwapAdjNeighborhood {
    public:
        using SwapAdjNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockSwapAdj().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 4 moves: swapping (1,2) / (2,3) / (3,4) / (4,5)
    EXPECT_EQ(interceptor.moves.size(), 4);
}

TEST(Neighborhoods, SwapNeighborhoodBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4});

    class MockSwap : public SwapNeighborhood {
    public:
        using SwapNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockSwap().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 6 moves: swapping (1,2) / (1,3) / (1,4) / (2,3) / (2,4) / (3,4)
    EXPECT_EQ(interceptor.moves.size(), 6);
}

TEST(Neighborhoods, TwoOptBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    class MockTwoOpt : public TwoOptNeighborhood {
    public:
        using TwoOptNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockTwoOpt().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 10 moves:
    // len=3 -> 4 moves: reversing [1,3] / [2,4] / [3,5] / [4,6]
    // len=3 -> 3 moves: reversing [1,4] / [2,5] / [3,6]
    // len=3 -> 2 moves: reversing [1,5] / [2,6]
    // len=3 -> 1 move:  reversing [1,6]
    EXPECT_EQ(interceptor.moves.size(), 10);
    EXPECT_EQ(interceptor.moves.front()->Type(), N4_2OPT);
}