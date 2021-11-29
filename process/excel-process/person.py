import os
from model import list as ls
from manage import initdata as init
from model import problem as pb
import pandas as pd

problem_type = ["单选题", "判断题", "简答题"]

global problem_record_list
problem_record_list = ["题号", "答题次数", "正确次数", "正确率"]
global log_head
log_head = ["序号", "时间", "单选题数", "单选题正确数", "判断题数", "判断题正确数", "简答题数", "总题数", "总正确数"]


def addLog(username, submit_time):
    current_path = data_path + str(username)
    current_excel = current_path + "/log.xlsx"
    if not os.path.exists(current_excel):
        df = pd.DataFrame({"序号": {}, "时间": {}, "单选题数": {}, "单选题正确数": {},
                           "判断题数": {}, "判断题正确数": {}, "简答题数": {}, "总题数": {}, "总正确数": {}})
        df.to_excel(current_excel, index=False)

    df = pd.read_excel(current_excel)
    cnt = df.shape[0]

    ds = pd.DataFrame({"序号": [cnt + 1],
                       "时间": [submit_time],
                       "单选题数": [ls.current_record["单选题"][0]],
                       "单选题正确数": [ls.current_record["单选题"][1]],
                       "判断题数": [ls.current_record["判断题"][0]],
                       "判断题正确数": [ls.current_record["判断题"][1]],
                       "简答题数": [ls.current_record["简答题"]],
                       "总题数": [ls.current_record["单选题"][0] + ls.current_record["判断题"][0]],
                       "总正确数": [ls.current_record["单选题"][1] + ls.current_record["判断题"][1]]})
    df = df.append(ds)
    df.to_excel(current_excel, index=False, header=True)


def appendPerson(new_person):
    df = pd.read_excel(init.user_path)
    ls.user_list.append(new_person)
    ds = pd.DataFrame({"序号": [str(len(ls.user_list))],
                       "姓名": [new_person.getName()],
                       "密码": [new_person.getPassword()],
                       "权限": [new_person.getPermission()]})
    df = df.append(ds)
    df.to_excel(init.user_path, index=False, header=True)


def changePassword(username, password):
    data = pd.read_excel(init.user_path)

    data['密码'][data['姓名'] == username] = password

    pd.DataFrame(data).to_excel(init.user_path, index=False, header=True)
    for user in ls.user_list:
        if user.getName() == username:
            user.changePassword(password)


data_path = "./data/user/"


def appendProblemRecord(name):
    current_path = data_path + str(name)
    current_excel = current_path + "/data.xlsx"
    writer = pd.ExcelWriter(current_excel)
    for type_record in problem_type:
        frame = ls.problem_record[type_record].getFrame()
        ds = pd.DataFrame(frame)
        ds.to_excel(writer, sheet_name=type_record, index=False)

    writer.save()
    writer.close()


def initDirectory(name):
    current_path = data_path + str(name)
    if not os.path.exists(current_path):
        os.mkdir(current_path)


def initExcel(name):
    current_path = data_path + str(name)
    current_excel = current_path + "/data.xlsx"
    if not os.path.exists(current_excel):
        writer = pd.ExcelWriter(current_excel)
        for pro_type in problem_type:
            ds = pd.DataFrame({"题号": [],
                               "答题次数": [],
                               "错误次数": [],
                               "错误率": []})
            ds.to_excel(writer, sheet_name=pro_type, index=False)
        writer.save()
        writer.close()

    ls.problem_record.clear()
    for pro_type in problem_type:
        type_record = pb.typeRecord(pro_type)
        ls.problem_record[pro_type] = type_record
        sheet = pd.read_excel(current_excel, sheet_name=pro_type)
        problem_cnt = sheet.shape[0]
        for i in range(problem_cnt):
            line = sheet.loc[i].values
            type_record.initRecord(line)


def initData(name):
    initDirectory(name)
    initExcel(name)
