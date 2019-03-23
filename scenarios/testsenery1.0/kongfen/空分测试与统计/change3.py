import os
f = open ('kongfen1.0.app')
lines = f.readlines()
f.close()
for line in lines:
    if(line.find('CBR 55 76 1000 512 10 ') == 0):
        line = 'CBR 55 76 1000 512 10 %s' % ('30MS 1S 50S Unicast 10S PRECEDENCE 0 ',) + '\n'
    if(line.find('CBR 54 80 1000 512 10 ') == 0):
        line = 'CBR 54 80 1000 512 10 %s' % ('30MS 1S 50S Unicast 10S PRECEDENCE 0 ',) + '\n'
    if(line.find('CBR 30 1 1000 512 10 ') == 0):
        line = 'CBR 30 1 1000 512 10 %s' % ('30MS 1S 50S Unicast 10S PRECEDENCE 0 ',) + '\n'
    if(line.find('CBR 34 9 1000 512 10 ') == 0):
        line = 'CBR 34 9 1000 512 10 %s' % ('30MS 1S 50S Unicast 10S PRECEDENCE 0 ',) + '\n'
    #rs = line.rstrip('\n')
    #newname=rs.replace(rs,'CBR 55 76 1000 512 10 40MS 1S 50S Unicast 10S PRECEDENCE 0')
    newfile = open('t1.app','a')
    newfile.write(line)
    newfile.close()
os.unlink('kongfen1.0.app')
os.rename('t1.app','kongfen1.0.app')