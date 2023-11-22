# Script to update main.cpp

import io
import os


s1 = "/* USER CODE BEGIN Includes */"
a1 = "#include \"mainProgram.h\"\n"
s2 = "/* USER CODE BEGIN WHILE */"
a2 = "  setup();\n"
s3 = "/* USER CODE BEGIN 3 */"
a3 = "    loop();\n"

file = open("main.c", "r")
s = file.read()
file.close()
i = s.index(s1)+len(s1)+2
s = s[:i] + a1 + s[i:]
i = s.index(s2)+len(s2)+2
s = s[:i] + a2 + s[i:]
i = s.index(s3)+len(s3)+2
s = s[:i] + a3 + s[i:]

# delete main.c
os.remove("main.c")

file = open("main.cpp", "w")
file.write(s)
file.close()


os.system("PAUSE")







