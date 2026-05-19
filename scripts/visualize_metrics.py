import os
import json
import sys
import re
import glob
import numpy as np
import matplotlib.pyplot as plt
import argparse

def visualize_single_file(file_path):
    """
    Принимает полный путь к файлу, чтобы не гадать, где он лежит.
    """
    file_name = os.path.basename(file_path)
    
    # 1. Определяем папку для сохранения графиков
    # Мы хотим, чтобы графика лежала в Generated_SPB_solutions/solution_ID/Graphics/
    task_dir = os.path.dirname(file_path)
    output_dir = os.path.join(task_dir, "Graphics")
    os.makedirs(output_dir, exist_ok=True)

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"Error reading {file_name}: {e}")
        return

    # Извлечение данных
    agent_ids = [f"A{i+1}" for i in range(len(data))]
    distances = [float(sol.get('total_distance', 0)) for sol in data]
    times = [float(sol.get('total_time', 0)) for sol in data]
    values = [float(sol.get('total_value', 0)) for sol in data]

    # Построение графиков
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 12))
    x = np.arange(len(agent_ids))
    width = 0.6 

    ax1.bar(x, distances, width, color='#3498db')
    ax1.set_title(f'Distance: {file_name}')
    ax1.set_xticks(x)
    ax1.set_xticklabels(agent_ids)

    ax2.bar(x, times, width, color='#e67e22')
    ax2.set_title('Time (Cost)')
    ax2.set_xticks(x)
    ax2.set_xticklabels(agent_ids)

    colors = ['#2ecc71' if v >= 0 else '#e74c3c' for v in values]
    ax3.bar(x, values, width, color=colors)
    ax3.set_title('Value (Scores - Costs)')
    ax3.set_xticks(x)
    ax3.set_xticklabels(agent_ids)
    ax3.axhline(0, color='black', linewidth=0.8)

    plt.tight_layout()
    
    # Сохраняем результат
    save_name = file_name.replace(".json", ".png")
    save_path = os.path.join(output_dir, save_name)
    plt.savefig(save_path)
    print(f"Saved: {save_path}")
    plt.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Visualize VRP metrics")
    parser.add_argument("--ids", type=str, help="ID задачи (например, 23), чтобы визуализировать все файлы в папке solution_23")
    parser.add_argument("--file", type=str, help="Полный путь к конкретному файлу для визуализации")

    args = parser.parse_args()

    base_dir = os.path.dirname(os.path.abspath(__file__))
    # Базовая директория с решениями
    solutions_root = os.path.join(base_dir, "..", "SPB_problems", "Generated_SPB_solutions")

    if args.ids:
        # Теперь ищем внутри конкретной папки решения: solution_ID/
        task_folder = os.path.join(solutions_root, f"solution_{args.ids}")
        
        if not os.path.exists(task_folder):
            print(f"Directory not found: {task_folder}")
        else:
            # Ищем все JSON файлы в этой папке, подходящие под маску
            pattern = os.path.join(task_folder, f"generated_solution_{args.ids}*.json")
            files = glob.glob(pattern)
            
            if not files:
                print(f"No files found in {task_folder} for ID: {args.ids}")
            else:
                print(f"Found {len(files)} files in solution_{args.ids}. Processing...")
                for f in sorted(files):
                    visualize_single_file(f)
    
    elif args.file:
        # Если передан путь к файлу, просто запускаем (проверив существование)
        if os.path.exists(args.file):
            visualize_single_file(args.file)
        else:
            print(f"File not found: {args.file}")
    
    else:
        print("Please provide --ids <number> (e.g. --ids 1) or --file <full_path>")