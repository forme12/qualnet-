import re
import sys
import os
import os.path

path = input('请输入文件路径（结尾加上/）：')
f = os.listdir(path)
#FileName = input("the object file:")
#KeyStr = input("the key string:")
#FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\BAC\\1.2_test\\N_3333\\n_3\\stat\\test0.stat"
KeyStr = '  28,               ,[1024], Application,  CBR Server,Unicast Received Throughput (bits/second) = '
FoundFlag = False
#打开文件
FileObj = open(FileName)
#读取行信息
LineTemp = FileObj.readline()
data = 0
while LineTemp:
    #判断是否包含关键字
    #不包含则循环操作，包好的话将航线是并退出程序
    if LineTemp.find(KeyStr) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        
        class Logger(object):
            def __init__(self, fileN="Default.log"):
                self.terminal = sys.stdout
                self.log = open(fileN, "a")
            def write(self, message):
                self.terminal.write(message)
                self.log.write(message)

            def flush(self):
                pass
        sys.stdout = Logger("D:\\12.txt")  # 保存到D盘
        print(data)
       
        
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()
if FoundFlag == False:
    print("Not found the string!")
input()
