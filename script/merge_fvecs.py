#!/usr/bin/env python3
import argparse
import os
import struct
import shutil
import tempfile


def check_fvecs_dim(path: str) -> int:
    """读取 fvecs 文件的第一个向量维度，如果文件为空则报错。"""
    size = os.path.getsize(path)
    if size < 4:
        raise RuntimeError(f"{path} 太小，连一个 dim 都没有")
    with open(path, "rb") as f:
        dim_bytes = f.read(4)
        if len(dim_bytes) != 4:
            raise RuntimeError(f"{path} 读取 dim 失败")
        dim = struct.unpack("i", dim_bytes)[0]
        if dim <= 0:
            raise RuntimeError(f"{path} 里 dim={dim} 非法")
    return dim


def concat_fvecs(first: str, second: str, output: str):
    """
    将 second.fvecs 的内容接在 first.fvecs 后面，写到 output.fvecs 里。
    要求两个文件的 dim 一样。
    """
    if not os.path.exists(first):
        raise FileNotFoundError(f"找不到第一个 fvecs 文件: {first}")
    if not os.path.exists(second):
        raise FileNotFoundError(f"找不到第二个 fvecs 文件: {second}")

    dim1 = check_fvecs_dim(first)
    dim2 = check_fvecs_dim(second)
    if dim1 != dim2:
        raise RuntimeError(f"两个 fvecs 维度不一致: {first} dim={dim1}, {second} dim={dim2}")

    # 为了支持 output == first 的情况，使用临时文件再覆盖
    tmp_fd, tmp_path = tempfile.mkstemp(suffix=".fvecs")
    os.close(tmp_fd)

    try:
        with open(tmp_path, "wb") as fout:
            # 先写第一个文件
            with open(first, "rb") as f1:
                while True:
                    chunk = f1.read(1 << 20)  # 1MB 一块
                    if not chunk:
                        break
                    fout.write(chunk)
            # 再写第二个文件
            with open(second, "rb") as f2:
                while True:
                    chunk = f2.read(1 << 20)
                    if not chunk:
                        break
                    fout.write(chunk)

        # 写完后移动到目标路径
        shutil.move(tmp_path, output)
        print(f"已将 {first} 和 {second} 连接到 {output}")
    finally:
        # 如果中途报错，清理临时文件
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except OSError:
                pass


def main():
    parser = argparse.ArgumentParser(
        description="将 second.fvecs 接到 first.fvecs 后面，生成新的 fvecs 文件"
    )
    parser.add_argument("first", help="第一个 fvecs 文件（在前）")
    parser.add_argument("second", help="第二个 fvecs 文件（接在后面）")
    parser.add_argument("output", help="输出 fvecs 文件路径")
    args = parser.parse_args()

    concat_fvecs(args.first, args.second, args.output)


if __name__ == "__main__":
    main()

'''

python3 /home/liangsiyuan/FMVS/FMVS/script/merge_fvecs.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/base_e1.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/base_e2.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/delete/total_vectors.fvecs

python3 /home/liangsiyuan/FMVS/FMVS/script/merge_fvecs.py \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/query_img_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data_src/query_text_emb.fvecs \
  /mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/query_total.fvecs

'''