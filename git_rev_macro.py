# this is run by platformio

import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()

# account for python version differences changing how this string is produced.
revision = str(revision);
if revision.startswith("b'"):
    revision = revision[2:]
if revision.endswith("'"):
    revision = revision[:-1]

# stdout is read by platformio.
print("-DRAPIDIA_SRC_REV=\"%s\"" % revision)