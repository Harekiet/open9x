#!/usr/bin/env python
# -*- coding: cp1252 -*-

# Imports
import argparse
import os
import os.path

translations = {'se': [('\\200', '�'),
                       ('\\201', '�'),
                       ('\\202', '�'),
                       ('\\203', '�'),
                       ('\\204', '�'),
                       ('\\205', '�')],
                
#                'fr': [('\\200', '�'),
#                       ('\\201', '�'),
#                       ('\\202', '�'),
#                       ('\\203', '�'),
#                       ('\\204', '�'),
#                       ('\\205', '�'),
#                       ('\\206', '�'),
#                       ('\\207', '�'),
#                       ('\\208', '�'),
#                       ('\\209', '�')],
               }

# Take care of command line options
parser = argparse.ArgumentParser(description='Decoder/Encoder for open9x translations')
parser.add_argument('input', action="store")
parser.add_argument('language', action="store")
args =  parser.parse_args()
if args.language not in translations:
    parser.error(args.language  + ' is not a supported language. Try one of the supported ones: ' + str(translations.keys()))
    system.exit()
    
# Read the input file into a buffer
in_file = open( args.input, 'r')
text = in_file.read()
in_file.close()

# Do the replacements
for before, after in translations[args.language]:
    if args.input.endswith("." + args.language):
        text = text.replace(after, before)
    else:
        text = text.replace(before, after)
      
# Write the result to a temporary file
if args.input.endswith("." + args.language):
    out_file = open(args.input[:-3], 'w')
else:
    out_file = open(args.input + "." + args.language, 'w')
out_file.write( text )
out_file.close()


