#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
import struct


def subset_fvecs(input_path: str, output_path: str, num: int, offset: int = 0):
    """
    从 input_path 的 fvecs 文件中，抽取 [offset, offset+num) 这 num 个向量，
    按原始 fvecs 格式写到 output_path。
    fvecs 格式：每个向量 = int32(dim) + dim 个 float32
    """
    if num <= 0:
        raise ValueError("num 必须是正整数")
    if offset < 0:
        raise ValueError("offset 不能为负")

    file_size = os.path.getsize(input_path)
    if file_size < 4:
        raise RuntimeError("文件太小，连一个 dim 都读不出来：{}".format(input_path))

    with open(input_path, "rb") as fin:
        # 先读第一个向量的维度
        dim_bytes = fin.read(4)
        if len(dim_bytes) != 4:
            raise RuntimeError("无法读取第一个向量的 dim")
        dim = struct.unpack("i", dim_bytes)[0]
        if dim <= 0:
            raise RuntimeError("非法 dim: {}".format(dim))

        vec_bytes = 4 + dim * 4  # 一个向量占多少字节
        if file_size % vec_bytes != 0:
            print("警告：文件大小不能被 (4 + 4*dim) 整除，可能不是标准 fvecs 格式")

        total_vecs = file_size // vec_bytes
        if offset >= total_vecs:
            raise RuntimeError(
                f"offset={offset} 超过文件向量数 total={total_vecs}"
            )

        # 真正能抽的数量不能超过剩余的
        num = min(num, total_vecs - offset)
        print(f"文件共有 {total_vecs} 个向量，将从第 {offset} 个开始抽取 {num} 个。")

        # 跳到 offset 的位置（注意要包含 dim 那 4 个字节）
        fin.seek(offset * vec_bytes)

        with open(output_path, "wb") as fout:
            for i in range(num):
                buf = fin.read(vec_bytes)
                if len(buf) != vec_bytes:
                    print(f"在第 {i} 个向量处提前读完文件，实际写入 {i} 个向量。")
                    break
                fout.write(buf)

    print("写入完成：", output_path)


def main():
    parser = argparse.ArgumentParser(
        description="从 fvecs 文件中抽取部分向量到新文件"
    )
    parser.add_argument("input", help="原始 fvecs 文件路径")
    parser.add_argument("output", help="输出 fvecs 文件路径")
    parser.add_argument(
        "num", type=int, help="要抽取的向量个数"
    )
    parser.add_argument(
        "--offset",
        type=int,
        default=0,
        help="起始向量下标（默认从 0 开始）",
    )

    args = parser.parse_args()
    subset_fvecs(args.input, args.output, args.num, args.offset)


if __name__ == "__main__":
    main()

'''
输入格式：

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/yexdata/sift1M/sift1M_base.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/data/total_vectors.fvecs \
  200000

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/yexdata/sift1M/sift1M_base.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/data/query_total.fvecs \
  2000 \
  --offset 300000 

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/yexdata/sift1M/sift1M_base.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/data/test_delete/total_vectors.fvecs \
  220000

Openimage

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/base_img_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/base_e1.fvecs \
  100000

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/base_text_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/base_e2.fvecs \
  100000

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/base_img_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/base_e1.fvecs \
  110000

python3 /home/liangsiyuan/FMVS/FMVS/script/get_vector.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/base_text_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/base_e2.fvecs \
  110000

'''