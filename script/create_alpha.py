#!/usr/bin/env python3
import argparse
import json
import random


def gen_floats(n: int):
    """
    生成 n 个 [0,1) 之间的随机浮点数。
    """
    if n <= 0:
        raise ValueError("n 必须是正整数")
    return [random.random() for _ in range(n)]  # uniform[0,1)


def main():
    parser = argparse.ArgumentParser(
        description="生成 n 个 [0,1) 的随机浮点数，并写入指定 JSON 文件"
    )
    parser.add_argument("n", type=int, help="生成的浮点数个数")
    parser.add_argument("output", help="输出 JSON 文件路径")
    args = parser.parse_args()

    nums = gen_floats(args.n)

    with open(args.output, "w", encoding="utf-8") as f:
        # 输出为 JSON 数组，例如 [0.123, 0.987, ...]
        json.dump(nums, f, ensure_ascii=False)

    print(f"生成了 {len(nums)} 个随机数，已写入 {args.output}")


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/create_alpha.py \
1000 \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/alpha.json

'''