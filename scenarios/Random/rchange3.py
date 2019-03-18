data = ''

with open('Random.config', 'r+') as f:
    for line in f.readlines():
        if(line.find('[ N8-169.0.0.0 ] PHY-ABSTRACT-TX-POWER') == 0):
            line = '[ N8-169.0.0.0 ] PHY-ABSTRACT-TX-POWER%s' % (' 33 ',) + '\n'

        data += line

with open('Random.config', 'r+') as f:
    f.writelines(data)
