#!/usr/bin/env python3
import argparse
import json


def main():
    parser = argparse.ArgumentParser(
        description="Generate a headstone json: valid = [1,...,1], invalid_count = 0."
    )
    parser.add_argument("output", help="output json file path")
    parser.add_argument("n", type=int, help="number of entries in valid array")
    args = parser.parse_args()

    if args.n < 0:
        raise ValueError("n must be non-negative")

    data = {
        "valid": [1] * args.n,       # 相当于 std::vector<uint8_t>(n, 1)
        "invalid_count": 0           # 对应 invalid_count = 0
    }

    # 和 nlohmann::json::dump() 类似的紧凑格式（不加多余空格）
    with open(args.output, "w") as f:
        json.dump(data, f, separators=(",", ":"))

    print(f"written {args.output} with n={args.n}")


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/create_headstone.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/delete/headstone.json \
110000

'''