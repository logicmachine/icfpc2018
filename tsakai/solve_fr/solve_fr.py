
import sys 
import struct

#argvs = sys.argv
#argc = len(argvs)

#if argc < 4:
#    print("Usage: [FDxxx.txt] [FAxxx.txt] [FRxxx.txt]")
#    exit(1)

def merge_binary(fd, fa, fr, fg):
    fd_file = open(fd, "rb")
    fd_all = fd_file.read()
    fd_len = len(fd_all) - 1
    fd_file.close()

    #print(fd_len)

    fd_file = open(fd, "rb")
    fd_content = fd_file.read(fd_len)
    fd_file.close()

    fa_file = open(fa, "rb")
    fa_content = fa_file.read()
    fa_file.close()

    outfile = open(fr, "wb")
    outfile.write(fd_content)
    if fg:
        special_file = open("trace.nbt", "rb")
        outfile.write(special_file.read())

    outfile.write(fa_content)
    outfile.close()


# get pair of FRxxx_xxx.mdl. if none, 
def get_pair(row, file):
    ar = row.split(',')
    if len(ar[1]) != 1:
        return ar[1].split('\n')[0]
    return ar[0]


def score_csv():
    scores = {}
    lines= open("../package_generator/result_merged.csv", "r").readlines()
    for line in lines:
        ar = line.split(",")
        scores[ar[0]] = ar[1]
    return scores

#merge_binary(argvs[1], argvs[2], argvs[3])


score_dict = score_csv()

fr_src_to_fd = open("../fr_1to1/fr_src_to_fd.csv", "r").readlines()
fr_tgt_to_fa = open("../fr_1to1/fr_tgt_to_fa.csv", "r").readlines()

fr_src_trace_dir = "fr_src/logicmachine-disasm_4"
fr_tgt_trace_dir = "fr_tgt" # not solved yet

bad_case = [
"FR036.nbt",
"FR051.nbt",
"FR052.nbt",
"FR067.nbt",
"FR070.nbt",
"FR085.nbt",
"FR088.nbt",
"FR066.nbt",
"FR090.nbt",
"FR094.nbt",
"FR102.nbt",
]


fr_size = 115
for i in range(0, fr_size):
    output_filename = "FR" + "{:03d}".format(i+1) + ".nbt"
    src_filename = "FR" + "{:3d}".format(i+1) + "_src.mdl"
    src_to_solve = get_pair(fr_src_to_fd[i], output_filename)

    src_binary_path=None
    
    # FD exists
    if src_to_solve[1] == 'D' :
        src_dir = score_dict[src_to_solve.split('_')[0]]        
    #    print("problem {}: src: {}".format(src_to_solve, src_dir))
        src_binary_path = "../score_calculator/" + src_dir + "/" + src_to_solve.split('_')[0] + ".nbt"
    else : # FD doesn't exists
        src_binary_path = "./" + fr_src_trace_dir + "/" + src_to_solve.split('_')[0] + ".nbt"

    tgt_filename = "FR" + "{:3d}".format(i+1) + "_tgt.mdl"
    tgt_to_solve = get_pair(fr_tgt_to_fa[i], output_filename)

    # FA exists
    if tgt_to_solve[1] == 'A' :
        tgt_dir = score_dict[tgt_to_solve.split('_')[0]]        
    #    print("problem {}: src: {}".format(src_to_solve, src_dir))
        tgt_binary_path = "../score_calculator/" + tgt_dir + "/" + tgt_to_solve.split('_')[0] + ".nbt"
    else : # FD doesn't exists
        print()
        tgt_binary_path = "./" + fr_tgt_trace_dir + "/" + tgt_to_solve.split('_')[0] + ".nbt"
        # this case is not solved...
        # continue

    #if (i == 44):
    #    print("{} {} {}".format(output_filename, src_binary_path, tgt_binary_path))


    #print("{} {} {}".format(src_binary_path, tgt_binary_path, output_filename))
    if not (output_filename in bad_case):
        continue
    merge_binary(src_binary_path, tgt_binary_path, output_filename, False)
    #print(output_filename)







