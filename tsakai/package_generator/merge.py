import os
import csv

class Score:
    def __init__(self, dir_path):
        self.results = {}
        self.id = dir_path

        csv = ""
        for file in os.listdir(dir_path):
            base, ext = os.path.splitext(file)
            if ext == ".csv":
                csv = file
                break

        if csv == "":
            raise "No csv included"

        # format of csv
        # FA/FD/FR,Case No,status,time,commands,energy
        f = open(dir_path + "/" + csv, 'r')
        f.readline()

        line = f.readline() # 1行を文字列として読み込む(改行文字も含まれる)
        while line:
            ar = line.split(',')
            testcase = ar[1]
            status = ar[2]
            energy = ar[5]
            if (status.find("Success") != -1):
                self.results[testcase] = int(energy)
            else :
                self.results[testcase] = int(-1)
            line = f.readline()
        f.close()
    
    def get_score(self, case):
        return self.dict[case]

def get_best_energy_and_id(energies, test_case):
    id = "dfltTracesF"
    best = -1

    for energy in energies:
        tenergy = energy.results[test_case]
        tid = energy.id
        if tenergy == -1:
            continue
        if best == -1 or tenergy < best:
            best = tenergy
            id = tid
            
    return id, best

path = os.getcwd()

files = os.listdir(path)
files_dir = [f for f in files if os.path.isdir(os.path.join(path, f))]
files_dir.remove("dfltTracesF")
#print(files_dir)    # ['dir1', 'dir2']


testcases = []
for file_dir in files_dir:
    try:
        score = Score(file_dir)
        testcases.append(score)
    except Exception:
        pass

best = {}


#for testcase in testcases:
#    print(testcase.results)

queries = ["FA", "FD", "FR"]
ranges = [[1, 186], [1, 186], [1, 115]]

f = open("result_merged.csv", "w")
writer = csv.writer(f, lineterminator='\n')

for i in range(0, 3):
    for j in range(ranges[i][0], ranges[i][1] + 1):
        target_case = queries[i] + "{:03d}".format(j)
        id, best = get_best_energy_and_id(testcases, target_case)
        writer.writerow([target_case, id, best])

f.close()
