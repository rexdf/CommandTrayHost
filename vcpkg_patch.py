# /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import codecs
import traceback

UTF16_GITCONFIG = '''[filter "utf16"]
    clean = iconv -f utf-16le -t utf-8
    smudge = iconv -f utf-8 -t utf-16le
    required
'''


def main():
    vcpkg_path = 'c:\\tools\\vcpkg\\installed'
    for folder in ('x64-windows', 'x86-windows'):
        nlohmann_json_path = os.path.join(
            vcpkg_path, folder, 'include\\nlohmann\\json.hpp')
        with open(nlohmann_json_path, "rb") as f:
            content = f.read().decode('utf-8')
        content = content.replace(
            'std::numeric_limits<size_t>::max(',
            '(std::numeric_limits<size_t>::max)('
        )
        for i in (64, 32, 16, 8):
            for fuc in ('max', 'min'):
                content = content.replace(f'std::numeric_limits<uint{i}_t>::{fuc}(',
                                          f'(std::numeric_limits<uint{i}_t>::{fuc})('
                                          )
        with open(nlohmann_json_path, "wb") as f:
            f.write(content.encode('utf-8'))

if __name__ == '__main__':
    print(sys.version_info)
    if (sys.version_info < (3, 0)):
        sys.exit(2)
    try:
        main()
    except:
        traceback.print_exc()
