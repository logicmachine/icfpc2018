# Reference: https://www.inet-solutions.jp/technology/python-selenium/#Python
# Reference: https://qiita.com/redoriva/items/aa9fa4c0bf2aeb8e1bff
# https://qiita.com/myasu398/items/9d81c1d58816ae60db5b

# memo
# chrome chrome driverを入れる
# firefox gecko driverを入れる。 https://github.com/mozilla/geckodriver/releases


import os
import csv
import configparser

from time import sleep
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import Select

kSteps = 90

kProblemDir = "problemsF"
#kNBTDir = "dfltTracesF"
kNBTDir = "tmp"

class ConfigReader:
    def __init__(self):
        print("HOOHOHOH")
        inifile = configparser.ConfigParser()
        inifile.read('settings.conf')
        self.browser = inifile.get('Global','Browser')
        if (self.browser == "Chrome"): 
            self.is_chrome = True
        else :
            self.is_chrome = False

        self.visualizer = inifile.get('Global', 'visualizer')
        self.visualizer = self.convert_to_bool(self.visualizer)
        self.problem_dir = inifile.get('Global', 'problem_dir')
        self.nbt_dir = inifile.get('Global', 'nbt_dir')

        # read FA
        self.FA_solve = self.convert_to_bool(inifile.get('FA','FA_solve'))
        self.FA_begin = int(inifile.get('FA', 'FA_begin'))
        self.FA_end =  int(inifile.get('FA', 'FA_end'))

        # read FD
        self.FD_solve = self.convert_to_bool(inifile.get('FD','FD_solve'))
        self.FD_begin = int(inifile.get('FD', 'FD_begin'))
        self.FD_end =  int(inifile.get('FD', 'FD_end'))

        # read FR
        self.FR_solve = self.convert_to_bool(inifile.get('FR','FR_solve'))
        self.FR_begin = int(inifile.get('FR', 'FR_begin'))
        self.FR_end =  int(inifile.get('FR', 'FR_end'))

    def convert_to_bool(self, var):
        if var == "True" or var == "true":
            return True
        elif var == "False" or var == "false":
            return False
        else :
            raise "Settings for True/False is wrong"

    def print_test(self):
        print("Global Settings")
        print(self.browser)
        print(self.is_chrome)
        print(self.visualizer)
        print(self.problem_dir)
        print(self.nbt_dir)

        print("FA")
        print(self.FA_solve)
        print(self.FA_begin)
        print(self.FA_end)

        print("FR")
        print(self.FR_solve)
        print(self.FR_begin)
        print(self.FR_end)

        print("FD")
        print(self.FD_solve)
        print(self.FD_begin)
        print(self.FD_end)


class Result:
    def __init__(self, result):
        self.raw = result
        ar = result.split("\n")

        self.status = result.split(":")[0]
        self.time = None
        self.commands = None
        self.energy = None

        # set time
        for str in ar:
            if (str.find("Time") != -1):
                print(str.split(":"))
                self.time = int(str.split(":")[1])
                break              

        for str in ar:
            if (str.find("Commands") != -1):
                self.commands = int(str.split(":")[1])
                break              

        for str in ar:
            if (str.find("Energy") != -1):
                print(str.split(":")[1])
                self.energy = int(str.split(":")[1])
                break              

#Failure::
#Halted with missing filled coordinates (479 = |{(1,3,8),(1,3,9),(1,4,8),(1,4,9),(2,3,8),...}|)
#Halted with excess filled coordinates (162 = |{(7,9,8),(7,9,9),(7,10,8),(7,10,9),(7,11,7),...}|)

#Time: 693
#Commands: 693
#Energy: 165905180
#Harmonics: Low
#Full: 194
#Active Bots: 

        # success
        #Success::
        #Time:      1398
        #Commands:  1398
        #Energy:    335123860
        #ClockTime: 175ms

    def get_status(self):
        return self.status
    
    def get_time(self):
        return self.time

    def get_commands(self):
        return self.commands

    def get_energy(self):
        return self.energy

class PageConfig:
    def __init__(self, config_reader):
        self.vis = config_reader.visualizer
        self.is_chrome = config_reader.is_chrome

        self.current_dir = os.getcwd()

        if self.is_chrome:
            self.current_dir = self.current_dir.replace('\\', '/')
            self.path_char = '/'
        else :
            self.path_char = '\\'
            self.current_dir = self.current_dir.replace('/', '\\')

        if self.is_chrome:
            self.driver = webdriver.Chrome("./chromedriver.exe")
        else :
            self.driver = webdriver.Firefox()

        if self.vis:
            self.driver.get("https://icfpcontest2018.github.io/full/exec-trace.html")
        else :
            self.driver.get("https://icfpcontest2018.github.io/full/exec-trace-novis.html")

        # get src model
        self.srcModelEmpty = self.driver.find_element_by_id("srcModelEmpty")
        self.srcModelFileIn = self.driver.find_element_by_id("srcModelFileIn")

        # get target model
        self.tgtModelEmpty = self.driver.find_element_by_id("tgtModelEmpty")
        self.tgtModelFileIn = self.driver.find_element_by_id("tgtModelFileIn")

        # get trace file in
        self.traceFileIn = self.driver.find_element_by_id("traceFileIn")

        # get steps per framce
        self.stepsPerFrame = self.driver.find_element_by_id("stepsPerFrame")
        self.stepsPerFrameSelector = Select(self.stepsPerFrame)

        if self.vis :
            self.stepsPerFrameSelector.select_by_value("1")
        else :
            self.stepsPerFrameSelector.select_by_value("4000")


        # execution button
        self.execTrace = self.driver.find_element_by_id("execTrace")

        # stdout
        self.stdout = self.driver.find_element_by_id("stdout")

        # canvas
        if self.vis :
            self.canvas = self.driver.find_element_by_id("glcanvas")
        else :
            self.canvas = None

    def _set_check_box(self, src_should_be_empty, tgt_should_be_empty):
        if (src_should_be_empty and (not self.srcModelEmpty.is_selected())
            or
            not src_should_be_empty and self.srcModelEmpty.is_selected()
            ):
            # src shuold be empty
            self.srcModelEmpty.click()

        if (tgt_should_be_empty and (not self.tgtModelEmpty.is_selected())
            or
            not tgt_should_be_empty and self.tgtModelEmpty.is_selected()
            ):
            self.tgtModelEmpty.click()

    def exec_trace(self):
        self.execTrace.click()

    def set_problem_class(self, config_str):
        if (config_str == 'FA'):
            # SourceModel should be empty
            # TargetModel should not be empty
            self._set_check_box(True, False)
        elif (config_str == "FD"):
            # SourceModel shoulud not be empty
            # TargetModel should be empty
            self._set_check_box(False, True)

        elif (config_str == "FR"):
            # SourceModel should not be empty
            # TargetModel should not be empty
            self._set_check_box(False, False)

    def set_in_file(self, ploblem_class, num):
        self.set_problem_class(problem_class)
        if (problem_class == "FD" or problem_class == "FR"):
            page.srcModelFileIn.clear()
            page.srcModelFileIn.send_keys(self.current_dir + self.path_char + kProblemDir + self.path_char + ploblem_class + num + "_src.mdl")
        if (problem_class == "FA" or problem_class == "FR"):
            page.tgtModelFileIn.clear()
            page.tgtModelFileIn.send_keys(self.current_dir + self.path_char + kProblemDir + self.path_char + problem_class + num + "_tgt.mdl")
        page.traceFileIn.clear()
        page.traceFileIn.send_keys(self.current_dir + self.path_char + kNBTDir + self.path_char + problem_class + num + ".nbt")

    def get_result(self):
        return Result(self.stdout.text)

    def is_execute_finished(self):
        txt = self.stdout.text
        return txt.find("Success") != -1 or txt.find("Failure") != -1

    def camera_iikanji(self):
        if self.vis :
            for j in range(0, 15):
                self.canvas.send_keys('a')
                self.canvas.send_keys('s')



# configure function will be used to change setting for "FA", "FR, "FD"
# because of python, state can be changed inside function...?
#def configure(srcModelEmpty):
#    srcModelEmpty.click()
# srcModelEmpty.click() # click to turn on or off checkbox
# srcModelEmpty.is_selected()


config_reader = ConfigReader()
kProblemDir = config_reader.problem_dir
kNBTDir = config_reader.nbt_dir


f = open("result.csv", 'w')
writer = csv.writer(f, lineterminator='\n')
writer.writerow(["FA/FD/FR", "Case No", "status", "time", "commands", "energy"])

page = PageConfig(config_reader)

queries = ["FA", "FD", "FR"]
tgt_or_src = [["tgt"],["src"],["tgt", "src"]]

to_be_solve = [True, True, True]

if (not config_reader.FA_solve):
    to_be_solve[0] = False

if (not config_reader.FD_solve):
    to_be_solve[1] = False

if (not config_reader.FR_solve):
    to_be_solve[2] = False

# range is [)
#rangies = [[1, 186], [1, 186], [1, 115]]
rangies = [
        [config_reader.FA_begin, config_reader.FA_end], 
        [config_reader.FD_begin, config_reader.FD_end], 
        [config_reader.FR_begin, config_reader.FR_end], 
        ]

# for q in range(0, 3):
for q in range(0, 3):
    if not to_be_solve[q]:
        continue
    r = rangies[q]
    problem_class = queries[q]
    for i in range(r[0], r[1]+1):
        try: 
            problem_num_str = "{:03d}".format(i)
            print(problem_class)
            print(problem_num_str)
            page.set_in_file(problem_class, problem_num_str)
            page.exec_trace()
            page.camera_iikanji()

            while True:
                if page.is_execute_finished():
                    break
                sleep(1)

            result = page.get_result()
            print("Status: {}, time: {}, commands: {}, energy: {}".format(result.status, result.time, result.commands, result.energy))
            writer.writerow([problem_class, problem_class + problem_num_str, result.status, result.time, result.commands,result.energy])
        except Exception:
            print("exception")
            writer.writerow([problem_class, problem_class + problem_num_str, "File Not Found", -1, -1, -1])



exit()


file_form = driver.find_element_by_id("modelFileIn")
button = driver.find_element(By.XPATH, '//button')
stdout = driver.find_element_by_id("stdout")

