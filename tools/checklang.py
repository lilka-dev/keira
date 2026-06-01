#!/usr/bin/python
##################################################################################
#                                                                                #
## This script to be used in workflows to check localization files consistency  ##
## across KeiraOS                                                               ##
#                                                                                #
##################################################################################
import os, re
from pathlib import Path

LANG_FILES_PATH = Path("src") / "keira" / "localizations"

LANG_FILE_PREFIX = "lang_"

LANG_FILE_MAIN = "uk"

LANG_FILE_RE = re.compile(r"^lang_[a-z]{2}\.h$")

LANG_FILE_MAIN_NAME = LANG_FILES_PATH  / f"{LANG_FILE_PREFIX}{LANG_FILE_MAIN}.h"

# Build a file list, skip non headers
lang_files_list = [p.name for p in LANG_FILES_PATH.glob("*.h")]

# Check main language file existence
if not(f"{LANG_FILE_PREFIX}{LANG_FILE_MAIN}.h" in lang_files_list):
	print(f"Error: Main language file for lang {LANG_FILE_MAIN_NAME} not found")
	exit(-1)

# Check filenames on a naming pattern
for lang_file in lang_files_list:
	if not(LANG_FILE_RE.match(lang_file)):
		print(f"Error: Language file {lang_file} have incorrect name")
		exit(-2)

# Routine to read lang file and transfrom it in a list of definitions
def lang_file_read(lfile):
	definitions = []
	with open(lfile) as lmain:
		for line in lmain:
			if(line.startswith("#define")):
				parsed_line = line.split()
				# extract tokens
				definitions.append(parsed_line[1])
	return definitions

	

# Build definitions list
token_list = lang_file_read(LANG_FILE_MAIN_NAME)

token_set = set(token_list)

error = 0

for lang_file in lang_files_list:
	# skip main file
	if lang_file.endswith(f"{LANG_FILE_PREFIX}{LANG_FILE_MAIN}.h"):
		continue
	set_cur = set(lang_file_read(LANG_FILES_PATH / lang_file))
	set_diff = token_set-set_cur
	if len(set_diff) > 0:
		print("=====================================================")
		print("Error: Not found tokens in {} ".format(lang_file))
		print("=====================================================")
		error = -3
	for val in set_diff:
		print("Missing {}".format(val));
	print("=====================================================")
	print("Checked {}".format(lang_file))
	print("=====================================================")

exit(error)
