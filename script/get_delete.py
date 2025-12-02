#!/usr/bin/env python3
import argparse
import json
import random


def gen_unique_numbers(n: int, x: int):
    """
    在 [1, n] 范围内随机生成 x 个互不相同的整数。
    返回一个列表。
    """
    if x > n:
        raise ValueError("x 不能大于 n，不然没法保证互不相同")
    if x < 0 or n <= 0:
        raise ValueError("n 必须 > 0 且 x 不能为负")

    return random.sample(range(1, n + 1), x)


def main():
    parser = argparse.ArgumentParser(
        description="在 [1, n] 中随机生成 x 个互不相同的整数，并输出为 JSON 文件"
    )
    parser.add_argument("n", type=int, help="上界 n（生成范围为 1..n）")
    parser.add_argument("x", type=int, help="要生成的整数个数 x（互不相同）")
    parser.add_argument("output", help="输出 JSON 文件路径")
    args = parser.parse_args()

    nums = gen_unique_numbers(args.n, args.x)

    # 输出成 JSON 数组，例如 [3, 7, 10, ...]
    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(nums, f, ensure_ascii=False)

    print(f"生成了 {len(nums)} 个数，已写入 {args.output}")


if __name__ == "__main__":
    main()


'''
python3 /home/liangsiyuan/FMVS/FMVS/script/get_delete.py \
  110000 \
  10000 \
  /mnt/win-dai/liangsiyuan/FMVS/data/test_delete/delete_file.json 

python3 /home/liangsiyuan/FMVS/FMVS/script/get_delete.py \
  110000 \
  10000 \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/delete_file.json

python3 /home/liangsiyuan/FMVS/FMVS/script/get_delete.py \
  109999 \
  10000 \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/delete/delete_file.json
'''