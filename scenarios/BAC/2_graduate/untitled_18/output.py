import re
import subprocess
import os
import time

def open_app_file():
    appfile = open('untitled_18.app', 'w')
    #appfile.clear()
    return appfile

def get_start_time():
    while True:
        try:
            start_time = float(input('Please input the app satrt time :'))
            return start_time
        except Exception as e:
            print('Fuck!\n Your input is wrong! That should be a number! again')
            #start_time = float(input('Please input the app satrt time :'))


def get_end_time():
    while True:
        try:
            end_time = float(input('Pleast input the app end time :'))
            return end_time
        except Exception as e:
            print('Fuck!\n Your input is wrong! That should be a number! again')
			

def get_src_id():
    while True:
        try:
            src = int(input('Pleast input the src id :'))
            return src
        except Exception as e:
            print('Fuck!\n Your input is wrong! That should be a number! again')
			
			
def get_dest_id():
    while True:
        try:
            dest = int(input('Pleast input the dest id :'))
            return dest
        except Exception as e:
            print('Fuck!\n Your input is wrong! That should be a number! again')		

def get_apptype():
    while True:
        try:
            apptype = raw_input('Pleast input the apptype :')
            return apptype
        except Exception as e:
            print('Fuck!\n Your input is wrong! That should be a number! again')				
			

def write_app_file(appfile, srcid, destid,item_to_send ,item_size,max_count ,interval, start, end,app_type,retrans_time,defa,defa_int):
    write_str = 'CBR ' + str(srcid) + ' ' + str(destid) + ' ' + str(item_to_send) + ' ' + str(item_size)\
    +' '+str(max_count)+' ' +str(interval)+'S ' +str(start)+'S '+str(end)+'S'+' ' +str(app_type)+' ' +str(retrans_time) + 'S'\
    +' '+str(defa)+' '+str(defa_int) 	
    
    appfile.write(write_str + '\n')
    
    appfile.close()

    return write_str


def run_bat(name):
    os.system('killqualnet.bat')
    os.system('runtest.bat')
    time.sleep(1)
    #print('run qualnet.exe 12')
    #p  = subprocess.Popen(['qualnet.exe', name], 
     #                       shell=True,
      #                      stdout = subprocess.PIPE,
       #                     stderr = subprocess.PIPE)
    #p.wait()
    
    #print('run qualnet.exe')
    #return p.stdout.readlines()




def main():

    #final_out_file = open('stats_out.txt', 'w')

    #final_out_file.clear()

    

	s_time = 1#get_start_time()

	e_time = 400#get_end_time()
	
	#src_id = get_src_id()
	
	#dest_id = get_dest_id()
	
	#app = get_apptype()
	
	send_interval=(0.004,0.006,0.008,0.01,0.012,0.014,0.016,0.018,0.02,0.025,0.03,0.035,0.04,0.045,0.05,0.055,0.06,0.065,0.07,0.075,0.08,0.085,0.09,0.095,0.1,0.12,0.14,0.16)
	#send_interval=(0.05,0.06,0.07)
	for i in range(0,29,1):

		app_file = open_app_file()
		print("_____________________________________________interval=",send_interval[i])
		wr_str = write_app_file(appfile = app_file,
						srcid = 13,
						destid = 16,
						item_to_send=3000,
						item_size = 680,
						max_count=10,
						interval = send_interval[i],
						start = s_time,
						end = e_time,
						app_type= 'Broadcast',
						retrans_time=20,
						defa='PRECEDENCE',
						defa_int=0	
					)

		
		run_bat('untitled_18.config')




                         
                    
    #final_out_file.close()
    


                    





if __name__ == '__main__':
    main()


        

    
        
    

        








