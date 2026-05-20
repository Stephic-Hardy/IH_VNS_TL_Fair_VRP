#include <gtest/gtest.h>
#include <numeric>

#include "move.h"
#include "route_pack.h"

namespace {
RoutePack CreateMockPack(std::vector<int> vertices) {
    RoutePack pack;
    pack.AddRoute(Route(std::move(vertices)));
    return pack;
}
}

TEST(SwapMove, SwapsCorrectly) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4});

    SwapMove move(0, 1, 3);
    RoutePack result = move.Apply(pack);

    std::vector<int> expected = {0, 3, 2, 1, 4};
    EXPECT_EQ(result.GetRoute(0).Vertices(), expected);
}

TEST(SwapMove, TypeInfersCorrectly) {
    EXPECT_EQ(SwapMove(0, 2, 3).Type(), N2_SWAP_ADJ);
    EXPECT_EQ(SwapMove(0, 1, 4).Type(), N3_SWAP);
}

TEST(TwoOptMove, ReversesSegmentCorrectly) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    TwoOptMove move(0, 2, 5);
    RoutePack result = move.Apply(pack);

    std::vector<int> expected = {0, 1, 5, 4, 3, 2, 6};
    EXPECT_EQ(result.GetRoute(0).Vertices(), expected);
}

TEST(BlockRelocateMove, MovesForwardCorrectly) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    BlockRelocateMove move(0, 1, 2, 4);
    RoutePack result = move.Apply(pack);

    std::vector<int> expected = {0, 3, 4, 5, 1, 2, 6};
    EXPECT_EQ(result.GetRoute(0).Vertices(), expected);
    EXPECT_EQ(move.Type(), N5_MOVE_FWD_K);
}

TEST(BlockRelocateMove, MovesBackwardCorrectly) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4, 5, 6});

    BlockRelocateMove move(0, 4, 2, 1);
    RoutePack result = move.Apply(pack);

    std::vector<int> expected = {0, 4, 5, 1, 2, 3, 6};
    EXPECT_EQ(result.GetRoute(0).Vertices(), expected);
    EXPECT_EQ(move.Type(), N6_MOVE_BWD_K);
}

TEST(ReorderBlockMove, ReordersCorrectly) {
    auto pack = CreateMockPack({0, 1, 2, 3, 4});

    std::vector<int> new_order = {3, 1, 2};
    ReorderBlockMove move(0, 1, new_order);
    RoutePack result = move.Apply(pack);

    std::vector<int> expected = {0, 3, 1, 2, 4};
    EXPECT_EQ(result.GetRoute(0).Vertices(), expected);
}