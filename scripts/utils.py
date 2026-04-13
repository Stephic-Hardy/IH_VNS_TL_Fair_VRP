import os
import re

def get_next_index(folder, prefix):
    files = os.listdir(folder)
    nums = []

    for f in files:
        match = re.match(rf"{prefix}_(\d+)\.json", f)
        if match:
            nums.append(int(match.group(1)))

    return max(nums, default=0) + 1


def parse_indices(indices_str):
    result = set()

    for part in indices_str.split(','):
        if '-' in part:
            l, r = map(int, part.split('-'))
            result.update(range(l, r + 1))
        else:
            result.add(int(part))

    return sorted(result)