# this is run by platformio

import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
print("-DRAPIDIA_SRC_REV=\"%s\"" % revision)