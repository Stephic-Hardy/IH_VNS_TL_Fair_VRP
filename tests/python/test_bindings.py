import tdtsp as vrp

def test_import_success():
    assert hasattr(vrp, 'InputData')
    assert hasattr(vrp, 'AgentSolution')
    assert hasattr(vrp, 'BenchmarkMetadata')
    assert hasattr(vrp, 'Solution')
    assert hasattr(vrp, 'BaselineSolver')
    assert hasattr(vrp, 'AnnealingSolver')
    assert hasattr(vrp, 'RebalancingSolver')


def test_solver():
    data = vrp.InputData(
        points_count=3,
        min_load=1,
        max_load=10,
        max_time=3600,
        max_distance=5000,
        distance_matrix=[
            [0, 10, 20],
            [10, 0, 15],
            [20, 15, 0]
        ],
        time_matrix=[
            [
                [0, 10, 20],
                [10, 0, 15],
                [20, 15, 0]
            ],
        ],
        point_scores=[100, 200],
        point_service_times=[300, 300]
    )
    
    solver = vrp.AnnealingSolver(
        st=100.0,
        aon=5,
        max_iter=100,
        time_limit=5,
        alpha=0.4
    )
    
    solution = solver.solve(data)
    
    assert isinstance(solution, vrp.Solution)
    assert len(solution.agents) == 1
    
    agent = solution.agents[0]
    assert isinstance(agent, vrp.AgentSolution)
    assert len(agent.route) == 4
    assert agent.solution_size == len(agent.route)
    assert agent.total_time == 645
    assert agent.total_distance == 45
    assert agent.total_value == 300 - 45
    
    assert isinstance(solution.meta, vrp.BenchmarkMetadata)
    assert solution.meta.st == 100.0
    assert solution.meta.aon == 5
    assert solution.meta.max_iter == 100
    assert solution.meta.time_limit == 5
    assert solution.meta.alpha == 0.4
    assert solution.meta.execution_time > 0
