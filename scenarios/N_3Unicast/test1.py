import re
FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\5ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\10ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\20ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\30ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\50ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\100ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\150ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()


FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\200ms.stat"
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
        print(data)
        f= open("D:\\snt\\qualnet\\6.1\\scenarios\\N_3Unicast\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()

if FoundFlag == False:
    print("Not found the string!")
input()

