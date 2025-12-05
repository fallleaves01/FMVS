#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import json
import random


def main():
    parser = argparse.ArgumentParser(
        description="生成打乱的 1..n 并保存到指定路径（JSON 数组）"
    )
    parser.add_argument("n", type=int, help="最大整数 n（生成 1..n）")
    parser.add_argument("output", help="输出文件路径，例如 /mnt/.../labels.json")
    parser.add_argument(
        "--seed",
        type=int,
        default=None,
        help="随机种子（可选，指定的话结果可复现）",
    )
    args = parser.parse_args()

    n = args.n
    if n <= 0:
        raise ValueError("n 必须为正整数")

    if args.seed is not None:
        random.seed(args.seed)

    # 生成 1..n
    arr = list(range(1, n + 1))
    # 随机打乱
    random.shuffle(arr)

    # 写成 JSON 数组
    with open(args.output, "w") as f:
        json.dump(arr, f)

    print(f"已生成打乱的 1..{n}，保存到: {args.output}")


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/gen_labels.py \
100000 \
/mnt/win-dai/liangsiyuan/FMVS/data/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/gen_labels.py \
110000 \
/mnt/win-dai/liangsiyuan/FMVS/data/test_delete/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/gen_labels.py \
100000 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/gen_labels.py \
110000 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/gen_labels.py \
507444 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/labels.json

'''