#!/usr/bin/env python

import sys

filename = sys.argv[1]
print filename
fr = file(filename)
fw = file(filename+".new", "w")
ew = file(filename+".en", "w")
hw = file(filename+".stringsh", "w") 
cw = file(filename+".stringsc", "w")

replacements = {} 

for line in fr.readlines():
  pos_pstr = line.find("PSTR(\"")
  MENU = False
  if pos_pstr < 0:
    pos_pstr = line.find("MENU(\"")
    MENU = True
  while pos_pstr >= 0:
    # print line,
    if MENU:
      pos_endparenthesis = line.find('"', pos_pstr+7)
    else:
      pos_endparenthesis = line.find("\")", pos_pstr)
    str = line[pos_pstr+6:pos_endparenthesis]

    str_rep = "STR_"+str.upper()
    for s in (" ", ".", "[", "]", "-", "!", "/", ")", "(", "%", "+", ":"):
      str_rep = str_rep.replace(s, "")

    if "14" in str or "32" in str or "@" in str or len(str_rep) <= 5:
      pos_pstr = -1
    else:
      if MENU:
        glob_str = line[pos_pstr+5:pos_endparenthesis+1]
        str_rep = "STR_MENU"+str.upper()
      else:
        glob_str = line[pos_pstr:pos_endparenthesis+2]
        str_rep = "STR_"+str.upper()
      for s in (" ", ".", "[", "]", "-", "!", "/", ")", "(", "%", "+", ":"):
        str_rep = str_rep.replace(s, "")

      line = line.replace(glob_str, str_rep)

      if str_rep in replacements.keys():
        if replacements[str_rep] != str:
          print "!!!!! NON !!!!!"
      else: 
        replacements[str_rep] = str
        print glob_str, "=>", str, str_rep
        ew.write("#define " + str_rep[1:] + " "*(17-len(str_rep)) + '"%s"\n' % str)
        hw.write("extern const PROGMEM char %s[];\n" % str_rep)
        cw.write("const prog_char APM %s[] = %s;\n" % (str_rep, str_rep[1:]))
 
      pos_pstr = line.find("PSTR(\"")

  fw.write(line) 

fw.close()
ew.close()
hw.close()
cw.close()

