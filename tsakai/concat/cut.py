
import sys 
import struct
# FA003.nbt
# FD003.nbt
# FR_test.nbt

# get length
fd_file = open("FD003.nbt", "rb")
fd_all = fd_file.read()
fd_len = len(fd_all) - 1
fd_file.close()

fd_file = open("FD003.nbt", "rb")
fd_content = fd_file.read(fd_len)
fd_file.close()

fa_file = open("FA003.nbt", "rb")
fa_content = fa_file.read()
fa_file.close()

outfile = open("FR_test.nbt", "wb")

outfile.write(fd_content)
outfile.write(fa_content)
outfile.close()