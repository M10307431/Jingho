
def CC2540EM():
    try:
        BLECMD=ble_builder.BLECMD
        
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        
        time.sleep(1)
        
        print "-------------------------Connect CC2540EM-------------------------"
        print_output(BLECMD("fe09", peer_addr="\xB6\xBC\x36\x29\x6A\xBC"))#\x1C\xBA\xD6\x14\x33\x88
        time.sleep(2) #\xB6\xBC\x36\x29\x6A\xBC
        I=raw_input("PAUSE")
        #================================================GATT_WriteCharValue
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E") #Trigger
        print datetime.datetime.now().strftime("%H:%M:%S.%f")
        while True:
		
            x=raw_input("Input:")
            if(x == '1'):
                print_output(BLECMD("fd9b",
                             conn_handle="\x00\x00",
                             handle="\x25\x00",
                             value="\x01"))
            if(x == '2'):
                print_output(BLECMD("fd9b",
                             conn_handle="\x00\x00",
                             handle="\x25\x00",
                             value="\x02"))
            if(x == '3'):
                print_output(BLECMD("fd9b",
                             conn_handle="\x00\x00",
                             handle="\x25\x00",
                             value="\x03"))
            
            time.sleep(1) # needs some delay to print
        '''
        print_output(BLECMD("fd92",
                                  conn_handle="\x00\x00",
                                  handle="\x25\x00",
                                  value="\x02"))
        time.sleep(10)
        '''
        
        
    except:
        print "=====================ERROR====================="
        pass
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    ble_parser.stop()
