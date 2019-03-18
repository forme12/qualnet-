import re
#FileName = input("the object file:")
#KeyStr = input("the key string:")
#FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\BAC\\1.2_test\\N_3333\\n_3\\stat\\test0.stat"
FileName = "D:\\snt\\qualnet\\6.1\\scenarios\\Gird\\City\\test0.stat"
#KeyStr = '  46,               ,[1024], Application,  CBR Server,Unicast Session Start (seconds) = '
KeyStr = '   2,               ,[1024], Application,  CBR Server,Total Multicast Fragments Received (fragments) = '
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
        #f= open("E:\\stat.txt","w+")
		f= open("D:\\snt\\qualnet\\6.1\\scenarios\\Gird\\City\\stat.txt","w+")
        f.write("data")
        
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()
if FoundFlag == False:
    print("Not found the string!")
input()
