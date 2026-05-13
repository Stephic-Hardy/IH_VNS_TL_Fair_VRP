#!/bin/bash


ST="0.025"
AON="7"
MAX_ITER="450"
TIME_LIMIT="30"
INPUT_JSON="vrp_problems/0.json"
OUTPUT_JSON="solution.json"


show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "TDTSP Solver Launcher"
    echo ""
    echo "Options:"
    echo "  -s VALUE    ST parameter (default: 0.05)"
    echo "  -a VALUE    AON parameter (default: 5)"
    echo "  -i VALUE    MAX_ITERATION_WITHOUT_IMPROVE (default: 150)"
    echo "  -t VALUE    TIME_LIMIT in seconds (default: 60)"
    echo "  -f FILE     Input JSON file path (default: vrp_problems/0.json)"
    echo "  -o FILE     Output JSON file path (default: solution.json)"
    echo "  -h          Show this help message"
    echo ""
    echo "Example:"
    echo "  $0 -s 0.01 -a 5 -i 150 -t 60 -f vrp_problems/0.json -o my_solution.json"
}


while getopts "s:a:i:t:f:o:h" opt; do
    case $opt in
        s)
            ST="$OPTARG"
            ;;
        a)
            AON="$OPTARG"
            ;;
        i)
            MAX_ITER="$OPTARG"
            ;;
        t)
            TIME_LIMIT="$OPTARG"
            ;;
        f)
            INPUT_JSON="$OPTARG"
            ;;
        o)
            OUTPUT_JSON="$OPTARG"
            ;;
        h)
            show_help
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            show_help
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            show_help
            exit 1
            ;;
    esac
done


if [ ! -f "$INPUT_JSON" ]; then
    echo "Error: Input JSON file '$INPUT_JSON' not found!" >&2
    echo "Current directory: $(pwd)" >&2
    exit 1
fi


if [ ! -f "../tdtsp_solver" ]; then
    echo "Error: Executable 'tdtsp_solver' not found!" >&2
    echo "Please build the project first with:" >&2
    echo "  mkdir build && cd build && cmake .. && make" >&2
    exit 1
fi


echo "=== TDTSP Solver ==="
echo "ST: $ST"
echo "AON: $AON"
echo "MAX_ITERATION_WITHOUT_IMPROVE: $MAX_ITER"
echo "TIME_LIMIT: $TIME_LIMIT"
echo "INPUT_JSON: $INPUT_JSON"
echo "OUTPUT_JSON: $OUTPUT_JSON"
echo "===================="

../tdtsp_solver "$ST" "$AON" "$MAX_ITER" "$TIME_LIMIT" "$INPUT_JSON" "$OUTPUT_JSON" --benchmark