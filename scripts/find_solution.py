import os
import argparse
import subprocess
import glob
from pathlib import Path

def run_solver(idx, dataset_dir, algorithm, fairness):
    prob_dir = f"{dataset_dir}/problems"
    sol_dir = f"{dataset_dir}/solutions"
    os.makedirs(sol_dir, exist_ok=True)

    input_file = os.path.join(prob_dir, f"{idx}.json")
    output_file = os.path.join(sol_dir, f"{idx}.json")

    if not os.path.exists(input_file):
        print(f"File {input_file} not found. Skipping")
        return

    print(f"Running algorithm for testcase {idx}...")
  
    cmd = ["../tdtsp_solver", "--st", "0.025", "--aon", "5", "--max-iter", "300", "--time-limit", "8", "--fairness", str(fairness), "--algorithm", algorithm, input_file, output_file]
    
    try:
        subprocess.run(cmd, check=True)
        print(f"Saved into: {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Error:\n{e}")

def main():
    parser = argparse.ArgumentParser(description="Run the routing algorithm")
    parser.add_argument("-a", "--all", action="store_true", help="Run for all test cases")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="Specify IDs of problems to run (e.g. -i 1 3 5)")
    parser.add_argument("-d", "--dataset", type=str, default="SPB", help="Dataset name (will be searched inside the ../data/ directory)")
    parser.add_argument("--dir", type=str, default="../data/", help="Datasets directory")
    parser.add_argument("--algorithm", type=str, help="Algorithm to use (rebalancing/annealing)")
    parser.add_argument("--fairness", type=float, default=0.5, help="Fairness importance (default: 0.5)")
    args = parser.parse_args()

    dataset_dir = Path(args.dir) / args.dataset
    if args.all:
        files = glob.glob(f"{dataset_dir}/problems/*.json")
        ids = sorted([int(os.path.basename(f).replace("problem_", "").replace(".json", "")) for f in files])
    elif args.ids:
        ids = args.ids
    else:
        print("Specify either --all or --ids. Use -h for help")
        return

    for idx in ids:
        run_solver(idx, dataset_dir, algorithm=args.algorithm, fairness=args.fairness)

if __name__ == "__main__":
    main()
