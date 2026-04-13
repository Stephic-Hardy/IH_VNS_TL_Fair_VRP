import os
import argparse
import json
import folium
import glob

def draw_map(idx):
    prob_dir = "../SPB_problems/Generates_SPB_problems/coords"
    sol_dir = "../SPB_problems/Generated_SPB_solutions"
    map_dir = "../SPB_problems/maps_solution_SPB"
    os.makedirs(map_dir, exist_ok=True)

    sol_file = os.path.join(sol_dir, f"generated_solution_{idx}.json")
    coords_file = os.path.join(prob_dir, f"Generated_problems_{idx}_coords.json")
    map_file = os.path.join(map_dir, f"map_results_{idx}.html")

    if not os.path.exists(sol_file) or not os.path.exists(coords_file):
        print(f"[-] Нет данных для решения {idx} (не найден JSON решения или координат). Пропускаем.")
        return

    with open(sol_file, 'r') as f:
        solutions = json.load(f)
    with open(coords_file, 'r') as f:
        coords_list = json.load(f)

    print(f"[+] Рисуем карту для задачи {idx}...")
    BASE_CORDS = coords_list[0]
    m = folium.Map(location=BASE_CORDS, zoom_start=13)

    colors = ['red', 'blue', 'green', 'purple', 'orange', 'darkred', 'lightred', 'beige', 'darkblue', 'darkgreen', 'cadetblue', 'darkpurple', 'white', 'pink', 'lightblue', 'lightgreen', 'gray', 'black', 'lightgray']

    for i, sol in enumerate(solutions):
        route_coords = [coords_list[node_idx] for node_idx in sol['route']]
        color = colors[i % len(colors)]
        
        folium.PolyLine(route_coords, color=color, weight=5, opacity=0.8, tooltip=f"Agent {i}").add_to(m)
        
        for p_idx, p_coord in enumerate(route_coords):
            folium.CircleMarker(
                location=p_coord,
                radius=3,
                color=color,
                fill=True,
                popup=f"Agent {i}, Point {p_idx}"
            ).add_to(m)

    m.save(map_file)
    print(f"    Сохранено: {map_file}")

def main():
    parser = argparse.ArgumentParser(description="Отрисовка решений на карте.")
    parser.add_argument("-a", "--all", action="store_true", help="Нарисовать карты для всех существующих решений")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="Список ID для отрисовки (например: -i 1 3 5)")
    args = parser.parse_args()

    if args.all:
        files = glob.glob("SPB_problems/Generated_SPB_solutions/generated_solution_*.json")
        ids = sorted([int(os.path.basename(f).replace("generated_solution_", "").replace(".json", "")) for f in files])
    elif args.ids:
        ids = args.ids
    else:
        print("Пожалуйста, укажите --all или --ids. Используйте -h для справки.")
        return

    for idx in ids:
        draw_map(idx)

if __name__ == "__main__":
    main()