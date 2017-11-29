# /usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import codecs

MAJOR_VERSION = '0'
MINOR_VERSION = '6'
FIX_VERSION = '0'

'''
    git checkout HEAD -- CommandTrayHost/CommandTrayHost.rc CommandTrayHost/stdafx.h
'''


def main():
    global MAJOR_VERSION, MINOR_VERSION, FIX_VERSION
    print(sys.argv)
    if len(sys.argv) < 2:
        print("must input build version")
        return False
    argc = len(sys.argv)
    if(argc > 1):
        build_version_number = sys.argv[1]
    print("build_version_number: ", build_version_number)
    if(argc > 2):
        MAJOR_VERSION = sys.argv[2]
    if(argc > 3):
        MINOR_VERSION = sys.argv[3]
    if(argc > 4):
        FIX_VERSION = sys.argv[4]
    # \r\n will have trouble with $ to match end
    pattern_rc = re.compile(
        r'^VALUE "ProductVersion", "\d+, \d+, \d+, \d+[\\]0"', re.M)
    rc_string = r'VALUE "ProductVersion", "{}, {}, {}, {}\\0"'.format(
        MAJOR_VERSION, MINOR_VERSION, FIX_VERSION, build_version_number)  # .encode('utf-8')

    pattern_stdafx_h = re.compile(
        r'^#define VERSION_NUMS L"\d+[.]\d+[.][0-9b-]+"', re.M)
    stdafx_h_string = r'#define VERSION_NUMS L"{}.{}.{}-b{}"'.format(
        MAJOR_VERSION, MINOR_VERSION, FIX_VERSION, build_version_number)  # .encode('utf-16le')

    base_dir = os.path.dirname(os.path.abspath(__file__))
    rc_file = os.path.join(base_dir, "CommandTrayHost", "CommandTrayHost.rc")
    stdafx_h_file = os.path.join(base_dir, "CommandTrayHost", "stdafx.h")

    print(stdafx_h_string, '\n', rc_string)

    for file_name, pattern_re, replace_string, encoding in (
        (stdafx_h_file, pattern_stdafx_h, stdafx_h_string, 'utf-16le'),
        (rc_file, pattern_rc, rc_string, 'utf-8'),
    ):
        # CommandTrayHost.rc # stdafx.h
        try:
            with open(file_name, "rb") as f:
                content = f.read()
        except:
            return False
        print(len(content), end=" ")
        if encoding == 'utf-16le':
            bom = codecs.BOM_UTF16_LE
        else:
            bom = codecs.BOM_UTF8
        if content.startswith(bom):
            content = content[len(bom):]
        content = content.decode(encoding)

        if pattern_re.search(content) is None:
            return False
        print(len(content), end=" ")
        content = pattern_re.sub(replace_string, content)
        print(len(content), end=" ")
        if encoding == 'utf-16le':
            content = bom + content.encode(encoding)
        else:
            content = content.encode(encoding)
        print(len(content))
        with open(file_name, "wb") as f:
            f.write(content)
        print(file_name, "repalced!\n")

    return True


if __name__ == '__main__':
    print(f"""{__file__} <build_number> [MAJOR_VERSION] [MINOR_VERSION] [FIX_VERSION]
[MAJOR_VERSION] [MINOR_VERSION] [FIX_VERSION] are optional.""")
    if (sys.version_info < (3, 0)):
        sys.exit(2)
    if not main():
        sys.exit(1)
