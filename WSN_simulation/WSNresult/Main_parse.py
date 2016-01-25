import os
import xlwt, xlrd

def parsefile(path, sheet, col):
    row=1
    
    sheet.write(row,col+1,"SetAmount_MeetRatio")
    sheet.write(row,col+2,"Missratio_perset")
    sheet.write(row,col+3,"Meetratio_perset")
    sheet.write(row,col+4,"MissLatency_perpkt")
    sheet.write(row,col+5,"MeetLatency_perpkt")
    sheet.write(row,col+6,"Lifetime")
    sheet.write(row,col+7,"AverageEnergy")
    row=row+1
    
    rate=80
    data=[]
    f=open(path,'r')
    for i in f:
        if i.find("SetAmount_MeetRatio")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            data.append(o)
            
        if i.find("Missratio_perset")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("Meetratio_perset")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("MissLatency_perpkt")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("MeetLatency_perpkt")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])

        if i.find("Lifetime")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])

        if i.find("AverageEnergy")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])
            
        if i.find("==============================================")>=0:
            
            sheet.write(row,col+0,str(rate))
            sheet.write(row,col+1,data[0])
            sheet.write(row,col+2,data[1])
            sheet.write(row,col+3,data[2])
            sheet.write(row,col+4,data[3])
            sheet.write(row,col+5,data[4])
            sheet.write(row,col+6,data[5])
            sheet.write(row,col+7,data[6])
            row=row+1
            
            data=[]
            rate+=40
    f.close()
    
def plotdata(filename, sheetname, tag, outfilename):
    datasize=8
    outfile=open(outfilename, 'w')
    read_wb=xlrd.open_workbook(filename)
    
    for i in range(len(read_wb.sheet_names())):
       
        if read_wb.sheet_names()[i]==sheetname:
            
            print read_wb.sheet_names()[i]
            
            s=read_wb.sheet_by_index(i)
            
            #====================Tag Name
            #print "rate",
            outfile.write("Rate"+" ")
            for v in range(0,len(s.row_values(0)),datasize):
                #print s.row_values(0)[v],
                outfile.write(s.row_values(0)[v]+" ")
            #print ""
            outfile.write("\n")
            
            #====================Tag Name's index
            tagindex=0
            while s.row_values(1)[tagindex]!=tag:
                tagindex=tagindex+1
                
            #print tag,tagindex
            #====================Data
            for row in range(2, len(s.col_values(0)), 1):
                #print s.row_values(row)[0],
                outfile.write(s.row_values(row)[0]+" ")
                for v in range(tagindex,len(s.row_values(row)),datasize):
                    #print s.row_values(row)[v],
                    outfile.write(s.row_values(row)[v]+" ")
                #print ""
                outfile.write("\n")
    outfile.close()
    
def main():
    filename="Result.xls"
    wb=xlwt.Workbook()
    #Write file
    
    for l in os.listdir('.'):
        if l.find('.')<0:
            col=0
            sheet=wb.add_sheet(l)
            for j in os.listdir('./'+l):
                if j.find('.')<0:                    
                    path='./'+l+'/'+j+'/FinalResult.txt' #l =>nodenum, j=>approach
                    print path
                    
                    try:
                        
                        f=open(path,'r')
                        f.close()
                        print "Get"
                        sheet.write(0,col,j)
                        parsefile(path,sheet, col)
                        
                        col=col+8
                        
                    except:
                        print "No file:",path
    wb.save(filename)

    plotdata(filename, "node3", "Meetratio_perset", "MeetRatio.txt")
    plotdata(filename, "node3", "Lifetime", "Lifetime.txt")
    plotdata(filename, "Single", "Meetratio_perset", "Single_MeetRatio.txt")
    plotdata(filename, "Single", "Lifetime", "Single_Lifetime.txt")
    plotdata(filename, "VariedSleep", "Lifetime", "CS_Lifetime.txt")
    
if __name__=="__main__":
    main()
