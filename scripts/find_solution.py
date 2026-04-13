import os
import argparse
import subprocess
import glob

def run_solver(idx):
    prob_dir = "../SPB_problems/Generates_SPB_problems/problems"
    sol_dir = "../SPB_problems/Generated_SPB_solutions"
    os.makedirs(sol_dir, exist_ok=True)

    input_file = os.path.join(prob_dir, f"Generated_problems_{idx}.json")
    output_file = os.path.join(sol_dir, f"generated_solution_{idx}.json")

    if not os.path.exists(input_file):
        print(f"[-] Файл {input_file} не найден. Пропускаем.")
        return

    print(f"[+] Запускаем C++ алгоритм для задачи {idx}...")
  
    cmd = ["../run.sh", "-s", "0.025", "-a", "5", "-i", "300", "-f", input_file, "-o", output_file, "-t", "15"]
    
    try:
        subprocess.run(cmd, check=True)
        print(f"    Готово! Результат: {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"[!] Ошибка при выполнении кода алгоритма {idx}:\n{e}")

def main():
    parser = argparse.ArgumentParser(description="Запуск алгоритма.")
    parser.add_argument("-a", "--all", action="store_true", help="Запустить для всех сгенерированных файлов")
    parser.add_argument("-i", "--ids", type=int, nargs='+', help="Список ID для запуска (например: -i 1 3 5)")
    args = parser.parse_args()

    if args.all:
        files = glob.glob("SPB_problems/Generates_SPB_problems/Generated_problems_*.json")
        files = [f for f in files if not f.endswith('_coords.json')]
        ids = sorted([int(os.path.basename(f).replace("Generated_problems_", "").replace(".json", "")) for f in files])
    elif args.ids:
        ids = args.ids
    else:
        print("Пожалуйста, укажите --all или --ids. Используйте -h для справки.")
        return

    for idx in ids:
        run_solver(idx)

if __name__ == "__main__":
    main()
