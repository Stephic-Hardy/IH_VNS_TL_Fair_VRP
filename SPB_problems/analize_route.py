import json
import sys

def analyze_agent_route(input_json_path, output_json_path, agent_index=0):
    # Загружаем входные данные (матрицы)
    with open(input_json_path, 'r', encoding='utf-8') as f:
        input_data = json.load(f)
    
    # Загружаем твой результат
    with open(output_json_path, 'r', encoding='utf-8') as f:
        output_data = json.load(f)

    if agent_index >= len(output_data):
        print(f"Ошибка: Агента с индексом {agent_index} не существует.")
        return

    route = output_data[agent_index]['route']
    dist_matrix = input_data['distance_matrix']
    time_matrix = input_data['time_matrix']
    scores = input_data['point_scores']
    service_times = input_data['point_service_times']
    time_duration = input_data.get('time_duration', 3600) # По умолчанию час, если нет в json

    print(f"=== Анализ маршрута Агента №{agent_index} ===")
    print(f"{'Откуда':<8} -> {'Куда':<8} | {'Расст.':<10} | {'Время':<10} | {'Прибытие':<10} | {'Value':<6}")
    print("-" * 70)

    current_time = 0
    total_dist = 0
    total_value = 0

    for i in range(len(route) - 1):
        u = route[i]
        v = route[i+1]
        
        # Считаем индекс временного слоя
        time_slot = int(current_time // time_duration)
        if time_slot >= len(time_matrix):
            time_slot = len(time_matrix) - 1
        
        d = dist_matrix[u][v]
        t = time_matrix[time_slot][u][v]
        val = scores[v]
        service = service_times[v]

        arrival_time = current_time + t
        print(f"{u:<8} -> {v:<8} | {d:<10.2f} | {t:<10.2f} | {arrival_time:<10.2f} | {val:<6}")
        
        current_time = arrival_time + service
        total_dist += d
        total_value += val

    print("-" * 70)
    print(f"ИТОГО: Расстояние: {total_dist:.2f}, Время: {current_time:.2f}, Value: {total_value}")

if __name__ == "__main__":
    # Использование: python analyze_route.py input.json output.json 0
    # 0 - это индекс агента (первый)
    path_in = "input.json"  # замени на свои пути
    path_out = "output.json"
    idx = 0
    if len(sys.argv) > 1: path_in = sys.argv[1]
    if len(sys.argv) > 2: path_out = sys.argv[2]
    if len(sys.argv) > 3: idx = int(sys.argv[3])
    
    analyze_agent_route(path_in, path_out, idx)