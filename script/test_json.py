#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import json
from typing import Any


def preview_array(arr, max_items=20):
    """返回数组前 max_items 个元素的字符串表示。"""
    k = min(max_items, len(arr))
    head = arr[:k]
    return json.dumps(head, ensure_ascii=False)


def walk(obj: Any, path: str = "root"):
    """递归遍历 JSON，遇到 list 就打印信息。"""
    if isinstance(obj, list):
        print(f"[ARRAY] path = {path}")
        print(f"  length = {len(obj)}")
        print(f"  first elements (up to 5) = {preview_array(obj)}")
        print()
        # 继续往里看，有没有嵌套数组/对象
        for idx, item in enumerate(obj):
            sub_path = f"{path}[{idx}]"
            if isinstance(item, (list, dict)):
                walk(item, sub_path)
    elif isinstance(obj, dict):
        for key, value in obj.items():
            sub_path = f"{path}.{key}"
            if isinstance(value, (list, dict)):
                walk(value, sub_path)
    # 其它类型就不处理了（int/str/...）


def main():
    parser = argparse.ArgumentParser(
        description="查看 JSON 文件中各数组的大小与前几个元素"
    )
    parser.add_argument("file", help="要查看的 JSON 文件路径")
    args = parser.parse_args()

    with open(args.file, "r", encoding="utf-8") as f:
        data = json.load(f)

    print(f"Loaded JSON from: {args.file}")
    print(f"Top-level type: {type(data).__name__}")
    print()

    walk(data)


if __name__ == "__main__":
    main()

'''
python3 /home/liangsiyuan/FMVS/FMVS/script/test_json.py \
/mnt/win-dai/liangsiyuan/FMVS/data/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/test_json.py \
/mnt/win-dai/liangsiyuan/FMVS/data/test_insert/labels.json

python3 /home/liangsiyuan/FMVS/FMVS/script/test_json.py \
/mnt/win-dai/liangsiyuan/FMVS/data/test_delete/delete_file.json

python3 /home/liangsiyuan/FMVS/FMVS/script/test_json.py \
/mnt/win-dai/liangsiyuan/FMVS/data/test_delete/headstone.json

python3 /home/liangsiyuan/FMVS/FMVS/script/test_json.py \
/mnt/win-dai/liangsiyuan/FMVS/dataset/Openimage/data/insert/labels.json
'''