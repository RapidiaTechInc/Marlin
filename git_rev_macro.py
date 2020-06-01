import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
print("-fmax-errors=5 -g -D__MARLIN_FIRMWARE__ -fmerge-all-constants -DRAPIDIA_SRC_REV=\"%s\"" % revision)