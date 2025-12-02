import argparse
import os
import struct
import sys


def main():
    parser = argparse.ArgumentParser(
        description="Split an fvecs file into two parts by number of vectors: first x, then n-x."
    )
    parser.add_argument("input", help="input fvecs file")
    parser.add_argument("out_first", help="output fvecs file for the first x vectors")
    parser.add_argument("out_rest", help="output fvecs file for the remaining n-x vectors")
    parser.add_argument("x", type=int, help="number of vectors for the first part")
    args = parser.parse_args()

    in_path = args.input
    out1_path = args.out_first
    out2_path = args.out_rest
    x = args.x

    # 先根据文件大小和维度算出 n
    try:
        file_size = os.path.getsize(in_path)
    except OSError as e:
        print(f"Error: cannot stat input file {in_path}: {e}", file=sys.stderr)
        sys.exit(1)

    with open(in_path, "rb") as fin:
        # 读取第一个向量的维度（int32）
        dim_bytes = fin.read(4)
        if len(dim_bytes) != 4:
            print("Error: input file is too small or empty.", file=sys.stderr)
            sys.exit(1)

        dim = struct.unpack("i", dim_bytes)[0]
        if dim <= 0:
            print(f"Error: invalid dimension {dim}", file=sys.stderr)
            sys.exit(1)

        bytes_per_vec = 4 + 4 * dim  # 每个向量：4字节维度 + dim * 4字节float32
        if file_size % bytes_per_vec != 0:
            print(
                f"Error: file size {file_size} is not a multiple of per-vector size {bytes_per_vec}.",
                file=sys.stderr,
            )
            sys.exit(1)

        n = file_size // bytes_per_vec
        print(f"[info] dim = {dim}, n = {n}, bytes_per_vec = {bytes_per_vec}")

        if not (0 < x < n):
            print(f"Error: x must be in (0, {n}), but got {x}", file=sys.stderr)
            sys.exit(1)

        # 回到文件开头，按块复制
        fin.seek(0)

        with open(out1_path, "wb") as fout1, open(out2_path, "wb") as fout2:
            for idx in range(n):
                chunk = fin.read(bytes_per_vec)
                if len(chunk) != bytes_per_vec:
                    print(
                        f"Error: unexpected EOF at vector {idx}, read {len(chunk)} bytes.",
                        file=sys.stderr,
                    )
                    sys.exit(1)

                if idx < x:
                    fout1.write(chunk)
                else:
                    fout2.write(chunk)

    print(f"[info] Wrote first {x} vectors to {out1_path}")
    print(f"[info] Wrote remaining {n - x} vectors to {out2_path}")
    print("[info] Done.")


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/split_fvecs.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/total_vectors_base.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/insert/total_vectors_base.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/insert/new_vectors_base.fvecs \
90000

python3 /home/liangsiyuan/FMVS/FMVS/script/split_fvecs.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/total_vectors_addition.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/insert/total_vectors_addition.fvecs \
/mnt/win-dai/liangsiyuan/FMVS/dataset/CC3M/data/insert/new_vectors_addition.fvecs \
90000
'''