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
from os import path
import re
import sys
import json
import shutil
from collections import OrderedDict
Import("env", "projenv")

firmware_root_path = os.getcwd()
rapidia_export_path = path.join(firmware_root_path, '.pioenvs/rapidia_export')
rapidia_host_path = path.join(firmware_root_path, '../RapidiaHost-V1')
hex_source_path = path.join(rapidia_export_path, 'firmware.hex') 
hex_destination_path = path.join(rapidia_host_path, 'firmware','firmwareV1.hex')
package_json_path = './package.json'

print("Running Rapidia Firmware Export Script")

if not(path.exists(rapidia_host_path)):
	raise Exception('../RapidiaHost-V1 directory not found!')

def after_build(source, target, env):
	print("Starting after-build export process..")
	f = []
	has_hex = False
	# Check that firmware files exist
	for (dirpath, dirnames, filenames) in walk(rapidia_export_path):
		for filename in filenames:
			if filename == "firmware.hex":
				has_hex = True
				print("firmware.hex found.")
		break

	if not(has_hex):
		raise Exception("FAIL: firmware.hex not found!")

	# Look for rapidia firmware version
	file = open('Marlin/src/inc/RapidiaVersion.h')
	firmware_version = "1.0.0"

	# change directory to rapidiahost
	os.chdir(rapidia_host_path)
	print("Changed dir to " + rapidia_host_path)

	# Check for unstaged/uncommitted changes
	git_status = os.popen('git status -s').read()
	if not(git_status == ''):
		raise Exception('ERROR: Uncommitted changes: ' + git_status)

	# git pull in case of conflicts
	os.system('git pull')

	# copy firmware files over to RapidiaHost
	shutil.copyfile(hex_source_path, hex_destination_path)
	print("Copied " + hex_source_path + " to " + hex_destination_path)

	# update package.json with firmware version
	if not(path.exists(package_json_path)):
		raise Exception('package.json not found!')
	package_json = open(package_json_path)
	package_data = json.load(package_json, object_pairs_hook=OrderedDict)
	package_data['packaged_firmware_version'] = firmware_version
	with open(package_json_path, 'w') as json_file:
		json.dump(package_data, json_file, indent=4)
		json_file.write('\n')

	# git commit and push
	# os.system('git add -A')
	# os.system('git commit -m "update firmware"')
	# os.system('git push origin master')

	print("SUCCESS")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)