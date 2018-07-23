
import sys 
import struct

argvs = sys.argv
argc = len(argvs)

if argc < 4:
    print("Usage: [FDxxx.txt] [FAxxx.txt] [FRxxx.txt]")
    exit(1)


# FA003.nbt
# FD003.nbt
# FR_test.nbt

# get length
fd_file = open(argvs[1], "rb")
fd_all = fd_file.read()
fd_len = len(fd_all) - 1
fd_file.close()

print(fd_len)

fd_file = open(argvs[1], "rb")
fd_content = fd_file.read(fd_len)
fd_file.close()

fa_file = open(argvs[2], "rb")
fa_content = fa_file.read()
fa_file.close()

outfile = open(argvs[3], "wb")
outfile.write(fd_content)
outfile.write(fa_content)
outfile.close()
