data = ''

with open('N_3Unicast.app', 'r+') as f:
    for line in f.readlines():
        if(line.find('CBR 31 28 500 512 10 ') == 0):
            line = 'CBR 31 28 500 512 10 %s' % ('10MS 1S 50S Unicast 10S PRECEDENCE 0 ',) + '\n'

        data += line

with open('N_3Unicast.app', 'r+') as f:
    f.writelines(data)
