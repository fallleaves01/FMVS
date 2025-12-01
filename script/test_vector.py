#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
import struct


def inspect_fvecs(path: str, show: int = 5):
    file_size = os.path.getsize(path)
    if file_size < 4:
        raise RuntimeError("文件太小，连一个 dim 都读不出来：{}".format(path))

    with open(path, "rb") as f:
        # 先读第一个向量的 dim
        dim_bytes = f.read(4)
        if len(dim_bytes) != 4:
            raise RuntimeError("无法读取第一个向量的 dim")
        dim = struct.unpack("i", dim_bytes)[0]
        if dim <= 0:
            raise RuntimeError(f"非法 dim: {dim}")

        vec_bytes = 4 + dim * 4  # 每个向量占用字节数
        if file_size % vec_bytes != 0:
            print(
                f"警告：文件大小 {file_size} 不能被单向量大小 {vec_bytes} 整除，"
                "可能不是标准 fvecs 格式"
            )

        total_vecs = file_size // vec_bytes
        print(f"文件: {path}")
        print(f"维度 dim = {dim}")
        print(f"共有向量数 = {total_vecs}")

        # 回到开头，每次按 [dim][data] 读一条
        f.seek(0, os.SEEK_SET)

        k = min(show, total_vecs)
        print(f"\n前 {k} 个向量：")
        for idx in range(k):
            dim_bytes = f.read(4)
            if len(dim_bytes) != 4:
                print(f"在第 {idx} 个向量处无法读取 dim，提前结束")
                break
            d = struct.unpack("i", dim_bytes)[0]
            if d != dim:
                print(f"第 {idx} 个向量的 dim={d} 和首向量 dim={dim} 不一致")

            floats_bytes = f.read(dim * 4)
            if len(floats_bytes) != dim * 4:
                print(f"在第 {idx} 个向量处无法完整读取数据，提前结束")
                break
            vec = struct.unpack(f"{dim}f", floats_bytes)

            show_dim = min(8, dim)
            preview = ", ".join(f"{v:.4f}" for v in vec[:show_dim])
            if show_dim < dim:
                preview += ", ..."
            print(f"vec[{idx}] = [{preview}]")

        print("\n检查完毕。")


def main():
    parser = argparse.ArgumentParser(
        description="统计 fvecs 文件向量数量并打印前几条向量"
    )
    parser.add_argument("file", help="fvecs 文件路径")
    parser.add_argument(
        "--show",
        type=int,
        default=5,
        help="要打印的向量个数（默认 5）",
    )
    args = parser.parse_args()
    inspect_fvecs(args.file, args.show)


if __name__ == "__main__":
    main()


'''
python3 /home/liangsiyuan/FMVS/FMVS/script/test_vector.py \
/mnt/win-dai/liangsiyuan/FMVS/data/total_vectors.fvecs

python3 /home/liangsiyuan/FMVS/FMVS/script/test_vector.py \
/mnt/win-dai/liangsiyuan/FMVS/data/query_total.fvecs

python3 /home/liangsiyuan/FMVS/FMVS/script/test_vector.py \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/total_vectors.fvecs

python3 /home/liangsiyuan/FMVS/FMVS/script/test_vector.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/total_vectors.fvecs
'''