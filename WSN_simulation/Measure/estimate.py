
def main():
    Q=0
    samplerate=0.00005
    f=open('EIF_Raw.txt','r')
    for i in f:
        v=float(i)
        if v>0:
            Q=Q+v*samplerate
        else:
            pass
    f.close()
    
    print Q
    Iavg=Q/float(0.5)
    L=0.23/Iavg
    print "Q:",Q
    print "Iavg:",Iavg
    print "Lifetime:",L
if __name__=="__main__":
    main()
