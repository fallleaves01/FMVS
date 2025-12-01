#!/usr/bin/env python3
import argparse
import json
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(
        description="多次执行查询程序，循环修改 beam_size"
    )
    parser.add_argument(
        "--config",
        default="query_config.json",
        help="配置文件路径（默认：query_config.json）",
    )
    parser.add_argument(
        "--exe",
        default="../../build/fmvs_query",
        help="C++ 可执行程序路径（默认：../../build/fmvs_query）",
    )
    parser.add_argument(
        "--beam-sizes",
        type=int,
        nargs="+",
        required=True,
        help="要尝试的一组 beam_size，例如：--beam-sizes 10 20 50 100",
    )
    args = parser.parse_args()

    config_path = Path(args.config)
    exe_path = Path(args.exe)

    if not config_path.exists():
        raise FileNotFoundError(f"找不到配置文件: {config_path}")

    if not exe_path.exists():
        raise FileNotFoundError(f"找不到可执行文件: {exe_path}")

    # 读取原始配置
    with open(config_path, "r", encoding="utf-8") as f:
        base_cfg = json.load(f)

    for b in args.beam_sizes:
        cfg = dict(base_cfg)
        cfg["beam_size"] = int(b)

        # 写回同一个配置文件（你的 C++ 程序直接读取这个文件）
        with open(config_path, "w", encoding="utf-8") as f:
            json.dump(cfg, f, ensure_ascii=False, indent=4)

        print(f"\n=== 正在运行 beam_size = {b} ===")
        # 在配置文件所在目录作为工作目录执行
        subprocess.run(
            [str(exe_path)],
            cwd=str(config_path.parent),
            check=True,
        )


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/run_query.py \
  --config /home/liangsiyuan/FMVS/FMVS/config/Openimage/delete/query_config.json \
  --exe /home/liangsiyuan/FMVS/FMVS/build/fmvs_query \
  --beam-sizes 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150
'''