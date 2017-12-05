# /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import codecs
import traceback
import itertools


def type_name_yeild():
    for itm in itertools.product(('uint', 'int'), (64, 32, 16, 8)):
        yield ''.join(itm) + '_t'
    for itm in ('float', 'double', 'size_t'):
        yield itm


def main():
    vcpkg_path = 'c:\\tools\\vcpkg\\installed'
    for folder in ('x64-windows', 'x86-windows'):
        for file_to_patch in ('include\\nlohmann\\json.hpp', 'include\\rapidjson\\document.h'):
            nlohmann_json_path = os.path.join(
                vcpkg_path, folder, file_to_patch)
            if os.path.isfile(nlohmann_json_path):
                with open(nlohmann_json_path, "rb") as f:
                    content = f.read().decode('utf-8')
                content2 = content

                for type_name in type_name_yeild():
                    for fuc in ('max', 'min'):
                        content2 = content2.replace(f'std::numeric_limits<{type_name}>::{fuc}(',
                                                    f'(std::numeric_limits<{type_name}>::{fuc})('
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
