import re
FileName = "5ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()



FileName = "10ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()




FileName = "20ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()




FileName = "30ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()




FileName = "40ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()




FileName = "60ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()



FileName = "80ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()



FileName = "100ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()



FileName = "150ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()



FileName = "200ms.stat"
KeyStr = '   1,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr1 = '   9,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr2 = '  76,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
KeyStr3 = '  80,               ,[1024], Application,  CBR Server,Average Unicast End-to-End Delay (seconds) = '
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr1) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
    if LineTemp.find(KeyStr2) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")
    if LineTemp.find(KeyStr3) == 0:
        FoundFlag = True
        #print("############")
        #print(re.findall(r'\d+\.?\d*',LineTemp))
        #print("*********************")
        data = re.findall(r'\d+\.?\d*',LineTemp)
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\kongfen1.0\\stat.txt","a+")
        f.write("data")        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()

