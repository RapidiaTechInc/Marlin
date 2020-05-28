#
# Rapidia Firmware Export Script
# This will build current commit and copy the firmware hex and elf files to
# RapidiaHost directory, which is assumed to be ../RapidiaHost-V1/
#
# Use with caution as this will directly git commit and git push!!
# Make sure your RapidiaHost git directory is clean before doing this
#
import os
from os import walk
import shutil
Import("env", "projenv")

rapidia_export_dir = '.pio/build/rapidia_export'
rapidia_host_dir = '../RapidiaHost-V1' # This is assumed!

print "Running Rapidia Firmware Export Script"

def after_build(source, target, env):
	print "Starting after-build export process.."
	f = []
	has_hex = False
	# Check that firmware files exist
	for (dirpath, dirnames, filenames) in walk(rapidia_export_dir):
		for filename in filenames:
			if filename == "firmware.hex":
				has_hex = True
				print "firmware.hex found!"
		break

	if has_hex:
		# copy firmware files over to RapidiaHost
		hex_source_path = rapidia_export_dir + '/firmware.hex' 
		hex_destination_path = rapidia_host_dir + '/firmware/firmware.hex'
		shutil.copyfile(hex_source_path, hex_destination_path)
		print 'Copied {hex_source_path} to {hex_destination_path}'

		# change directory to rapidiahost
		os.chdir(rapidia_host_dir)
		print 'Changed dir to {rapidia_host_dir}'

		# update package.json with firmware version

		# git commit and push
		os.system('git pull')
		os.system('git add -A')
		os.system('git commit -m "update firmware"')
		os.system('git push')

		print "SUCCESS"
	else:
		print "FAIL: firmware.hex not found!"

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)