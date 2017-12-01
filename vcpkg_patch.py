# /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import codecs
import traceback


def main():
    vcpkg_path = 'c:\\tools\\vcpkg\\installed'
    for folder in ('x64-windows', 'x86-windows'):
        nlohmann_json_path = os.path.join(
            vcpkg_path, folder, 'include\\nlohmann\\json.hpp')
        if os.path.isfile(nlohmann_json_path):
            with open(nlohmann_json_path, "rb") as f:
                content = f.read().decode('utf-8')
            content2 = content.replace(
                'std::numeric_limits<size_t>::max(',
                '(std::numeric_limits<size_t>::max)('
            )
            for i in (64, 32, 16, 8):
                for fuc in ('max', 'min'):
                    for unt in ('uint', 'int'):
                        content2 = content2.replace(f'std::numeric_limits<{unt}{i}_t>::{fuc}(',
                                                    f'(std::numeric_limits<{unt}{i}_t>::{fuc})('
                                                   )
            with open(nlohmann_json_path, "wb") as f:
                f.write(content2.encode('utf-8'))

            print(nlohmann_json_path)
            print(
                f'{len(content)} --> {len(content2)}  diff/2: {(len(content2)-len(content))/2}')
        else:
            print(nlohmann_json_path, " not exist!")


if __name__ == '__main__':
    print(sys.version_info)
    if sys.version_info < (3, 0):
        sys.exit(2)
    try:
        main()
    except:
        traceback.print_exc()
