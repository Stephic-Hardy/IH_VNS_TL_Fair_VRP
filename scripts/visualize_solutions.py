import os
import argparse
import json
import math
import glob
import folium
from folium import plugins
from itertools import combinations
import osmnx as ox
from pyrosm import OSM

def get_direction(lat1, lon1, lat2, lon2):
    lat1, lon1, lat2, lon2 = map(math.radians, [lat1, lon1, lat2, lon2])
    dlon = lon2 - lon1
    y = math.sin(dlon) * math.cos(lat2)
    x = math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(dlon)
    brng = math.degrees(math.atan2(y, x))
    return (brng + 360) % 360

def draw_map(idx, show_labels=False, show_connections=False, target_routes=None, road_graph=None):
    prob_coords_dir = "../SPB_problems/Generates_SPB_problems/coords"
    prob_main_dir = "../SPB_problems/Generates_SPB_problems/problems"
    sol_dir = "../SPB_problems/Generated_SPB_solutions"
    map_dir = "../SPB_problems/maps_solution_SPB"
    os.makedirs(map_dir, exist_ok=True)

    sol_file = os.path.join(sol_dir, f"generated_solution_{idx}.json")
    coords_file = os.path.join(prob_coords_dir, f"Generated_problems_{idx}_coords.json")
    prob_file = os.path.join(prob_main_dir, f"Generated_problems_{idx}.json")
    map_file = os.path.join(map_dir, f"map_results_{idx}.html")

    if not os.path.exists(sol_file) or not os.path.exists(coords_file):
        print(f"Missing data for task {idx}. Skipping")
        return

    with open(sol_file, 'r') as f:
        solutions = json.load(f)
    with open(coords_file, 'r') as f:
        coords_list = json.load(f)

    if road_graph is not None:
        lats = [p[0] for p in coords_list]
        lons = [p[1] for p in coords_list]
        snapped_nodes = ox.nearest_nodes(road_graph, lons, lats)
        coords_list = [[road_graph.nodes[n]['y'], road_graph.nodes[n]['x']] for n in snapped_nodes]

    dist_matrix = None
    if show_labels or show_connections:
        if not os.path.exists(prob_file):
            print(f"Problem file {prob_file} not found. Edge info will be hidden")
            show_labels = False
            show_connections = False
        else:
            with open(prob_file, 'r') as f:
                dist_matrix = json.load(f)["distance_matrix"]

    print(f"Drawing map for task {idx}...")
    BASE_CORDS = coords_list[0]
    m = folium.Map(location=BASE_CORDS, zoom_start=13)

    colors = ['red', 'blue', 'green', 'purple', 'orange', 'darkred', 'lightred', 'beige', 'darkblue', 'darkgreen', 'cadetblue', 'darkpurple', 'black']

    for i, sol in enumerate(solutions):
        if target_routes is not None and i not in target_routes:
            continue

        route = sol['route']
        route_coords = [coords_list[node_idx] for node_idx in route]
        color = colors[i % len(colors)]
        route_edges = {tuple(sorted((route[j-1], route[j]))) for j in range(1, len(route))}

        if show_connections and dist_matrix:
            unique_nodes = list(dict.fromkeys(route))
            for u, v in combinations(unique_nodes, 2):
                if tuple(sorted((u, v))) in route_edges:
                    continue

                coord_u = coords_list[u]
                coord_v = coords_list[v]
                dist_uv = dist_matrix[u][v]
                dist_vu = dist_matrix[v][u]

                folium.PolyLine([coord_u, coord_v], color=color, weight=1, opacity=0.3, dash_array='5, 5').add_to(m)

                mid_lat = (coord_u[0] + coord_v[0]) / 2.0
                mid_lon = (coord_u[1] + coord_v[1]) / 2.0

                html_bg = f'''
                <div style="font-size: 7pt; color: #444; background-color: rgba(255,255,255,0.75); 
                            border-radius: 3px; padding: 1px 3px; border: 1px solid {color};
                            white-space: nowrap; width: fit-content; text-align: center;">
                    {u}➔{v}: {dist_uv}s<br>{v}➔{u}: {dist_vu}s
                </div>
                '''
                folium.Marker(location=[mid_lat, mid_lon], icon=folium.DivIcon(icon_size=(100, 30), icon_anchor=(50, 15), html=html_bg)).add_to(m)

        line = folium.PolyLine(route_coords, color=color, weight=5, opacity=0.7, tooltip=f"Agent {i}")
        line.add_to(m)

        if show_labels:
            plugins.PolyLineTextPath(line, '  ►  ', repeat=True, offset=6, attributes={'fill': color, 'font-weight': 'bold', 'font-size': '16'}).add_to(m)

        for p_idx, node_idx in enumerate(route):
            p_coord = coords_list[node_idx]
            folium.CircleMarker(location=p_coord, radius=4, color=color, fill=True, popup=f"Agent {i}, Order {p_idx}, Node {node_idx}").add_to(m)

            if show_labels:
                folium.Marker(location=p_coord, icon=folium.DivIcon(icon_size=(150, 36), icon_anchor=(-8, 12),
                                                                    html=f'<div style="font-size: 11pt; font-weight: bold; color: {color}; text-shadow: -1px -1px 0 #fff, 1px -1px 0 #fff, -1px 1px 0 #fff, 1px 1px 0 #fff;">{node_idx}</div>')).add_to(m)

                if p_idx > 0:
                    prev_node_idx = route[p_idx - 1]
                    prev_coord = coords_list[prev_node_idx]
                    edge_time = dist_matrix[prev_node_idx][node_idx]
                    mid_lat, mid_lon = (prev_coord[0] + p_coord[0]) / 2.0, (prev_coord[1] + p_coord[1]) / 2.0
                    bearing = get_direction(prev_coord[0], prev_coord[1], p_coord[0], p_coord[1])
                    rotation = bearing - 90

                    html_badge = f'''
                    <div style="font-size: 9pt; font-weight: bold; color: black; background-color: rgba(255,255,255,0.9); 
                                border-radius: 4px; padding: 2px 5px; display: flex; align-items: center; gap: 4px; 
                                border: 2px solid {color}; white-space: nowrap; width: fit-content; box-shadow: 1px 1px 3px rgba(0,0,0,0.3); z-index: 1000;">
                        <span>{edge_time}s</span>
                        <span style="transform: rotate({rotation}deg); display: inline-block;">➤</span>
                    </div>
                    '''
                    folium.Marker(location=[mid_lat, mid_lon], icon=folium.DivIcon(icon_size=(150, 36), icon_anchor=(30, 10), html=html_badge)).add_to(m)

    m.save(map_file)
    print(f"Saved: {map_file}")

def main():
    parser = argparse.ArgumentParser(description="Visualize solver solutions on a map")
    parser.add_argument("-a", "--all", action="store_true", help="Draw maps for all existing solutions")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="List of solution IDs to draw")
    parser.add_argument("-l", "--labels", action="store_true", help="Show Node IDs and edge costs")
    parser.add_argument("-c", "--connections", action="store_true", help="Show all possible internal connections")
    parser.add_argument("-r", "--routes", type=int, nargs='+', help="Filter specific agent indices")

    parser.add_argument("--no-snapped", action="store_true", help="Disable snapping points to actual road nodes (show raw coordinates)")

    args = parser.parse_args()

    road_graph = None
    if not args.no_snapped:
        print("Loading OSM Map for road-node snapping (might take a few seconds)...")
        try:
            osm = OSM("../lesnaya_area.pbf")
            nodes_data, edges_data = osm.get_network(network_type="driving", nodes=True)
            road_graph = osm.to_graph(nodes_data, edges_data, graph_type="networkx")
            print(f"Loaded {len(road_graph.nodes)} nodes")
        except Exception as e:
            print(f"Error loading map: {e}. Falling back to raw coordinates")

    if args.all:
        files = glob.glob("../SPB_problems/Generated_SPB_solutions/generated_solution_*.json")
        ids = sorted([int(os.path.basename(f).replace("generated_solution_", "").replace(".json", "")) for f in files])
    elif args.ids:
        ids = args.ids
    else:
        print("Please specify --all or --ids.")
        return

    for idx in ids:
        draw_map(idx, show_labels=args.labels, show_connections=args.connections, target_routes=args.routes, road_graph=road_graph)

if __name__ == "__main__":
    main()