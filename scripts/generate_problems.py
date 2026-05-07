import os
import argparse
import json
import numpy as np
import networkx as nx
import osmnx as ox
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
    parser.add_argument("-d", "--dir", type=str, default="../data/SPB", help="Dataset directory")
    parser.add_argument("-o", "--osm", type=str, default="../lesnaya_area.pbf", help="Open Street Map data")
    args = parser.parse_args()

    out_dir_problems = f"{args.dir}/problems"
    out_dir_coords = f"{args.dir}/coords"

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
        while len(points) < NUM_POINTS:
            lat = np.random.normal(BASE_CORDS[0], std_dev)
            lon = np.random.normal(BASE_CORDS[1], std_dev)
            p = Point(lon, lat)
            if boundary.contains(p):
                points.append((lat, lon))
        
        coords_list = [BASE_CORDS] + points

        print("Snapping points to the omp nodes...")
        nodes = ox.nearest_nodes(graph, [p[1] for p in coords_list], [p[0] for p in coords_list])

        print("Calculating distance matrix...")
        dist_matrix = []
        for source in nodes:
            lengths = nx.single_source_dijkstra_path_length(graph, source, weight='travel_time')
            row = [int(lengths.get(target, 1000000)) for target in nodes]
            dist_matrix.append(row)

        problem_data = {
            "points_count": len(coords_list),
            "min_load": 10,
            "max_load": 35, 
            "max_time": 36000,
            "max_distance": 1000000,
            "distance_matrix": dist_matrix,
            "time_matrix": [dist_matrix * 14], 
            "point_scores": [1000] * (len(coords_list) - 1),
            "point_service_times": [300] * (len(coords_list) - 1)
        }

        next_idx = get_next_index(out_dir_problems)
        
        problem_path = os.path.join(out_dir_problems, f"{next_idx}.json")
        with open(problem_path, 'w') as f:
            json.dump(problem_data, f)
            
        coords_path = os.path.join(out_dir_coords, f"{next_idx}.json")
        with open(coords_path, 'w') as f:
            json.dump(coords_list, f)

        print(f"Saved: {problem_path}")

if __name__ == "__main__":
    main()
