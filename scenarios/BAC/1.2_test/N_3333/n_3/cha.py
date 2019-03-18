FileName = input("the object file:")
KeyStr = input("the key string:")
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
        print("the line is :"+LineTemp,end='')
        print("*********************")
        break
    else:
        LineTemp = FileObj.readline()
FileObj.close()
if FoundFlag == False:
    print("Not found the string!")
input()
