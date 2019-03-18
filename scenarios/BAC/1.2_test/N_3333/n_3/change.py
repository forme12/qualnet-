# -*- coding: utf-8 -*-
"""
Created on Thu Feb  1 12:24:18 2018

@author: Administrator
"""

def main():
    file=open("student.txt",'r') 
    lines=file.readlines()  #使用readlines()函数 读取文件的全部内容，存成一个列表，每一项都是以换行符结尾的一个字符串，对应着文件的一行

    list_name=[]  #初始化一个空列表 用来存该文件的姓名 也就是第一列
    list_scores=[]
    list_gpa=[]

    for line in lines:     #开始进行处理 把第一列存到list_name 第二列存到list_scores,,,,,
        elements=line.split()
        list_name.append(elements[0])
        list_scores.append(elements[1])
        list_gpa.append(elements[2])
    index = 0
    for i in range(len(list_gpa)):
        index=i    
        print("the person is {0} and the scores are {1} ,the gpa is {2}".format(list_name[index],list_scores[index],list_gpa[index]))
   

main()
