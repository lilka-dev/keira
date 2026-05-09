#!/usr/bin/python
##################################################################################
#                                                                                #
## This script to be used in workflows to check localization files consistency  ##
## across KeiraOS                                                               ##
#                                                                                #
##################################################################################
import os

lang_files_path = "src/keira/localizations"

lang_file_prefix = "lang_"

lang_main = "uk"

lang_main_filename = lang_files_path + "/" + lang_file_prefix + lang_main + ".h"

lang_files_list = []




# Build a file list, skip non headers
for filename in os.listdir(lang_files_path):
	if filename.endswith(".h"):
		lang_files_list.append(filename)

# Check main language file existence
if not((lang_file_prefix + lang_main+ ".h") in lang_files_list):
	print("Error: Main language file for lang {} not found".format(lang_main))
	exit(-1)

# Check filenames on a naming pattern
for lang_file in lang_files_list:
	if not(lang_file.startswith(lang_file_prefix) or (len(lang_file) == (len(lang_main) + len(lang_file_prefix) +2) )):
		print("Error: Language file {} have incorrect name".format(lang_file))
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
token_list = lang_file_read(lang_main_filename)

token_set = set(token_list)

error = 0

for lang_file in lang_files_list:
	# skip main file
	if lang_file.endswith(lang_file_prefix+lang_main+".h"):
		continue
	set_cur = set(lang_file_read(lang_files_path+ "/" +lang_file))
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
