#!/usr/bin/env python3
"""
源码收集程序
功能：递归收集 ./kernel/src 目录下所有 .c, .h, .asm, .s, .S 文件，
      合并生成一个 txt 文件，每个文件内容前标注文件路径。
"""

import os
import sys

# 配置参数
SOURCE_DIR = "./kernel/src"  # 源码根目录
OUTPUT_FILE = "merged_source_code.txt"  # 合并后的输出文件
ALLOWED_EXTENSIONS = {".c", ".h", ".asm", ".s", ".S"}


def collect_and_merge(src_dir, output_path):
    """
    遍历 src_dir，收集符合条件的文件，合并写入 output_path
    """
    if not os.path.isdir(src_dir):
        print(f"错误：目录 '{src_dir}' 不存在，请检查路径。")
        sys.exit(1)

    total_files = 0
    with open(output_path, "w", encoding="utf-8") as outfile:
        # 使用 os.walk 递归遍历目录
        for root, dirs, files in os.walk(src_dir):
            for file in files:
                # 检查文件扩展名
                ext = os.path.splitext(file)[1]
                if ext.lower() not in ALLOWED_EXTENSIONS:
                    continue

                file_path = os.path.join(root, file)
                # 计算相对于 src_dir 的路径（使输出更清晰）
                rel_path = os.path.relpath(file_path, start=src_dir)

                try:
                    with open(file_path, "r", encoding="utf-8") as infile:
                        content = infile.read()
                except Exception as e:
                    print(f"警告：无法读取文件 {file_path}，跳过。错误：{e}")
                    continue

                # 写入分隔符和文件路径信息
                outfile.write(f"\n{'=' * 80}\n")
                outfile.write(f"文件: {rel_path}\n")
                outfile.write(f"{'=' * 80}\n\n")
                outfile.write(content)
                outfile.write("\n")  # 文件末尾加一个换行，避免粘连
                total_files += 1

    print(f"合并完成！共收集 {total_files} 个文件，输出文件：{output_path}")


if __name__ == "__main__":
    collect_and_merge(SOURCE_DIR, OUTPUT_FILE)
