import osmnx as ox
import networkx as nx
import numpy as np
import logging
import json
import subprocess
import folium
from shapely.geometry import Point
from matplotlib import colormaps
from pyrosm import OSM, get_data
import requests


print("Загружаем карту:")
osm = OSM("lesnaya_area.pbf")
BASE_CORDS = (60.00771529992149, 30.370180423873254)
RADIUS = 500
NUM_POINTS = 200
print("Парсим:")
nodes, edges = osm.get_network(network_type="driving", nodes=True)

print("Загружаем граф дорог...")
graph = osm.to_graph(nodes, edges, graph_type="networkx")

graph = ox.add_edge_speeds(graph)
graph = ox.add_edge_travel_times(graph)

print(f"Граф готов: {len(graph.nodes)} узлов")

kalininsky = ox.geocode_to_gdf("Kalininsky District, Saint Petersburg, Russia").geometry.iloc[0]
vyborgsky = ox.geocode_to_gdf("Vyborgsky District, Saint Petersburg, Russia").geometry.iloc[0]
boundary = kalininsky.union(vyborgsky)


print("Генерируем точки...")
points = []
std_dev = 0.015 

while len(points) < NUM_POINTS:
    lat = np.random.normal(BASE_CORDS[0], std_dev)
    lon = np.random.normal(BASE_CORDS[1], std_dev)
    p = Point(lon, lat)
    if boundary.contains(p):
        points.append((lat, lon))
coords_list = [BASE_CORDS] + points

print("Считаем матрицу времени... Сорян, долговато")
nodes = ox.nearest_nodes(graph, [p[1] for p in coords_list], [p[0] for p in coords_list])


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
    "point_scores": [100] * (len(coords_list) - 1),
    "point_service_times": [300] * (len(coords_list) - 1)
}

with open('generated_problem.json', 'w') as f:
    json.dump(problem_data, f)

print("Запускаем C++ алгоритм...")
subprocess.run(["../build/app", "-p", "generated_problem.json", "-s", "generated_solution.json", "-t", "30"])


print("Рисуем карту...")
m = folium.Map(location=BASE_CORDS, zoom_start=13)

with open('generated_solution.json', 'r') as f:
    solutions = json.load(f)


colors = ['red', 'blue', 'green', 'purple', 'orange', 'darkred', 'lightred', 'beige', 'darkblue', 'darkgreen', 'cadetblue', 'darkpurple', 'white', 'pink', 'lightblue', 'lightgreen', 'gray', 'black', 'lightgray']

for idx, sol in enumerate(solutions):
    route_coords = [coords_list[node_idx] for node_idx in sol['route']]
    color = colors[idx % len(colors)]
    
    folium.PolyLine(route_coords, color=color, weight=5, opacity=0.8, tooltip=f"Agent {idx}").add_to(m)
    
    for p_idx, p_coord in enumerate(route_coords):
        folium.CircleMarker(
            location=p_coord,
            radius=3,
            color=color,
            fill=True,
            popup=f"Agent {idx}, Point {p_idx}"
        ).add_to(m)

m.save('map_results.html')
