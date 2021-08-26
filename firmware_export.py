#
# Rapidia Firmware Export Script
# This will build current commit and copy the firmware elf file to
# RapidiaHost directory, which is assumed to be ../rapidia-host-react-v1/
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

ELF = ".elf"
HEX = ".hex"
firmware_root_path = os.getcwd()
rapidia_export_path = path.join(firmware_root_path, '.pio/build/rapidia_export')
rapidia_host_path = path.join(firmware_root_path, '../rapidia-host-react-v1')
firmware_source_path = path.join(rapidia_export_path, 'firmware') 
firmware_destination_path = path.join(rapidia_host_path,'assets', 'firmware','firmware')
package_json_path = './src/package.json'

print("Running Rapidia Firmware Export Script")

if not(path.exists(rapidia_host_path)):
	raise Exception('../rapidia-host-react-v1 directory not found!')

def check_firmware_exists(build_path, file_extension):
	has_file = False
	# Check that firmware files exist
	for (dirpath, dirnames, filenames) in walk(build_path):
		for filename in filenames:
			if filename == "firmware" + file_extension:
				has_file = True
				print("firmware" + file_extension +" found.")
		break

	if not(has_file):
		raise Exception("FAIL: firmware" + file_extension + " not found!")

def copy_firmware_to_destination(source_path, destination_path, file_extension):
	shutil.copyfile(source_path + file_extension, destination_path + file_extension)
	print("Copied " + source_path + file_extension + " to " + destination_path + file_extension)

def after_build(source, target, env):
	print("Starting after-build export process..")
	f = []
	
	check_firmware_exists(rapidia_export_path, ELF)

	# Look for rapidia firmware version
	file = open('Marlin/src/inc/RapidiaVersion.h')
	firmware_version = 0
	marlin_version = ''
	rapidia_version = ''
	for line in file:
		if line.startswith('#define MARLIN_SHORT_BUILD_VERSION'):
			marlin_version = line.replace('#define MARLIN_SHORT_BUILD_VERSION','')
			marlin_version = marlin_version.replace(' ','')
			marlin_version = marlin_version.replace('"','')
		if line.startswith('#define RAPIDIA_SHORT_BUILD_VERSION'):
			rapidia_version = line.replace('#define RAPIDIA_SHORT_BUILD_VERSION','')
			rapidia_version = rapidia_version.replace(' ','')
			rapidia_version = rapidia_version.replace('"','')
	firmware_version = marlin_version.rstrip() + 'r' + rapidia_version.rstrip()
	print('Rapidia firmware version found: ' + firmware_version)

	if firmware_version == 0 or firmware_version == '' or firmware_version == 'r':
		raise Exception("ERROR: Rapidia Firmware Version not found")

	# change directory to rapidiahost
	os.chdir(rapidia_host_path)
	print("Changed dir to " + rapidia_host_path)

	# Check for unstaged/uncommitted changes
	git_status = os.popen('git status -s').read()
	if not(git_status == ''):
		raise Exception('ERROR: Uncommitted changes: ' + git_status)

	# git pull in case of conflicts
	os.system('git pull')

	copy_firmware_to_destination(firmware_source_path, firmware_destination_path, ELF)

	# update package.json with firmware version
	if not(path.exists(package_json_path)):
		raise Exception(package_json_path + ' not found!')
	package_json = open(package_json_path)
	package_data = json.load(package_json, object_pairs_hook=OrderedDict)
	package_data['packaged_firmware_version'] = firmware_version
	with open(package_json_path, 'w') as json_file:
		json.dump(package_data, json_file, indent=4)
		json_file.write('\n')

	print("SUCCESS")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_build)