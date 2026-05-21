import os
import argparse
import json
import numpy as np
import networkx as nx
import osmnx as ox
from pathlib import Path
from shapely.geometry import Point
from pyrosm import OSM
from functools import partial
from concurrent.futures import ProcessPoolExecutor

def get_next_index(directory, prefix=""):
    existing_files = [f for f in os.listdir(directory) if f.endswith('.json')]
    indices = [int(f.replace(prefix, '').replace('.json', '')) for f in existing_files
                if f.replace(prefix, '').replace('.json', '').isdigit()]
    return max(indices) + 1 if indices else 1

def main():
    parser = argparse.ArgumentParser(description="Generator of problem datasets from real OSM data")
    parser.add_argument("-n", "--count", type=int, default=1, help="Number of problems to generate")
    parser.add_argument("-d", "--dataset", type=str, default="SPB", help="Dataset name (will be searched inside the ../data/ directory)")
    parser.add_argument("--dir", type=str, default="../data/", help="Datasets directory")
    parser.add_argument("-o", "--osm", type=str, default="../data/SPB/lesnaya_area.pbf", help="Open Street Map data")
    args = parser.parse_args()

    dataset_dir = Path(args.dir) / args.dataset
    out_dir_problems = f"{dataset_dir}/problems"
    out_dir_coords = f"{dataset_dir}/coords"

    os.makedirs(out_dir_problems, exist_ok=True)
    os.makedirs(out_dir_coords, exist_ok=True)


    print("Loading Open Street Map data...")
    osm = OSM(args.osm)
    nodes_data, edges_data = osm.get_network(network_type="driving", nodes=True)
    graph = osm.to_graph(nodes_data, edges_data, graph_type="networkx")
    graph = ox.add_edge_speeds(graph)
    graph = ox.add_edge_travel_times(graph)
    print(f"Loaded Road Graph: {len(graph.nodes)} nodes")

    kalininsky = ox.geocode_to_gdf("Kalininsky District, Saint Petersburg, Russia").geometry.iloc[0]
    vyborgsky = ox.geocode_to_gdf("Vyborgsky District, Saint Petersburg, Russia").geometry.iloc[0]
    boundary = kalininsky.union(vyborgsky)

    BASE_CORDS = (60.00771529992149, 30.370180423873254)
    NUM_POINTS = 200
    std_dev = 0.015

    for iteration in range(args.count):
        print()
        print(f"Generating {iteration + 1}/{args.count}")
        points = []
        used_node_ids = set()


        base_node, base_dist = ox.distance.nearest_nodes(graph, BASE_CORDS[1], BASE_CORDS[0], return_dist=True)
        used_node_ids.add(base_node)

        final_data_list = [[BASE_CORDS[0], BASE_CORDS[1], base_node]]

        while len(points) < NUM_POINTS:
            lat = np.random.normal(BASE_CORDS[0], std_dev)
            lon = np.random.normal(BASE_CORDS[1], std_dev)
            p = Point(lon, lat)
            if boundary.contains(p):
                node_id, dist = ox.distance.nearest_nodes(graph, lon, lat, return_dist=True)

                if dist <= 15.0 and node_id not in used_node_ids:
                    used_node_ids.add(node_id)
                    points.append((lat, lon))
                    final_data_list.append([lat, lon, node_id])


        nodes = [item[2] for item in final_data_list]
        coords_for_save = [[item[0], item[1]] for item in final_data_list]

        print("Calculating distance matrix...")
        dist_matrix = []
        for source in nodes:
            lengths = nx.single_source_dijkstra_path_length(graph, source, weight='travel_time')
            row = [int(lengths.get(target, 1000000)) for target in nodes]
            dist_matrix.append(row)

        problem_data = {
            "points_count": len(final_data_list),
            "min_load": 10,
            "max_load": 35, 
            "max_time": 36000,
            "max_distance": 1000000,
            "distance_matrix": dist_matrix,
            "time_matrix": [dist_matrix] * 14,
            "point_scores": [1000] * (len(final_data_list) - 1),
            "point_service_times": [300] * (len(final_data_list) - 1)
        }

        next_idx = get_next_index(out_dir_problems)
        
        problem_path = os.path.join(out_dir_problems, f"{next_idx}.json")
        with open(problem_path, 'w') as f:
            json.dump(problem_data, f)
            
        coords_path = os.path.join(out_dir_coords, f"{next_idx}.json")
        with open(coords_path, 'w') as f:
            json.dump(coords_for_save, f)

        print(f"Saved: {problem_path}")

if __name__ == "__main__":
    main()
