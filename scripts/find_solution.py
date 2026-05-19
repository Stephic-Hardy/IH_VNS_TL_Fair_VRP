import os
import argparse
import subprocess
import glob
import shutil

def run_solver(idx):
    prob_dir = "../SPB_problems/Generates_SPB_problems/problems"
    sol_base_dir = "../SPB_problems/Generated_SPB_solutions"
    
    # Создаем отдельную папку для результатов этой конкретной задачи
    task_sol_dir = os.path.join(sol_base_dir, f"solution_{idx}")
    os.makedirs(task_sol_dir, exist_ok=True)

    input_file = os.path.join(prob_dir, f"Generated_problems_{idx}.json")
    # Базовый путь для вывода (алгоритм сам добавит суффиксы _BEFORE и т.д.)
    output_base = os.path.join(task_sol_dir, f"generated_solution_{idx}.json")

    if not os.path.exists(input_file):
        print(f"[-] Файл {input_file} не найден. Пропускаем.")
        return

    print(f"[+] Запускаем C++ алгоритм для задачи {idx}...")
  
    # Запускаем алгоритм. Параметр -o теперь ведет в подпапку задачи
    cmd = ["../run.sh", "-s", "0.025", "-a", "5", "-i", "300", "-f", input_file, "-o", output_base, "-t", "15"]
    
    try:
        subprocess.run(cmd, check=True)
        print(f"    Готово! Все этапы решения сохранены в: {task_sol_dir}")
    except subprocess.CalledProcessError as e:
        print(f"[!] Ошибка при выполнении кода алгоритма {idx}:\n{e}")

def main():
    parser = argparse.ArgumentParser(description="Запуск алгоритма.")
    parser.add_argument("-a", "--all", action="store_true", help="Запустить для всех сгенерированных файлов")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="Список ID для запуска")
    args = parser.parse_args()

    if args.all:
        files = glob.glob("../SPB_problems/Generates_SPB_problems/problems/Generated_problems_*.json")
        ids = sorted([int(os.path.basename(f).replace("Generated_problems_", "").replace(".json", "")) for f in files])
    elif args.ids:
        ids = args.ids
    else:
        print("Используйте -a или -i <id>")
        return

    for idx in ids:
        run_solver(idx)

if __name__ == "__main__":
    main()