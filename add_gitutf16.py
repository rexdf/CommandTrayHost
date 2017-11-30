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
    git_config_path = os.path.expandvars(r'%USERPROFILE%\.gitconfig')
    if os.path.isfile(git_config_path):
        with open(git_config_path, "rb") as f:
            content = f.read().decode('utf-8').replace('\r\n', '\n')
    else:
        print(git_config_path, "not exist")
        content = ''
    if UTF16_GITCONFIG not in content:
        print(f"No UTF16_GITCONFIG in {git_config_path}")
        content = content + '\n' + UTF16_GITCONFIG if content else UTF16_GITCONFIG
        content = content.replace('\n', '\r\n').encode('utf-8')
        with open(git_config_path, "wb") as f:
            f.write(content)
        print("changed .gitconfig\n", content.decode('utf-8'))
    else:
        print('.gitconfig already includes [filter "utf16"]')

if __name__ == '__main__':
    print(sys.version_info)
    if (sys.version_info < (3, 0)):
        sys.exit(2)
    main()
