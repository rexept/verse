#!/usr/bin/env python3
import sys
from better_profanity import profanity

profanity.load_censor_words()

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file) as f, open(output_file, "w") as out:
    for line in f:
        out.write("1\n" if profanity.contains_profanity(line) else "0\n")
