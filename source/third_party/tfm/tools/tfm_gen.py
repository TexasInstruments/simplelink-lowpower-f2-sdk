#-------------------------------------------------------------------------------
# Copyright (c) 2019, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

import io, os, sys
from jinja2 import Environment, FileSystemLoader, select_autoescape

try:
    import yaml
except ImportError as e:
    print(e, "To install it, type:")
    print("pip install PyYAML")
    exit(1)

def main(argv):
    if (len(sys.argv) < 4) or (len(sys.argv) > 5):
        print("usage: tfm_gen.py <context file> <search path> <output file>")
        print("or:    tfm_gen.py <context file> <search path> <template file> <output file>")
        exit(1)

    ctx_file = argv[1]
    search_path = argv[2]
    tmplt_file = argv[3] + ".template"
    if len(sys.argv) == 5:
        out_file = argv[4]
    else:
        out_file = os.path.join(search_path, argv[3])

    env = Environment(
        loader = FileSystemLoader(search_path),
        autoescape = select_autoescape(['html', 'xml']),
        lstrip_blocks = True,
        trim_blocks = True,
        keep_trailing_newline = False
    )

    with open(ctx_file) as f:
        ctx = yaml.load(f, Loader=yaml.FullLoader)

    template = env.get_template(tmplt_file)

    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with io.open(out_file, "w", newline='\n') as f:
        f.write(template.render(ctx))

if __name__ == "__main__":
    main(sys.argv)
