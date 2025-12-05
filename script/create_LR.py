#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import random
import json


def main():
    parser = argparse.ArgumentParser(
        description="随机生成 x 个区间，每个区间端点在 [1, n] 内，并写入 JSON 文件"
    )
    parser.add_argument("n", type=int, help="端点最大值 n（区间在 [1, n] 内）")
    parser.add_argument("x", type=int, help="要生成的区间个数 x")
    parser.add_argument("output", type=str, help="输出 JSON 文件路径")
    parser.add_argument(
        "--seed", type=int, default=None,
        help="随机种子（可选，指定后结果可复现）"
    )

    args = parser.parse_args()

    n = args.n
    x = args.x
    out_path = args.output

    if n <= 0 or x <= 0:
        raise ValueError("n 和 x 必须是正整数")

    if args.seed is not None:
        random.seed(args.seed)

    intervals = []
    for _ in range(x):
        a = random.randint(1, n)
        b = random.randint(1, n)
        L = min(a, b)
        R = max(a, b)
        intervals.append([L, R])  # 用 [L,R] 形式，兼容 nlohmann::json → pair/array

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(intervals, f, ensure_ascii=False, indent=2)

    print(f"已生成 {x} 个区间，端点范围 [1, {n}]，保存到：{out_path}")


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/create_LR.py \
99999 \
1000 \
/mnt/win-dai/liangsiyuan/FMVS/data/query_label.json

python3 /home/liangsiyuan/FMVS/FMVS/script/create_LR.py \
99999 \
1000 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/query_label.json

python3 /home/liangsiyuan/FMVS/FMVS/script/create_LR.py \
500000 \
1000 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/query_label.json
'''