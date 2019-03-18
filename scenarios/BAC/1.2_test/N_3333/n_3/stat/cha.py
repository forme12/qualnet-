import re
#FileName = input("the object file:")
#KeyStr = input("the key string:")
FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\BAC\\1.2_test\\N_3333\\n_3\\stat\\test0.stat"
KeyStr = '  28,               ,[1024], Application,  CBR Server,Total Unicast Fragments Received (fragments) = '
KeyStr1 = '  28,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
FoundFlag = False
#打开文件
FileObj = open(FileName)
#读取行信息
LineTemp = FileObj.readline()

while LineTemp:
    #判断是否包含关键字
    #不包含则循环操作，包好的话将航线是并退出程序
    if LineTemp.find(KeyStr) == 0:
        FoundFlag = True
        print(("############"))
        print(re.findall(r'\d+\.?\d*',LineTemp))
        print("*********************")
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()
if FoundFlag == False:
    print("Not found the string!")
input()
