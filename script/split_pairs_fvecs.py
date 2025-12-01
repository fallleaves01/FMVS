#!/usr/bin/env python3
import argparse
import os
import struct


def split_fvecs_pairs(input_path: str,
                      output_path_1: str,
                      output_path_2: str,
                      x: int):
    """
    输入：
      - input_path: 原始 fvecs 文件，内容为 2n 个向量，
                    前 n 个是 s 向量，后 n 个是 e 向量，i 和 i+n 是一对
      - output_path_1: 输出文件 1（前 x 对）
      - output_path_2: 输出文件 2（后 n-x 对）
      - x: 前面保留的 pair 数（0 <= x <= n）

    拆分规则：
      - 文件 1: s_0, s_1, ..., s_{x-1}, e_0, e_1, ..., e_{x-1}
      - 文件 2: s_x, s_{x+1}, ..., s_{n-1}, e_x, e_{x+1}, ..., e_{n-1}
    下标这里用 0-based，和代码一致。
    """
    if x < 0:
        raise ValueError("x 不能为负")

    file_size = os.path.getsize(input_path)
    if file_size < 4:
        raise RuntimeError(f"文件太小，无法读取 dim: {input_path}")

    with open(input_path, "rb") as fin:
        # 先读第一个向量的维度
        dim_bytes = fin.read(4)
        if len(dim_bytes) != 4:
            raise RuntimeError("无法读取第一个向量的 dim")
        dim = struct.unpack("i", dim_bytes)[0]
        if dim <= 0:
            raise RuntimeError(f"非法 dim: {dim}")

        vec_bytes = 4 + dim * 4  # 每个向量占用字节数
        if file_size % vec_bytes != 0:
            print(
                "警告：文件大小不能被 (4 + 4*dim) 整除，可能不是标准 fvecs 格式"
            )

        total_vecs = file_size // vec_bytes
        if total_vecs % 2 != 0:
            raise RuntimeError(
                f"向量总数 = {total_vecs} 不是偶数，无法解释为 (s,e) 对"
            )

        n = total_vecs // 2  # pair 数量
        if x > n:
            raise RuntimeError(f"x={x} 超过 pair 数 n={n}")

        print(
            f"文件共有 {total_vecs} 个向量，其中 pair 数量 n={n}，将拆为 "
            f"{2*x} 个向量写入 {output_path_1}，"
            f"{2*(n-x)} 个向量写入 {output_path_2}。"
        )

        # 回到文件开头，重新按向量粒度读取
        fin.seek(0)

        # 打开两个输出文件
        with open(output_path_1, "wb") as fout1, open(
            output_path_2, "wb"
        ) as fout2:
            # 先写前 n 个向量（s）
            for k in range(n):
                buf = fin.read(vec_bytes)
                if len(buf) != vec_bytes:
                    raise RuntimeError(
                        f"读取 s 向量时在 k={k} 处提前 EOF"
                    )
                if k < x:
                    fout1.write(buf)
                else:
                    fout2.write(buf)

            # 再写后 n 个向量（e）
            for k in range(n):
                buf = fin.read(vec_bytes)
                if len(buf) != vec_bytes:
                    raise RuntimeError(
                        f"读取 e 向量时在 k={k} 处提前 EOF"
                    )
                if k < x:
                    fout1.write(buf)
                else:
                    fout2.write(buf)

    print("拆分完成。")
    print("输出文件 1:", output_path_1)
    print("输出文件 2:", output_path_2)


def main():
    parser = argparse.ArgumentParser(
        description=(
            "将一个 2n 向量的 fvecs（前 n 为 s，后 n 为 e）拆成 "
            "2x 和 2(n-x) 个向量的两个 fvecs，保持 pair 对应关系。"
        )
    )
    parser.add_argument("input", help="原始 fvecs 文件路径")
    parser.add_argument("output1", help="输出 fvecs 文件 1（前 x 对）")
    parser.add_argument("output2", help="输出 fvecs 文件 2（后 n-x 对）")
    parser.add_argument(
        "x",
        type=int,
        help="要保留到第几个 pair（0-based, pair 数量为 n，0<=x<=n）",
    )

    args = parser.parse_args()
    split_fvecs_pairs(args.input, args.output1, args.output2, args.x)


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/split_pairs_fvecs.py \
/mnt/win-dai/liangsiyuan/FMVS/data/total_vectors.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/total_vectors.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/new_vectors.fvecs \
90000

python3 /home/liangsiyuan/FMVS/FMVS/script/split_pairs_fvecs.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/total_vectors.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/insert/total_vectors.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/insert/new_vectors.fvecs \
90000
'''