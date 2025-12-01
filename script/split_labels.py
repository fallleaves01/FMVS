#!/usr/bin/env python3
import argparse
import json
import os


def split_labels(input_path: str,
                 output_path_1: str,
                 output_path_2: str,
                 x: int):
    """
    从一个 label json 文件中按顺序拆成两部分：

    假设原始内容是一个数组：
        [l0, l1, ..., l_{n-1}]
    拆分规则：
        文件1: [l0, l1, ..., l_{x-1}]
        文件2: [l_x, l_{x+1}, ..., l_{n-1}]

    要求：0 <= x <= n
    """

    if x < 0:
        raise ValueError("x 不能为负")

    if not os.path.exists(input_path):
        raise FileNotFoundError(f"找不到输入文件: {input_path}")

    with open(input_path, "r", encoding="utf-8") as fin:
        data = json.load(fin)

    if not isinstance(data, list):
        raise RuntimeError(
            f"输入 JSON 不是数组类型，而是 {type(data).__name__}，"
            "请确认 label 文件是一个一维数组。"
        )

    n = len(data)
    if x > n:
        raise RuntimeError(f"x={x} 超过 label 数量 n={n}")

    part1 = data[:x]
    part2 = data[x:]

    print(f"总标签数: {n}，将前 {x} 个写入 {output_path_1}，"
          f"剩余 {n - x} 个写入 {output_path_2}。")

    with open(output_path_1, "w", encoding="utf-8") as fout1:
        json.dump(part1, fout1, ensure_ascii=False)

    with open(output_path_2, "w", encoding="utf-8") as fout2:
        json.dump(part2, fout2, ensure_ascii=False)

    print("拆分完成。")


def main():
    parser = argparse.ArgumentParser(
        description="将一个 label JSON 数组按前 x / 后 n-x 拆分为两个文件。"
    )
    parser.add_argument("input", help="原始 label json 文件路径")
    parser.add_argument("output1", help="输出 label json 文件 1（前 x 个）")
    parser.add_argument("output2", help="输出 label json 文件 2（后 n-x 个）")
    parser.add_argument(
        "x",
        type=int,
        help="前一部分的标签数量 x（0 <= x <= n）"
    )

    args = parser.parse_args()
    split_labels(args.input, args.output1, args.output2, args.x)


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/split_labels.py \
/mnt/win-dai/liangsiyuan/FMVS/data/labels.json \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/labels.json \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/new_labels.json \
90000

python3 /home/liangsiyuan/FMVS/FMVS/script/split_labels.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/labels.json \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/insert/labels.json \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/insert/new_labels.json \
90000
'''