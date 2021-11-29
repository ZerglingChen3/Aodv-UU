import os
import pandas as pd
from model import list as ls
from model import person, problem

global user_path
user_path = "./data/user.xlsx"
global problem_path
problem_path = "./data/problem.xlsx"
global problem_type
problem_type = ["单选题", "判断题", "简答题"]


def user_init():
    global user_path
    if not os.path.exists(user_path):
        df = pd.DataFrame({"序号": {"1"}, "姓名": {"admin"}, "密码": {"1234"}, "权限": {"admin"}})
        df.to_excel(user_path, index=False)
    data = pd.read_excel(user_path)

    person_cnt = data.shape[0]

    for i in range(person_cnt):
        line = data.loc[i].values
        name = line[1]
        password = line[2]
        permission = line[3]
        user = person.person(name, password, permission)
        ls.user_list.append(user)


def problem_init():
    global problem_path
    if not os.path.exists(problem_path):
        print("no problem file, error!!!")
        exit(-1)

    global problem_type
    for p_type in problem_type:
        sheet = pd.read_excel(problem_path, sheet_name=p_type)
        problem_cnt = sheet.shape[0]
        prob_ls = []
        for i in range(problem_cnt):
            line = sheet.loc[i].values
            pro = problem.problem(line, p_type)
            prob_ls.append(pro)
        ls.problem_list[p_type] = prob_ls


def init():
    user_init()
    problem_init()
