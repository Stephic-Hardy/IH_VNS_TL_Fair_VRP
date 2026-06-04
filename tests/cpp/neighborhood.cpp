#include <gtest/gtest.h>
#include <vector>
#include "move.h"

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

RoutePack CreateMultiMockPack(std::vector<std::vector<int>> routes_data) {
    RoutePack pack;
    for (auto& data : routes_data) {
        pack.AddRoute(Route(std::move(data)));
    }
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
    // len=4 -> 3 moves: reversing [1,4] / [2,5] / [3,6]
    // len=5 -> 2 moves: reversing [1,5] / [2,6]
    // len=6 -> 1 move:  reversing [1,6]
    EXPECT_EQ(interceptor.moves.size(), 10);
    EXPECT_EQ(interceptor.moves.front()->Type(), N4_2OPT);
}

TEST(Neighborhoods, TwoOptSmallTest) {
    auto pack = CreateMockPack({0, 1, 2});

    class MockTwoOpt : public TwoOptNeighborhood {
    public:
        using TwoOptNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockTwoOpt().VisitEachMove(pack, 0, interceptor.evaluate);

    EXPECT_EQ(interceptor.moves.size(), 0);
}

TEST(Neighborhoods, BlockMoveForwardBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    class MockBlockMoveForward : public BlockMoveForwardNeighborhood {
    public:
        using BlockMoveForwardNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockBlockMoveForward().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 3 moves: [1,3] / [2,4] / [3,5]
    EXPECT_EQ(interceptor.moves.size(), 3);
    EXPECT_EQ(interceptor.moves.front()->Type(), N5_MOVE_FWD_K);
}

TEST(Neighborhoods, BlockMoveForwardSmallTest) {
    auto pack = CreateMockPack({0, 1, 2});

    class MockBlockMoveForward : public BlockMoveForwardNeighborhood {
    public:
        using BlockMoveForwardNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockBlockMoveForward().VisitEachMove(pack, 0, interceptor.evaluate);

    EXPECT_EQ(interceptor.moves.size(), 0);
}

TEST(Neighborhoods, BlockMoveBackwardBoundsTest) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    class MockBlockMoveBackward : public BlockMoveBackwardNeighborhood {
    public:
        using BlockMoveBackwardNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockBlockMoveBackward().VisitEachMove(pack, 0, interceptor.evaluate);

    // Expect 3 moves: [2,4] / [3,5] / [4,6]
    EXPECT_EQ(interceptor.moves.size(), 3);
    EXPECT_EQ(interceptor.moves.front()->Type(), N6_MOVE_BWD_K);
}

TEST(Neighborhoods, BlockMoveBackwardSmallTest) {
    auto pack = CreateMockPack({0, 1, 2});

    class MockBlockMoveBackward : public BlockMoveBackwardNeighborhood {
    public:
        using BlockMoveBackwardNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockBlockMoveBackward().VisitEachMove(pack, 0, interceptor.evaluate);

    EXPECT_EQ(interceptor.moves.size(), 0);
}

TEST(Neighborhoods, InterRelocateBoundsTest) {
    auto pack = CreateMultiMockPack({{0, 1}, {0, 2}});

    class MockInterRelocate : public InterRelocateNeighborhood {
    public:
        using InterRelocateNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockInterRelocate().VisitEachMove(pack, interceptor.evaluate);

    // Expect 4 moves:
    // R0->R1: 2 moves
    // R1->R0: 2 moves
    EXPECT_EQ(interceptor.moves.size(), 4);
}

TEST(Neighborhoods, InterSwapBoundsTest) {
    auto pack = CreateMultiMockPack({{0, 1, 2}, {0, 3}});

    class MockInterSwap : public InterSwapNeighborhood {
    public:
        using InterSwapNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockInterSwap().VisitEachMove(pack, interceptor.evaluate);

    // Expect 2 moves: swapping (R0:1, R1:3) / (R0:2, R1:3)
    EXPECT_EQ(interceptor.moves.size(), 2);
}

TEST(Neighborhoods, TwoOptStarBoundsTest) {
    auto pack = CreateMultiMockPack({{0, 1, 2, 3}, {0, 4, 5, 6}});

    class MockTwoOptStar : public TwoOptStarNeighborhood {
    public:
        using TwoOptStarNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockTwoOptStar().VisitEachMove(pack, interceptor.evaluate);

    // Expect 4 moves:
    // Swap pairs of edges from R0: [1,2] / [2,3] and R1: [4,5] / [5,6]
    EXPECT_EQ(interceptor.moves.size(), 4);
}

TEST(Neighborhoods, CrossExchangeBoundsTest) {
    auto pack = CreateMultiMockPack({{0, 1, 2, 3}, {0, 4, 5, 6}});

    class MockCrossExchange : public CrossExchangeNeighborhood {
    public:
        using CrossExchangeNeighborhood::VisitEachMove;
    };

    MoveInterceptor interceptor;
    MockCrossExchange().VisitEachMove(pack, interceptor.evaluate);

    // Expect 36 moves:
    // R0 -> 6 moves: [1], [2], [3], [1,2], [2,3], [1,2,3]
    // R1 -> 6 moves
    EXPECT_EQ(interceptor.moves.size(), 36);
}