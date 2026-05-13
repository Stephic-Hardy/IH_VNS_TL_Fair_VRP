import os
import glob
import json
import argparse
import pandas as pd
import numpy as np

def calculate_gini(distances):
    if not distances:
        return 0.0
    sorted_dist = sorted(distances)
    n = len(distances)
    sum_dist = sum(sorted_dist)
    if sum_dist == 0:
        return 0.0
    weighted_sum = sum((i + 1) * d for i, d in enumerate(sorted_dist))
    return (2.0 * weighted_sum) / (n * sum_dist) - (n + 1.0) / n

def evaluate_solution(prob_file, sol_file):
    with open(prob_file, 'r') as f:
        prob = json.load(f)
    with open(sol_file, 'r') as f:
        sol_data = json.load(f)

    if isinstance(sol_data, dict):
        metadata = sol_data.get("metadata", {})
        routes = sol_data.get("solutions", [])
    else:
        metadata = {}
        routes = sol_data

    dist_matrix = prob["distance_matrix"]

    agent_distances = []
    total_time = 0
    total_distance = 0

    for agent in routes:
        path = agent["route"]
        agent_dist = 0

        for i in range(len(path) - 1):
            agent_dist += dist_matrix[path[i]][path[i+1]]

        agent_distances.append(agent_dist)
        total_distance += agent_dist
        total_time += agent.get("total_time", 0)

    gini = calculate_gini(agent_distances)
    max_distance = max(agent_distances) if agent_distances else 0
    min_distance = min(agent_distances) if agent_distances else 0
    max_min_diff = max_distance - min_distance if agent_distances else 0
    max_min_ratio = max_distance / min_distance if agent_distances else 0
    std_dev = np.std(agent_distances) if agent_distances else 0

    return {
        "agents_used": len(routes),
        "total_distance": total_distance,
        "total_time": total_time,
        "min_distance": min_distance,
        "max_distance": max_distance,
        "fairness_gini": gini,
        "fairness_max_min_diff": max_min_diff,
        "fairness_max_min_ratio": max_min_ratio,
        "fairness_std_dev": std_dev,
        **metadata # Injects ST, AON, exec_time if present
    }

def main():
    parser = argparse.ArgumentParser(description="Aggregate benchmarking results")
    parser.add_argument("-d", "--dir", type=str, default="../data/SPB", help="Problem directory")
    parser.add_argument("-m", "--osm", type=str, default="../lesnaya_area.pbf", help="Open Street Map data")
    parser.add_argument("-o", "--output_file", type=str, default="benchmark_results.csv", help="File to write benchmark results into")
    args = parser.parse_args()

    sol_files = glob.glob(f"{args.dir}/solutions/*.json")
    results = []

    print(f"Found {len(sol_files)} solutions")

    for sol_file in sol_files:
        idx = os.path.basename(sol_file).replace(".json", "")
        prob_file = f"{args.dir}/problems/{idx}.json"

        if not os.path.exists(prob_file):
            print(f"Warning: Problem file for idx {idx} missing. Skipping")
            continue

        metrics = evaluate_solution(prob_file, sol_file)
        metrics["problem_id"] = idx
        results.append(metrics)

    df = pd.DataFrame(results)


    cols = ['problem_id', 'execution_time_sec', 'total_distance', 'total_time',
            'fairness_gini', 'fairness_max_min_diff',  'fairness_max_min_ratio', 'fairness_std_dev']
    extra_cols = [c for c in df.columns if c not in cols]
    df = df[cols + extra_cols]
    os.makedirs(os.path.join(args.dir, "statistics"), exist_ok=True)
    output_file = os.path.join(args.dir, "statistics", args.output_file)
    df.to_csv(output_file, index=False)
    print(f"Saved to {output_file}")
    print(df.head())

if __name__ == "__main__":
    main()
