import os
import argparse
import json
import folium
import glob

def draw_map_for_file(sol_file, coords_file, map_dir, idx):
    # Определяем имя для HTML на основе имени JSON
    base_name = os.path.basename(sol_file).replace(".json", "")
    map_file = os.path.join(map_dir, f"map_{base_name}.html")

    try:
        with open(sol_file, 'r') as f:
            solutions = json.load(f)
        with open(coords_file, 'r') as f:
            coords_list = json.load(f) # Здесь теперь просто [[lat, lon], ...]
    except Exception as e:
        print(f"    [!] Ошибка загрузки файлов для {base_name}: {e}")
        return

    print(f"    -> Рисуем карту: {base_name}")
    
    # Центрируем карту на депо (первая точка)
    m = folium.Map(location=coords_list[0], zoom_start=13)
    
    colors = ['red', 'blue', 'green', 'purple', 'orange', 'darkred', 'lightred', 'beige', 'darkblue', 'darkgreen', 'cadetblue', 'darkpurple', 'pink', 'lightblue', 'lightgreen', 'gray', 'black', 'lightgray']

    for i, sol in enumerate(solutions):
        # Собираем координаты маршрута, используя индексы из решения
        route_coords = [coords_list[node_idx] for node_idx in sol['route']]
        color = colors[i % len(colors)]
        
        # Рисуем линию маршрута
        folium.PolyLine(route_coords, color=color, weight=5, opacity=0.8, tooltip=f"Agent {i}").add_to(m)
        
        # Рисуем маркеры точек
        for p_idx, p_coord in enumerate(route_coords):
            folium.CircleMarker(
                location=p_coord,
                radius=3,
                color=color,
                fill=True,
                popup=f"Agent {i}, Point {p_idx} (Node {sol['route'][p_idx]})"
            ).add_to(m)

    m.save(map_file)

def process_task_visualization(idx):
    prob_dir = "../SPB_problems/Generates_SPB_problems/coords"
    sol_base_dir = "../SPB_problems/Generated_SPB_solutions"
    
    # Путь к папке, где лежат JSON-ы задачи (теперь они в подпапках solution_ID)
    task_sol_dir = os.path.join(sol_base_dir, f"solution_{idx}")
    
    # Создаем папку для карт этой задачи
    map_task_dir = os.path.join("../SPB_problems/maps_solution_SPB", f"maps_{idx}")
    os.makedirs(map_task_dir, exist_ok=True)

    coords_file = os.path.join(prob_dir, f"Generated_problems_{idx}_coords.json")

    if not os.path.exists(task_sol_dir):
        # Если подпапки нет, пробуем поискать в корне sol_base_dir (для старых версий)
        if os.path.exists(os.path.join(sol_base_dir, f"generated_solution_{idx}.json")):
            task_sol_dir = sol_base_dir
        else:
            print(f"[-] Нет данных решения для задачи {idx}. Пропускаем.")
            return

    if not os.path.exists(coords_file):
        print(f"[-] Файл координат {coords_file} не найден. Пропускаем.")
        return

    # Ищем все JSON решения для этой задачи (основной, BEFORE, AFTER, FINAL)
    solution_files = glob.glob(os.path.join(task_sol_dir, f"generated_solution_{idx}*.json"))
    
    if not solution_files:
        print(f"[-] В папке {task_sol_dir} не найдено JSON-файлов для задачи {idx}.")
        return

    print(f"[+] Обработка карт для задачи {idx} ({len(solution_files)} файлов)...")
    for sol_file in sorted(solution_files):
        draw_map_for_file(sol_file, coords_file, map_task_dir, idx)
    print(f"    Все карты сохранены в: {map_task_dir}")

def main():
    parser = argparse.ArgumentParser(description="Отрисовка решений на карте.")
    parser.add_argument("-a", "--all", action="store_true", help="Нарисовать карты для всех существующих решений")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="Список ID для отрисовки")
    args = parser.parse_args()

    if args.all:
        # Ищем все папки или файлы решений
        sol_base_dir = "../SPB_problems/Generated_SPB_solutions"
        pattern = os.path.join(sol_base_dir, "solution_*")
        ids = []
        for d in glob.glob(pattern):
            try:
                name = os.path.basename(d)
                ids.append(int(name.replace("solution_", "")))
            except:
                continue
        ids = sorted(list(set(ids)))
    elif args.ids:
        ids = args.ids
    else:
        print("Используйте -a или -i <id>")
        return

    for idx in ids:
        process_task_visualization(idx)

if __name__ == "__main__":
    main()