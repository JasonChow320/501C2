import requests

OGurl = "http://127.0.0.1:5000/"
Gtest = "https://httpbin.org/get"
Ptest = "https://httpbin.org/post"
sql = "sqlcheck"
add = "checkIn"
headers = {"User-Agent": "IWasBornInTheUSA"}
# payload = {"authorize_code": "nuts",
#            "agentId": "know",
#            "IP": "127.0.0.14",
#            "sleepTime": 33,
#            "guido": "GUID goes here",
#            "computerName": "StinkyMac",
#            "DHkey": "Not Sure Yet",
#            "cmdQ" : "[]"}

payload = {
           "sleepTime": 33,
           "guido": "GUID goes here",
           "computerName": "StinkyMac",
           "DHkey": "Not Sure Yet",
           "agentId": "know"}


def get(url="", headers=headers):
    r = requests.get((OGurl+url), headers=headers)
    print(r)
    print(r.text)


def post(url="", deets=payload, headers=headers):
    r = requests.post(OGurl+ url, json=deets, headers=headers)
    print(r)
    print(r.text)

def remote(IP, cmds):
    print("passed in the following commands")
    print(cmds)
    post("remote", {"IP":IP,"cmds":cmds})
    
    
instructions = "\n\
Welcome to the client for the C2 server.\n\
Use the following commands to get started:\n\
help - displays this message\n\
sql - print out entire SQL database\n\
gtest {url}- run get request to url\n\
ptest {url}- run get request to url\n\
remote {ip} {cmds} - execute cmds in machine with certain ip\n\
add - adds a fake Implant to sql database for testing purposes"


def mainLoop():
    running = True
    while(running):
        try:
            cmd=input("Type Cmd here:").split()
            #activate py10
            match cmd:
                case ["help"]:
                    print(instructions)
                case ["exit"]:
                    print("See ya")
                    running = False
                case ["sql"]:
                    get(sql)
                case ["add"]:
                    post(add)
                case ["home"]:
                    post(OGurl)
                case ["ptest"]:
                    post(Ptest)
                case ["ptest", url]:
                    post(url)
                case ["gtest"]:
                    get(Gtest)
                case ["gtest", url]:
                    get(url)
                case ["remote", IP]:
                    print("Not enough arguments (Did you forget the IP or cmds?)")
                case ["remote", IP, *commands]:
                    remote(IP, [*commands])
                case _:
                    print(
                        f"Error, I'm not really sure what you meant when you said:\n {cmd}")
        except Exception as e:
            print(e)


if __name__ == '__main__':
    print(instructions)
    mainLoop()
